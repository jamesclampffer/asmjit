// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_ARM_ARMINST_H
#define _ASMJIT_ARM_ARMINST_H

// [Dependencies]
#include "../base/assembler.h"
#include "../base/globals.h"
#include "../base/operand.h"
#include "../base/utils.h"

// [Api-Begin]
#include "../asmjit_apibegin.h"

namespace asmjit {

//! \addtogroup asmjit_arm
//! \{

// ============================================================================
// [asmjit::ArmInst]
// ============================================================================

//! ARM instruction data (ARM32 and/or THUMBx).
struct ArmInst {
  //! Instruction id.
  //!
  //! Note that these instruction codes are AsmJit specific. Each instruction
  //! has a unique ID that is used as an index to AsmJit instruction table. The
  //! list is sorted alphabetically. Please use \ref `ArmInst::getIdByName()` if
  //! you need instruction name to ID mapping.
  ASMJIT_ENUM(Id) {
    // ${idData:Begin}
    kIdNone = 0,
    kIdAdc,                              // [TA]
    kIdAdcs,                             // [TA]
    kIdAdd,                              // [TA]
    kIdAdds,                             // [TA]
    kIdAdr,                              // [TA]
    kIdAesd,                             // [TA]
    kIdAese,                             // [TA]
    kIdAesimc,                           // [TA]
    kIdAesmc,                            // [TA]
    kIdAnd,                              // [TA]
    kIdAnds,                             // [TA]
    kIdAsr,                              // [TA]
    kIdAsrs,                             // [TA]
    kIdB,                                // [TA]
    kIdBfc,                              // [TA]
    kIdBfi,                              // [TA]
    kIdBic,                              // [TA]
    kIdBics,                             // [TA]
    kIdBkpt,                             // [TA]
    kIdBl,                               // [TA]
    kIdBlx,                              // [TA]
    kIdBx,                               // [TA]
    kIdBxj,                              // [TA]
    kIdCbnz,                             // [T ]
    kIdCbz,                              // [T ]
    kIdClrex,                            // [TA]
    kIdClz,                              // [TA]
    kIdCmn,                              // [TA]
    kIdCmp,                              // [TA]
    kIdCps,                              // [ A]
    kIdCpsid,                            // [ A]
    kIdCpsie,                            // [ A]
    kIdCrc32b,                           // [TA]
    kIdCrc32cb,                          // [TA]
    kIdCrc32ch,                          // [TA]
    kIdCrc32cw,                          // [TA]
    kIdCrc32h,                           // [TA]
    kIdCrc32w,                           // [TA]
    kIdDbg,                              // [TA]
    kIdDmb,                              // [TA]
    kIdDsb,                              // [TA]
    kIdEor,                              // [TA]
    kIdEors,                             // [TA]
    kIdEret,                             // [ A]
    kIdFldmdbx,                          // [TA]
    kIdFldmiax,                          // [TA]
    kIdFstmdbx,                          // [TA]
    kIdFstmiax,                          // [TA]
    kIdHlt,                              // [ A]
    kIdHvc,                              // [ A]
    kIdIsb,                              // [TA]
    kIdIt{x}{y}{z},                      // [T ]
    kIdLda,                              // [TA]
    kIdLdab,                             // [TA]
    kIdLdaex,                            // [TA]
    kIdLdaexb,                           // [TA]
    kIdLdaexd,                           // [TA]
    kIdLdaexh,                           // [TA]
    kIdLdah,                             // [TA]
    kIdLdm,                              // [TA]
    kIdLdmda,                            // [ A]
    kIdLdmdb,                            // [TA]
    kIdLdmib,                            // [ A]
    kIdLdr,                              // [TA]
    kIdLdrb,                             // [TA]
    kIdLdrbt,                            // [TA]
    kIdLdrd,                             // [TA]
    kIdLdrex,                            // [TA]
    kIdLdrexb,                           // [TA]
    kIdLdrexd,                           // [TA]
    kIdLdrexh,                           // [TA]
    kIdLdrh,                             // [TA]
    kIdLdrht,                            // [TA]
    kIdLdrsb,                            // [TA]
    kIdLdrsbt,                           // [TA]
    kIdLdrsh,                            // [TA]
    kIdLdrsht,                           // [TA]
    kIdLdrt,                             // [TA]
    kIdLsl,                              // [TA]
    kIdLsls,                             // [TA]
    kIdLsr,                              // [TA]
    kIdLsrs,                             // [TA]
    kIdMcr,                              // [TA]
    kIdMcr2,                             // [TA]
    kIdMcrr,                             // [TA]
    kIdMcrr2,                            // [TA]
    kIdMla,                              // [TA]
    kIdMlas,                             // [ A]
    kIdMls,                              // [TA]
    kIdMov,                              // [TA]
    kIdMovs,                             // [TA]
    kIdMovt,                             // [TA]
    kIdMovw,                             // [TA]
    kIdMrc,                              // [TA]
    kIdMrc2,                             // [TA]
    kIdMrrc,                             // [TA]
    kIdMrrc2,                            // [TA]
    kIdMrs,                              // [TA]
    kIdMsr,                              // [TA]
    kIdMul,                              // [TA]
    kIdMuls,                             // [TA]
    kIdMvn,                              // [TA]
    kIdMvns,                             // [TA]
    kIdNop,                              // [TA]
    kIdOrn,                              // [T ]
    kIdOrns,                             // [T ]
    kIdOrr,                              // [TA]
    kIdOrrs,                             // [TA]
    kIdPkhbt,                            // [TA]
    kIdPkhtb,                            // [TA]
    kIdPld,                              // [TA]
    kIdPldw,                             // [TA]
    kIdPli,                              // [TA]
    kIdPop,                              // [TA]
    kIdPush,                             // [TA]
    kIdQadd,                             // [TA]
    kIdQadd16,                           // [TA]
    kIdQadd8,                            // [TA]
    kIdQasx,                             // [TA]
    kIdQdadd,                            // [TA]
    kIdQdsub,                            // [TA]
    kIdQsax,                             // [TA]
    kIdQsub,                             // [TA]
    kIdQsub16,                           // [TA]
    kIdQsub8,                            // [TA]
    kIdRbit,                             // [TA]
    kIdRev,                              // [TA]
    kIdRev16,                            // [TA]
    kIdRevsh,                            // [TA]
    kIdRfe,                              // [ A]
    kIdRfeda,                            // [ A]
    kIdRfedb,                            // [ A]
    kIdRfeib,                            // [ A]
    kIdRor,                              // [TA]
    kIdRors,                             // [TA]
    kIdRrx,                              // [TA]
    kIdRrxs,                             // [TA]
    kIdRsb,                              // [TA]
    kIdRsbs,                             // [TA]
    kIdRsc,                              // [ A]
    kIdRscs,                             // [ A]
    kIdSadd16,                           // [TA]
    kIdSadd8,                            // [TA]
    kIdSasx,                             // [TA]
    kIdSbc,                              // [TA]
    kIdSbcs,                             // [TA]
    kIdSbfx,                             // [TA]
    kIdSdiv,                             // [TA]
    kIdSel,                              // [TA]
    kIdSetend,                           // [TA]
    kIdSev,                              // [TA]
    kIdSevl,                             // [ A]
    kIdSha1c,                            // [TA]
    kIdSha1h,                            // [TA]
    kIdSha1m,                            // [TA]
    kIdSha1p,                            // [TA]
    kIdSha1su0,                          // [TA]
    kIdSha1su1,                          // [TA]
    kIdSha256h,                          // [TA]
    kIdSha256h2,                         // [TA]
    kIdSha256su0,                        // [TA]
    kIdSha256su1,                        // [TA]
    kIdShadd16,                          // [TA]
    kIdShadd8,                           // [TA]
    kIdShasx,                            // [TA]
    kIdShsax,                            // [TA]
    kIdShsub16,                          // [TA]
    kIdShsub8,                           // [TA]
    kIdSmc,                              // [ A]
    kIdSmlabb,                           // [TA]
    kIdSmlabt,                           // [TA]
    kIdSmlad,                            // [TA]
    kIdSmladx,                           // [TA]
    kIdSmlal,                            // [TA]
    kIdSmlalbb,                          // [TA]
    kIdSmlalbt,                          // [TA]
    kIdSmlald,                           // [TA]
    kIdSmlaldx,                          // [TA]
    kIdSmlals,                           // [ A]
    kIdSmlaltb,                          // [TA]
    kIdSmlaltt,                          // [TA]
    kIdSmlatb,                           // [TA]
    kIdSmlatt,                           // [TA]
    kIdSmlawb,                           // [TA]
    kIdSmlawt,                           // [TA]
    kIdSmlsd,                            // [TA]
    kIdSmlsdx,                           // [TA]
    kIdSmlsld,                           // [TA]
    kIdSmlsldx,                          // [TA]
    kIdSmmla,                            // [TA]
    kIdSmmlar,                           // [TA]
    kIdSmmls,                            // [TA]
    kIdSmmlsr,                           // [TA]
    kIdSmmul,                            // [TA]
    kIdSmmulr,                           // [TA]
    kIdSmuad,                            // [TA]
    kIdSmuadx,                           // [TA]
    kIdSmulbb,                           // [TA]
    kIdSmulbt,                           // [TA]
    kIdSmull,                            // [TA]
    kIdSmulls,                           // [ A]
    kIdSmultb,                           // [TA]
    kIdSmultt,                           // [TA]
    kIdSmulwb,                           // [TA]
    kIdSmulwt,                           // [TA]
    kIdSmusd,                            // [TA]
    kIdSmusdx,                           // [TA]
    kIdSrs,                              // [ A]
    kIdSrsda,                            // [ A]
    kIdSrsdb,                            // [ A]
    kIdSrsib,                            // [ A]
    kIdSsat,                             // [TA]
    kIdSsat16,                           // [TA]
    kIdSsax,                             // [TA]
    kIdSsub16,                           // [TA]
    kIdSsub8,                            // [TA]
    kIdStl,                              // [TA]
    kIdStlb,                             // [TA]
    kIdStlex,                            // [TA]
    kIdStlexb,                           // [TA]
    kIdStlexd,                           // [TA]
    kIdStlexh,                           // [TA]
    kIdStlh,                             // [TA]
    kIdStm,                              // [TA]
    kIdStmda,                            // [ A]
    kIdStmdb,                            // [TA]
    kIdStmib,                            // [ A]
    kIdStr,                              // [TA]
    kIdStrb,                             // [TA]
    kIdStrbt,                            // [TA]
    kIdStrd,                             // [TA]
    kIdStrex,                            // [TA]
    kIdStrexb,                           // [TA]
    kIdStrexd,                           // [TA]
    kIdStrexh,                           // [TA]
    kIdStrh,                             // [TA]
    kIdStrht,                            // [TA]
    kIdStrt,                             // [TA]
    kIdSub,                              // [TA]
    kIdSubs,                             // [TA]
    kIdSvc,                              // [TA]
    kIdSwp,                              // [ A]
    kIdSwpb,                             // [ A]
    kIdSxtab,                            // [TA]
    kIdSxtab16,                          // [TA]
    kIdSxtah,                            // [TA]
    kIdSxtb,                             // [TA]
    kIdSxtb16,                           // [TA]
    kIdSxth,                             // [TA]
    kIdTbb,                              // [T ]
    kIdTbh,                              // [T ]
    kIdTeq,                              // [TA]
    kIdTst,                              // [TA]
    kIdUadd16,                           // [TA]
    kIdUadd8,                            // [TA]
    kIdUasx,                             // [TA]
    kIdUbfx,                             // [TA]
    kIdUdf,                              // [ A]
    kIdUdiv,                             // [TA]
    kIdUhadd16,                          // [TA]
    kIdUhadd8,                           // [TA]
    kIdUhasx,                            // [TA]
    kIdUhsax,                            // [TA]
    kIdUhsub16,                          // [TA]
    kIdUhsub8,                           // [TA]
    kIdUmaal,                            // [TA]
    kIdUmlal,                            // [TA]
    kIdUmlals,                           // [ A]
    kIdUmull,                            // [TA]
    kIdUmulls,                           // [ A]
    kIdUqadd16,                          // [TA]
    kIdUqadd8,                           // [TA]
    kIdUqasx,                            // [TA]
    kIdUqsax,                            // [TA]
    kIdUqsub16,                          // [TA]
    kIdUqsub8,                           // [TA]
    kIdUsad8,                            // [TA]
    kIdUsada8,                           // [TA]
    kIdUsat,                             // [TA]
    kIdUsat16,                           // [TA]
    kIdUsax,                             // [TA]
    kIdUsub16,                           // [TA]
    kIdUsub8,                            // [TA]
    kIdUxtab,                            // [TA]
    kIdUxtab16,                          // [TA]
    kIdUxtah,                            // [TA]
    kIdUxtb,                             // [TA]
    kIdUxtb16,                           // [TA]
    kIdUxth,                             // [TA]
    kIdVaba,                             // [TA]
    kIdVabal,                            // [TA]
    kIdVabd,                             // [TA]
    kIdVabdl,                            // [TA]
    kIdVabs,                             // [TA]
    kIdVacge,                            // [TA]
    kIdVacgt,                            // [TA]
    kIdVacle,                            // [TA]
    kIdVaclt,                            // [TA]
    kIdVadd,                             // [TA]
    kIdVaddhn,                           // [TA]
    kIdVaddl,                            // [TA]
    kIdVaddw,                            // [TA]
    kIdVand,                             // [TA]
    kIdVbic,                             // [TA]
    kIdVbif,                             // [TA]
    kIdVbit,                             // [TA]
    kIdVbsl,                             // [TA]
    kIdVceq,                             // [TA]
    kIdVcge,                             // [TA]
    kIdVcgt,                             // [TA]
    kIdVcle,                             // [TA]
    kIdVcls,                             // [TA]
    kIdVclt,                             // [TA]
    kIdVclz,                             // [TA]
    kIdVcmp,                             // [TA]
    kIdVcmpe,                            // [TA]
    kIdVcnt,                             // [TA]
    kIdVcvt,                             // [TA]
    kIdVcvta,                            // [TA]
    kIdVcvtb,                            // [TA]
    kIdVcvtm,                            // [TA]
    kIdVcvtn,                            // [TA]
    kIdVcvtp,                            // [TA]
    kIdVcvtr,                            // [TA]
    kIdVcvtt,                            // [TA]
    kIdVdiv,                             // [TA]
    kIdVdup,                             // [TA]
    kIdVeor,                             // [TA]
    kIdVext,                             // [TA]
    kIdVfma,                             // [TA]
    kIdVfms,                             // [TA]
    kIdVfnma,                            // [TA]
    kIdVfnms,                            // [TA]
    kIdVhadd,                            // [TA]
    kIdVhsub,                            // [TA]
    kIdVmax,                             // [TA]
    kIdVmaxnm,                           // [TA]
    kIdVmin,                             // [TA]
    kIdVminnm,                           // [TA]
    kIdVmla,                             // [TA]
    kIdVmlal,                            // [TA]
    kIdVmls,                             // [TA]
    kIdVmlsl,                            // [TA]
    kIdVmov,                             // [TA]
    kIdVmovl,                            // [TA]
    kIdVmovn,                            // [TA]
    kIdVmul,                             // [TA]
    kIdVmull,                            // [TA]
    kIdVmvn,                             // [TA]
    kIdVneg,                             // [TA]
    kIdVnmla,                            // [TA]
    kIdVnmls,                            // [TA]
    kIdVnmul,                            // [TA]
    kIdVorn,                             // [TA]
    kIdVorr,                             // [TA]
    kIdVpadal,                           // [TA]
    kIdVpadd,                            // [TA]
    kIdVpaddl,                           // [TA]
    kIdVpmax,                            // [TA]
    kIdVpmin,                            // [TA]
    kIdVqabs,                            // [TA]
    kIdVqadd,                            // [TA]
    kIdVqdmlal,                          // [TA]
    kIdVqdmlsl,                          // [TA]
    kIdVqdmulh,                          // [TA]
    kIdVqdmull,                          // [TA]
    kIdVqmovn,                           // [TA]
    kIdVqmovun,                          // [TA]
    kIdVqneg,                            // [TA]
    kIdVqrdmulh,                         // [TA]
    kIdVqrshl,                           // [TA]
    kIdVqrshrn,                          // [TA]
    kIdVqrshrun,                         // [TA]
    kIdVqshl,                            // [TA]
    kIdVqshlu,                           // [TA]
    kIdVqshrn,                           // [TA]
    kIdVqshrun,                          // [TA]
    kIdVqsub,                            // [TA]
    kIdVraddhn,                          // [TA]
    kIdVrecpe,                           // [TA]
    kIdVrecps,                           // [TA]
    kIdVrev16,                           // [TA]
    kIdVrev32,                           // [TA]
    kIdVrev64,                           // [TA]
    kIdVrhadd,                           // [TA]
    kIdVrinta,                           // [TA]
    kIdVrintm,                           // [TA]
    kIdVrintn,                           // [TA]
    kIdVrintp,                           // [TA]
    kIdVrintr,                           // [TA]
    kIdVrintx,                           // [TA]
    kIdVrintz,                           // [TA]
    kIdVrshl,                            // [TA]
    kIdVrshr,                            // [TA]
    kIdVrshrn,                           // [TA]
    kIdVrsqrte,                          // [TA]
    kIdVrsqrts,                          // [TA]
    kIdVrsra,                            // [TA]
    kIdVrsubhn,                          // [TA]
    kIdVseleq,                           // [TA]
    kIdVselge,                           // [TA]
    kIdVselgt,                           // [TA]
    kIdVselvs,                           // [TA]
    kIdVshl,                             // [TA]
    kIdVshll,                            // [TA]
    kIdVshr,                             // [TA]
    kIdVshrl,                            // [TA]
    kIdVsli,                             // [TA]
    kIdVsqrt,                            // [TA]
    kIdVsra,                             // [TA]
    kIdVsri,                             // [TA]
    kIdVsub,                             // [TA]
    kIdVsubhn,                           // [TA]
    kIdVsubl,                            // [TA]
    kIdVsubw,                            // [TA]
    kIdVswp,                             // [TA]
    kIdVtbl,                             // [TA]
    kIdVtbx,                             // [TA]
    kIdVtrn,                             // [TA]
    kIdVtst,                             // [TA]
    kIdVuzp,                             // [TA]
    kIdVzip,                             // [TA]
    kIdWfe,                              // [TA]
    kIdWfi,                              // [TA]
    kIdYield,                            // [TA]
    _kIdCount
    // ${idData:End}
  };

