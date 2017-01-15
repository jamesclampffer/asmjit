// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_BASE_RAPASS_P_H
#define _ASMJIT_BASE_RAPASS_P_H

#include "../asmjit_build.h"
#if !defined(ASMJIT_DISABLE_COMPILER)

// [Dependencies]
#include "../base/codecompiler.h"
#include "../base/zone.h"

// [Api-Begin]
#include "../asmjit_apibegin.h"

#if !defined(ASMJIT_DISABLE_LOGGING)
# define ASMJIT_RA_LOG_INIT(LOGGER) \
  Logger* logger = LOGGER;
# define ASMJIT_RA_LOG_FORMAT(...) \
  do {                             \
    if (logger)                    \
      logger->logf(__VA_ARGS__);   \
  } while (0)
#else
# define ASMJIT_RA_LOG_INIT(LOGGER) do {} while (0)
# define ASMJIT_RA_LOG_FORMAT(...)    do {} while (0)
#endif

namespace asmjit {

//! \addtogroup asmjit_base
//! \{

// ============================================================================
// [Forward Declarations]
// ============================================================================

class RABlock;
class WorkReg;

typedef ZoneVector<RABlock*> RABlocks;
typedef ZoneVector<WorkReg*> WorkRegs;

// ============================================================================
// [asmjit::RABits]
// ============================================================================

//! BitArray of fixed size.
struct RABits {
  // --------------------------------------------------------------------------
  // [Enums]
  // --------------------------------------------------------------------------

  enum {
    kEntitySize = static_cast<int>(sizeof(uintptr_t)),
    kEntityBits = kEntitySize * 8
  };

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE uintptr_t getBit(uint32_t index) const noexcept {
    return (data[index / kEntityBits] >> (index % kEntityBits)) & 1;
  }

  ASMJIT_INLINE void setBit(uint32_t index) noexcept {
    data[index / kEntityBits] |= static_cast<uintptr_t>(1) << (index % kEntityBits);
  }

  ASMJIT_INLINE void clearBit(uint32_t index) noexcept {
    data[index / kEntityBits] &= ~(static_cast<uintptr_t>(1) << (index % kEntityBits));
  }

  // --------------------------------------------------------------------------
  // [Interface]
  // --------------------------------------------------------------------------

  //! Copy bits from `s0`, returns `true` if at least one bit is set in `s0`.
  ASMJIT_INLINE bool copyBits(const RABits* s0, uint32_t len) noexcept {
    uintptr_t r = 0;
    for (uint32_t i = 0; i < len; i++) {
      uintptr_t t = s0->data[i];
      data[i] = t;
      r |= t;
    }
    return r != 0;
  }

  ASMJIT_INLINE bool addBits(const RABits* s0, uint32_t len) noexcept {
    return addBits(this, s0, len);
  }

  ASMJIT_INLINE bool addBits(const RABits* s0, const RABits* s1, uint32_t len) noexcept {
    uintptr_t r = 0;
    for (uint32_t i = 0; i < len; i++) {
      uintptr_t t = s0->data[i] | s1->data[i];
      data[i] = t;
      r |= t;
    }
    return r != 0;
  }

  ASMJIT_INLINE bool andBits(const RABits* s1, uint32_t len) noexcept {
    return andBits(this, s1, len);
  }

  ASMJIT_INLINE bool andBits(const RABits* s0, const RABits* s1, uint32_t len) noexcept {
    uintptr_t r = 0;
    for (uint32_t i = 0; i < len; i++) {
      uintptr_t t = s0->data[i] & s1->data[i];
      data[i] = t;
      r |= t;
    }
    return r != 0;
  }

  ASMJIT_INLINE bool clearBits(const RABits* s1, uint32_t len) noexcept {
    return clearBits(this, s1, len);
  }

  ASMJIT_INLINE bool clearBits(const RABits* s0, const RABits* s1, uint32_t len) noexcept {
    uintptr_t r = 0;
    for (uint32_t i = 0; i < len; i++) {
      uintptr_t t = s0->data[i] & ~s1->data[i];
      data[i] = t;
      r |= t;
    }
    return r != 0;
  }

  ASMJIT_INLINE bool _addBitsDelSource(RABits* s1, uint32_t len) noexcept {
    return _addBitsDelSource(this, s1, len);
  }

  ASMJIT_INLINE bool _addBitsDelSource(const RABits* s0, RABits* s1, uint32_t len) noexcept {
    uintptr_t r = 0;
    for (uint32_t i = 0; i < len; i++) {
      uintptr_t a = s0->data[i];
      uintptr_t b = s1->data[i];

      this->data[i] = a | b;
      b &= ~a;

      s1->data[i] = b;
      r |= b;
    }
    return r != 0;
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  uintptr_t data[1];
};

// ============================================================================
// [asmjit::RARegCount]
// ============================================================================

//! Registers count.
struct RARegCount {
  // --------------------------------------------------------------------------
  // [Reset]
  // --------------------------------------------------------------------------

  //! Reset all counters to zero.
  ASMJIT_INLINE void reset() noexcept { _packed = 0; }

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get register count by a register `kind`.
  ASMJIT_INLINE uint32_t get(uint32_t kind) const noexcept {
    ASMJIT_ASSERT(kind < Globals::kMaxVRegKinds);

    uint32_t shift = Utils::byteShiftOfDWordStruct(kind);
    return (_packed >> shift) & static_cast<uint32_t>(0xFF);
  }

  //! Set register count by a register `kind`.
  ASMJIT_INLINE void set(uint32_t kind, uint32_t n) noexcept {
    ASMJIT_ASSERT(kind < Globals::kMaxVRegKinds);
    ASMJIT_ASSERT(n <= 0xFF);

    uint32_t shift = Utils::byteShiftOfDWordStruct(kind);
    _packed = (_packed & ~static_cast<uint32_t>(0xFF << shift)) + (n << shift);
  }

  //! Add register count by a register `kind`.
  ASMJIT_INLINE void add(uint32_t kind, uint32_t n = 1) noexcept {
    ASMJIT_ASSERT(kind < Globals::kMaxVRegKinds);
    ASMJIT_ASSERT(0xFF - static_cast<uint32_t>(_regs[kind]) >= n);

    uint32_t shift = Utils::byteShiftOfDWordStruct(kind);
    _packed += n << shift;
  }

