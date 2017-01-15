// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Guard]
#include "../asmjit_build.h"
#if !defined(ASMJIT_DISABLE_COMPILER)

// [Dependencies]
#include "../base/rapass_p.h"
#include "../base/utils.h"

// [Api-Begin]
#include "../asmjit_apibegin.h"

namespace asmjit {

// ============================================================================
// [asmjit::RABlock]
// ============================================================================

Error RABlock::appendSuccessor(RABlock* successor) noexcept {
  RABlock* predecessor = this;

  if (predecessor->_successors.contains(successor))
    return kErrorOk;
  ASMJIT_ASSERT(!successor->_predecessors.contains(predecessor));

  ASMJIT_PROPAGATE(successor->_predecessors.willGrow());
  ASMJIT_PROPAGATE(predecessor->_successors.willGrow());

  predecessor->_successors.appendUnsafe(successor);
  successor->_predecessors.appendUnsafe(predecessor);

  return kErrorOk;
}

Error RABlock::prependSuccessor(RABlock* successor) noexcept {
  RABlock* predecessor = this;

  if (predecessor->_successors.contains(successor))
    return kErrorOk;
  ASMJIT_ASSERT(!successor->_predecessors.contains(predecessor));

  ASMJIT_PROPAGATE(successor->_predecessors.willGrow());
  ASMJIT_PROPAGATE(predecessor->_successors.willGrow());

  predecessor->_successors.prependUnsafe(successor);
  successor->_predecessors.prependUnsafe(predecessor);

  return kErrorOk;
}

// ============================================================================
// [asmjit::RAPass - Construction / Destruction]
// ============================================================================

RAPass::RAPass() noexcept
  : CCFuncPass("RAPass"),
    _heap(),
    _logger(nullptr),
    _func(nullptr),
    _stop(nullptr),
    _extraBlock(nullptr),
    _archRegCount(),
    _allocableRegs(),
    _clobberedRegs(),
    _nodesCount(0),
    _lastMark(0) {}
RAPass::~RAPass() noexcept {}

// ============================================================================
// [asmjit::RAPass - Run]
// ============================================================================

static void RAPass_reset(RAPass* self, ZoneHeap* heap) noexcept {
  self->_blocks.reset(heap);
  self->_exits.reset(heap);
  self->_pov.reset(heap);
  self->_workRegs.reset(heap);
  self->_stack.reset();

  self->_archRegCount.reset();
  self->_allocableRegs.reset();
  self->_clobberedRegs.reset();
  self->_nodesCount = 0;
  self->_lastMark = 0;
}

static void RAPass_resetVirtRegData(RAPass* self) noexcept {
  WorkRegs& wRegs = self->_workRegs;
  size_t count = wRegs.getLength();

  for (size_t i = 0; i < count; i++) {
    WorkReg* wReg = wRegs[i];
    VirtReg* vReg = wReg->getVirtReg();

    // Zero everything so it cannot be used by mistake.
    vReg->_tiedReg = nullptr;
    vReg->_workReg = nullptr;
    vReg->_stackSlot = nullptr;
  }
}

Error RAPass::runOnFunction(Zone* zone, CCFunc* func) noexcept {
  // Initialize all core structures to use `zone` and `func`.
  CBNode* end = func->getEnd();

  _heap.reset(zone);
  _logger = cc()->getCode()->getLogger();

  _func = func;
  _stop = end->getNext();
  _extraBlock = end;
  RAPass_reset(this, &_heap);

  // Initialize architecture-specific members.
  onInit();

  // Not a real loop, just to make error handling easier.
  Error err;
  for (;;) {
    // STEP 1: Construct control-flow graph (CFG).
    err = constructCFG();
    if (err) break;

    // STEP 2: Construct post-order-view (POV)
    err = constructPOV();
    if (err) break;

    // STEP 3: Construct dominance tree (DOM).
    err = constructDOM();
    if (err) break;

    // STEP 4: Construct liveness analysis.
    err = constructLiveness();
    if (err) break;

    // TODO:
    break;
  }

  // Regardless of the status this must be called.
  onDone();

  // Reset possible connections introduced by the register allocator.
  RAPass_resetVirtRegData(this);

  // Reset all core structures and everything that depends on the passed `Zone`.
  RAPass_reset(this, nullptr);
  _heap.reset(nullptr);
  _logger = nullptr;

  _func = nullptr;
  _stop = nullptr;
  _extraBlock = nullptr;

  // Reset `Zone` as nothing should persist between `runOnFunction()` calls.
  zone->reset();

  // We alter the compiler cursor, because it doesn't make sense to reference
  // it after the compilation - some nodes may disappear and it's forbidden to
  // add new code after the compilation is done.
  cc()->_setCursor(cc()->getLastNode());

  return err;
}

// ============================================================================
// [asmjit::RAPass - ConstructPOV]
// ============================================================================

Error RAPass::constructPOV() noexcept {
  ASMJIT_RA_LOG_INIT(getLogger());
  ASMJIT_RA_LOG_FORMAT("[RA::ConstructPOV]\n");

  RABlocks& input = _blocks;
  RABlocks& output = _pov;

  size_t count = input.getLength();
  if (ASMJIT_UNLIKELY(!count))
    return kErrorOk;

  ZoneHeap* heap = output.getHeap();
  ASMJIT_PROPAGATE(output.reserve(count));

  class POVStackItem {
  public:
    ASMJIT_INLINE POVStackItem(RABlock* block, size_t index) noexcept
      : _block(block),
        _index(index) {}

    ASMJIT_INLINE POVStackItem(const POVStackItem& other) noexcept
      : _block(other._block),
        _index(other._index) {}

    ASMJIT_INLINE RABlock* getBlock() const noexcept { return _block; }
    ASMJIT_INLINE size_t getIndex() const noexcept { return _index; }

    RABlock* _block;
    size_t _index;
  };

  ZoneStack<POVStackItem> stack;
  ASMJIT_PROPAGATE(stack.init(heap));

  ZoneBitVector visited(heap);
  ASMJIT_PROPAGATE(visited.resize(count, false));

  RABlock* current = input[0];
  size_t i = 0;

  for (;;) {
    for (;;) {
      if (i >= current->getSuccessors().getLength())
        break;

      // Skip if already visited.
      RABlock* child = current->getSuccessors().getAt(i++);
      if (visited.getAt(child->getBlockId()))
        continue;

      // Mark as visited to prevent visiting the same node multiple times.
      visited.setAt(child->getBlockId(), true);

      // Add the current node on the stack, we will get back to it later.
      ASMJIT_PROPAGATE(stack.append(POVStackItem(current, i)));
      current = child;
      i = 0;
    }

    current->_povOrder = static_cast<uint32_t>(output.getLength());
    output.appendUnsafe(current);
    if (stack.isEmpty())
      break;

    POVStackItem top = stack.pop();
    current = top.getBlock();
    i = top.getIndex();
  }

  return kErrorOk;
}

// ============================================================================
// [asmjit::RAPass - ConstructDOM]
// ============================================================================

static ASMJIT_INLINE RABlock* intersectBlocks(RABlock* b1, RABlock* b2) noexcept {
  while (b1 != b2) {
    while (b2->getPovOrder() > b1->getPovOrder())
      b1 = b1->getIDom();
    while (b1->getPovOrder() > b2->getPovOrder())
      b2 = b2->getIDom();
  }
  return b1;
}

Error RAPass::constructDOM() noexcept {
  // Based on "A Simple, Fast Dominance Algorithm".
  ASMJIT_RA_LOG_INIT(getLogger());
  ASMJIT_RA_LOG_FORMAT("[RA::ConstructDOM]\n");

  if (_blocks.isEmpty())
    return kErrorOk;

  RABlock* entryBlock = getEntryBlock();
  entryBlock->setIDom(entryBlock);

  bool changed = true;
  uint32_t nIters = 0;

  while (changed) {
    nIters++;
    changed = false;

    size_t i = _pov.getLength();
    while (i) {
      RABlock* block = _pov[--i];
      if (block == entryBlock)
        continue;

      RABlock* iDom = nullptr;
      const RABlocks& preds = block->getPredecessors();

      size_t j = preds.getLength();
      while (j) {
        RABlock* p = preds[--j];
        if (!p->hasIDom())
          continue;
        iDom = !iDom ? p : intersectBlocks(iDom, p);
      }

      if (block->getIDom() != iDom) {
        ASMJIT_RA_LOG_FORMAT("  IDom of #%u -> #%u\n", block->getBlockId(), iDom->getBlockId());
        block->setIDom(iDom);
        changed = true;
      }
    }
  }

  ASMJIT_RA_LOG_FORMAT("  Done (%u iterations)\n", static_cast<unsigned int>(nIters));
  return kErrorOk;
}

// ============================================================================
// [asmjit::RAPass - ConstructLiveness]
// ============================================================================

namespace LiveOps {
  typedef LiveBits::BitWord BitWord;

