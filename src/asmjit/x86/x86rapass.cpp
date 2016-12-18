// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Guard]
#include "../asmjit_build.h"
#if defined(ASMJIT_BUILD_X86) && !defined(ASMJIT_DISABLE_COMPILER)

// [Dependencies]
#include "../base/cpuinfo.h"
#include "../base/utils.h"
#include "../x86/x86assembler.h"
#include "../x86/x86compiler.h"
#include "../x86/x86internal_p.h"
#include "../x86/x86rapass_p.h"

// [Api-Begin]
#include "../asmjit_apibegin.h"

namespace asmjit {

// ============================================================================
// [asmjit::X86RAPass - OpRWData]
// ============================================================================

#define R(IDX) { uint8_t(IDX), uint8_t(Any), uint16_t(RATiedReg::kRReg) }
#define W(IDX) { uint8_t(Any), uint8_t(IDX), uint16_t(RATiedReg::kWReg) }
#define X(IDX) { uint8_t(IDX), uint8_t(IDX), uint16_t(RATiedReg::kXReg) }
#define NONE() { uint8_t(Any), uint8_t(Any), uint16_t(0) }
#define DEFINE_OPS(NAME, ...) static const OpRWData NAME[6] = { __VA_ARGS__ }
#define RETURN_OPS(...) do { DEFINE_OPS(ops, __VA_ARGS__); return ops; } while(0)

struct OpRWData {
  uint8_t rPhysId;
  uint8_t wPhysId;
  uint16_t flags;
};

static ASMJIT_INLINE const OpRWData* OpRWData_get(
  uint32_t instId, const X86Inst& instData, const Operand* opArray, uint32_t opCount) noexcept {

  enum {
    Any = Globals::kInvalidRegId,
    Zax = X86Gp::kIdAx,
    Zbx = X86Gp::kIdBx,
    Zcx = X86Gp::kIdCx,
    Zdx = X86Gp::kIdDx,
    Zsi = X86Gp::kIdSi,
    Zdi = X86Gp::kIdDi
  };

  // Common cases.
  DEFINE_OPS(rwiRO  , R(Any), R(Any), R(Any), R(Any), R(Any), R(Any));
  DEFINE_OPS(rwiWO  , W(Any), R(Any), R(Any), R(Any), R(Any), R(Any));
  DEFINE_OPS(rwiRW  , X(Any), R(Any), R(Any), R(Any), R(Any), R(Any));
  DEFINE_OPS(rwiXchg, X(Any), X(Any), R(Any), R(Any), R(Any), R(Any));

  const X86Inst::CommonData& commonData = instData.getCommonData();
  if (!commonData.isSpecial()) {
    // Common cases.
    if (commonData.isRW()) return rwiRW;
    if (commonData.isWO()) return rwiWO;
    if (commonData.isRO()) return rwiRO;
    if (commonData.isXchg()) return rwiXchg;
  }
  else {
    switch (instId) {
      // Deprecated.
      case X86Inst::kIdAaa:
      case X86Inst::kIdAad:
      case X86Inst::kIdAam:
      case X86Inst::kIdAas:
      case X86Inst::kIdDaa:
      case X86Inst::kIdDas:
        RETURN_OPS(X(Zax));

      // CPUID.
      case X86Inst::kIdCpuid:
        RETURN_OPS(X(Zax), W(Zbx), X(Zcx), W(Zdx));

      // Extend.
      case X86Inst::kIdCbw:
      case X86Inst::kIdCdqe:
      case X86Inst::kIdCwde:
        RETURN_OPS(X(Zax));

      case X86Inst::kIdCdq:
      case X86Inst::kIdCwd:
      case X86Inst::kIdCqo:
        RETURN_OPS(W(Zdx), R(Zax));

      // Cmpxchg.
      case X86Inst::kIdCmpxchg:
        RETURN_OPS(X(Any), R(Any), X(Zax));

      case X86Inst::kIdCmpxchg8b:
      case X86Inst::kIdCmpxchg16b:
        RETURN_OPS(NONE(), X(Zdx), X(Zax), R(Zcx), R(Zbx));

      // Mul/Div.
      case X86Inst::kIdDiv:
      case X86Inst::kIdIdiv:
        if (opCount == 2)
          RETURN_OPS(X(Zax), R(Any));
        else
          RETURN_OPS(X(Zdx), X(Zax), R(Any));

      case X86Inst::kIdImul:
        if (opCount == 2)
          return rwiRW;
        if (opCount == 3 && !(opArray[0].isReg() && opArray[1].isReg() && opArray[2].isRegOrMem()))
          return rwiRW;
        ASMJIT_FALLTHROUGH;

      case X86Inst::kIdMul:
        if (opCount == 2)
          RETURN_OPS(X(Zax), R(Any));
        else
          RETURN_OPS(W(Zdx), X(Zax), R(Any));

      case X86Inst::kIdMulx:
        RETURN_OPS(W(Any), W(Any), R(Any), R(Zdx));

      // Jecxz/Loop.
      case X86Inst::kIdJecxz:
      case X86Inst::kIdLoop:
      case X86Inst::kIdLoope:
      case X86Inst::kIdLoopne:
        RETURN_OPS(R(Zcx));

      // Lahf/Sahf.
      case X86Inst::kIdLahf: RETURN_OPS(W(Zax));
      case X86Inst::kIdSahf: RETURN_OPS(R(Zax));

      // Enter/Leave.
      case X86Inst::kIdEnter: break;
      case X86Inst::kIdLeave: break;

      // Ret.
      case X86Inst::kIdRet: break;

      // Monitor/MWait.
      case X86Inst::kIdMonitor    : return nullptr; // TODO: [COMPILER] Monitor/MWait.
      case X86Inst::kIdMwait      : return nullptr; // TODO: [COMPILER] Monitor/MWait.

      // Push/Pop.
      case X86Inst::kIdPush       : return rwiRO;
      case X86Inst::kIdPop        : return rwiWO;

      // Shift/Rotate.
      case X86Inst::kIdRcl:
      case X86Inst::kIdRcr:
      case X86Inst::kIdRol:
      case X86Inst::kIdRor:
      case X86Inst::kIdSal:
      case X86Inst::kIdSar:
      case X86Inst::kIdShl:
      case X86Inst::kIdShr:
        // Only special if the last operand is register.
        if (opArray[1].isReg())
          RETURN_OPS(X(Any), R(Zcx));
        else
          return rwiRW;

      case X86Inst::kIdShld:
      case X86Inst::kIdShrd:
        // Only special if the last operand is register.
        if (opArray[2].isReg())
          RETURN_OPS(X(Any), R(Any), R(Zcx));
        else
          return rwiRW;

      // RDTSC.
      case X86Inst::kIdRdtsc:
      case X86Inst::kIdRdtscp:
        RETURN_OPS(W(Zdx), W(Zax), W(Zcx));

      // Xsave/Xrstor.
      case X86Inst::kIdXrstor:
      case X86Inst::kIdXrstor64:
      case X86Inst::kIdXsave:
      case X86Inst::kIdXsave64:
      case X86Inst::kIdXsaveopt:
      case X86Inst::kIdXsaveopt64:
        RETURN_OPS(W(Any), R(Zdx), R(Zax));

      // Xsetbv/Xgetbv.
      case X86Inst::kIdXgetbv:
        RETURN_OPS(W(Zdx), W(Zax), R(Zcx));

      case X86Inst::kIdXsetbv:
        RETURN_OPS(R(Zdx), R(Zax), R(Zcx));

      // In/Out.
      case X86Inst::kIdIn  : RETURN_OPS(W(Zax), R(Zdx));
      case X86Inst::kIdIns : RETURN_OPS(X(Zdi), R(Zdx));
      case X86Inst::kIdOut : RETURN_OPS(R(Zdx), R(Zax));
      case X86Inst::kIdOuts: RETURN_OPS(R(Zdx), X(Zsi));

      // String instructions.
      case X86Inst::kIdCmps: RETURN_OPS(X(Zsi), X(Zdi));
      case X86Inst::kIdLods: RETURN_OPS(W(Zax), X(Zsi));
      case X86Inst::kIdMovs: RETURN_OPS(X(Zdi), X(Zsi));
      case X86Inst::kIdScas: RETURN_OPS(X(Zdi), R(Zax));
      case X86Inst::kIdStos: RETURN_OPS(X(Zdi), R(Zax));

      // SSE+/AVX+.
      case X86Inst::kIdMaskmovq:
      case X86Inst::kIdMaskmovdqu:
      case X86Inst::kIdVmaskmovdqu:
        RETURN_OPS(R(Any), R(Any), R(Zdi));

      // SSE4.1+ and SHA.
      case X86Inst::kIdBlendvpd:
      case X86Inst::kIdBlendvps:
      case X86Inst::kIdPblendvb:
      case X86Inst::kIdSha256rnds2:
        RETURN_OPS(W(Any), R(Any), R(0));

      // SSE4.2+.
      case X86Inst::kIdPcmpestri  :
      case X86Inst::kIdVpcmpestri : RETURN_OPS(R(Any), R(Any), NONE(), W(Zcx));
      case X86Inst::kIdPcmpistri  :
      case X86Inst::kIdVpcmpistri : RETURN_OPS(R(Any), R(Any), NONE(), W(Zcx), R(Zax), R(Zdx));
      case X86Inst::kIdPcmpestrm  :
      case X86Inst::kIdVpcmpestrm : RETURN_OPS(R(Any), R(Any), NONE(), W(0));
      case X86Inst::kIdPcmpistrm  :
      case X86Inst::kIdVpcmpistrm : RETURN_OPS(R(Any), R(Any), NONE(), W(0)  , R(Zax), R(Zdx));
    }
  }

  return rwiRW;
}

#undef RETURN_OPS
#undef DEFINE_OPS
#undef NONE
#undef X
#undef W
#undef R

// ============================================================================
// [asmjit::X86RAPass - Construction / Destruction]
// ============================================================================

X86RAPass::X86RAPass() noexcept
  : RAPass() {}
X86RAPass::~X86RAPass() noexcept {}

// ============================================================================
// [asmjit::X86RAPass - Prepare / Cleanup]
// ============================================================================

Error X86RAPass::prepare(CCFunc* func) noexcept {
  ASMJIT_PROPAGATE(Base::prepare(func));
  uint32_t archType = cc()->getArchType();

  _archRegCount.set(X86Reg::kKindGp , archType == ArchInfo::kTypeX86 ? 7 : 15);
  _archRegCount.set(X86Reg::kKindMm , 8);
  _archRegCount.set(X86Reg::kKindK  , 7);
  _archRegCount.set(X86Reg::kKindVec, archType == ArchInfo::kTypeX86 ? 8 : 16);

  _allocableRegs.set(X86Reg::kKindGp , Utils::bits(_archRegCount.get(X86Reg::kKindGp ) & ~Utils::mask(X86Gp::kIdSp)));
  _allocableRegs.set(X86Reg::kKindMm , Utils::bits(_archRegCount.get(X86Reg::kKindMm )));
  _allocableRegs.set(X86Reg::kKindK  , Utils::bits(_archRegCount.get(X86Reg::kKindK  ) & ~1U));
  _allocableRegs.set(X86Reg::kKindVec, Utils::bits(_archRegCount.get(X86Reg::kKindVec)));

  if (func->getFrameInfo().hasPreservedFP()) {
    _archRegCount._regs[X86Reg::kKindGp]--;
    _allocableRegs.andNot(X86Reg::kKindGp, Utils::mask(X86Gp::kIdBp));
  }

  _zsp = cc()->zsp();
  _zbp = cc()->zbp();

  _indexRegs = _allocableRegs.get(X86Reg::kKindGp) & ~Utils::mask(4);
  _avxEnabled = false;

  return kErrorOk;
}

// ============================================================================
// [asmjit::X86RAPass - Steps - ConstructCFG]
// ============================================================================

static Error dumpSuccessors(const RABlock* block) noexcept {
  StringBuilderTmp<1024> sb;

  const RABlocks& successors = block->getSuccessors();
  if (successors.isEmpty())
    return kErrorOk;

  sb.appendString("=> { ");
  for (size_t i = 0, len = successors.getLength(); i < len; i++) {
    const RABlock* successor = successors[i];
    if (i != 0)
      sb.appendString(", ");
    sb.appendFormat("#%u", static_cast<unsigned int>(successor->getBlockId()));
  }
  sb.appendString(" }");
  printf("  %s\n", sb.getData());
  return kErrorOk;
}

Error X86RAPass::constructCFG() noexcept {
  printf("[RA::ConstructCFG]\n");
  X86Compiler* cc = this->cc();

  CCFunc* func = getFunc();
  CBNode* node = func;

  // Create the first (entry) block.
  RABlock* currentBlock = newBlock(node);
  if (ASMJIT_UNLIKELY(!currentBlock))
    return DebugUtils::errored(kErrorNoHeapMemory);

  bool hasCode = false;
  size_t blockIndex = 0;
  uint32_t position = 0;
  RATiedBuilder tb(this);

#if !defined(ASMJIT_DISABLE_LOGGING)
  StringBuilderTmp<256> sb;
  RABlock* lastPrintedBlock = nullptr;

  lastPrintedBlock = currentBlock;
  printf("{Block #%u}\n", lastPrintedBlock->getBlockId());
#endif // !ASMJIT_DISABLE_LOGGING

  for (;;) {
    for (;;) {
      ASMJIT_ASSERT(!node->hasPosition());
      node->setPosition(++position);

      if (node->getType() == CBNode::kNodeLabel) {
        if (!currentBlock) {
          // If the current code is unreachable the label makes it reachable again.
          currentBlock = node->as<CBLabel>()->getBlock();
          if (currentBlock) {
            // If the label has a block assigned we can either continue with
            // it or skip it if the block has been constructed already.
            if (currentBlock->isConstructed())
              break;
          }
          else {
            // Only create a new block if the label doesn't have assigned one.
            currentBlock = newBlock(node);
            if (ASMJIT_UNLIKELY(!currentBlock))
              return DebugUtils::errored(kErrorNoHeapMemory);

            node->as<CBLabel>()->setBlock(currentBlock);
            hasCode = false;
          }
        }
        else {
          // Label makes the current block constructed. There is a chance that the
          // Label is not used, but we don't know ihat at this point. Later, when
          // we have enough information we will be able to merge continuous blocks
          // into a single one if it's beneficial.
          currentBlock->setLast(node->getPrev());
          currentBlock->makeConstructed();

          if (node->as<CBLabel>()->hasBlock()) {
            RABlock* successor = node->as<CBLabel>()->getBlock();
            if (currentBlock == successor) {
              // The label currently processed is part of the current block. This
              // is only possible for multiple labels that are right next to each
              // other, or are separated by .align directives and/or comments.
              if (hasCode)
                return DebugUtils::errored(kErrorInvalidState);
            }
            else {
              ASMJIT_PROPAGATE(currentBlock->appendSuccessor(successor));
              dumpSuccessors(currentBlock);

              currentBlock = successor;
              hasCode = false;
            }
          }
          else {
            // First time we see this label.
            if (hasCode) {
              // Cannot continue the current block if it already contains some
              // code. We need to create a new block and make it a successor.
              currentBlock->setLast(node->getPrev());
              currentBlock->makeConstructed();

              RABlock* successor = newBlock(node);
              if (ASMJIT_UNLIKELY(!successor))
                return DebugUtils::errored(kErrorNoHeapMemory);

              ASMJIT_PROPAGATE(currentBlock->appendSuccessor(successor));
              dumpSuccessors(currentBlock);

              currentBlock = successor;
              hasCode = false;
            }

            node->as<CBLabel>()->setBlock(currentBlock);
          }
        }
#if !defined(ASMJIT_DISABLE_LOGGING)
        if (lastPrintedBlock != currentBlock) {
          lastPrintedBlock = currentBlock;
          printf("{Block #%u}\n", lastPrintedBlock->getBlockId());
        }

        sb.clear();
        Logging::formatNode(sb, 0, cc, node);
        printf("  %s\n", sb.getData());
#endif // !ASMJIT_DISABLE_LOGGING
      }
      else {
#if !defined(ASMJIT_DISABLE_LOGGING)
        sb.clear();
        Logging::formatNode(sb, 0, cc, node);
        printf("  %s\n", sb.getData());
#endif // !ASMJIT_DISABLE_LOGGING

        if (node->actsAsInst()) {
          if (ASMJIT_UNLIKELY(!currentBlock)) {
            // If this code is unreachable then it has to be removed.
            CBNode* next = node->getNext();
            cc->removeNode(node);
            node = next;

            position--;
            continue;
          }
          else {
            tb.reset(this);
            hasCode = true;

            // Handle `CBInst`, `CCFuncCall`, and `CCFuncRet`. All of
            // these share the `CBInst` interface and contain operands.
            CBInst* inst = node->as<CBInst>();
            uint32_t instId = inst->getInstId();

            if (ASMJIT_UNLIKELY(!X86Inst::isDefinedId(instId)))
              return DebugUtils::errored(kErrorInvalidInstruction);

            const X86Inst& instData = X86Inst::getInst(instId);
            const X86Inst::CommonData& commonData = instData.getCommonData();
            uint32_t numVirtRegs = static_cast<uint32_t>(cc->getVirtRegArray().getLength());

            uint32_t opCount = inst->getOpCount();
            uint32_t singleRegOps = 0;

            if (opCount) {
              const Operand* opArray = inst->getOpArray();
              const OpRWData* rwArray = OpRWData_get(instId, instData, opArray, opCount);

              for (uint32_t i = 0; i < opCount; i++) {
                const Operand& op = opArray[i];
                if (op.isReg()) {
                  // Register operand.
                  const X86Reg& reg = op.as<X86Reg>();
                  uint32_t vIndex = Operand::unpackId(reg.getId());

                  if (vIndex < Operand::kPackedIdCount) {
                    if (ASMJIT_UNLIKELY(vIndex >= numVirtRegs))
                      return DebugUtils::errored(kErrorInvalidVirtId);

                    VirtReg* vReg = cc->getVirtRegAt(vIndex);
                    uint32_t kind = vReg->getKind();

                    uint32_t allocable = _allocableRegs.get(kind);
                    ASMJIT_PROPAGATE(tb.add(vReg, 0, allocable, rwArray[i].rPhysId, rwArray[i].wPhysId));

                    if (singleRegOps == i)
                      singleRegOps++;
                  }
                }
                else if (op.isMem()) {
                  // Memory operand.
                  const X86Mem& mem = op.as<X86Mem>();
                  if (mem.hasBaseReg()) {
                    uint32_t vIndex = Operand::unpackId(mem.getBaseId());
                    if (vIndex < Operand::kPackedIdCount) {
                      if (ASMJIT_UNLIKELY(vIndex >= numVirtRegs))
                        return DebugUtils::errored(kErrorInvalidVirtId);

                      VirtReg* vReg = cc->getVirtRegAt(vIndex);
                      uint32_t kind = vReg->getKind();

                      uint32_t allocable = _allocableRegs.get(kind);
                      ASMJIT_PROPAGATE(tb.add(vReg, 0, allocable, kAnyReg, kAnyReg));
                    }
                  }

                  if (mem.hasIndexReg()) {
                    uint32_t vIndex = Operand::unpackId(mem.getIndexId());
                    if (vIndex < Operand::kPackedIdCount) {
                      if (ASMJIT_UNLIKELY(vIndex >= numVirtRegs))
                        return DebugUtils::errored(kErrorInvalidVirtId);

                      VirtReg* vReg = cc->getVirtRegAt(vIndex);
                      uint32_t kind = vReg->getKind();

                      uint32_t allocable = _allocableRegs.get(kind);
                      ASMJIT_PROPAGATE(tb.add(vReg, 0, allocable, kAnyReg, kAnyReg));
                    }
                  }
                }
              }
            }

            // Handle extra operand (either REP CX|ECX|RCX or AVX-512 {k} selector).
            if (inst->hasOpExtra()) {
              const Operand& opExtra = inst->getOpExtra();
              if (opExtra.isReg()) {
                const X86Reg& reg = opExtra.as<X86Reg>();
                uint32_t vIndex = Operand::unpackId(reg.getId());

                if (vIndex < Operand::kPackedIdCount) {
                  if (ASMJIT_UNLIKELY(vIndex >= numVirtRegs))
                    return DebugUtils::errored(kErrorInvalidVirtId);

                  VirtReg* vReg = cc->getVirtRegAt(vIndex);
                  uint32_t kind = vReg->getKind();

                  if (kind == X86Gp::kKindK) {
                    // AVX512 mask selector {k} register.
                    //   (read-only, allocable to any register except {k0})
                    ASMJIT_PROPAGATE(tb.add(vReg, RATiedReg::kRReg, _allocableRegs.get(kind), kAnyReg, kAnyReg));
                    singleRegOps = 0;
                  }
                  else {
                    // REP {cx|ecx|rcx} register - read & write.
                    ASMJIT_PROPAGATE(tb.add(vReg, RATiedReg::kXReg, 0, X86Gp::kIdCx, X86Gp::kIdCx));
                  }
                }
                else {
                  uint32_t kind = reg.getKind();
                  if (kind == X86Gp::kKindK && reg.getId() != 0)
                    singleRegOps = 0;
                }
              }
            }

            // Handle special cases of some instructions where all operands share
            // the same register. In such case the single operand becomes read-only
            // or write-only.
            if (singleRegOps == opCount && tb.getTotal() == 1) {
              switch (commonData.getSingleRegCase()) {
                case X86Inst::kSingleRegNone:
                  break;
                case X86Inst::kSingleRegRO:
                  tb.tmp[0].flags &= ~RATiedReg::kWReg;
                  break;
                case X86Inst::kSingleRegWO:
                  tb.tmp[0].flags &= ~RATiedReg::kRReg;
                  break;
              }
            }

            // Support for non `CBInst` nodes like `CCFuncCall` and `CCFuncRet`.
            // It's unlikely as we expect more instruction nodes than special nodes
            // like`CCFuncCall` and `CCFuncRet`.
            if (ASMJIT_UNLIKELY(inst->getType() != CBNode::kNodeInst)) {
              // TODO:
              ASMJIT_ASSERT(!"IMPLEMENTED");
            }

            // Support for conditional and unconditional jumps.
            if (commonData.isFlow()) {
              if (instId != X86Inst::kIdCall && instId != X86Inst::kIdRet) {
                // Jmp/Jcc/Call/Loop/etc...
                const Operand* opArray = inst->getOpArray();

                // The last operand must be label (this supports also instructions
                // like jecx in explicit form).
                if (opCount == 0 || !opArray[opCount - 1].isLabel())
                  return DebugUtils::errored(kErrorInvalidState);

                CBLabel* cbLabel;
                ASMJIT_PROPAGATE(cc->getCBLabel(&cbLabel, opArray[opCount - 1].as<Label>()));

                RABlock* jumpSuccessor = newBlockOrMergeWith(cbLabel);
                if (ASMJIT_UNLIKELY(!jumpSuccessor))
                  return DebugUtils::errored(kErrorNoHeapMemory);

                currentBlock->setLast(node);
                currentBlock->makeConstructed();
                ASMJIT_PROPAGATE(currentBlock->appendSuccessor(jumpSuccessor));

                if (instId == X86Inst::kIdJmp) {
                  // Unconditional jump makes the code after the jump unreachable,
                  // which will be removed instantly during the CFG construction;
                  // as we cannot allocate registers for instructions that are not
                  // part of any block. Of course we can leave these instructions
                  // as they are, however, that would only postpone the problem as
                  // X86Assembler can encode instructions that use only physical
                  // registers.
                  dumpSuccessors(currentBlock);
                  currentBlock = nullptr;
                }
                else {
                  node = node->getNext();
                  if (ASMJIT_UNLIKELY(!node))
                    return DebugUtils::errored(kErrorInvalidState);

                  RABlock* flowSuccessor;
                  if (node->getType() == CBNode::kNodeLabel) {
                    if (node->as<CBLabel>()->hasBlock()) {
                      flowSuccessor = node->as<CBLabel>()->getBlock();
                    }
                    else {
                      flowSuccessor = newBlock(node);
                      if (ASMJIT_UNLIKELY(!flowSuccessor))
                        return DebugUtils::errored(kErrorNoHeapMemory);
                      node->as<CBLabel>()->setBlock(flowSuccessor);
                    }
                  }
                  else {
                    flowSuccessor = newBlock(node);
                    if (ASMJIT_UNLIKELY(!flowSuccessor))
                      return DebugUtils::errored(kErrorNoHeapMemory);
                  }

                  ASMJIT_PROPAGATE(currentBlock->prependSuccessor(flowSuccessor));
                  dumpSuccessors(currentBlock);

                  currentBlock = flowSuccessor;
                  hasCode = false;

                  if (currentBlock->isConstructed())
                    break;

                  lastPrintedBlock = currentBlock;
                  printf("{Block #%u}\n", lastPrintedBlock->getBlockId());
                  continue;
                }
              }
            }
          }
        }
        else if (node->getType() == CBNode::kNodeSentinel) {
          // Sentinel could be anything, however, if this is the end of function
          // marker it's the function's exit. This means this node must be added
          // to `_exits`.
          if (node == getFunc()->getEnd()) {
            // Only add the current block to exists if it's reachable.
            if (currentBlock) {
              currentBlock->setLast(node);
              currentBlock->makeConstructed();
              ASMJIT_PROPAGATE(_exits.append(currentBlock));
            }
            break;
          }
        }
        else if (node->getType() == CBNode::kNodeFunc) {
          // CodeCompiler can only compile single function at a time. If we
          // encountered a function it must be the current one, bail if not.
          if (ASMJIT_UNLIKELY(node != func))
            return DebugUtils::errored(kErrorInvalidState);
          // PASS if this is the first node.
        }
        else {
          // PASS if this is a non-interesting or unknown node.
        }
      }

      // Advance to the next node.
      node = node->getNext();

      // NOTE: We cannot encounter a NULL node, because every function must be
      // terminated by a `stop` node. If we encountered a NULL node it means that
      // something went wrong and this node list is corrupted; bail in such case.
      if (ASMJIT_UNLIKELY(!node))
        return DebugUtils::errored(kErrorInvalidState);
    }

    // We finalized the current block so find another to process or return if
    // there are no more blocks.
    do {
      if (++blockIndex >= _blocks.getLength())
        return kErrorOk;

      currentBlock = _blocks[blockIndex];
    } while (currentBlock->isConstructed());

    node = currentBlock->getLast();
    hasCode = false;
  }
}

} // asmjit namespace