  // --------------------------------------------------------------------------
  // [Misc]
  // --------------------------------------------------------------------------

  //! Build register indexes based on the given `count` of registers.
  ASMJIT_INLINE void indexFromRegCount(const RARegCount& count) noexcept {
    uint32_t x = static_cast<uint32_t>(count._regs[0]);
    uint32_t y = static_cast<uint32_t>(count._regs[1]) + x;
    uint32_t z = static_cast<uint32_t>(count._regs[2]) + y;

    ASMJIT_ASSERT(y <= 0xFF);
    ASMJIT_ASSERT(z <= 0xFF);
    _packed = Utils::pack32_4x8(0, x, y, z);
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  union {
    uint8_t _regs[4];
    uint32_t _packed;
  };
};

// ============================================================================
// [asmjit::RARegMask]
// ============================================================================

//! Registers mask.
struct RARegMask {
  // --------------------------------------------------------------------------
  // [Reset]
  // --------------------------------------------------------------------------

  //! Reset all register masks to zero.
  ASMJIT_INLINE void reset() noexcept {
    for (uint32_t i = 0; i < Globals::kMaxVRegKinds; i++)
      _masks[i] = 0;
  }

  ASMJIT_INLINE void reset(uint32_t kind) noexcept {
    ASMJIT_ASSERT(kind < Globals::kMaxVRegKinds);
    _masks[kind] = 0;
  }

  // --------------------------------------------------------------------------
  // [IsEmpty / Has]
  // --------------------------------------------------------------------------

  //! Get whether all register masks are zero (empty).
  ASMJIT_INLINE bool isEmpty() const noexcept {
    uint32_t m = 0;
    for (uint32_t i = 0; i < Globals::kMaxVRegKinds; i++)
      m |= _masks[i];
    return m == 0;
  }

  ASMJIT_INLINE bool has(uint32_t kind, uint32_t mask = 0xFFFFFFFFU) const noexcept {
    ASMJIT_ASSERT(kind < Globals::kMaxVRegKinds);
    return (_masks[kind] & mask) != 0;
  }

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE uint32_t get(uint32_t kind) const noexcept {
    ASMJIT_ASSERT(kind < Globals::kMaxVRegKinds);
    return _masks[kind];
  }

  ASMJIT_INLINE void set(const RARegMask& other) noexcept {
    for (uint32_t i = 0; i < Globals::kMaxVRegKinds; i++)
      _masks[i] = other._masks[i];
  }

  ASMJIT_INLINE void set(uint32_t kind, uint32_t mask) noexcept {
    ASMJIT_ASSERT(kind < Globals::kMaxVRegKinds);
    _masks[kind] = mask;
  }

  ASMJIT_INLINE void and_(const RARegMask& other) noexcept {
    for (uint32_t i = 0; i < Globals::kMaxVRegKinds; i++)
      _masks[i] &= other._masks[i];
  }

  ASMJIT_INLINE void and_(uint32_t kind, uint32_t mask) noexcept {
    ASMJIT_ASSERT(kind < Globals::kMaxVRegKinds);
    _masks[kind] &= mask;
  }

  ASMJIT_INLINE void andNot(const RARegMask& other) noexcept {
    for (uint32_t i = 0; i < Globals::kMaxVRegKinds; i++)
      _masks[i] &= ~other._masks[i];
  }

  ASMJIT_INLINE void andNot(uint32_t kind, uint32_t mask) noexcept {
    ASMJIT_ASSERT(kind < Globals::kMaxVRegKinds);
    _masks[kind] &= ~mask;
  }

  ASMJIT_INLINE void or_(const RARegMask& other) noexcept {
    for (uint32_t i = 0; i < Globals::kMaxVRegKinds; i++)
      _masks[i] |= other._masks[i];
  }

  ASMJIT_INLINE void or_(uint32_t kind, uint32_t mask) noexcept {
    ASMJIT_ASSERT(kind < Globals::kMaxVRegKinds);
    _masks[kind] |= mask;
  }

  ASMJIT_INLINE void xor_(const RARegMask& other) noexcept {
    for (uint32_t i = 0; i < Globals::kMaxVRegKinds; i++)
      _masks[i] ^= other._masks[i];
  }

  ASMJIT_INLINE void xor_(uint32_t kind, uint32_t mask) noexcept {
    ASMJIT_ASSERT(kind < Globals::kMaxVRegKinds);
    _masks[kind] ^= mask;
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  uint32_t _masks[Globals::kMaxVRegKinds];
};

// ============================================================================
// [asmjit::LiveBits]
// ============================================================================

typedef ZoneBitVector LiveBits;

// ============================================================================
// [asmjit::LiveSpan]
// ============================================================================

class LiveSpan {
public:
  ASMJIT_INLINE LiveSpan() noexcept : a(0), b(0) {}
  ASMJIT_INLINE LiveSpan(const LiveSpan& other) noexcept : a(other.a), b(other.b) {}
  ASMJIT_INLINE LiveSpan(uint32_t a, uint32_t b) noexcept : a(a), b(b) {}

  uint32_t a, b;
};

// ============================================================================
// [asmjit::LiveRange]
// ============================================================================

class LiveRange {
public:
  ASMJIT_NONCOPYABLE(LiveRange)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  explicit ASMJIT_INLINE LiveRange(ZoneHeap* heap) noexcept
    : _spans(heap) {}

  // --------------------------------------------------------------------------
  // [Reset]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE bool isInitialized() const noexcept {
    return _spans.isInitialized();
  }

  ASMJIT_INLINE void reset(ZoneHeap* heap) noexcept {
    _spans.reset(heap);
  }

  // --------------------------------------------------------------------------
  // [Interface]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE bool isEmpty() const noexcept { return _spans.isEmpty(); }
  ASMJIT_INLINE size_t getLength() const noexcept { return _spans.getLength(); }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  ZoneVector<LiveSpan> _spans;
};

// ============================================================================
// [asmjit::RAStackSlot]
// ============================================================================

//! Stack slot.
struct RAStackSlot {
  RAStackSlot* next;                     //!< Next active cell.
  int32_t offset;                        //!< Cell offset, relative to base-offset.
  uint32_t size;                         //!< Cell size.
  uint32_t alignment;                    //!< Cell alignment.
};

// ============================================================================
// [asmjit::RAStackManager]
// ============================================================================

//! Stack management.
struct RAStackManager {
  enum Size {
    kSize1     = 0,
    kSize2     = 1,
    kSize4     = 2,
    kSize8     = 3,
    kSize16    = 4,
    kSize32    = 5,
    kSize64    = 6,
    kSizeStack = 7,
    kSizeCount = 8
  };