  struct Or  { static ASMJIT_INLINE BitWord op(BitWord dst, BitWord a) noexcept { return dst | a; } };
  struct And { static ASMJIT_INLINE BitWord op(BitWord dst, BitWord a) noexcept { return dst & a; } };
  struct Xor { static ASMJIT_INLINE BitWord op(BitWord dst, BitWord a) noexcept { return dst ^ a; } };

  struct LiveIn {
    static ASMJIT_INLINE BitWord op(BitWord dst, BitWord out, BitWord gen, BitWord kill) noexcept { return (out | gen) & ~kill; }
  };

  template<typename CustomOp>
  static ASMJIT_INLINE bool op(BitWord* dst, const BitWord* a, uint32_t n) noexcept {
    BitWord changed = 0;

    for (uint32_t i = 0; i < n; i++) {
      BitWord before = dst[i];
      BitWord after = CustomOp::op(before, a[i]);

      dst[i] = after;
      changed |= (before ^ after);
    }

    return changed != 0;
  }

  template<typename CustomOp>
  static ASMJIT_INLINE bool op(BitWord* dst, const BitWord* a, const BitWord* b, uint32_t n) noexcept {
    BitWord changed = 0;

    for (uint32_t i = 0; i < n; i++) {
      BitWord before = dst[i];
      BitWord after = CustomOp::op(before, a[i], b[i]);

      dst[i] = after;
      changed |= (before ^ after);
    }

    return changed != 0;
  }

