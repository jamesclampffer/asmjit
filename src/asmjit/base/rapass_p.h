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
//
// [Dependencies]
#include "../base/codecompiler.h"
#include "../base/zone.h"

// [Api-Begin]
#include "../asmjit_apibegin.h"

namespace asmjit {

//! \addtogroup asmjit_base
//! \{

// ============================================================================
// [Forward Declarations]
// ============================================================================

class RABlock;
class RALocal;

typedef ZoneVector<RABlock*> RABlocks;
typedef ZoneVector<RALocal*> RALocals;

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
// [asmjit::RALiveRange]
// ============================================================================

class RALiveRange {
public:
  ASMJIT_NONCOPYABLE(RALiveRange)

  class Segment {
  public:
    ASMJIT_INLINE Segment() noexcept : a(0), b(0) {}
    ASMJIT_INLINE Segment(const Segment& other) noexcept : a(other.a), b(other.b) {}
    ASMJIT_INLINE Segment(uint32_t a, uint32_t b) noexcept : a(a), b(b) {}

    uint32_t a, b;
  };

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  explicit ASMJIT_INLINE RALiveRange(ZoneHeap* heap) noexcept
    : _segments(heap) {}

  // --------------------------------------------------------------------------
  // [Reset]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE void reset(ZoneHeap* heap) noexcept {
    _segments.reset(heap);
  }

  // --------------------------------------------------------------------------
  // [Interface]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE bool isEmpty() const noexcept { return _segments.isEmpty(); }
  ASMJIT_INLINE size_t getLength() const noexcept { return _segments.getLength(); }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  ZoneVector<Segment> _segments;
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
    kFlagHasFixedRegs     = 0x00000010U, //!< Block contains fixed registers (precolored).
    kFlagHasFuncCalls     = 0x00000020U  //!< Block contains function call(s).
  };

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE RABlock(ZoneHeap* heap, uint32_t blockId = 0) noexcept
    : _blockId(blockId),
      _flags(0),
      _weight(1),
      _povOrder(0xFFFFFFFFU),
      _lastMark(0),
      _predecessors(heap),
      _successors(heap),
      _first(nullptr),
      _last(nullptr),
      _idom(nullptr) {}

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE uint32_t getBlockId() const noexcept { return _blockId; }
  ASMJIT_INLINE uint32_t getFlags() const noexcept { return _flags; }

  ASMJIT_INLINE bool hasFlag(uint32_t flag) const noexcept { return (_flags & flag) != 0; }
  ASMJIT_INLINE uint32_t addFlags(uint32_t flags) noexcept { return _flags |= flags; }

  ASMJIT_INLINE bool isConstructed() const noexcept { return hasFlag(kFlagIsConstructed); }
  ASMJIT_INLINE void makeConstructed() noexcept { addFlags(kFlagIsConstructed); }

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
  uint32_t _weight;                      //!< Weight of this block (default 1).

  uint32_t _povOrder;                    //!< POV (post-order view) order.
  mutable uint64_t _lastMark;            //!< Last mark (used by block visitors).

  RABlocks _predecessors;                //!< Blocks predecessors.
  RABlocks _successors;                  //!< Block successors.

  CBNode* _first;                        //!< First `CBNode` of this block (inclusive).
  CBNode* _last;                         //!< Last `CBNode` of this block (inclusive).

  RABlock* _idom;                        //!< Immediate dominator.
};

// ============================================================================
// [asmjit::RALocal]
// ============================================================================

class RALocal {
public:
  ASMJIT_NONCOPYABLE(RALocal)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE RALocal(ZoneHeap* heap, VirtReg* vReg, uint32_t localId) noexcept
    : _localId(localId),
      _vReg(vReg),
      _liveRange(heap),
      _refs(heap) {}

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE uint32_t getLocalId() const noexcept { return _localId; }
  ASMJIT_INLINE VirtReg* getVirtReg() const noexcept { return _vReg; }