  // --------------------------------------------------------------------------
  // [Reset]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE void reset() noexcept { ::memset(this, 0, sizeof(*this)); }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  uint32_t _bytesUsed;                   //!< Count of bytes used.
  uint32_t _alignment;                   //!< Calculated alignment.
  uint32_t _usageCount[kSizeCount];      //!< Number of used cells by size.

  RAStackSlot* _homeList;                //!< Spill slots of `VirtReg`s.
  RAStackSlot* _stackList;               //!< Stack slots used by the function.
};

// ============================================================================
// [asmjit::RABlock]
// ============================================================================

class RABlock {
public:
  ASMJIT_NONCOPYABLE(RABlock)

  ASMJIT_ENUM(Flags) {
    kFlagIsConstructed    = 0x00000001U, //!< Block has been constructed from nodes.
    kFlagIsSinglePass     = 0x00000002U, //!< Executed only once (initialization code).
    kFlagHasLiveness      = 0x00000004U, //!< Used during liveness analysis.
    kFlagHasFixedRegs     = 0x00000010U, //!< Block contains fixed registers (precolored).
    kFlagHasFuncCalls     = 0x00000020U  //!< Block contains function calls.
  };

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE RABlock(ZoneHeap* heap, uint32_t blockId = 0) noexcept
    : _blockId(blockId),
      _flags(0),
      _first(nullptr),
      _last(nullptr),
      _weight(1),
      _povOrder(0xFFFFFFFFU),
      _regKindsUsed(0x0),
      _lastMark(0),
      _predecessors(heap),
      _successors(heap),
      _in(heap),
      _out(heap),
      _gen(heap),
      _kill(heap),
      _idom(nullptr) {}

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE uint32_t getBlockId() const noexcept { return _blockId; }
  ASMJIT_INLINE uint32_t getFlags() const noexcept { return _flags; }

  ASMJIT_INLINE bool hasFlag(uint32_t flag) const noexcept { return (_flags & flag) != 0; }
  ASMJIT_INLINE uint32_t addFlags(uint32_t flags) noexcept { return _flags |= flags; }

  ASMJIT_INLINE bool isConstructed() const noexcept { return hasFlag(kFlagIsConstructed); }
  ASMJIT_INLINE void makeConstructed(uint32_t regKindsUsed) noexcept {
    _flags |= kFlagIsConstructed;
    // Restrict `regKindsUsed` to register kinds that can have virtual registers.
    _regKindsUsed |= regKindsUsed & Utils::bits(Globals::kMaxVRegKinds);
  }

  ASMJIT_INLINE uint32_t getRegKindsUsed() const noexcept { return _regKindsUsed; }

  ASMJIT_INLINE bool isSinglePass() const noexcept { return hasFlag(kFlagIsSinglePass); }
  ASMJIT_INLINE bool isEntryBlock() const noexcept { return _predecessors.isEmpty(); }
  ASMJIT_INLINE bool isExitBlock() const noexcept { return _successors.isEmpty(); }

  ASMJIT_INLINE bool hasPredecessors() const noexcept { return !_predecessors.isEmpty(); }
  ASMJIT_INLINE bool hasSuccessors() const noexcept { return !_successors.isEmpty(); }

  ASMJIT_INLINE const RABlocks& getPredecessors() const noexcept { return _predecessors; }
  ASMJIT_INLINE const RABlocks& getSuccessors() const noexcept { return _successors; }

  ASMJIT_INLINE CBNode* getFirst() const noexcept { return _first; }
  ASMJIT_INLINE void setFirst(CBNode* node) noexcept { _first = node; }

  ASMJIT_INLINE CBNode* getLast() const noexcept { return _last; }
  ASMJIT_INLINE void setLast(CBNode* node) noexcept { _last = node; }

  ASMJIT_INLINE uint32_t getPovOrder() const noexcept { return _povOrder; }
  ASMJIT_INLINE uint64_t getLastMark() const noexcept { return _lastMark; }
  ASMJIT_INLINE void setLastMark(uint64_t mark) const noexcept { _lastMark = mark; }

  ASMJIT_INLINE bool hasIDom() const noexcept { return _idom != nullptr; }
  ASMJIT_INLINE RABlock* getIDom() noexcept { return _idom; }
  ASMJIT_INLINE const RABlock* getIDom() const noexcept { return _idom; }
  ASMJIT_INLINE void setIDom(RABlock* block) noexcept { _idom = block; }

  ASMJIT_INLINE Error resizeLiveBits(size_t size) noexcept {
    ASMJIT_PROPAGATE(_in.resize(size));
    ASMJIT_PROPAGATE(_out.resize(size));
    ASMJIT_PROPAGATE(_gen.resize(size));
    ASMJIT_PROPAGATE(_kill.resize(size));
    return kErrorOk;
  }

  // --------------------------------------------------------------------------
  // [Ops]
  // --------------------------------------------------------------------------

  //! Adds a successor to this block, and predecessor to `successor`, making
  //! connection on both sides.
  //!
  //! This API must be used to manage successors and predecessors, never manage
  //! it manually.
  Error appendSuccessor(RABlock* successor) noexcept;

  //! Similar to `appendSuccessor()`, but prepends it instead of appending it.
  //!
  //! This function is used to add a successor after a conditional jump
  //! destination has been added.
  Error prependSuccessor(RABlock* successor) noexcept;

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  uint32_t _blockId;                     //!< Block id (indexed from zero).
  uint32_t _flags;                       //!< Block flags, see \ref Flags.

  CBNode* _first;                        //!< First `CBNode` of this block (inclusive).
  CBNode* _last;                         //!< Last `CBNode` of this block (inclusive).

  uint32_t _weight;                      //!< Weight of this block (default 1).
  uint32_t _povOrder;                    //!< Post-order view order, used during computing.
  uint32_t _regKindsUsed;                //!< Mask of all register kinds used by the block.