// [Api-End]
#include "../asmjit_apiend.h"

// [Guard]
#endif // ASMJIT_BUILD_X86 && !ASMJIT_DISABLE_COMPILER


































































#if 0
#define RA_POPULATE(NODE) \
  do { \
    RAData* raData = newRAData(0); \
    if (!raData) goto NoMem; \
    NODE->setPassData(raData); \
  } while (0)

#define RA_DECLARE() \
  do { \
    RARegCount tiedCount; \
    RARegCount tiedIndex; \
    uint32_t tiedTotal = 0; \
    \
    RARegMask inRegs; \
    RARegMask outRegs; \
    RARegMask clobberedRegs; \
    \
    tiedCount.reset(); \
    inRegs.reset(); \
    outRegs.reset(); \
    clobberedRegs.reset()

#define RA_FINALIZE(NODE) \
    { \
      RAData* raData = newRAData(tiedTotal); \
      if (!raData) goto NoMem; \
      \
      tiedIndex.indexFromRegCount(tiedCount); \
      raData->tiedCount = tiedCount; \
      raData->tiedIndex = tiedIndex; \
      \
      raData->inRegs = inRegs; \
      raData->outRegs = outRegs; \
      raData->clobberedRegs = clobberedRegs; \
      \
      RATiedReg* tied = agTmp; \
      while (tiedTotal) { \
        VirtReg* vreg = tied->vreg; \
        \
        uint32_t _kind  = vreg->getKind(); \
        uint32_t _index = tiedIndex.get(_kind); \
        \
        tiedIndex.add(_kind); \
        if (tied->inRegs) \
          tied->allocableRegs = tied->inRegs; \
        else if (tied->outPhysId != Globals::kInvalidRegId) \
          tied->allocableRegs = Utils::mask(tied->outPhysId); \
        else \
          tied->allocableRegs &= ~inRegs.get(_kind); \
        \
        vreg->_tied = nullptr; \
        raData->setTiedAt(_index, *tied); \
        \
        tied++; \
        tiedTotal--; \
      } \
      NODE->setPassData(raData); \
     } \
  } while (0)

  // --------------------------------------------------------------------------
  // [Loop]
  // --------------------------------------------------------------------------

  do {
_Do:
    while (node_->hasPassData()) {
_NextGroup:
      if (!jLink)
        jLink = _jccList.getFirst();
      else
        jLink = jLink->getNext();

      if (!jLink) goto _Done;
      node_ = X86RAPass_getOppositeJccFlow(static_cast<CBJump*>(jLink->getValue()));
    }
    flowId++;

    next = node_->getNext();
    node_->setFlowId(flowId);

    switch (node_->getType()) {
      // ----------------------------------------------------------------------
      // [Align/Embed]
      // ----------------------------------------------------------------------

      case CBNode::kNodeAlign:
      case CBNode::kNodeData:
      default:
        RA_POPULATE(node_);
        break;

      // ----------------------------------------------------------------------
      // [Label]
      // ----------------------------------------------------------------------

      case CBNode::kNodeLabel: {
        RA_POPULATE(node_);
        if (node_ == func->getExitNode()) {
          ASMJIT_PROPAGATE(addReturningNode(node_));
          goto _NextGroup;
        }
        break;
      }

      // ----------------------------------------------------------------------
      // [Inst]
      // ----------------------------------------------------------------------

      case CBNode::kNodeInst: {
        CBInst* node = static_cast<CBInst*>(node_);

        uint32_t instId = node->getInstId();
        uint32_t flags = node->getFlags();
        uint32_t options = node->getOptions();
        uint32_t gpAllowedMask = 0xFFFFFFFF;

        Operand* opArray = node->getOpArray();
        uint32_t opCount = node->getOpCount();

        const X86Inst::CommonData& commonData = X86Inst::getInst(instId).getCommonData();

        RA_DECLARE();
        if (opCount) {
          const OpRWData* special = nullptr;

          // Collect instruction flags and merge all 'RATiedReg's.
          if (commonData.isSpecial() && (special = OpRWData_get(instId, opArray, opCount)) != nullptr)
            flags |= CBNode::kFlagIsSpecial;

          for (uint32_t i = 0; i < opCount; i++) {
            Operand* op = &opArray[i];
            VirtReg* vreg;
            RATiedReg* tied;

            if (op->isVirtReg()) {
              vreg = cc()->getVirtRegById(op->getId());
              if (vreg->isFixed()) continue;

              RA_MERGE(vreg, tied, 0, gaRegs[vreg->getKind()] & gpAllowedMask);
              if (static_cast<X86Reg*>(op)->isGpb()) {
                tied->flags |= static_cast<X86Gp*>(op)->isGpbLo() ? RATiedReg::kX86GpbLo : RATiedReg::kX86GpbHi;
                if (archType == ArchInfo::kTypeX86) {
                  // If a byte register is accessed in 32-bit mode we have to limit
                  // all allocable registers for that variable to eax/ebx/ecx/edx.
                  // Other variables are not affected.
                  tied->allocableRegs &= 0x0F;
                }
                else {
                  // It's fine if lo-byte register is accessed in 64-bit mode;
                  // however, hi-byte has to be checked and if it's used all
                  // registers (GP/XMM) could be only allocated in the lower eight
                  // half. To do that, we patch 'allocableRegs' of all variables
                  // we collected until now and change the allocable restriction
                  // for variables that come after.
                  if (static_cast<X86Gp*>(op)->isGpbHi()) {
                    tied->allocableRegs &= 0x0F;
                    if (gpAllowedMask != 0xFF) {
                      for (uint32_t j = 0; j < i; j++)
                        agTmp[j].allocableRegs &= (agTmp[j].flags & RATiedReg::kX86GpbHi) ? 0x0F : 0xFF;
                      gpAllowedMask = 0xFF;
                    }
                  }
                }
              }

              if (special) {
                uint32_t inReg = special[i].inReg;
                uint32_t outReg = special[i].outReg;
                uint32_t c;

                if (static_cast<const X86Reg*>(op)->isGp())
                  c = X86Reg::kKindGp;
                else
                  c = X86Reg::kKindVec;

                if (inReg != Globals::kInvalidRegId) {
                  uint32_t mask = Utils::mask(inReg);
                  inRegs.or_(c, mask);
                  tied->inRegs |= mask;
                }

                if (outReg != Globals::kInvalidRegId) {
                  uint32_t mask = Utils::mask(outReg);
                  outRegs.or_(c, mask);
                  tied->setOutPhysId(outReg);
                }

                tied->flags |= special[i].flags;
              }
              else {
                uint32_t inFlags = RATiedReg::kRReg;
                uint32_t outFlags = RATiedReg::kWReg;
                uint32_t combinedFlags;

                if (i == 0) {
                  // Read/Write is usually the combination of the first operand.
                  combinedFlags = inFlags | outFlags;

                  if (node->getOptions() & CodeEmitter::kOptionOverwrite) {
                    // Manually forcing write-only.
                    combinedFlags = outFlags;
                  }
                  else if (commonData.isWO()) {
                    // Write-only instruction.
                    uint32_t movSize = commonData.getWriteSize();
                    uint32_t regSize = vreg->getSize();

                    // Exception - If the source operand is a memory location
                    // promote move size into 16 bytes.
                    if (commonData.isZeroIfMem() && opArray[1].isMem())
                      movSize = 16;

                    if (static_cast<const X86Reg*>(op)->isGp()) {
                      uint32_t opSize = static_cast<const X86Reg*>(op)->getSize();

                      // Move size is zero in case that it should be determined
                      // from the destination register.
                      if (movSize == 0)
                        movSize = opSize;

                      // Handle the case that a 32-bit operation in 64-bit mode
                      // always clears the rest of the destination register and
                      // the case that move size is actually greater than or
                      // equal to the size of the variable.
                      if (movSize >= 4 || movSize >= regSize)
                        combinedFlags = outFlags;
                    }
                    else if (movSize >= regSize) {
                      // If move size is greater than or equal to the size of
                      // the variable there is nothing to do, because the move
                      // will overwrite the variable in all cases.
                      combinedFlags = outFlags;
                    }
                  }
                  else if (commonData.isRO()) {
                    // Comparison/Test instructions don't modify any operand.
                    combinedFlags = inFlags;
                  }
                  else if (instId == X86Inst::kIdImul && opCount == 3) {
                    // Imul.
                    combinedFlags = outFlags;
                  }
                }
                else {
                  // Read-Only is usualy the combination of the second/third/fourth operands.
                  combinedFlags = inFlags;

                  // Idiv is a special instruction, never handled here.
                  ASMJIT_ASSERT(instId != X86Inst::kIdIdiv);

                  // Xchg/Xadd/Imul.
                  if (commonData.isXchg() || (instId == X86Inst::kIdImul && opCount == 3 && i == 1))
                    combinedFlags = inFlags | outFlags;
                }
                tied->flags |= combinedFlags;
              }
            }
            else if (op->isMem()) {
              X86Mem* m = static_cast<X86Mem*>(op);
              node->setMemOpIndex(i);

              uint32_t specBase = special ? uint32_t(special[i].inReg) : uint32_t(Globals::kInvalidRegId);

              if (m->hasBaseReg()) {
                uint32_t id = m->getBaseId();
                if (cc()->isVirtRegValid(id)) {
                  vreg = cc()->getVirtRegById(id);
                  if (!vreg->isStack() && !vreg->isFixed()) {
                    RA_MERGE(vreg, tied, 0, gaRegs[vreg->getKind()] & gpAllowedMask);
                    if (m->isRegHome()) {
                      uint32_t inFlags = RATiedReg::kRMem;
                      uint32_t outFlags = RATiedReg::kWMem;
                      uint32_t combinedFlags;

                      if (i == 0) {
                        // Default for the first operand.
                        combinedFlags = inFlags | outFlags;

                        if (commonData.isWO()) {
                          // Move to memory - setting the right flags is important
                          // as if it's just move to the register. It's just a bit
                          // simpler as there are no special cases.
                          uint32_t movSize = Utils::iMax<uint32_t>(commonData.getWriteSize(), m->getSize());
                          uint32_t regSize = vreg->getSize();

                          if (movSize >= regSize)
                            combinedFlags = outFlags;
                        }
                        else if (commonData.isRO()) {
                          // Comparison/Test instructions don't modify any operand.
                          combinedFlags = inFlags;
                        }
                      }
                      else {
                        // Default for the second operand.
                        combinedFlags = inFlags;

                        // Handle Xchg instruction (modifies both operands).
                        if (commonData.isXchg())
                          combinedFlags = inFlags | outFlags;
                      }

                      tied->flags |= combinedFlags;
                    }
                    else {
                      if (specBase != Globals::kInvalidRegId) {
                        uint32_t mask = Utils::mask(specBase);
                        inRegs.or_(vreg->getKind(), mask);
                        outRegs.or_(vreg->getKind(), mask);
                        tied->inRegs |= mask;
                        tied->setOutPhysId(specBase);
                        tied->flags |= special[i].flags;
                      }
                      else {
                        tied->flags |= RATiedReg::kRReg;
                      }
                    }
                  }
                }
              }

              if (m->hasIndexReg()) {
                uint32_t id = m->getIndexId();
                if (cc()->isVirtRegValid(id)) {
                  // Restrict allocation to all registers except ESP|RSP.
                  vreg = cc()->getVirtRegById(m->getIndexId());
                  if (!vreg->isFixed()) {
                    // TODO: AVX vector operands support.
                    RA_MERGE(vreg, tied, 0, gaRegs[X86Reg::kKindGp] & gpAllowedMask);
                    tied->allocableRegs &= indexMask;
                    tied->flags |= RATiedReg::kRReg;
                  }
                }
              }
            }
          }

          node->setFlags(flags);
          if (tiedTotal) {
            // Handle instructions which result in zeros/ones or nop if used with the
            // same destination and source operand.
            if (tiedTotal == 1 && opCount >= 2 && opArray[0].isVirtReg() && opArray[1].isVirtReg() && !node->hasMemOp())
              X86RAPass_prepareSingleVarInst(instId, &agTmp[0]);
          }
        }

        const Operand_& opExtra = node->getOpExtra();
        if ((options & CodeEmitter::kOptionOpExtra) != 0 && opExtra.isReg()) {
          uint32_t id = opExtra.as<Reg>().getId();
          if (cc()->isVirtRegValid(id)) {
            VirtReg* vreg = cc()->getVirtRegById(id);
            RATiedReg* tied;
            RA_MERGE(vreg, tied, 0, gaRegs[vreg->getKind()] & gpAllowedMask);

            if (options & (X86Inst::kOptionRep | X86Inst::kOptionRepnz)) {
              tied->allocableRegs = Utils::mask(X86Gp::kIdCx);
              tied->flags |= RATiedReg::kXReg;
            }
            else {
              tied->flags |= RATiedReg::kRReg;
            }
            hasExtraReg = true;
          }
        }

        RA_FINALIZE(node_);

        // Handle conditional/unconditional jump.
        if (node->isJmpOrJcc()) {
          CBJump* jNode = static_cast<CBJump*>(node);
          CBLabel* jTarget = jNode->getTarget();

          // If this jump is unconditional we put next node to unreachable node
          // list so we can eliminate possible dead code. We have to do this in
          // all cases since we are unable to translate without fetch() step.
          //
          // We also advance our node pointer to the target node to simulate
          // natural flow of the function.
          if (jNode->isJmp()) {
            if (!next->hasPassData())
              ASMJIT_PROPAGATE(addUnreachableNode(next));

            // Jump not followed.
            if (!jTarget) {
              ASMJIT_PROPAGATE(addReturningNode(jNode));
              goto _NextGroup;
            }

            node_ = jTarget;
            goto _Do;
          }
          else {
            // Jump not followed.
            if (!jTarget) break;

            if (jTarget->hasPassData()) {
              uint32_t jTargetFlowId = jTarget->getFlowId();

              // Update CBNode::kFlagIsTaken to true if this is a conditional
              // backward jump. This behavior can be overridden by using
              // `X86Inst::kOptionTaken` when the instruction is created.
              if (!jNode->isTaken() && opCount == 1 && jTargetFlowId <= flowId) {
                jNode->_flags |= CBNode::kFlagIsTaken;
              }
            }
            else if (next->hasPassData()) {
              node_ = jTarget;
              goto _Do;
            }
            else {
              ASMJIT_PROPAGATE(addJccNode(jNode));
              node_ = X86RAPass_getJccFlow(jNode);
              goto _Do;
            }
          }
        }
        break;
      }

      // ----------------------------------------------------------------------
      // [Func-Entry]
      // ----------------------------------------------------------------------

      case CBNode::kNodeFunc: {
        ASMJIT_ASSERT(node_ == func);
        X86RAPass_assignStackArgsRegId(this, func);

        FuncDetail& fd = func->getDetail();
        RATiedReg* tied;

        RA_DECLARE();
        cc()->setCursor(node_);

        X86Gp saReg;
        uint32_t argCount = fd.getArgCount();

        for (uint32_t i = 0; i < argCount; i++) {
          const FuncDetail::Value& arg = fd.getArg(i);

          VirtReg* vReg = func->getArg(i);
          if (!vReg) continue;

          // Overlapped function arguments.
          if (vReg->_tied)
            return DebugUtils::errored(kErrorOverlappedRegs);

          uint32_t aTypeId = arg.getTypeId();
          uint32_t vTypeId = vReg->getTypeId();

          uint32_t aKind = X86Reg::kindOf(arg.getRegType());
          uint32_t vKind = vReg->getKind();

          if (arg.byReg()) {
            if (aKind == vKind) {
              RA_INSERT(vReg, tied, RATiedReg::kWReg, 0);
              tied->setOutPhysId(arg.getRegId());
            }
            else {
              X86Reg rTmp = cc()->newReg(arg.getTypeId(), "arg%u", i);
              VirtReg* vTmp = cc()->getVirtReg(rTmp);

              RA_INSERT(vTmp, tied, RATiedReg::kWReg, 0);
              tied->setOutPhysId(arg.getRegId());

              X86Reg dstReg(X86Reg::fromSignature(vReg->getSignature(), vReg->getId()));
              X86Reg srcReg(X86Reg::fromSignature(vTmp->getSignature(), vTmp->getId()));

              // Emit conversion after the prolog.
              return X86Internal::emitArgMove(reinterpret_cast<X86Emitter*>(cc()),
                dstReg, vReg->getTypeId(),
                srcReg, vTmp->getTypeId(), _avxEnabled);
            }
          }
          else {
            // Instead of complicating the prolog allocation we create a virtual
            // register that holds the base address to all arguments passed by
            // stack and then insert nodes that copy these arguments to registers.
            if (!saReg.isValid()) {
              saReg = cc()->newGpz("__args");
              if (!saReg.isValid()) goto NoMem;

              VirtReg* saBase = cc()->getVirtReg(saReg);
              RA_INSERT(saBase, tied, RATiedReg::kWReg, 0);

              if (func->getFrameInfo().hasPreservedFP())
                saBase->_isFixed = true;
              tied->setOutPhysId(func->getFrameInfo().getStackArgsRegId());
            }

            // Argument passed by stack is handled after the prolog.
            X86Gp aReg = X86Gp::fromSignature(vReg->getSignature(), vReg->getId());
            X86Mem aMem = x86::ptr(saReg, arg.getStackOffset());
            aMem.markArgHome();

            ASMJIT_PROPAGATE(
              X86Internal::emitArgMove(reinterpret_cast<X86Emitter*>(cc()),
                aReg, vReg->getTypeId(), aMem, arg.getTypeId(), _avxEnabled));
          }
        }

        // If saReg is not needed, clear it also from FuncFrameInfo.
        if (!saReg.isValid())
          func->getFrameInfo().setStackArgsRegId(Globals::kInvalidRegId);

        RA_FINALIZE(node_);
        next = node_->getNext();
        break;
      }

      // ----------------------------------------------------------------------
      // [End]
      // ----------------------------------------------------------------------

      case CBNode::kNodeSentinel: {
        RA_POPULATE(node_);
        ASMJIT_PROPAGATE(addReturningNode(node_));
        goto _NextGroup;
      }

      // ----------------------------------------------------------------------
      // [Func-Exit]
      // ----------------------------------------------------------------------

      case CBNode::kNodeFuncRet: {
        CCFuncRet* node = node_->as<CCFuncRet>();
        ASMJIT_PROPAGATE(addReturningNode(node));

        FuncDetail& fd = func->getDetail();
        RA_DECLARE();

        if (fd.hasRet()) {
          const FuncDetail::Value& ret = fd.getRet(0);
          uint32_t retKind = X86Reg::kindOf(ret.getRegType());

          for (uint32_t i = 0; i < 2; i++) {
            Operand_* op = &node->_ret[i];
            if (op->isVirtReg()) {
              VirtReg* vreg = cc()->getVirtRegById(op->getId());
              RATiedReg* tied;
              RA_MERGE(vreg, tied, 0, 0);

              if (retKind == vreg->getKind()) {
                tied->flags |= RATiedReg::kRReg;
                tied->inRegs = Utils::mask(ret.getRegId());
                inRegs.or_(retKind, tied->inRegs);
              }
              else if (retKind == X86Reg::kKindFp) {
                uint32_t fldFlag = ret.getTypeId() == TypeId::kF32 ? RATiedReg::kX86Fld4 : RATiedReg::kX86Fld8;
                tied->flags |= RATiedReg::kRMem | fldFlag;
              }
              else {
                // TODO: Fix possible other return type conversions.
                ASMJIT_NOT_REACHED();
              }
            }
          }
        }
        RA_FINALIZE(node_);

        if (!next->hasPassData())
          ASMJIT_PROPAGATE(addUnreachableNode(next));
        goto _NextGroup;
      }

      // ----------------------------------------------------------------------
      // [Func-Call]
      // ----------------------------------------------------------------------

      case CBNode::kNodeFuncCall: {
        CCFuncCall* node = node_->as<CCFuncCall>();
        FuncDetail& fd = node->getDetail();

        Operand_* target = node->_opArray;
        Operand_* args = node->_args;
        Operand_* rets = node->_ret;

        func->getFrameInfo().enableCalls();
        func->getFrameInfo().mergeCallFrameSize(fd.getArgStackSize());

        // TODO: Each function frame should also define its stack arguments' alignment.
        // func->getFrameInfo().mergeCallFrameAlignment();

        uint32_t i;
        uint32_t argCount = fd.getArgCount();
        uint32_t sArgCount = 0;
        uint32_t gpAllocableMask = gaRegs[X86Reg::kKindGp] & ~node->getDetail().getUsedRegs(X86Reg::kKindGp);

        VirtReg* vreg;
        RATiedReg* tied;

        RA_DECLARE();

        // Function-call operand.
        if (target->isVirtReg()) {
          vreg = cc()->getVirtRegById(target->getId());
          RA_MERGE(vreg, tied, 0, 0);

          tied->flags |= RATiedReg::kRReg | RATiedReg::kRCall;
          if (tied->inRegs == 0)
            tied->allocableRegs |= gpAllocableMask;
        }
        else if (target->isMem()) {
          X86Mem* m = static_cast<X86Mem*>(target);

          if (m->hasBaseReg() &&  Operand::isPackedId(m->getBaseId())) {
            vreg = cc()->getVirtRegById(m->getBaseId());
            if (!vreg->isStack()) {
              RA_MERGE(vreg, tied, 0, 0);
              if (m->isRegHome()) {
                tied->flags |= RATiedReg::kRMem | RATiedReg::kRCall;
              }
              else {
                tied->flags |= RATiedReg::kRReg | RATiedReg::kRCall;
                if (tied->inRegs == 0)
                  tied->allocableRegs |= gpAllocableMask;
              }
            }
          }

          if (m->hasIndexReg() && Operand::isPackedId(m->getIndexId())) {
            // Restrict allocation to all registers except ESP/RSP.
            vreg = cc()->getVirtRegById(m->getIndexId());
            RA_MERGE(vreg, tied, 0, 0);

            tied->flags |= RATiedReg::kRReg | RATiedReg::kRCall;
            if ((tied->inRegs & ~indexMask) == 0)
              tied->allocableRegs &= gpAllocableMask & indexMask;
          }
        }

        // Function-call arguments.
        for (i = 0; i < argCount; i++) {
          Operand_* op = &args[i];
          if (!op->isVirtReg()) continue;

          vreg = cc()->getVirtRegById(op->getId());
          const FuncDetail::Value& arg = fd.getArg(i);

          if (arg.byReg()) {
            RA_MERGE(vreg, tied, 0, 0);

            uint32_t argType = arg.getTypeId();
            uint32_t argClass = X86Reg::kindOf(arg.getRegType());

            if (vreg->getKind() == argClass) {
              tied->inRegs |= Utils::mask(arg.getRegId());
              tied->flags |= RATiedReg::kRReg | RATiedReg::kRFunc;
            }
            else {
              // TODO: Function-call argument conversion.
            }
          }
          // If this is a stack-based argument we insert CCPushArg instead of
          // using RATiedReg. It improves the code, because the argument can be
          // moved onto stack as soon as it is ready and the register used by
          // the variable can be reused for something else. It is also much
          // easier to handle argument conversions, because there will be at
          // most only one node per conversion.
          else {
            if (X86RAPass_insertPushArg(this, node, vreg, gaRegs, arg, i, sArgList, sArgCount) != kErrorOk)
              goto NoMem;
          }
        }

        // Function-call returns.
        for (i = 0; i < 2; i++) {
          Operand_* op = &rets[i];
          if (!op->isVirtReg()) continue;

          const FuncDetail::Value& ret = fd.getRet(i);
          if (ret.byReg()) {
            uint32_t retType = ret.getTypeId();
            uint32_t retKind = X86Reg::kindOf(ret.getRegType());

            vreg = cc()->getVirtRegById(op->getId());
            RA_MERGE(vreg, tied, 0, 0);

            if (vreg->getKind() == retKind) {
              tied->setOutPhysId(ret.getRegId());
              tied->flags |= RATiedReg::kWReg | RATiedReg::kWFunc;
            }
            else {
              // TODO: Function-call return value conversion.
            }
          }
        }

        // Init clobbered.
        clobberedRegs.set(X86Reg::kKindGp , Utils::bits(_regCount.getGp())  & (~fd.getPreservedRegs(X86Reg::kKindGp )));
        clobberedRegs.set(X86Reg::kKindMm , Utils::bits(_regCount.getMm())  & (~fd.getPreservedRegs(X86Reg::kKindMm )));
        clobberedRegs.set(X86Reg::kKindK  , Utils::bits(_regCount.getK())   & (~fd.getPreservedRegs(X86Reg::kKindK  )));
        clobberedRegs.set(X86Reg::kKindVec, Utils::bits(_regCount.getVec()) & (~fd.getPreservedRegs(X86Reg::kKindVec)));

        RA_FINALIZE(node_);
        break;
      }
    }

    node_ = next;
  } while (node_ != stop);

_Done:
  // Mark exit label and end node as fetched, otherwise they can be removed by
  // `removeUnreachableCode()`, which would lead to a crash in some later step.
  node_ = func->getEnd();
  if (!node_->hasPassData()) {
    CBLabel* fExit = func->getExitNode();
    RA_POPULATE(fExit);
    fExit->setFlowId(++flowId);

    RA_POPULATE(node_);
    node_->setFlowId(++flowId);
  }

  return kErrorOk;

  // --------------------------------------------------------------------------
  // [Failure]
  // --------------------------------------------------------------------------

NoMem:
  return DebugUtils::errored(kErrorNoHeapMemory);
#endif