  template<typename CustomOp>
  static ASMJIT_INLINE bool op(BitWord* dst, const BitWord* a, const BitWord* b, const BitWord* c, uint32_t n) noexcept {
    BitWord changed = 0;

    for (uint32_t i = 0; i < n; i++) {
      BitWord before = dst[i];
      BitWord after = CustomOp::op(before, a[i], b[i], c[i]);

      dst[i] = after;
      changed |= (before ^ after);
    }

    return changed != 0;
  }
}

Error RAPass::constructLiveness() noexcept {
  ASMJIT_RA_LOG_INIT(getLogger());
  ASMJIT_RA_LOG_FORMAT("[RA::ConstructLiveness]\n");

  uint32_t numBlocks = static_cast<uint32_t>(_blocks.getLength());
  uint32_t numWorkRegs = static_cast<uint32_t>(_workRegs.getLength());
  uint32_t numBitWords = (numWorkRegs + LiveBits::kBitsPerWord - 1) / LiveBits::kBitsPerWord;

  if (!numWorkRegs) {
    ASMJIT_RA_LOG_FORMAT("  Done (no virtual registers)\n");
    return kErrorOk;
  }

  ZoneStack<RABlock*> workList;
  ASMJIT_PROPAGATE(workList.init(&_heap));

  // 1. Calculate `DEF` and `KILL`.
  uint32_t blockId = numBlocks;
  while (blockId) {
    RABlock* block = _pov[--blockId];

    ASMJIT_PROPAGATE(block->resizeLiveBits(numWorkRegs));
    ASMJIT_PROPAGATE(workList.append(block));

    CBNode* node = block->getLast();
    CBNode* stop = block->getFirst();

    for (;;) {
      if (node->actsAsInst()) {
        CBInst* inst = node->as<CBInst>();
        RAData* data = inst->getPassData<RAData>();
        ASMJIT_ASSERT(data != nullptr);

        const TiedReg* tRegs = data->getTiedArray();
        uint32_t count = data->getTiedCount();

        for (uint32_t i = 0; i < count; i++) {
          const TiedReg& tReg = tRegs[i];
          const WorkReg* wReg = tReg.vreg->getWorkReg();

          uint32_t workId = wReg->getWorkId();
          if (tReg.isWriteOnly()) {
            // KILL.
            block->_kill.setAt(workId, true);
          }
          else {
            // GEN.
            block->_kill.setAt(workId, false);
            block->_gen.setAt(workId, true);
          }
        }
      }

      if (node == stop)
        break;

      node = node->getPrev();
      ASMJIT_ASSERT(node != nullptr);
    }
  }

  // 2. Calculate `IN` and `OUT`.
  uint32_t nVisits = numBlocks * 2;
  while (!workList.isEmpty()) {
    RABlock* block = workList.pop();

    // Always changed if visited first time.
    bool changed = !block->hasFlag(RABlock::kFlagHasLiveness);
    if (changed)
      block->addFlags(RABlock::kFlagHasLiveness);

    // Calculate `OUT` based on `IN` of all successors.
    const RABlocks& successors = block->getSuccessors();
    size_t numSuccessors = successors.getLength();

    for (size_t i = 0; i < numSuccessors; i++) {
      changed |= LiveOps::op<LiveOps::Or>(
        block->_out.getData(),
        successors[i]->_in.getData(), numBitWords);
    }

    // Calculate `IN` based on `OUT`, `GEN`, and `KILL` bits.
    if (changed) {
      changed = LiveOps::op<LiveOps::LiveIn>(
        block->_in.getData(),
        block->_out.getData(),
        block->_gen.getData(),
        block->_kill.getData(), numBitWords);

      // Add all predecessors to the `workList` if liveness of this block changed.
      if (changed) {
        const RABlocks& predecessors = block->getPredecessors();
        size_t numPredecessors = predecessors.getLength();

        for (size_t i = 0; i < numPredecessors; i++) {
          RABlock* pred = predecessors[i];
          if (pred->hasFlag(RABlock::kFlagHasLiveness)) {
            ASMJIT_PROPAGATE(workList.append(pred));
            nVisits++;
          }
        }
      }
    }
  }

  ASMJIT_RA_LOG_FORMAT("  Done (%u visits)\n", static_cast<unsigned int>(nVisits));
  return kErrorOk;
}

// ============================================================================
// [asmjit::RAPass - Blocks]
// ============================================================================

RABlock* RAPass::newBlock(CBNode* initialNode) noexcept {
  if (ASMJIT_UNLIKELY(_blocks.willGrow() != kErrorOk))
    return nullptr;

  RABlock* block = getZone()->allocT<RABlock>();
  if (ASMJIT_UNLIKELY(!block))
    return nullptr;

  uint32_t blockId = static_cast<uint32_t>(_blocks.getLength());
  new(block) RABlock(&_heap, blockId);

  block->setFirst(initialNode);
  block->setLast(initialNode);

  _blocks.appendUnsafe(block);
  return block;
}

RABlock* RAPass::newBlockOrMergeWith(CBLabel* cbLabel) noexcept {
  if (cbLabel->hasBlock())
    return cbLabel->getBlock();

  CBNode* node = cbLabel->getPrev();
  RABlock* block = nullptr;

  // Try to find some label, but terminate the loop on any code.
  size_t nPendingLabels = 0;
  while (node) {
    if (node->getType() == CBNode::kNodeLabel) {
      block = node->as<CBLabel>()->getBlock();
      if (block) break;

      nPendingLabels++;
    }
    else if (node->getType() == CBNode::kNodeAlign) {
      // Align node is fine.
    }
    else {
      break;
    }

    node = node->getPrev();
  }

  if (!block) {
    block = newBlock();
    if (ASMJIT_UNLIKELY(!block)) return nullptr;
  }

  cbLabel->setBlock(block);
  node = cbLabel;

  while (nPendingLabels) {
    node = node->getPrev();
    for (;;) {
      if (node->getType() == CBNode::kNodeLabel) {
        node->as<CBLabel>()->setBlock(block);
        nPendingLabels--;
        break;
      }

      node = node->getPrev();
      ASMJIT_ASSERT(node != nullptr);
    }
  }

  if (!block->getFirst()) {
    block->setFirst(node);
    block->setLast(cbLabel);
  }

  return block;
}

CBNode* RAPass::findSuccessorStartingAt(CBNode* node) noexcept {
  while (node && (node->isInformative() || node->hasNoEffect()))
    node = node->getNext();
  return node;
}

bool RAPass::_strictlyDominates(const RABlock* a, const RABlock* b) const noexcept {
  // There must be at least one block if this function is
  // called, as both `a` and `b` must be valid blocks.
  ASMJIT_ASSERT(a != nullptr);
  ASMJIT_ASSERT(b != nullptr);
  ASMJIT_ASSERT(a != b); // Checked by `dominates()` and `strictlyDominates()`.

  // Nothing strictly dominates the entry block.
  const RABlock* entryBlock = getEntryBlock();
  if (a == entryBlock)
    return false;

  const RABlock* iDom = b->getIDom();
  while (iDom != a && iDom != entryBlock)
    iDom = iDom->getIDom();

  return iDom != entryBlock;
}

const RABlock* RAPass::_nearestCommonDominator(const RABlock* a, const RABlock* b) const noexcept {
  // There must be at least one block if this function is
  // called, as both `a` and `b` must be valid blocks.
  ASMJIT_ASSERT(a != nullptr);
  ASMJIT_ASSERT(b != nullptr);
  ASMJIT_ASSERT(a != b); // Checked by `dominates()` and `properlyDominates()`.

  if (a == b)
    return a;

  // If `a` strictly dominates `b` then `a` is the nearest common dominator.
  if (_strictlyDominates(a, b))
    return a;

  // If `b` strictly dominates `a` then `b` is the nearest common dominator.
  if (_strictlyDominates(b, a))
    return b;

  const RABlock* entryBlock = getEntryBlock();
  uint64_t mark = nextMark();

  // Mark all A's dominators.
  const RABlock* block = a->getIDom();
  while (block != entryBlock) {
    block->setLastMark(mark);
    block = block->getIDom();
  }

  // Check all B's dominators against marked dominators of A.
  block = b->getIDom();
  while (block != entryBlock) {
    if (block->getLastMark() == mark)
      return block;
    block = block->getIDom();
  }

  return entryBlock;
}

// ============================================================================
// [asmjit::RAPass - Work Registers]
// ============================================================================

Error RAPass::_addToWorkRegs(VirtReg* vReg) noexcept {
  // Checked by `::addToWorkRegs()` - must be true.
  ASMJIT_ASSERT(vReg->_workReg == nullptr);

  WorkRegs& workRegs = _workRegs;
  ASMJIT_PROPAGATE(workRegs.willGrow());

  WorkReg* workReg = getZone()->allocT<WorkReg>();
  if (ASMJIT_UNLIKELY(!workReg))
    return DebugUtils::errored(kErrorNoHeapMemory);

  uint32_t workId = static_cast<uint32_t>(workRegs.getLength());
  vReg->setWorkReg(new(workReg) WorkReg(&_heap, vReg, workId));

  workRegs.appendUnsafe(workReg);
  return kErrorOk;
}

// ============================================================================
// [asmjit::RAPass - Logging]
// ============================================================================

#if !defined(ASMJIT_DISABLE_LOGGING)
Error RAPass::_logBlockIds(const RABlocks& blocks) noexcept {
  // Can only be called if the `Logger` is present.
  ASMJIT_ASSERT(hasLogger());

  StringBuilderTmp<1024> sb;
  sb.appendString("  => { ");

  if (blocks.isEmpty()) {
    sb.appendString("none");
  }
  else {
    for (size_t i = 0, len = blocks.getLength(); i < len; i++) {
      const RABlock* block = blocks[i];
      if (i != 0)
        sb.appendString(", ");
      sb.appendFormat("#%u", static_cast<unsigned int>(block->getBlockId()));
    }
  }

  sb.appendString(" }\n");
  return getLogger()->log(sb.getData(), sb.getLength());
}
#endif

} // asmjit namespace

// [Api-End]
#include "../asmjit_apiend.h"

// [Guard]
#endif // !ASMJIT_DISABLE_COMPILER