  mutable uint64_t _lastMark;            //!< Last mark (used by block visitors).
  RABlock* _idom;                        //!< Immediate dominator of this block.

  RABlocks _predecessors;                //!< Block predecessors.
  RABlocks _successors;                  //!< Block successors.

  LiveBits _in;                          //!< Liveness in.
  LiveBits _out;                         //!< Liveness out.
  LiveBits _gen;                         //!< Liveness gen.
  LiveBits _kill;                        //!< Liveness kill.
};

// ============================================================================
// [asmjit::WorkReg]
// ============================================================================

class WorkReg {
public:
  ASMJIT_NONCOPYABLE(WorkReg)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE WorkReg(ZoneHeap* heap, VirtReg* vReg, uint32_t workId) noexcept
    : _workId(workId),
      _virtId(vReg->getId()),
      _kind(vReg->getKind()),
      _virtReg(vReg),
      _liveIn(heap),
      _liveOut(heap),
      _liveRange(heap),
      _refs(heap) {}

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE uint32_t getWorkId() const noexcept { return _workId; }
  ASMJIT_INLINE uint32_t getVirtId() const noexcept { return _virtId; }
  ASMJIT_INLINE uint32_t getKind() const noexcept { return _kind; }

  ASMJIT_INLINE VirtReg* getVirtReg() const noexcept { return _virtReg; }

  ASMJIT_INLINE LiveBits& getLiveIn() noexcept { return _liveIn; }
  ASMJIT_INLINE const LiveBits& getLiveIn() const noexcept { return _liveIn; }

  ASMJIT_INLINE LiveBits& getLiveOut() noexcept { return _liveOut; }
  ASMJIT_INLINE const LiveBits& getLiveOut() const noexcept { return _liveOut; }

  ASMJIT_INLINE LiveRange& getLiveRange() noexcept { return _liveRange; }
  ASMJIT_INLINE const LiveRange& getLiveRange() const noexcept { return _liveRange; }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  uint32_t _workId;                      //!< Work id, used during register allocation.
  uint32_t _virtId;                      //!< Virtual id as used by `VirtReg`.

  uint8_t _kind;                         //!< Register kind.

  VirtReg* _virtReg;                     //!< `VirtReg` associated with this `WorkReg`.

  LiveBits _liveIn;                      //!< Live-in bits, each bit per node-id.
  LiveBits _liveOut;                     //!< Live-out bits, each bit per node-id.
  LiveRange _liveRange;                  //!< Live range of the `VirtReg`.
  ZoneVector<CBNode*> _refs;             //!< All nodes that use this `VirtReg`.
};

// ============================================================================
// [asmjit::TiedReg]
// ============================================================================

//! Tied register (CodeCompiler).
//!
//! Tied register is used to describe one ore more register operands that share
//! the same virtual register. Tied register contains all the data that is
//! essential for register allocation.
struct TiedReg {
  //! Flags.
  ASMJIT_ENUM(Flags) {
    kRReg        = 0x00000001U,          //!< Register read.
    kWReg        = 0x00000002U,          //!< Register write.
    kXReg        = 0x00000003U,          //!< Register read-write.

    kRMem        = 0x00000004U,          //!< Can be replaced by memory read.
    kWMem        = 0x00000008U,          //!< Can be replaced by memory write.
    kXMem        = 0x0000000CU,          //!< Can be replaced by memory read-write.

    kRFunc       = 0x00000010U,          //!< Function argument passed in register.
    kWFunc       = 0x00000020U,          //!< Function return value passed into register.
    kXFunc       = 0x00000030U,          //!< Function argument and return value.

    kWExclusive  = 0x00000080U           //!< Has an exclusive write operand.
  };

  // --------------------------------------------------------------------------
  // [Init / Reset]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE void init(VirtReg* vReg, uint32_t flags, uint32_t allocableRegs, uint32_t rPhysId, uint32_t wPhysId) noexcept {
    this->vreg = vReg;
    this->flags = flags;
    this->allocableRegs = allocableRegs;
    this->refCount = 1;
    this->rPhysId = static_cast<uint8_t>(rPhysId);
    this->wPhysId = static_cast<uint8_t>(wPhysId);
    this->reserved = 0;
  }

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get allocation flags, see \ref Flags.
  ASMJIT_INLINE uint32_t getFlags() const noexcept { return flags; }

  ASMJIT_INLINE bool isReadOnly() const noexcept { return (flags & kXReg) == kRReg; }
  ASMJIT_INLINE bool isWriteOnly() const noexcept { return (flags & kXReg) == kWReg; }
  ASMJIT_INLINE bool isReadWrite() const noexcept { return (flags & kXReg) == kXReg; }

  //! Get whether the variable has to be allocated in a specific input register.
  ASMJIT_INLINE uint32_t hasRPhysId() const noexcept { return rPhysId != Globals::kInvalidRegId; }
  //! Get whether the variable has to be allocated in a specific output register.
  ASMJIT_INLINE uint32_t hasWPhysId() const noexcept { return wPhysId != Globals::kInvalidRegId; }

  //! Set the input register index.
  ASMJIT_INLINE void setRPhysId(uint32_t index) noexcept { rPhysId = static_cast<uint8_t>(index); }
  //! Set the output register index.
  ASMJIT_INLINE void setWPhysId(uint32_t index) noexcept { wPhysId = static_cast<uint8_t>(index); }

  // --------------------------------------------------------------------------
  // [Operator Overload]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE TiedReg& operator=(const TiedReg& other) noexcept {
    ::memcpy(this, &other, sizeof(TiedReg));
    return *this;
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! Pointer to the associated \ref VirtReg.
  VirtReg* vreg;

  //! Allocation flags.
  uint32_t flags;

  //! Allocable input registers.
  //!
  //! Optional input registers is a mask of all allocable registers for a given
  //! variable where we have to pick one of them. This mask is usually not used
  //! when _inRegs is set. If both masks are used then the register
  //! allocator tries first to find an intersection between these and allocates
  //! an extra slot if not found.
  uint32_t allocableRegs;