  //! Instruction encodings, used by \ref ArmAssembler.
  ASMJIT_ENUM(EncodingType) {
    kEncodingNone = 0,                   //!< Never used.
    _kEncodingCount                      //!< Count of instruction encodings.
  };

  //! Instruction family.
  //!
  //! Specifies which table should be used to interpret `_familyDataIndex`.
  ASMJIT_ENUM(FamilyType) {
    kFamilyNone           = 0,           //!< General purpose or special instruction.
    kFamilyNeon           = 1            //!< NEON family instruction.
  };

  //! \internal
  //!
  //! Instruction flags.
  ASMJIT_ENUM(InstFlags) {
    kInstFlagNone         = 0x00000000U  //!< No flags.
  };

  //! Specifies meaning of all bits in an opcode (AsmJit specific).
  ASMJIT_ENUM(OpCodeBits) {
  };

  //! Instruction options.
  ASMJIT_ENUM(Options) {
    // NOTE: Don't collide with reserved bits used by CodeEmitter (0x000000FF).

    kOptionOp4            = CodeEmitter::kOptionOp4,
    kOptionOp5            = CodeEmitter::kOptionOp5,
    kOptionOpExtra        = CodeEmitter::kOptionOpExtra
  };

  //! Supported architectures.
  ASMJIT_ENUM(ArchMask) {
    kArchMaskArm32        = 0x01,        //!< ARM32 mode supported.
    kArchMaskArm64        = 0x02         //!< ARM64 mode supported.
  };

