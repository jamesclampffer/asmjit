// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS
#define ASMJIT_EXPORTS_ARM_OPERAND

// [Guard]
#include "../asmjit_build.h"
#if defined(ASMJIT_BUILD_ARM)

// [Dependencies]
#include "../arm/armoperand.h"

// [Api-Begin]
#include "../asmjit_apibegin.h"

namespace asmjit {

// ============================================================================
// [asmjit::ArmOpData]
// ============================================================================

// Register Signature {
//   uint8_t opType;
//   uint8_t regType;
//   uint8_t kind;
//   uint8_t size;
// }
#define ASMJIT_ARM_REG_SIGNATURE(TYPE) {{        \
  uint8_t(                                       \
    ArmRegTraits<TYPE>::kValid                   \
      ? Operand::kOpReg                          \
      : Operand::kOpNone),                       \
  uint8_t(                                       \
    ArmRegTraits<TYPE>::kValid                   \
      ? TYPE                                     \
      : 0),                                      \
  uint8_t(ArmRegTraits<TYPE>::kKind),            \
  uint8_t(ArmRegTraits<TYPE>::kSize)             \
}}

// Register Operand {
//   uint8_t opType;
//   uint8_t regType;
//   uint8_t kind;
//   uint8_t size;
//   uint32_t id;
//   uint32_t reserved8_4;
//   uint32_t reserved12_4;
// }
#define ASMJIT_ARM_REG_01(TYPE, ID) {{{    \
  uint8_t(Operand::kOpReg),                \
  uint8_t(TYPE),                           \
  uint8_t(ArmRegTraits<TYPE>::kKind),      \
  uint8_t(ArmRegTraits<TYPE>::kSize),      \
  uint32_t(ID),                            \
  uint32_t(0),                             \
  uint32_t(0)                              \
}}}

#define ASMJIT_ARM_REG_04(TYPE, ID) \
  ASMJIT_ARM_REG_01(TYPE, ID + 0 ), \
  ASMJIT_ARM_REG_01(TYPE, ID + 1 ), \
  ASMJIT_ARM_REG_01(TYPE, ID + 2 ), \
  ASMJIT_ARM_REG_01(TYPE, ID + 3 )

#define ASMJIT_ARM_REG_08(TYPE, ID) \
  ASMJIT_ARM_REG_04(TYPE, ID + 0 ), \
  ASMJIT_ARM_REG_04(TYPE, ID + 4 )

#define ASMJIT_ARM_REG_16(TYPE, ID) \
  ASMJIT_ARM_REG_08(TYPE, ID + 0 ), \
  ASMJIT_ARM_REG_08(TYPE, ID + 8 )

#define ASMJIT_ARM_REG_32(TYPE, ID) \
  ASMJIT_ARM_REG_16(TYPE, ID + 0 ), \
  ASMJIT_ARM_REG_16(TYPE, ID + 16)

const ArmOpData armOpData = {
  // --------------------------------------------------------------------------
  // [ArchRegs]
  // --------------------------------------------------------------------------

  {
    // RegType[].
    {
      ASMJIT_ARM_REG_SIGNATURE(0 ), ASMJIT_ARM_REG_SIGNATURE(1 ),
      ASMJIT_ARM_REG_SIGNATURE(2 ), ASMJIT_ARM_REG_SIGNATURE(3 ),
      ASMJIT_ARM_REG_SIGNATURE(4 ), ASMJIT_ARM_REG_SIGNATURE(5 ),
      ASMJIT_ARM_REG_SIGNATURE(6 ), ASMJIT_ARM_REG_SIGNATURE(7 ),
      ASMJIT_ARM_REG_SIGNATURE(8 ), ASMJIT_ARM_REG_SIGNATURE(9 ),
      ASMJIT_ARM_REG_SIGNATURE(10), ASMJIT_ARM_REG_SIGNATURE(11),
      ASMJIT_ARM_REG_SIGNATURE(12), ASMJIT_ARM_REG_SIGNATURE(13),
      ASMJIT_ARM_REG_SIGNATURE(14), ASMJIT_ARM_REG_SIGNATURE(15),
      ASMJIT_ARM_REG_SIGNATURE(16), ASMJIT_ARM_REG_SIGNATURE(17),
      ASMJIT_ARM_REG_SIGNATURE(18), ASMJIT_ARM_REG_SIGNATURE(19),
      ASMJIT_ARM_REG_SIGNATURE(20), ASMJIT_ARM_REG_SIGNATURE(21),
      ASMJIT_ARM_REG_SIGNATURE(22), ASMJIT_ARM_REG_SIGNATURE(23),
      ASMJIT_ARM_REG_SIGNATURE(24), ASMJIT_ARM_REG_SIGNATURE(25),
      ASMJIT_ARM_REG_SIGNATURE(26), ASMJIT_ARM_REG_SIGNATURE(27),
      ASMJIT_ARM_REG_SIGNATURE(28), ASMJIT_ARM_REG_SIGNATURE(29),
      ASMJIT_ARM_REG_SIGNATURE(30), ASMJIT_ARM_REG_SIGNATURE(31)
    },
    // RegTypeToTypeId[].
    {
      X86RegTraits< 0>::kTypeId, X86RegTraits< 1>::kTypeId,
      X86RegTraits< 2>::kTypeId, X86RegTraits< 3>::kTypeId,
      X86RegTraits< 4>::kTypeId, X86RegTraits< 5>::kTypeId,
      X86RegTraits< 6>::kTypeId, X86RegTraits< 7>::kTypeId,
      X86RegTraits< 8>::kTypeId, X86RegTraits< 9>::kTypeId,
      X86RegTraits<10>::kTypeId, X86RegTraits<11>::kTypeId,
      X86RegTraits<12>::kTypeId, X86RegTraits<13>::kTypeId,
      X86RegTraits<14>::kTypeId, X86RegTraits<15>::kTypeId,
      X86RegTraits<16>::kTypeId, X86RegTraits<17>::kTypeId,
      X86RegTraits<18>::kTypeId, X86RegTraits<19>::kTypeId,
      X86RegTraits<20>::kTypeId, X86RegTraits<21>::kTypeId,
      X86RegTraits<22>::kTypeId, X86RegTraits<23>::kTypeId,
      X86RegTraits<24>::kTypeId, X86RegTraits<25>::kTypeId,
      X86RegTraits<26>::kTypeId, X86RegTraits<27>::kTypeId,
      X86RegTraits<28>::kTypeId, X86RegTraits<29>::kTypeId,
      X86RegTraits<30>::kTypeId, X86RegTraits<31>::kTypeId
    }
  },

  // --------------------------------------------------------------------------
  // [Registers]
  // --------------------------------------------------------------------------

  { ASMJIT_ARM_REG_32(ArmReg::kRegGpw, 0) },
  { ASMJIT_ARM_REG_32(ArmReg::kRegGpx, 0) }
};

#undef ASMJIT_ARM_REG_32
#undef ASMJIT_ARM_REG_16
#undef ASMJIT_ARM_REG_08
#undef ASMJIT_ARM_REG_04
#undef ASMJIT_ARM_REG_01
#undef ASMJIT_ARM_REG_SIGNATURE

} // asmjit namespace

// [Api-End]
#include "../asmjit_apiend.h"

// [Guard]
#endif // ASMJIT_BUILD_ARM