  union {
    struct {
      //! How many times the variable is referenced by the instruction / node.
      uint8_t refCount;
      //! Input register id or `Globals::kInvalidRegId` if it's not given.
      //!
      //! Even if the input register id is not given (i.e. it may by any
      //! register), register allocator should assign some id that will be
      //! used to persist a virtual register into this specific id. It's
      //! helpful in situations where one virtual register has to be allocated
      //! in multiple registers to determine the register which will be persistent.
      uint8_t rPhysId;
      //! Output register index or `Globals::kInvalidRegId` if it's not given.
      //!
      //! Typically `Globals::kInvalidRegId` if variable is only used on input.
      uint8_t wPhysId;
      //! \internal
      uint8_t reserved;
    };

    //! \internal
    //!
    //! Packed data #0.
    uint32_t packed;
  };
};

// ============================================================================
// [asmjit::RAData]
// ============================================================================

//! Register allocator's data associated with each \ref CBNode.
struct RAData {
  ASMJIT_INLINE RAData(uint32_t tiedTotal) noexcept {
    this->tiedTotal = tiedTotal;
    this->inRegs.reset();
    this->outRegs.reset();
    this->clobberedRegs.reset();
    this->tiedIndex.reset();
    this->tiedCount.reset();
  }

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get `TiedReg` array.
  ASMJIT_INLINE TiedReg* getTiedArray() const noexcept {
    return const_cast<TiedReg*>(tiedArray);
  }

  //! Get `TiedReg` array for a given register `kind`.
  ASMJIT_INLINE TiedReg* getTiedArrayByKind(uint32_t kind) const noexcept {
    return const_cast<TiedReg*>(tiedArray) + tiedIndex.get(kind);
  }

  //! Get `TiedReg` index for a given register `kind`.
  ASMJIT_INLINE uint32_t getTiedStart(uint32_t kind) const noexcept {
    return tiedIndex.get(kind);
  }

  //! Get count of all tied registers.
  ASMJIT_INLINE uint32_t getTiedCount() const noexcept {
    return tiedTotal;
  }

  //! Get count of tied registers of a given `kind`.
  ASMJIT_INLINE uint32_t getTiedCountByKind(uint32_t kind) const noexcept {
    return tiedCount.get(kind);
  }

  //! Get `TiedReg` at the specified `index`.
  ASMJIT_INLINE TiedReg* getTiedAt(uint32_t index) const noexcept {
    ASMJIT_ASSERT(index < tiedTotal);
    return getTiedArray() + index;
  }

  //! Get TiedReg at the specified index for a given register `kind`.
  ASMJIT_INLINE TiedReg* getTiedAtByKind(uint32_t kind, uint32_t index) const noexcept {
    ASMJIT_ASSERT(index < tiedCount._regs[kind]);
    return getTiedArrayByKind(kind) + index;
  }

  ASMJIT_INLINE void setTiedAt(uint32_t index, TiedReg& tied) noexcept {
    ASMJIT_ASSERT(index < tiedTotal);
    tiedArray[index] = tied;
  }

  // --------------------------------------------------------------------------
  // [Utils]
  // --------------------------------------------------------------------------

  //! Find TiedReg.
  ASMJIT_INLINE TiedReg* findTied(VirtReg* vReg) const noexcept {
    TiedReg* tiedArray = getTiedArray();
    uint32_t tiedCount = tiedTotal;

    for (uint32_t i = 0; i < tiedCount; i++)
      if (tiedArray[i].vreg == vReg)
        return &tiedArray[i];

    return nullptr;
  }

  //! Find TiedReg (by class).
  ASMJIT_INLINE TiedReg* findTiedByKind(uint32_t kind, VirtReg* vReg) const noexcept {
    TiedReg* tiedArray = getTiedArrayByKind(kind);
    uint32_t tiedCount = getTiedCountByKind(kind);

    for (uint32_t i = 0; i < tiedCount; i++)
      if (tiedArray[i].vreg == vReg)
        return &tiedArray[i];

    return nullptr;
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  uint32_t tiedTotal;                    //!< Total count of \ref TiedReg regs.

  //! Special registers on input.
  //!
  //! Special register(s) restricted to one or more physical register. If there
  //! is more than one special register it means that we have to duplicate the
  //! variable content to all of them (it means that the same variable was used
  //! by two or more operands). We forget about duplicates after the register
  //! allocation finishes and marks all duplicates as non-assigned.
  RARegMask inRegs;

  //! Special registers on output.
  //!
  //! Special register(s) used on output. Each variable can have only one
  //! special register on the output, 'RAData' contains all registers from
  //! all 'TiedReg's.
  RARegMask outRegs;

  //! Clobbered registers (by a function call).
  RARegMask clobberedRegs;

  //! Start indexes of `TiedReg`s per register kind.
  RARegCount tiedIndex;
  //! Count of variables per register kind.
  RARegCount tiedCount;

  //! Linked registers.
  TiedReg tiedArray[1];
};

// ============================================================================
// [asmjit::RAState]
// ============================================================================

//! Variables' state.
struct RAState {
  //! Cell.
  struct Cell {
    ASMJIT_INLINE void reset() noexcept { _state = 0; }

    ASMJIT_INLINE uint32_t getState() const noexcept { return _state; }
    ASMJIT_INLINE void setState(uint32_t state) noexcept { _state = static_cast<uint8_t>(state); }

    uint8_t _state;
  };

  // --------------------------------------------------------------------------
  // [Reset]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE void reset(size_t numCells) noexcept {
    ::memset(this, 0, sizeof(_allocatedRegs) + sizeof(_allocatedMask) + numCells * sizeof(Cell));
  }

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE VirtReg** getAllocatedRegs() noexcept { return _allocatedRegs; }
  ASMJIT_INLINE VirtReg* const* getAllocatedRegs() const noexcept { return _allocatedRegs; }

  ASMJIT_INLINE RARegMask& getAllocatedMask() noexcept { return _allocatedMask; }
  ASMJIT_INLINE const RARegMask& getAllocatedMask() const noexcept { return _allocatedMask; }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! Allocated registers array.
  VirtReg* _allocatedRegs[Globals::kMaxPhysRegs];

  //! Allocated registers mask.
  RARegMask _allocatedMask;

  //! Variables data, the length is stored in `X86RAPass`.
  Cell _cells[1];
};

// ============================================================================
// [asmjit::RAPass]
// ============================================================================

//! \internal
//!
//! Register allocation pass (abstract) used by \ref CodeCompiler.
class RAPass : public CCFuncPass {
public:
  ASMJIT_NONCOPYABLE(RAPass)
  typedef CCFuncPass Base;