  //! Common data - aggregated data that is shared across many instructions.
  struct CommonData {
    // ------------------------------------------------------------------------
    // [Accessors]
    // ------------------------------------------------------------------------

    //! Get all instruction flags, see \ref InstFlags.
    ASMJIT_INLINE uint32_t getFlags() const noexcept { return _flags; }
    //! Get whether the instruction has a `flag`, see `InstFlags`.
    ASMJIT_INLINE bool hasFlag(uint32_t flag) const noexcept { return (_flags & flag) != 0; }

    // ------------------------------------------------------------------------
    // [Members]
    // ------------------------------------------------------------------------

    uint32_t _flags;                     //!< Instruction flags.
  };

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get instruction name (null terminated).
  //!
  //! NOTE: If AsmJit was compiled with `ASMJIT_DISABLE_TEXT` then this will
  //! return an empty string (null terminated string of zero length).
  ASMJIT_INLINE const char* getName() const noexcept;
  //! Get index to `ArmInstDB::nameData` of this instruction.
  //!
  //! NOTE: If AsmJit was compiled with `ASMJIT_DISABLE_TEXT` then this will
  //! always return zero.
  ASMJIT_INLINE uint32_t getNameDataIndex() const noexcept { return _nameDataIndex; }

  //! Get \ref CommonData of the instruction.
  ASMJIT_INLINE const CommonData& getCommonData() const noexcept;
  //! Get index to `ArmInstDB::commonData` of this instruction.
  ASMJIT_INLINE uint32_t getCommonDataIndex() const noexcept { return _commonDataIndex; }

