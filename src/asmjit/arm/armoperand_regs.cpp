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
#define ASMJIT_ARM_INV_SIGNATURE(TYPE) {{  \
  uint8_t(Operand::kOpNone),               \
  uint8_t(0),                              \
  uint8_t(0),                              \
  uint8_t(0)                               \
}}

#define ASMJIT_ARM_REG_SIGNATURE(TYPE) {{  \
  uint8_t(Operand::kOpReg),                \
  uint8_t(TYPE),                           \
  uint8_t(ArmRegTraits<TYPE>::kKind),      \
  uint8_t(ArmRegTraits<TYPE>::kSize)       \
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
      ASMJIT_ARM_INV_SIGNATURE(0),  // #00 (NONE).
      ASMJIT_ARM_INV_SIGNATURE(1),  // #01 (LABEL).
      ASMJIT_ARM_INV_SIGNATURE(2),  // #02.
      ASMJIT_ARM_REG_SIGNATURE(3),  // #03 (GPW).
      ASMJIT_ARM_REG_SIGNATURE(4),  // #04 (GPX).
      ASMJIT_ARM_INV_SIGNATURE(5),  // #05.
      ASMJIT_ARM_INV_SIGNATURE(6),  // #06.
      ASMJIT_ARM_INV_SIGNATURE(7),  // #07.
      ASMJIT_ARM_INV_SIGNATURE(8),  // #08.
      ASMJIT_ARM_INV_SIGNATURE(9),  // #09.
      ASMJIT_ARM_INV_SIGNATURE(10), // #10.
      ASMJIT_ARM_INV_SIGNATURE(11), // #11.
      ASMJIT_ARM_INV_SIGNATURE(12), // #12.
      ASMJIT_ARM_INV_SIGNATURE(13), // #13.
      ASMJIT_ARM_INV_SIGNATURE(14), // #14.
      ASMJIT_ARM_INV_SIGNATURE(15), // #15.
      ASMJIT_ARM_INV_SIGNATURE(16), // #16.
      ASMJIT_ARM_INV_SIGNATURE(17), // #17.
      ASMJIT_ARM_INV_SIGNATURE(18), // #18.
      ASMJIT_ARM_INV_SIGNATURE(19)  // #19.
    },
    // RegTypeToTypeId[].
    {
      ArmRegTraits< 0>::kTypeId,
      ArmRegTraits< 1>::kTypeId,
      ArmRegTraits< 2>::kTypeId,
      ArmRegTraits< 3>::kTypeId,
      ArmRegTraits< 4>::kTypeId,
      ArmRegTraits< 5>::kTypeId,
      ArmRegTraits< 6>::kTypeId,
      ArmRegTraits< 7>::kTypeId,
      ArmRegTraits< 8>::kTypeId,
      ArmRegTraits< 9>::kTypeId,
      ArmRegTraits<10>::kTypeId,
      ArmRegTraits<11>::kTypeId,
      ArmRegTraits<12>::kTypeId,
      ArmRegTraits<13>::kTypeId,
      ArmRegTraits<14>::kTypeId,
      ArmRegTraits<15>::kTypeId,
      ArmRegTraits<16>::kTypeId,
      ArmRegTraits<17>::kTypeId,
      ArmRegTraits<18>::kTypeId,
      ArmRegTraits<19>::kTypeId
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
#undef ASMJIT_ARM_INV_SIGNATURE

} // asmjit namespace

// [Api-End]
#include "../asmjit_apiend.h"

// [Guard]
#endif // ASMJIT_BUILD_ARM