  enum Limits {
    kMaxVRegKinds = Globals::kMaxVRegKinds
  };

  // Shortcuts...
  enum {
    kAnyReg = Globals::kInvalidRegId
  };

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  RAPass() noexcept;
  virtual ~RAPass() noexcept;

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get the associated `CodeCompiler`.
  ASMJIT_INLINE CodeCompiler* cc() const noexcept { return static_cast<CodeCompiler*>(_cb); }

  //! Get if the logging is enabled, in that case `getLogger()` returns a valid `Logger` instance.
  ASMJIT_INLINE bool hasLogger() const noexcept { return _logger != nullptr; }

  //! Get `Logger` instance or null.
  ASMJIT_INLINE Logger* getLogger() const noexcept { return _logger; }

  //! Get `Zone` passed to `runOnFunction()`.
  ASMJIT_INLINE Zone* getZone() const { return _heap.getZone(); }

  //! Get function.
  ASMJIT_INLINE CCFunc* getFunc() const noexcept { return _func; }
  //! Get stop node.
  ASMJIT_INLINE CBNode* getStop() const noexcept { return _stop; }

  //! Get extra block.
  ASMJIT_INLINE CBNode* getExtraBlock() const noexcept { return _extraBlock; }
  //! Set extra block.
  ASMJIT_INLINE void setExtraBlock(CBNode* node) noexcept { _extraBlock = node; }

  ASMJIT_INLINE RABlock* getEntryBlock() noexcept {
    ASMJIT_ASSERT(!_blocks.isEmpty());
    return _blocks[0];
  }
  ASMJIT_INLINE const RABlock* getEntryBlock() const noexcept {
    ASMJIT_ASSERT(!_blocks.isEmpty());
    return _blocks[0];
  }

  ASMJIT_INLINE uint64_t nextMark() const noexcept { return ++_lastMark; }

  // --------------------------------------------------------------------------
  // [Run]
  // --------------------------------------------------------------------------

  //! Run the register allocator for the given `func`.
  Error runOnFunction(Zone* zone, CCFunc* func) noexcept override;

  // --------------------------------------------------------------------------
  // [Init / Done]
  // --------------------------------------------------------------------------

  //! Called by `runOnFunction()` to initialize an architecture-specific data
  //! used by the register allocator. It initialize everything as it's called
  //! per function.
  virtual void onInit() noexcept = 0;

  //! Called after `compile()` to clean everything up, no matter if `compile()`
  //! succeeded or failed.
  virtual void onDone() noexcept = 0;

  // --------------------------------------------------------------------------
  // [Steps]
  // --------------------------------------------------------------------------

  //! STEP 1:
  //!
  //! Traverse the whole function and do the following:
  //!
  //!   1. Construct CFG (represented by RABlock) by populating `_blocks` and
  //!      `_exits`. Blocks describe the control flow of the function and contain
  //!      some additional information that is used by the register allocator.
  //!   2. Remove unreachable code immediately. This is not strictly necessary
  //!      for CodeCompiler itself as the register allocator cannot reach such
  //!      nodes, but keeping virtual registers would fail during emitting to
  //!      the Assembler.
  virtual Error constructCFG() noexcept = 0;

  //! STEP 2:
  //!
  //! Construct post-order-view (POV).
  Error constructPOV() noexcept;

  //! STEP 3:
  //!
  //! Construct a dominator-tree from CFG.
  //!
  //! Terminology:
  //!   - A node `X` dominates a node `Z` if any path from the entry point to
  //!     `Z` has to go through `X`.
  //!   - A node `Z` post-dominates a node `X` if any path from `X` to the end
  //!     of the graph has to go through `Z`.
  Error constructDOM() noexcept;

  //! STEP 4:
  //!
  //! Perform liveness analysis and construct live intervals.
  Error constructLiveness() noexcept;

  // --------------------------------------------------------------------------
  // [Work Registers]
  // --------------------------------------------------------------------------

  Error _addToWorkRegs(VirtReg* vReg) noexcept;

  //! Creates a `WorkReg` data for the given `vReg`. The function does nothing
  //! if `vReg` already contains link to `WorkReg`. Called by `constructCFG()`.
  ASMJIT_INLINE Error addToWorkRegs(VirtReg* vReg) noexcept {
    // Likely as one virtual register should be used more than once.
    if (ASMJIT_LIKELY(vReg->_workReg))
      return kErrorOk;
    return _addToWorkRegs(vReg);
  }

  // --------------------------------------------------------------------------
  // [Block Management]
  // --------------------------------------------------------------------------

  //! Creates a new `RABlock` and returns it.
  RABlock* newBlock(CBNode* initialNode = nullptr) noexcept;

  //! Tries to find a neighboring CBLabel (without going through code) that is
  //! already connected with `RABlock`. If no label is found then a new RABlock
  //! is created and assigned to all labels in backward direction.
  RABlock* newBlockOrMergeWith(CBLabel* cbLabel) noexcept;

  //! Returns `node` or some node after that is ideal for beginning a new block.
  //! This function is mostly used after a conditional or unconditional jump to
  //! select the successor node. In some cases the next node could be a label,
  //! which means it could have assigned the block already.
  CBNode* findSuccessorStartingAt(CBNode* node) noexcept;

  //! \internal
  bool _strictlyDominates(const RABlock* a, const RABlock* b) const noexcept;
  //! Get whether the block `a` dominates `b`
  //!
  //! This is a strict check, returns false if `a` == `b`.
  ASMJIT_INLINE bool strictlyDominates(const RABlock* a, const RABlock* b) const noexcept {
    if (a == b) return false;
    return _strictlyDominates(a, b);
  }
  //! Get whether the block `a` dominates `b`
  //!
  //! This is a non-strict check, returns true if `a` == `b`.
  ASMJIT_INLINE bool dominates(const RABlock* a, const RABlock* b) const noexcept {
    if (a == b) return true;
    return _strictlyDominates(a, b);
  }

  //! \internal
  const RABlock* _nearestCommonDominator(const RABlock* a, const RABlock* b) const noexcept;
  //! Get a nearest common dominator of `a` and `b`.
  ASMJIT_INLINE RABlock* nearestCommonDominator(RABlock* a, RABlock* b) const noexcept {
    return const_cast<RABlock*>(_nearestCommonDominator(a, b));
  }
  //! \overload
  ASMJIT_INLINE const RABlock* nearestCommonDominator(const RABlock* a, const RABlock* b) const noexcept {
    return _nearestCommonDominator(a, b);
  }

