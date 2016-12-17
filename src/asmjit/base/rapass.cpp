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
  : CBPass("RA"),
    _zone(nullptr),
    _heap(),
    _archRegCount(),
    _allocableRegs(),
    _clobberedRegs(),
    _func(nullptr),
    _stop(nullptr),
    _extraBlock(nullptr),
    _lastMark(0) {}
RAPass::~RAPass() noexcept {}

// ============================================================================
// [asmjit::RAPass - Process]
// ============================================================================

Error RAPass::process(Zone* zone) noexcept {
  _zone = zone;
  _heap.reset(zone);

  Error err = kErrorOk;
  CBNode* node = cc()->getFirstNode();

  if (!node)
    return err;

  do {
    if (node->getType() == CBNode::kNodeFunc) {
      CCFunc* func = node->as<CCFunc>();
      node = func->getEnd();

      err = compile(func);
      if (err) break;
    }

    // Find a function by skipping all nodes that are not `kNodeFunc`.
    do {
      node = node->getNext();
    } while (node && node->getType() != CBNode::kNodeFunc);
  } while (node);

  _heap.reset(nullptr);
  _zone = nullptr;
  return err;
}

Error RAPass::compile(CCFunc* func) noexcept {
  ASMJIT_PROPAGATE(prepare(func));

  // Not a real loop, just to make error handling easier.
  Error err;
  for (;;) {
    err = constructCFG();
    if (err) break;

    err = constructDomTree();
    if (err) break;

    // TODO:
    break;
  }

  cleanup();

  // We alter the compiler cursor, because it doesn't make sense to reference
  // it after compilation - some nodes may disappear and it's forbidden to add
  // new code after the compilation is done.
  cc()->_setCursor(cc()->getLastNode());

  return err;
}

// ============================================================================
// [asmjit::RAPass - Prepare / Cleanup]
// ============================================================================

Error RAPass::prepare(CCFunc* func) noexcept {
  CBNode* end = func->getEnd();

  _func = func;
  _stop = end->getNext();
  _extraBlock = end;

  _blocks.reset(&_heap);
  _exits.reset(&_heap);
  _lastMark = 0;

  for (uint32_t kind = 0; kind < kMaxVRegKinds; kind++) {
    RALocals& lRegs = _localRegs[kind];
    lRegs.reset(&_heap);
  }

  _stack.reset();

  return kErrorOk;
}

void RAPass::cleanup() noexcept {
  _archRegCount.reset();
  _allocableRegs.reset();
  _clobberedRegs.reset();

  _func = nullptr;
  _stop = nullptr;
  _extraBlock = nullptr;

  _blocks.reset(nullptr);
  _exits.reset(nullptr);

  for (uint32_t kind = 0; kind < kMaxVRegKinds; kind++) {
    RALocals& lRegs = _localRegs[kind];
    RALocal** vRegs = lRegs.getData();

    size_t count = lRegs.getLength();
    for (size_t i = 0; i < count; i++) {
      RALocal* lReg = vRegs[i];
      VirtReg* vReg = lReg->getVirtReg();

      // Zero everything so it cannot be not used by mistake.
      vReg->_tied = nullptr;
      vReg->_local = nullptr;
      vReg->_stackSlot = nullptr;
    }

    _localRegs[kind].reset(nullptr);
  }

  _stack.reset();
}

// ============================================================================
// [asmjit::RAPass - Steps]
// ============================================================================

static Error constructPostOrderView(RABlocks& output, RABlocks& input) noexcept {
  printf("[RA::ConstructPOV]\n");

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

  ZoneBits visited(heap);
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

      // Mark visited to prevent visiting the same node multiple times.
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

static RABlock* intersectBlocks(RABlock* b1, RABlock* b2) noexcept {
  while (b1 != b2) {
    while (b2->getPovOrder() > b1->getPovOrder())
      b1 = b1->getIDom();
    while (b1->getPovOrder() > b2->getPovOrder())
      b2 = b2->getIDom();
  }
  return b1;
}

Error RAPass::constructDomTree() noexcept {
  // Based on "A Simple, Fast Dominance Algorithm".
  RABlocks pov(&_heap);
  ASMJIT_PROPAGATE(constructPostOrderView(pov, _blocks));

  printf("[RA::ConstructDOM]\n");
  if (_blocks.isEmpty())
    return kErrorOk;

  RABlock* entryBlock = getEntryBlock();
  entryBlock->setIDom(entryBlock);

  bool changed = true;
  uint32_t nIter = 0;
  while (changed) {
    nIter++;
    changed = false;

    size_t i = pov.getLength();
    while (i) {
      RABlock* block = pov[--i];
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
        printf("  IDom of #%u -> #%u\n", block->getBlockId(), iDom->getBlockId());
        block->setIDom(iDom);
        changed = true;
      }
    }
  }

  printf("  Done (%u iterations)\n", static_cast<unsigned int>(nIter));
  return kErrorOk;
}

// ============================================================================
// [asmjit::RAPass - Blocks]
// ============================================================================

RABlock* RAPass::newBlock(CBNode* initialNode) noexcept {
  if (ASMJIT_UNLIKELY(_blocks.willGrow() != kErrorOk))
    return nullptr;

  RABlock* block = _zone->allocT<RABlock>();
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
// [asmjit::RAPass - LocalRegs]
// ============================================================================

Error RAPass::_makeLocal(VirtReg* vReg) noexcept {
  // Checked by `makeLocal()` - must be true.
  ASMJIT_ASSERT(vReg->_local == nullptr);

  uint32_t kind = vReg->getKind();
  if (ASMJIT_UNLIKELY(kind >= Globals::kMaxVRegKinds))
    return DebugUtils::errored(kErrorInvalidRegKind);

  RALocals& localRegs = _localRegs[kind];
  ASMJIT_PROPAGATE(localRegs.willGrow());

  RALocal* localReg = _zone->allocT<RALocal>();
  if (ASMJIT_UNLIKELY(!localReg))
    return DebugUtils::errored(kErrorNoHeapMemory);

  uint32_t localId = static_cast<uint32_t>(localRegs.getLength());
  vReg->setLocal(new(localReg) RALocal(&_heap, vReg, localId));

  localRegs.appendUnsafe(localReg);
  return kErrorOk;
}

} // asmjit namespace

// [Api-End]
#include "../asmjit_apiend.h"

// [Guard]
#endif // !ASMJIT_DISABLE_COMPILER