  //! Get instruction encoding, see \ref EncodingType.
  ASMJIT_INLINE uint32_t getEncodingType() const noexcept { return _encodingType; }

  //! Get instruction opcode, see \ref OpCodeBits.
  ASMJIT_INLINE uint32_t getOpCode() const noexcept { return _opCode; }

  //! Get whether the instruction has flag `flag`, see \ref InstFlags.
  ASMJIT_INLINE bool hasFlag(uint32_t flag) const noexcept { return getCommonData().hasFlag(flag); }
  //! Get instruction flags, see \ref InstFlags.
  ASMJIT_INLINE uint32_t getFlags() const noexcept { return getCommonData().getFlags(); }

  // --------------------------------------------------------------------------
  // [Get]
  // --------------------------------------------------------------------------

  //! Get if the `instId` is defined (counts also kInvalidInstId, which is zero).
  static ASMJIT_INLINE bool isDefinedId(uint32_t instId) noexcept { return instId < _kIdCount; }

  //! Get instruction information based on the instruction `instId`.
  //!
  //! NOTE: `instId` has to be a valid instruction ID, it can't be greater than
  //! or equal to `ArmInst::_kIdCount`. It asserts in debug mode.
  static ASMJIT_INLINE const ArmInst& getInst(uint32_t instId) noexcept;