  // --------------------------------------------------------------------------
  // [Logging]
  // --------------------------------------------------------------------------

#if !defined(ASMJIT_DISABLE_LOGGING)
  Error _logBlockIds(const RABlocks& blocks) noexcept;

  ASMJIT_INLINE Error logSuccessors(const RABlock* block) noexcept {
    return hasLogger() ? _logBlockIds(block->getSuccessors()) : static_cast<Error>(kErrorOk);
  }
#else
  ASMJIT_INLINE Error logSuccessors(const RABlock* block) noexcept {
    return kErrorOk;
  }
#endif

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  ZoneHeap _heap;                        //!< ZoneHeap that uses zone passed to `runOnFunction()`.
  Logger* _logger;                       //!< Pass loggins is enabled and logger valid if non-null.

  CCFunc* _func;                         //!< Function being processed.
  CBNode* _stop;                         //!< Stop node.
  CBNode* _extraBlock;                   //!< Node that is used to insert extra code after the function body.

  RABlocks _blocks;                      //!< Blocks (first block is the entry, always exists).
  RABlocks _exits;                       //!< Function exit blocks (usually one, but can contain more).
  RABlocks _pov;                         //!< Post order view (POV) of all `_blocks`.
  WorkRegs _workRegs;                    //!< Work registers (referenced by the function).
  RAStackManager _stack;                 //!< Stack manager.

  RARegCount _archRegCount;              //!< Count of machine registers.
  RARegMask _allocableRegs;              //!< Allocable registers (global).
  RARegMask _clobberedRegs;              //!< Clobbered registers of all blocks.
  uint32_t _nodesCount;                  //!< Count of nodes, for allocating liveness bits.
  mutable uint64_t _lastMark;            //!< Mark counter for mutable block visiting.
};

// ============================================================================
// [asmjit::RATiedBuilder]
// ============================================================================

class RATiedBuilder {
public:
  ASMJIT_NONCOPYABLE(RATiedBuilder)

  enum { kAnyReg = Globals::kInvalidRegId };

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE RATiedBuilder(RAPass* pass) noexcept {
    reset(pass);
  }

  // --------------------------------------------------------------------------
  // [Reset / Done]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE void reset(RAPass* pass) noexcept {
    this->pass = pass;
    this->index.reset();
    this->count.reset();
    this->cur = tmp;
  }

  ASMJIT_INLINE void done() noexcept {
    index.indexFromRegCount(count);
  }

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE uint32_t getTotal() const noexcept {
    return static_cast<uint32_t>((size_t)(cur - tmp));
  }

  // --------------------------------------------------------------------------
  // [Add]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE Error add(VirtReg* vReg, uint32_t flags, uint32_t allocable, uint32_t rPhysId, uint32_t wPhysId) noexcept {
    TiedReg* tReg = vReg->getTiedReg();
    if (!tReg) {
      // Could happen when the builder is not reset properly after each instruction.
      ASMJIT_ASSERT(getTotal() < ASMJIT_ARRAY_SIZE(tmp));

      ASMJIT_PROPAGATE(pass->addToWorkRegs(vReg));
      tReg = cur++;
      tReg->init(vReg, flags, allocable, rPhysId, wPhysId);
      return kErrorOk;
    }
    else {
      // Already used by this node.
      ASMJIT_ASSERT(vReg->hasWorkReg());

      if (ASMJIT_UNLIKELY(wPhysId != kAnyReg)) {
        if (ASMJIT_UNLIKELY(tReg->wPhysId != kAnyReg))
          return DebugUtils::errored(kErrorOverlappedRegs);
        tReg->wPhysId = static_cast<uint8_t>(wPhysId);
      }

      tReg->refCount++;
      tReg->flags |= flags;
      tReg->allocableRegs &= allocable;
      return kErrorOk;
    }
  }

  // --------------------------------------------------------------------------
  // [Store]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE Error storeTo(CBNode* node) noexcept {
    uint32_t total = getTotal();
    size_t size = sizeof(RAData) - sizeof(TiedReg) + total * sizeof(TiedReg);
    RAData* raData = pass->getZone()->allocT<RAData>(size);

    if (ASMJIT_UNLIKELY(!raData))
      return kErrorNoHeapMemory;

    raData->tiedTotal = total;
    raData->inRegs.reset();
    raData->outRegs.reset();
    raData->clobberedRegs.reset();
    raData->tiedIndex = index;
    raData->tiedCount = count;
    ::memcpy(raData->tiedArray, tmp, total * sizeof(TiedReg));

    node->setPassData<RAData>(raData);
    return kErrorOk;
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  RAPass* pass;

  RARegCount index;                      //!< Index of tied registers per kind.
  RARegCount count;                      //!< Count of tied registers per kind.

  TiedReg* cur;                          //!< Current tied register.
  TiedReg tmp[80];                       //!< Array of tied registers (temporary).
};

// ============================================================================
// [asmjit::RACFGBuilder]
// ============================================================================

template<typename This>
class RACFGBuilder {
public:
  ASMJIT_INLINE RACFGBuilder(RAPass* pass) noexcept
    : _pass(pass) {}

