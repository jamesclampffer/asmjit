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
#include "../base/misc_p.h"
#include "../arm/armoperand.h"

// [Api-Begin]
#include "../asmjit_apibegin.h"

namespace asmjit {

// ============================================================================
// [asmjit::ArmOpData]
// ============================================================================

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
    // RegInfo[] {
    //   uint8_t opType;
    //   uint8_t regType;
    //   uint8_t kind;
    //   uint8_t size;
    // }
    {
#define ASMJIT_ARM_REG_SIGNATURE(TYPE)        \
      {{                                      \
          uint8_t(ArmRegTraits<TYPE>::kValid  \
            ? Operand::kOpReg                 \
            : Operand::kOpNone),              \
          uint8_t(ArmRegTraits<TYPE>::kValid  \
            ? TYPE                            \
            : 0),                             \
          uint8_t(ArmRegTraits<TYPE>::kKind), \
          uint8_t(ArmRegTraits<TYPE>::kSize)  \
      }}
      ASMJIT_TABLE_16(ASMJIT_ARM_REG_SIGNATURE,  0),
      ASMJIT_TABLE_16(ASMJIT_ARM_REG_SIGNATURE, 16)
#undef ASMJIT_ARM_REG_SIGNATURE
    },

    // RegCount[]
    {
#define ASMJIT_ARM_REG_COUNT(TYPE) ArmRegTraits<TYPE>::kCount
      ASMJIT_TABLE_16(ASMJIT_ARM_REG_COUNT,  0),
      ASMJIT_TABLE_16(ASMJIT_ARM_REG_COUNT, 16)
#undef ASMJIT_ARM_REG_COUNT
    },

    // RegTypeToTypeId[]
    {
#define ASMJIT_ARM_REG_TYPE_ID(TYPE) ArmRegTraits<TYPE>::kTypeId
      ASMJIT_TABLE_16(ASMJIT_ARM_REG_TYPE_ID,  0),
      ASMJIT_TABLE_16(ASMJIT_ARM_REG_TYPE_ID, 16)
#undef ASMJIT_ARM_REG_TYPE_ID
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
