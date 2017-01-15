// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_X86_X86GLOBALS_H
#define _ASMJIT_X86_X86GLOBALS_H

// [Dependencies]
#include "../base/globals.h"

// [Api-Begin]
#include "../asmjit_apibegin.h"

namespace asmjit {

//! \addtogroup asmjit_x86
//! \{

// ============================================================================
// [asmjit::x86regs::]
// ============================================================================

//! X86 registers.
namespace x86regs {}

// ============================================================================
// [asmjit::x86defs::]
// ============================================================================

//! X86 definitions.
namespace x86defs {

// ============================================================================
// [asmjit::x86defs::EFlags]
// ============================================================================

//! EFLAGs bits (AsmJit specific).
//!
//! Each instruction stored in AsmJit database contains flags that instruction
//! uses (reads) and flags that instruction modifies (writes). This is used by
//! instruction reordering, but can be used by third parties as it's part of
//! AsmJit API.
//!
//! NOTE: Flags defined here don't correspond to real flags used by X86/X64
//! architecture, defined in Intel's Manual Section `3.4.3 - EFLAGS Register`.
//!
//! NOTE: Flags are designed to fit in an 8-bit integer.
ASMJIT_ENUM(EFlags) {
  // --------------------------------------------------------------------------
  // TODO: This is wrong.
  // generate-x86.js relies on the values of these masks, the tool has to be
  // changed as you plan to modify `X86EFlags`.
  // --------------------------------------------------------------------------

  kEFlagO               = 0x01U,         //!< Overflow flag (OF).
  kEFlagS               = 0x02U,         //!< Sign flag (SF).
  kEFlagZ               = 0x04U,         //!< Zero flag (ZF).
  kEFlagA               = 0x08U,         //!< Adjust flag (AF).
  kEFlagP               = 0x10U,         //!< Parity flag (PF).
  kEFlagC               = 0x20U,         //!< Carry flag (CF).
  kEFlagD               = 0x40U,         //!< Direction flag (DF).
  kEFlagX               = 0x80U          //!< Any other flag that AsmJit doesn't use.
};

// ============================================================================
// [asmjit::x86defs::FpSW]
// ============================================================================

//! FPU status word.
ASMJIT_ENUM(FpuSW) {
  kFpuSW_Invalid        = 0x0001U,
  kFpuSW_Denormalized   = 0x0002U,
  kFpuSW_DivByZero      = 0x0004U,
  kFpuSW_Overflow       = 0x0008U,
  kFpuSW_Underflow      = 0x0010U,
  kFpuSW_Precision      = 0x0020U,
  kFpuSW_StackFault     = 0x0040U,
  kFpuSW_Interrupt      = 0x0080U,
  kFpuSW_C0             = 0x0100U,
  kFpuSW_C1             = 0x0200U,
  kFpuSW_C2             = 0x0400U,
  kFpuSW_Top            = 0x3800U,
  kFpuSW_C3             = 0x4000U,
  kFpuSW_Busy           = 0x8000U
};

// ============================================================================
// [asmjit::x86defs::FpCW]
// ============================================================================

//! FPU control word.
ASMJIT_ENUM(FpuCW) {
  // Bits 0-5.
  kFpuCW_EM_Mask        = 0x003FU,
  kFpuCW_EM_Invalid     = 0x0001U,
  kFpuCW_EM_Denormal    = 0x0002U,
  kFpuCW_EM_DivByZero   = 0x0004U,
  kFpuCW_EM_Overflow    = 0x0008U,
  kFpuCW_EM_Underflow   = 0x0010U,
  kFpuCW_EM_Inexact     = 0x0020U,

  // Bits 8-9.
  kFpuCW_PC_Mask        = 0x0300U,
  kFpuCW_PC_Float       = 0x0000U,
  kFpuCW_PC_Reserved    = 0x0100U,
  kFpuCW_PC_Double      = 0x0200U,
  kFpuCW_PC_Extended    = 0x0300U,

  // Bits 10-11.
  kFpuCW_RC_Mask        = 0x0C00U,
  kFpuCW_RC_Nearest     = 0x0000U,
  kFpuCW_RC_Down        = 0x0400U,
  kFpuCW_RC_Up          = 0x0800U,
  kFpuCW_RC_Truncate    = 0x0C00U,

  // Bit 12.
  kFpuCW_IC_Mask        = 0x1000U,
  kFpuCW_IC_Projective  = 0x0000U,
  kFpuCW_IC_Affine      = 0x1000U
};

// ============================================================================
// [asmjit::x86defs::Cond]
// ============================================================================

//! Condition codes.
ASMJIT_ENUM(Cond) {
  kCondO                = 0x00U,         //!<                 OF==1
  kCondNO               = 0x01U,         //!<                 OF==0
  kCondB                = 0x02U,         //!< CF==1                  (unsigned < )
  kCondC                = 0x02U,         //!< CF==1
  kCondNAE              = 0x02U,         //!< CF==1                  (unsigned < )
  kCondAE               = 0x03U,         //!< CF==0                  (unsigned >=)
  kCondNB               = 0x03U,         //!< CF==0                  (unsigned >=)
  kCondNC               = 0x03U,         //!< CF==0
  kCondE                = 0x04U,         //!<         ZF==1          (any_sign ==)
  kCondZ                = 0x04U,         //!<         ZF==1          (any_sign ==)
  kCondNE               = 0x05U,         //!<         ZF==0          (any_sign !=)
  kCondNZ               = 0x05U,         //!<         ZF==0          (any_sign !=)
  kCondBE               = 0x06U,         //!< CF==1 | ZF==1          (unsigned <=)
  kCondNA               = 0x06U,         //!< CF==1 | ZF==1          (unsigned <=)
  kCondA                = 0x07U,         //!< CF==0 & ZF==0          (unsigned > )
  kCondNBE              = 0x07U,         //!< CF==0 & ZF==0          (unsigned > )
  kCondS                = 0x08U,         //!<                 SF==1  (is negative)
  kCondNS               = 0x09U,         //!<                 SF==0  (is positive or zero)
  kCondP                = 0x0AU,         //!< PF==1
  kCondPE               = 0x0AU,         //!< PF==1
  kCondPO               = 0x0BU,         //!< PF==0
  kCondNP               = 0x0BU,         //!< PF==0
  kCondL                = 0x0CU,         //!<                 SF!=OF (signed   < )
  kCondNGE              = 0x0CU,         //!<                 SF!=OF (signed   < )
  kCondGE               = 0x0DU,         //!<                 SF==OF (signed   >=)
  kCondNL               = 0x0DU,         //!<                 SF==OF (signed   >=)
  kCondLE               = 0x0EU,         //!<         ZF==1 | SF!=OF (signed   <=)
  kCondNG               = 0x0EU,         //!<         ZF==1 | SF!=OF (signed   <=)
  kCondG                = 0x0FU,         //!<         ZF==0 & SF==OF (signed   > )
  kCondNLE              = 0x0FU,         //!<         ZF==0 & SF==OF (signed   > )
  kCondCount            = 0x10U,

  // Simplified condition codes.
  kCondSign             = kCondS,        //!< Sign.
  kCondNotSign          = kCondNS,       //!< Not Sign.

  kCondOverflow         = kCondO,        //!< Signed overflow.
  kCondNotOverflow      = kCondNO,       //!< Not signed overflow.

  kCondEqual            = kCondE,        //!< Equal      `a == b`.
  kCondNotEqual         = kCondNE,       //!< Not Equal  `a != b`.

  kCondSignedLT         = kCondL,        //!< Signed     `a <  b`.
  kCondSignedLE         = kCondLE,       //!< Signed     `a <= b`.
  kCondSignedGT         = kCondG,        //!< Signed     `a >  b`.
  kCondSignedGE         = kCondGE,       //!< Signed     `a >= b`.

  kCondUnsignedLT       = kCondB,        //!< Unsigned   `a <  b`.
  kCondUnsignedLE       = kCondBE,       //!< Unsigned   `a <= b`.
  kCondUnsignedGT       = kCondA,        //!< Unsigned   `a >  b`.
  kCondUnsignedGE       = kCondAE,       //!< Unsigned   `a >= b`.

  kCondZero             = kCondZ,
  kCondNotZero          = kCondNZ,

  kCondNegative         = kCondS,
  kCondPositive         = kCondNS,

  kCondParityEven       = kCondP,
  kCondParityOdd        = kCondPO
};

// ============================================================================
// [asmjit::x86defs::CmpPredicate]
// ============================================================================

//! A predicate used by CMP[PD|PS|SD|SS] instructions.
ASMJIT_ENUM(CmpPredicate) {
  kCmpEQ                = 0x00U,         //!< Equal             (Quiet).
  kCmpLT                = 0x01U,         //!< Less              (Signaling).
  kCmpLE                = 0x02U,         //!< Less/Equal        (Signaling).
  kCmpUNORD             = 0x03U,         //!< Unordered         (Quiet).
  kCmpNEQ               = 0x04U,         //!< Not Equal         (Quiet).
  kCmpNLT               = 0x05U,         //!< Not Less          (Signaling).
  kCmpNLE               = 0x06U,         //!< Not Less/Equal    (Signaling).
  kCmpORD               = 0x07U          //!< Ordered           (Quiet).
};

// ============================================================================
// [asmjit::x86defs::VCmpPredicate]
// ============================================================================

//! A predicate used by VCMP[PD|PS|SD|SS] instructions.
//!
//! The first 8 values are compatible with \ref CmpPredicate.
ASMJIT_ENUM(VCmpPredicate) {
  kVCmpEQ_OQ            = 0x00U,         //!< Equal             (Quiet    , Ordered).
  kVCmpLT_OS            = 0x01U,         //!< Less              (Signaling, Ordered).
  kVCmpLE_OS            = 0x02U,         //!< Less/Equal        (Signaling, Ordered).
  kVCmpUNORD_Q          = 0x03U,         //!< Unordered         (Quiet).
  kVCmpNEQ_UQ           = 0x04U,         //!< Not Equal         (Quiet    , Unordered).
  kVCmpNLT_US           = 0x05U,         //!< Not Less          (Signaling, Unordered).
  kVCmpNLE_US           = 0x06U,         //!< Not Less/Equal    (Signaling, Unordered).
  kVCmpORD_Q            = 0x07U,         //!< Ordered           (Quiet).
  kVCmpEQ_UQ            = 0x08U,         //!< Equal             (Quiet    , Unordered).
  kVCmpNGE_US           = 0x09U,         //!< Not Greater/Equal (Signaling, Unordered).
  kVCmpNGT_US           = 0x0AU,         //!< Not Greater       (Signaling, Unordered).
  kVCmpFALSE_OQ         = 0x0BU,         //!< False             (Quiet    , Ordered).
  kVCmpNEQ_OQ           = 0x0CU,         //!< Not Equal         (Quiet    , Ordered).
  kVCmpGE_OS            = 0x0DU,         //!< Greater/Equal     (Signaling, Ordered).
  kVCmpGT_OS            = 0x0EU,         //!< Greater           (Signaling, Ordered).
  kVCmpTRUE_UQ          = 0x0FU,         //!< True              (Quiet    , Unordered).
  kVCmpEQ_OS            = 0x10U,         //!< Equal             (Signaling, Ordered).
  kVCmpLT_OQ            = 0x11U,         //!< Less              (Quiet    , Ordered).
  kVCmpLE_OQ            = 0x12U,         //!< Less/Equal        (Quiet    , Ordered).
  kVCmpUNORD_S          = 0x13U,         //!< Unordered         (Signaling).
  kVCmpNEQ_US           = 0x14U,         //!< Not Equal         (Signaling, Unordered).
  kVCmpNLT_UQ           = 0x15U,         //!< Not Less          (Quiet    , Unordered).
  kVCmpNLE_UQ           = 0x16U,         //!< Not Less/Equal    (Quiet    , Unordered).
  kVCmpORD_S            = 0x17U,         //!< Ordered           (Signaling).
  kVCmpEQ_US            = 0x18U,         //!< Equal             (Signaling, Unordered).
  kVCmpNGE_UQ           = 0x19U,         //!< Not Greater/Equal (Quiet    , Unordered).
  kVCmpNGT_UQ           = 0x1AU,         //!< Not Greater       (Quiet    , Unordered).
  kVCmpFALSE_OS         = 0x1BU,         //!< False             (Signaling, Ordered).
  kVCmpNEQ_OS           = 0x1CU,         //!< Not Equal         (Signaling, Ordered).
  kVCmpGE_OQ            = 0x1DU,         //!< Greater/Equal     (Quiet    , Ordered).
  kVCmpGT_OQ            = 0x1EU,         //!< Greater           (Quiet    , Ordered).
  kVCmpTRUE_US          = 0x1FU          //!< True              (Signaling, Unordered).
};

// ============================================================================
// [asmjit::x86defs::PCmpStrPredicate]
// ============================================================================

//! A predicate used by [V]PCMP[I|E]STR[I|M] instructions.
ASMJIT_ENUM(PCmpStrPredicate) {
  // Source data format:
  kPCmpStrUB            = 0x00U << 0,    //!< The source data format is unsigned bytes.
  kPCmpStrUW            = 0x01U << 0,    //!< The source data format is unsigned words.
  kPCmpStrSB            = 0x02U << 0,    //!< The source data format is signed bytes.
  kPCmpStrSW            = 0x03U << 0,    //!< The source data format is signed words.

  // Aggregation operation:
  kPCmpStrEqualAny      = 0x00U << 2,    //!< The arithmetic comparison is "equal".
  kPCmpStrRanges        = 0x01U << 2,    //!< The arithmetic comparison is “greater than or equal”
                                         //!< between even indexed elements and “less than or equal”
                                         //!< between odd indexed elements.
  kPCmpStrEqualEach     = 0x02U << 2,    //!< The arithmetic comparison is "equal".
  kPCmpStrEqualOrdered  = 0x03U << 2,    //!< The arithmetic comparison is "equal".

  // Polarity:
  kPCmpStrPosPolarity   = 0x00U << 4,    //!< IntRes2 = IntRes1.
  kPCmpStrNegPolarity   = 0x01U << 4,    //!< IntRes2 = -1 XOR IntRes1.
  kPCmpStrPosMasked     = 0x02U << 4,    //!< IntRes2 = IntRes1.
  kPCmpStrNegMasked     = 0x03U << 4,    //!< IntRes2[i] = second[i] == invalid ? IntRes1[i] : ~IntRes1[i].

  // Output selection (pcmpstri):
  kPCmpStrOutputLSI     = 0x00U << 6,    //!< The index returned to ECX is of the least significant set bit in IntRes2.
  kPCmpStrOutputMSI     = 0x01U << 6,    //!< The index returned to ECX is of the most significant set bit in IntRes2.

  // Output selection (pcmpstrm):
  kPCmpStrBitMask       = 0x00U << 6,    //!< IntRes2 is returned as the mask to the least significant bits of XMM0.
  kPCmpStrIndexMask     = 0x01U << 6     //!< IntRes2 is expanded into a byte/word mask and placed in XMM0.
};

// ============================================================================
// [asmjit::x86defs::VPCmpPredicate]
// ============================================================================

//! A predicate used by VPCMP[U][B|W|D|Q] instructions (AVX-512).
ASMJIT_ENUM(VPCmpPredicate) {
  kVPCmpEQ              = 0x00U,         //!< Equal.
  kVPCmpLT              = 0x01U,         //!< Less.
  kVPCmpLE              = 0x02U,         //!< Less/Equal.
  kVPCmpFALSE           = 0x03U,         //!< False.
  kVPCmpNE              = 0x04U,         //!< Not Equal.
  kVPCmpGE              = 0x05U,         //!< Greater/Equal.
  kVPCmpGT              = 0x06U,         //!< Greater.
  kVPCmpTRUE            = 0x07U          //!< True.
};

// ============================================================================
// [asmjit::x86defs::VPComPredicate]
// ============================================================================

//! A predicate used by VPCOM[U][B|W|D|Q] instructions (XOP).
ASMJIT_ENUM(VPComPredicate) {
  kVPComLT              = 0x00U,         //!< Less.
  kVPComLE              = 0x01U,         //!< Less/Equal
  kVPComGT              = 0x02U,         //!< Greater.
  kVPComGE              = 0x03U,         //!< Greater/Equal.
  kVPComEQ              = 0x04U,         //!< Equal.
  kVPComNE              = 0x05U,         //!< Not Equal.
  kVPComFALSE           = 0x06U,         //!< False.
  kVPComTRUE            = 0x07U          //!< True.
};

// ============================================================================
// [asmjit::x86defs::VFPClassPredicate]
// ============================================================================

//! A predicate used by VFPCLASS[PD|PS|SD|SS] instructions (AVX-512).
ASMJIT_ENUM(VFPClassPredicate) {
  kVFPClassQNaN         = 0x00U,
  kVFPClassPZero        = 0x01U,
  kVFPClassNZero        = 0x02U,
  kVFPClassPInf         = 0x03U,
  kVFPClassNInf         = 0x04U,
  kVFPClassDenormal     = 0x05U,
  kVFPClassNegative     = 0x06U,
  kVFPClassSNaN         = 0x07U
};

// ============================================================================
// [asmjit::x86defs::VFixupImmPredicate]
// ============================================================================

//! A predicate used by VFIXUPIMM[PD|PS|SD|SS] instructions (AVX-512).
ASMJIT_ENUM(VFixupImmPredicate) {
  kVFixupImmZEOnZero    = 0x01U,
  kVFixupImmIEOnZero    = 0x02U,
  kVFixupImmZEOnOne     = 0x04U,
  kVFixupImmIEOnOne     = 0x08U,
  kVFixupImmIEOnSNaN    = 0x10U,
  kVFixupImmIEOnNInf    = 0x20U,
  kVFixupImmIEOnNegative= 0x40U,
  kVFixupImmIEOnPInf    = 0x80U
};

// ============================================================================
// [asmjit::x86defs::VGetMantPredicate]
// ============================================================================

//! A predicate used by VGETMANT[PD|PS|SD|SS] instructions (AVX-512).
ASMJIT_ENUM(VGetMantPredicate) {
  kVGetMant1To2         = 0x00U,
  kVGetMant1Div2To2     = 0x01U,
  kVGetMant1Div2To1     = 0x02U,
  kVGetMant3Div4To3Div2 = 0x03U,
  kVGetMantNoSign       = 0x04U,
  kVGetMantQNaNIfSign   = 0x08U
};

// ============================================================================
// [asmjit::x86defs::VRangePredicate]
// ============================================================================

//! A predicate used by VRANGE[PD|PS|SD|SS] instructions (AVX-512).
ASMJIT_ENUM(VRangePredicate) {
  kVRangeSelectMin      = 0x00U,         //!< Select minimum value.
  kVRangeSelectMax      = 0x01U,         //!< Select maximum value.
  kVRangeSelectAbsMin   = 0x02U,         //!< Select minimum absolute value.
  kVRangeSelectAbsMax   = 0x03U,         //!< Select maximum absolute value.
  kVRangeSignSrc1       = 0x00U,         //!< Select sign of SRC1.
  kVRangeSignSrc2       = 0x04U,         //!< Select sign of SRC2.
  kVRangeSign0          = 0x08U,         //!< Set sign to 0.
  kVRangeSign1          = 0x0CU          //!< Set sign to 1.
};

// ============================================================================
// [asmjit::x86defs::VReducePredicate]
// ============================================================================

//! A predicate used by VREDUCE[PD|PS|SD|SS] instructions (AVX-512).
ASMJIT_ENUM(VReducePredicate) {
  kVReduceRoundCurrent  = 0x00U,         //!< Round to the current mode set.
  kVReduceRoundEven     = 0x04U,         //!< Round to nearest even.
  kVReduceRoundDown     = 0x05U,         //!< Round down.
  kVReduceRoundUp       = 0x06U,         //!< Round up.
  kVReduceRoundTrunc    = 0x07U,         //!< Truncate.
  kVReduceSuppress      = 0x08U          //!< Suppress exceptions.
};

// ============================================================================
// [asmjit::x86defs::TLogPredicate]
// ============================================================================

//! A predicate that can be used to create an immediate for VTERNLOG[D|Q].
ASMJIT_ENUM(TLogPredicate) {
  kTLog0                = 0x00U,
  kTLog1                = 0xFFU,
  kTLogA                = 0xF0U,
  kTLogB                = 0xCCU,
  kTLogC                = 0xAAU,
  kTLogNotA             = kTLogA ^ 0xFFU,
  kTLogNotB             = kTLogB ^ 0xFFU,
  kTLogNotC             = kTLogC ^ 0xFFU,

  kTLogAB               = kTLogA & kTLogB,
  kTLogAC               = kTLogA & kTLogC,
  kTLogBC               = kTLogB & kTLogC,
  kTLogNotAB            = kTLogAB ^ 0xFFU,
  kTLogNotAC            = kTLogAC ^ 0xFFU,
  kTLogNotBC            = kTLogBC ^ 0xFFU,

  kTLogABC              = kTLogA & kTLogB & kTLogC,
  kTLogNotABC           = kTLogABC ^ 0xFFU
};

// ============================================================================
// [asmjit::x86defs::RoundPredicate]
// ============================================================================

//! A predicate used by ROUND[PD|PS|SD|SS] instructions.
ASMJIT_ENUM(RoundPredicate) {
  kRoundNearest         = 0x00U,         //!< Round to nearest (even).
  kRoundDown            = 0x01U,         //!< Round to down toward -INF (floor),
  kRoundUp              = 0x02U,         //!< Round to up toward +INF (ceil).
  kRoundTrunc           = 0x03U,         //!< Round toward zero (truncate).
  kRoundCurrent         = 0x04U,         //!< Round to the current rounding mode set (ignores other RC bits).
  kRoundInexact         = 0x08U          //!< Avoids inexact exception, if set.
};

} // x86defs namespace

// ============================================================================
// [asmjit::x86::]
// ============================================================================

//! X86 constants, registers, and utilities.
namespace x86 {

// Include all x86 specific namespaces here.
using namespace x86defs;
using namespace x86regs;

//! Pack a shuffle constant to be used by SSE/AVX/AVX-512 instructions (2 values).
//!
//! \param a Position of the first  component [0, 1].
//! \param b Position of the second component [0, 1].
//!
//! Shuffle constants can be used to encode an immediate for these instructions:
//!   - `shufpd`
static ASMJIT_INLINE int shufImm(uint32_t a, uint32_t b) noexcept {
  ASMJIT_ASSERT(a <= 1 && b <= 1);
  return static_cast<int>((a << 1) | b);
}

//! Pack a shuffle constant to be used by SSE/AVX/AVX-512 instructions (4 values).
//!
//! \param a Position of the first  component [0, 3].
//! \param b Position of the second component [0, 3].
//! \param c Position of the third  component [0, 3].
//! \param d Position of the fourth component [0, 3].
//!
//! Shuffle constants can be used to encode an immediate for these instructions:
//!   - `pshufw()`
//!   - `pshufd()`
//!   - `pshuflw()`
//!   - `pshufhw()`
//!   - `shufps()`
static ASMJIT_INLINE int shufImm(uint32_t a, uint32_t b, uint32_t c, uint32_t d) noexcept {
  ASMJIT_ASSERT(a <= 3 && b <= 3 && c <= 3 && d <= 3);
  return static_cast<int>((a << 6) | (b << 4) | (c << 2) | d);
}

//! Create an immediate that can be used by VTERNLOG[D|Q] instructions.
static ASMJIT_INLINE int tlogImm(
  uint32_t b000, uint32_t b001, uint32_t b010, uint32_t b011,
  uint32_t b100, uint32_t b101, uint32_t b110, uint32_t b111) noexcept {

  ASMJIT_ASSERT(b000 <= 1 && b001 <= 1 && b010 <= 1 && b011 <= 1 &&
                b100 <= 1 && b101 <= 1 && b110 <= 1 && b111 <= 1);
  return static_cast<int>((b000 << 0) | (b001 << 1) | (b010 << 2) | (b011 << 3) |
                          (b100 << 4) | (b101 << 5) | (b110 << 6) | (b111 << 7));
}

//! Create an immediate that can be used by VTERNLOG[D|Q] instructions.
static ASMJIT_INLINE int tlogVal(int x) noexcept { return x & 0xFF; }
//! Negate an immediate that can be used by VTERNLOG[D|Q] instructions.
static ASMJIT_INLINE int tlogNot(int x) noexcept { return x ^ 0xFF; }
//! Create an if/else logic that can be used by VTERNLOG[D|Q] instructions.
static ASMJIT_INLINE int tlogIf(int cond, int a, int b) noexcept { return (cond & a) | (tlogNot(cond) & b); }

} // x86 namespace

//! \}

} // asmjit namespace

// [Api-End]
#include "../asmjit_apiend.h"

// [Guard]
#endif // _ASMJIT_X86_X86GLOBALS_H