  ASMJIT_INLINE Error run() noexcept {
    ASMJIT_RA_LOG_INIT(_pass->getLogger());
    ASMJIT_RA_LOG_FORMAT("[RA::ConstructCFG]\n");

    CodeCompiler* cc = _pass->cc();
    CCFunc* func = _pass->getFunc();
    CBNode* node = func;

    // Create the first (entry) block.
    RABlock* currentBlock = _pass->newBlock(node);
    if (ASMJIT_UNLIKELY(!currentBlock))
      return DebugUtils::errored(kErrorNoHeapMemory);

    bool hasCode = false;
    size_t blockIndex = 0;
    uint32_t position = 0;
    uint32_t kindsUsed = 0;

#if !defined(ASMJIT_DISABLE_LOGGING)
    StringBuilderTmp<256> sb;
    RABlock* lastPrintedBlock = nullptr;

    if (logger) {
      lastPrintedBlock = currentBlock;
      logger->logf("{Block #%u}\n", lastPrintedBlock->getBlockId());
    }
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
              currentBlock = _pass->newBlock(node);
              if (ASMJIT_UNLIKELY(!currentBlock))
                return DebugUtils::errored(kErrorNoHeapMemory);

              node->as<CBLabel>()->setBlock(currentBlock);
              hasCode = false;
              kindsUsed = 0;
            }
          }
          else {
            // Label makes the current block constructed. There is a chance that the
            // Label is not used, but we don't know that at this point. Later, when
            // we have enough information we will be able to merge continuous blocks
            // into a single one if it's beneficial.
            currentBlock->setLast(node->getPrev());
            currentBlock->makeConstructed(kindsUsed);

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
                _pass->logSuccessors(currentBlock);

                currentBlock = successor;
                hasCode = false;
                kindsUsed = 0;
              }
            }
            else {
              // First time we see this label.
              if (hasCode) {
                // Cannot continue the current block if it already contains some
                // code. We need to create a new block and make it a successor.
                currentBlock->setLast(node->getPrev());
                currentBlock->makeConstructed(kindsUsed);

                RABlock* successor = _pass->newBlock(node);
                if (ASMJIT_UNLIKELY(!successor))
                  return DebugUtils::errored(kErrorNoHeapMemory);

                ASMJIT_PROPAGATE(currentBlock->appendSuccessor(successor));
                _pass->logSuccessors(currentBlock);

                currentBlock = successor;
                hasCode = false;
                kindsUsed = 0;
              }

              node->as<CBLabel>()->setBlock(currentBlock);
            }
          }
#if !defined(ASMJIT_DISABLE_LOGGING)
          if (logger) {
            if (lastPrintedBlock != currentBlock) {
              lastPrintedBlock = currentBlock;
              logger->logf("{Block #%u}\n", lastPrintedBlock->getBlockId());
            }

            sb.clear();
            Logging::formatNode(sb, 0, cc, node);
            logger->logf("  %s\n", sb.getData());
          }
#endif // !ASMJIT_DISABLE_LOGGING
        }
        else {
#if !defined(ASMJIT_DISABLE_LOGGING)
          if (logger) {
            sb.clear();
            Logging::formatNode(sb, 0, cc, node);
            logger->logf("  %s\n", sb.getData());
          }
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
              // Handle `CBInst`, `CCFuncCall`, and `CCFuncRet`. All of
              // these share the `CBInst` interface and contain operands.
              hasCode = true;

              CBInst* inst = node->as<CBInst>();
              uint32_t jumpType = AnyInst::kJumpTypeNone;

              static_cast<This*>(this)->onInst(inst, jumpType, kindsUsed);

              // Support for conditional and unconditional jumps.
              if (jumpType == AnyInst::kJumpTypeDirect || jumpType == AnyInst::kJumpTypeConditional) {
                // Jmp/Jcc/Call/Loop/etc...
                uint32_t opCount = inst->getOpCount();
                const Operand* opArray = inst->getOpArray();

                // The last operand must be label (this supports also instructions
                // like jecx in explicit form).
                if (opCount == 0 || !opArray[opCount - 1].isLabel())
                  return DebugUtils::errored(kErrorInvalidState);

                CBLabel* cbLabel;
                ASMJIT_PROPAGATE(cc->getCBLabel(&cbLabel, opArray[opCount - 1].as<Label>()));

                RABlock* jumpSuccessor = _pass->newBlockOrMergeWith(cbLabel);
                if (ASMJIT_UNLIKELY(!jumpSuccessor))
                  return DebugUtils::errored(kErrorNoHeapMemory);

                currentBlock->setLast(node);
                currentBlock->makeConstructed(kindsUsed);
                ASMJIT_PROPAGATE(currentBlock->appendSuccessor(jumpSuccessor));

                if (jumpType == AnyInst::kJumpTypeDirect) {
                  // Unconditional jump makes the code after the jump unreachable,
                  // which will be removed instantly during the CFG construction;
                  // as we cannot allocate registers for instructions that are not
                  // part of any block. Of course we can leave these instructions
                  // as they are, however, that would only postpone the problem as
                  // assemblers can't encode instructions that use virtual registers.
                  _pass->logSuccessors(currentBlock);
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
                      flowSuccessor = _pass->newBlock(node);
                      if (ASMJIT_UNLIKELY(!flowSuccessor))
                        return DebugUtils::errored(kErrorNoHeapMemory);
                      node->as<CBLabel>()->setBlock(flowSuccessor);
                    }
                  }
                  else {
                    flowSuccessor = _pass->newBlock(node);
                    if (ASMJIT_UNLIKELY(!flowSuccessor))
                      return DebugUtils::errored(kErrorNoHeapMemory);
                  }

                  ASMJIT_PROPAGATE(currentBlock->prependSuccessor(flowSuccessor));
                  _pass->logSuccessors(currentBlock);

                  currentBlock = flowSuccessor;
                  hasCode = false;
                  kindsUsed = 0;

                  if (currentBlock->isConstructed())
                    break;

                  lastPrintedBlock = currentBlock;
                  ASMJIT_RA_LOG_FORMAT("{Block #%u}\n", lastPrintedBlock->getBlockId());
                  continue;
                }
              }
            }
          }
          else if (node->getType() == CBNode::kNodeSentinel) {
            // Sentinel could be anything, however, if this is the end of function
            // marker it's the function's exit. This means this node must be added
            // to `_exits`.
            if (node == func->getEnd()) {
              // Only add the current block to exists if it's reachable.
              if (currentBlock) {
                currentBlock->setLast(node);
                currentBlock->makeConstructed(kindsUsed);
                ASMJIT_PROPAGATE(_pass->_exits.append(currentBlock));
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
        if (++blockIndex >= _pass->_blocks.getLength()) {
          _pass->_nodesCount = position;
          return kErrorOk;
        }

        currentBlock = _pass->_blocks[blockIndex];
      } while (currentBlock->isConstructed());

      node = currentBlock->getLast();
      hasCode = false;
      kindsUsed = 0;
    }
  }

  RAPass* _pass;
};

//! \}

} // asmjit namespace

// [Api-End]
#include "../asmjit_apiend.h"

// [Guard]
#endif // !ASMJIT_DISABLE_COMPILER
#endif // _ASMJIT_BASE_RAPASS_P_H