  // --------------------------------------------------------------------------
  // [Id <-> Name]
  // --------------------------------------------------------------------------

#if !defined(ASMJIT_DISABLE_TEXT)
  //! Get an instruction ID from a given instruction `name`.
  //!
  //! NOTE: Instruction name MUST BE in lowercase, otherwise there will be no
  //! match. If there is an exact match the instruction id is returned, otherwise
  //! `kInvalidInstId` (zero) is returned instead. The given `name` doesn't have
  //! to be null-terminated if `len` is provided.
  ASMJIT_API static uint32_t getIdByName(const char* name, size_t len = Globals::kInvalidIndex) noexcept;

  //! Get an instruction name from a given instruction id `instId`.
  ASMJIT_API static const char* getNameById(uint32_t instId) noexcept;
#endif // !ASMJIT_DISABLE_TEXT

  // --------------------------------------------------------------------------
  // [Validation]
  // --------------------------------------------------------------------------

#if !defined(ASMJIT_DISABLE_VALIDATION)
  ASMJIT_API static Error validate(
    uint32_t archType, uint32_t instId, uint32_t options,
    const Operand_& opExtra,
    const Operand_* opArray, uint32_t opCount) noexcept;
#endif // !ASMJIT_DISABLE_VALIDATION

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  uint32_t _encodingType    : 8;         //!< Instruction encoding.
  uint32_t _nameDataIndex   : 14;        //!< Index to `X86InstDB::nameData` table.
  uint32_t _commonDataIndex : 10;        //!< Index to `X86InstDB::commonData` table.
  uint32_t _opCode;                      //!< Instruction opcode.
};

//! ARM instruction data under a single namespace.
struct ArmInstDB {
  ASMJIT_API static const ArmInst instData[];
  ASMJIT_API static const ArmInst::CommonData commonData[];
  ASMJIT_API static const char nameData[];
};

//! \}

} // asmjit namespace

#undef _OP_ID

// [Api-End]
#include "../asmjit_apiend.h"

// [Guard]
#endif // _ASMJIT_ARM_ARMINST_H