  ASMJIT_INLINE RALiveRange& getLiveRange() noexcept { return _liveRange; }
  ASMJIT_INLINE const RALiveRange& getLiveRange() const noexcept { return _liveRange; }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  uint32_t _localId;                     //!< Local id used during register allocation.
  VirtReg* _vReg;                        //!< VirtReg associated with this RALocal.
  RALiveRange _liveRange;                //!< Live range of the VirtReg.
  ZoneVector<CBNode*> _refs;             //!< All nodes this VirtReg is used by.
};

// ============================================================================
// [asmjit::RATiedReg]
// ============================================================================

//! Tied register (CodeCompiler).
//!
//! Tied register is used to describe one ore more register operands that share
//! the same virtual register. Tied register contains all the data that is
//! essential for register allocation.
struct RATiedReg {
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

  ASMJIT_INLINE void init(VirtReg* vreg, uint32_t flags, uint32_t allocableRegs, uint32_t rPhysId, uint32_t wPhysId) noexcept {
    this->vreg = vreg;
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

  //! Get whether the variable has to be allocated in a specific input register.
  ASMJIT_INLINE uint32_t hasRPhysId() const noexcept { return rPhysId != Globals::kInvalidReg; }
  //! Get whether the variable has to be allocated in a specific output register.
  ASMJIT_INLINE uint32_t hasWPhysId() const noexcept { return wPhysId != Globals::kInvalidReg; }

  //! Set the input register index.
  ASMJIT_INLINE void setRPhysId(uint32_t index) noexcept { rPhysId = static_cast<uint8_t>(index); }
  //! Set the output register index.
  ASMJIT_INLINE void setWPhysId(uint32_t index) noexcept { wPhysId = static_cast<uint8_t>(index); }

  // --------------------------------------------------------------------------
  // [Operator Overload]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE RATiedReg& operator=(const RATiedReg& other) noexcept {
    ::memcpy(this, &other, sizeof(RATiedReg));
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
      //! Input register index or `Globals::kInvalidReg` if it's not given.
      //!
      //! Even if the input register index is not given (i.e. it may by any
      //! register), register allocator should assign an index that will be
      //! used to persist a variable into this specific index. It's helpful
      //! in situations where one variable has to be allocated in multiple
      //! registers to determine the register which will be persistent.
      uint8_t rPhysId;
      //! Output register index or `Globals::kInvalidReg` if it's not given.
      //!
      //! Typically `Globals::kInvalidReg` if variable is only used on input.
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

  //! Get RATiedReg array.
  ASMJIT_INLINE RATiedReg* getTiedArray() const noexcept {
    return const_cast<RATiedReg*>(tiedArray);
  }

  //! Get RATiedReg array for a given register `kind`.
  ASMJIT_INLINE RATiedReg* getTiedArrayByKind(uint32_t kind) const noexcept {
    return const_cast<RATiedReg*>(tiedArray) + tiedIndex.get(kind);
  }

  //! Get RATiedReg index for a given register `kind`.
  ASMJIT_INLINE uint32_t getTiedStart(uint32_t kind) const noexcept {
    return tiedIndex.get(kind);
  }

  //! Get RATiedReg count for a given register `kind`.
  ASMJIT_INLINE uint32_t getTiedCountByKind(uint32_t kind) const noexcept {
    return tiedCount.get(kind);
  }

  //! Get RATiedReg at the specified `index`.
  ASMJIT_INLINE RATiedReg* getTiedAt(uint32_t index) const noexcept {
    ASMJIT_ASSERT(index < tiedTotal);
    return getTiedArray() + index;
  }

  //! Get RATiedReg at the specified index for a given register `kind`.
  ASMJIT_INLINE RATiedReg* getTiedAtByKind(uint32_t kind, uint32_t index) const noexcept {
    ASMJIT_ASSERT(index < tiedCount._regs[kind]);
    return getTiedArrayByKind(kind) + index;
  }

  ASMJIT_INLINE void setTiedAt(uint32_t index, RATiedReg& tied) noexcept {
    ASMJIT_ASSERT(index < tiedTotal);
    tiedArray[index] = tied;
  }

  // --------------------------------------------------------------------------
  // [Utils]
  // --------------------------------------------------------------------------

  //! Find RATiedReg.
  ASMJIT_INLINE RATiedReg* findTied(VirtReg* vreg) const noexcept {
    RATiedReg* tiedArray = getTiedArray();
    uint32_t tiedCount = tiedTotal;

    for (uint32_t i = 0; i < tiedCount; i++)
      if (tiedArray[i].vreg == vreg)
        return &tiedArray[i];

    return nullptr;
  }

  //! Find RATiedReg (by class).
  ASMJIT_INLINE RATiedReg* findTiedByKind(uint32_t kind, VirtReg* vreg) const noexcept {
    RATiedReg* tiedArray = getTiedArrayByKind(kind);
    uint32_t tiedCount = getTiedCountByKind(kind);

    for (uint32_t i = 0; i < tiedCount; i++)
      if (tiedArray[i].vreg == vreg)
        return &tiedArray[i];

    return nullptr;
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  uint32_t tiedTotal;                    //!< Total count of \ref RATiedReg regs.

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
  //! all 'RATiedReg's.
  RARegMask outRegs;

  //! Clobbered registers (by a function call).
  RARegMask clobberedRegs;

  //! Start indexes of `RATiedReg`s per register kind.
  RARegCount tiedIndex;
  //! Count of variables per register kind.
  RARegCount tiedCount;

  //! Linked registers.
  RATiedReg tiedArray[1];
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
class RAPass : public CBPass {
public:
  ASMJIT_NONCOPYABLE(RAPass)
  typedef CBPass Base;

  enum Limits {
    kMaxVRegKinds = Globals::kMaxVRegKinds
  };

  // Shortcuts...
  enum {
    kAnyReg = Globals::kInvalidReg
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
  // [Process]
  // --------------------------------------------------------------------------

  //! Executes `compile()` for each function it finds.
  virtual Error process(Zone* zone) noexcept override;

  //! Run the register allocator for the given `func`.
  virtual Error compile(CCFunc* func) noexcept;

  // --------------------------------------------------------------------------
  // [Prepare / Cleanup]
  // --------------------------------------------------------------------------

  //! Called by `compile()` to prepare the register allocator to process the
  //! given function. It should reset and set-up everything (i.e. there should
  //! be no garbage from the previous compilation).
  virtual Error prepare(CCFunc* func) noexcept;

  //! Called after `compile()` to clean everything up, no matter if `compile()`
  //! succeeded or failed.
  virtual void cleanup() noexcept;

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
  //! Construct a dominator-tree from CFG.
  //!
  //! Terminology:
  //!   - A node `X` dominates a node `Z` if any path from the entry point to
  //!     `Z` has to go through `X`.
  //!   - A node `Z` post-dominates a node `X` if any path from `X` to the end
  //!     of the graph has to go through `Z`.
  Error constructDomTree() noexcept;

  // --------------------------------------------------------------------------
  // [Local Registers]
  // --------------------------------------------------------------------------

  Error _makeLocal(VirtReg* vReg) noexcept;

  //! Creates a `RALocal` data for the given `vReg`. The function does
  //! nothing if `vReg` already contains link to `RALocal`. Called by
  //! `constructCFG()`.
  ASMJIT_INLINE Error makeLocal(VirtReg* vReg) noexcept {
    // Likely as one virtual register should be used more than once.
    return ASMJIT_LIKELY(vReg->_local) ? static_cast<uint32_t>(kErrorOk) : _makeLocal(vReg);
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
  // [Bits]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE RABits* newBits(uint32_t len) noexcept {
    return static_cast<RABits*>(
      _zone->allocZeroed(static_cast<size_t>(len) * RABits::kEntitySize));
  }

  ASMJIT_INLINE RABits* dupBits(const RABits* src, uint32_t len) noexcept {
    return static_cast<RABits*>(
      _zone->dup(src, static_cast<size_t>(len) * RABits::kEntitySize));
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  Zone* _zone;                           //!< Zone passed to `process()`.
  ZoneHeap _heap;                        //!< ZoneHeap that uses `_zone`.

  RARegCount _archRegCount;              //!< Count of machine registers.
  RARegMask _allocableRegs;              //!< Allocable registers (global).
  RARegMask _clobberedRegs;              //!< Clobbered registers of all blocks.

  CCFunc* _func;                         //!< Function being processed.
  CBNode* _stop;                         //!< Stop node.
  CBNode* _extraBlock;                   //!< Node that is used to insert extra code after the function body.

  RABlocks _blocks;                      //!< Blocks (first block is the entry, always exists).
  RABlocks _exits;                       //!< Function exit blocks (usually one, but can contain more).
  mutable uint64_t _lastMark;            //!< Mark counter for mutable block visiting.

  RALocals _localRegs[kMaxVRegKinds];    //!< Local registers (referenced by the function).
  RAStackManager _stack;                 //!< Stack manager.
};

// ============================================================================
// [asmjit::RATiedBuilder]
// ============================================================================

class RATiedBuilder {
public:
  ASMJIT_NONCOPYABLE(RATiedBuilder)

  enum { kAnyReg = Globals::kInvalidReg };

  ASMJIT_INLINE RATiedBuilder(RAPass* pass) noexcept {
    reset(pass);
  }

  ASMJIT_INLINE void reset(RAPass* pass) noexcept {
    this->pass = pass;
    this->index.reset();
    this->count.reset();
    this->cur = tmp;
  }

  ASMJIT_INLINE void done() noexcept {
    index.indexFromRegCount(count);
  }

  ASMJIT_INLINE uint32_t getTotal() const noexcept {
    return static_cast<uint32_t>((size_t)(cur - tmp));
  }

  ASMJIT_INLINE Error add(VirtReg* vReg, uint32_t flags, uint32_t allocable, uint32_t rPhysId, uint32_t wPhysId) noexcept {
    RATiedReg* tied = vReg->getTied();
    if (!tied) {
      // Happens when the builder is not reset after each instruction.
      ASMJIT_ASSERT(getTotal() < ASMJIT_ARRAY_SIZE(tmp));

      ASMJIT_PROPAGATE(pass->makeLocal(vReg));
      tied = cur++;
      tied->init(vReg, flags, allocable, rPhysId, wPhysId);
      return kErrorOk;
    }
    else {
      // Already used by this node.
      ASMJIT_ASSERT(vReg->hasLocal());

      if (ASMJIT_UNLIKELY(wPhysId != kAnyReg)) {
        if (ASMJIT_UNLIKELY(tied->wPhysId != kAnyReg))
          return DebugUtils::errored(kErrorOverlappedRegs);
        tied->wPhysId = static_cast<uint8_t>(wPhysId);
      }

      tied->refCount++;
      tied->flags |= flags;
      tied->allocableRegs &= allocable;
      return kErrorOk;
    }
  }

  RAPass* pass;

  RARegCount index;                      //!< Index of tied registers per kind.
  RARegCount count;                      //!< Count of tied registers per kind.

  RATiedReg* cur;                        //!< Current tied register.
  RATiedReg tmp[80];                     //!< Array of tied registers (temporary).
};

// ============================================================================
// [asmjit::RACFGBuilder]
// ============================================================================

template<typename This>
class RACFGBuilder {
public:
};

//! \}

} // asmjit namespace

// [Api-End]
#include "../asmjit_apiend.h"

// [Guard]
#endif // !ASMJIT_DISABLE_COMPILER
#endif // _ASMJIT_BASE_RAPASS_P_H
