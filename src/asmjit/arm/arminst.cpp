// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Guard]
#include "../asmjit_build.h"
#if defined(ASMJIT_BUILD_ARM)

// [Dependencies]
#include "../base/utils.h"
#include "../arm/arminst.h"
#include "../arm/armoperand.h"

// [Api-Begin]
#include "../asmjit_apibegin.h"

namespace asmjit {

// ============================================================================
// [asmjit::ArmInst]
// ============================================================================

// Instruction opcode definitions:
#define O(OPCODE) (OPCODE)                          // Instruction OpCode.
#define F(FLAG) ArmInst::kInstFlag##FLAG            // Instruction Base Flag(s) `F(...)`.
#define Enc(ENCODING) ArmInst::kEncoding##ENCODING  // Instruction Encoding `Enc(...)`.

// Don't store `_nameDataIndex` if instruction names are disabled. Since
// some  APIs can use `_nameDataIndex` it's much safer if it's zero.
#if defined(ASMJIT_DISABLE_TEXT)
# define NAME_DATA_INDEX(X) 0
#else
# define NAME_DATA_INDEX(X) X
#endif

// Defines an ARM32/ARM64 instruction.
#define INST(id, name, encoding, opcode0, instFlags, nameDataIndex, commonIndex) { \
  uint32_t(encoding),                       \
  uint32_t(NAME_DATA_INDEX(nameDataIndex)), \
  uint32_t(commonIndex),                    \
  0,                                        \
  opcode0                                   \
}

const ArmInst ArmInstDB::instData[] = {
  // <-----------------+------------------------+------+---------------------------------------+-----+----+
  //                   |                        |      |                                       |     |    |
  //  Instruction Id   |  Instruction Encoding  |OpCode|          Instruction Flags            |NameX|ComX|
  //                   |                        |      |                                       |     |    |
  // <-----------------+------------------------+------+---------------------------------------+-----+----+
  // ${instData:Begin}
  INST(None            , undefined              , undefined                 , 0   , 0  ),
  INST(Adc             , undefined              , undefined                 , 1   , 0  ),
  INST(Adcs            , undefined              , undefined                 , 5   , 0  ),
  INST(Add             , undefined              , undefined                 , 2083, 0  ),
  INST(Adds            , undefined              , undefined                 , 10  , 0  ),
  INST(Adr             , undefined              , undefined                 , 15  , 0  ),
  INST(Aesd            , undefined              , undefined                 , 19  , 0  ),
  INST(Aese            , undefined              , undefined                 , 24  , 0  ),
  INST(Aesimc          , undefined              , undefined                 , 29  , 0  ),
  INST(Aesmc           , undefined              , undefined                 , 36  , 0  ),
  INST(And             , undefined              , undefined                 , 1585, 0  ),
  INST(Ands            , undefined              , undefined                 , 42  , 0  ),
  INST(Asr             , undefined              , undefined                 , 47  , 0  ),
  INST(Asrs            , undefined              , undefined                 , 51  , 0  ),
  INST(B               , undefined              , undefined                 , 126 , 0  ),
  INST(Bfc             , undefined              , undefined                 , 56  , 0  ),
  INST(Bfi             , undefined              , undefined                 , 60  , 0  ),
  INST(Bic             , undefined              , undefined                 , 1590, 0  ),
  INST(Bics            , undefined              , undefined                 , 64  , 0  ),
  INST(Bkpt            , undefined              , undefined                 , 69  , 0  ),
  INST(Bl              , undefined              , undefined                 , 2271, 0  ),
  INST(Blx             , undefined              , undefined                 , 74  , 0  ),
  INST(Bx              , undefined              , undefined                 , 185 , 0  ),
  INST(Bxj             , undefined              , undefined                 , 78  , 0  ),
  INST(Cbnz            , undefined              , undefined                 , 82  , 0  ),
  INST(Cbz             , undefined              , undefined                 , 87  , 0  ),
  INST(Clrex           , undefined              , undefined                 , 91  , 0  ),
  INST(Clz             , undefined              , undefined                 , 1640, 0  ),
  INST(Cmn             , undefined              , undefined                 , 97  , 0  ),
  INST(Cmp             , undefined              , undefined                 , 1645, 0  ),
  INST(Cps             , undefined              , undefined                 , 2055, 0  ),
  INST(Cpsid           , undefined              , undefined                 , 101 , 0  ),
  INST(Cpsie           , undefined              , undefined                 , 107 , 0  ),
  INST(Crc32b          , undefined              , undefined                 , 113 , 0  ),
  INST(Crc32cb         , undefined              , undefined                 , 120 , 0  ),
  INST(Crc32ch         , undefined              , undefined                 , 128 , 0  ),
  INST(Crc32cw         , undefined              , undefined                 , 136 , 0  ),
  INST(Crc32h          , undefined              , undefined                 , 144 , 0  ),
  INST(Crc32w          , undefined              , undefined                 , 151 , 0  ),
  INST(Dbg             , undefined              , undefined                 , 158 , 0  ),
  INST(Dmb             , undefined              , undefined                 , 162 , 0  ),
  INST(Dsb             , undefined              , undefined                 , 166 , 0  ),
  INST(Eor             , undefined              , undefined                 , 1718, 0  ),
  INST(Eors            , undefined              , undefined                 , 170 , 0  ),
  INST(Eret            , undefined              , undefined                 , 175 , 0  ),
  INST(Fldmdbx         , undefined              , undefined                 , 180 , 0  ),
  INST(Fldmiax         , undefined              , undefined                 , 188 , 0  ),
  INST(Fstmdbx         , undefined              , undefined                 , 196 , 0  ),
  INST(Fstmiax         , undefined              , undefined                 , 204 , 0  ),
  INST(Hlt             , undefined              , undefined                 , 212 , 0  ),
  INST(Hvc             , undefined              , undefined                 , 216 , 0  ),
  INST(Isb             , undefined              , undefined                 , 220 , 0  ),
  INST(It{x}{y}{z}     , undefined              , undefined                 , 224 , 0  ),
  INST(Lda             , undefined              , undefined                 , 236 , 0  ),
  INST(Ldab            , undefined              , undefined                 , 240 , 0  ),
  INST(Ldaex           , undefined              , undefined                 , 245 , 0  ),
  INST(Ldaexb          , undefined              , undefined                 , 251 , 0  ),
  INST(Ldaexd          , undefined              , undefined                 , 258 , 0  ),
  INST(Ldaexh          , undefined              , undefined                 , 265 , 0  ),
  INST(Ldah            , undefined              , undefined                 , 272 , 0  ),
  INST(Ldm             , undefined              , undefined                 , 277 , 0  ),
  INST(Ldmda           , undefined              , undefined                 , 281 , 0  ),
  INST(Ldmdb           , undefined              , undefined                 , 287 , 0  ),
  INST(Ldmib           , undefined              , undefined                 , 293 , 0  ),
  INST(Ldr             , undefined              , undefined                 , 299 , 0  ),
  INST(Ldrb            , undefined              , undefined                 , 303 , 0  ),
  INST(Ldrbt           , undefined              , undefined                 , 308 , 0  ),
  INST(Ldrd            , undefined              , undefined                 , 314 , 0  ),
  INST(Ldrex           , undefined              , undefined                 , 319 , 0  ),
  INST(Ldrexb          , undefined              , undefined                 , 325 , 0  ),
  INST(Ldrexd          , undefined              , undefined                 , 332 , 0  ),
  INST(Ldrexh          , undefined              , undefined                 , 339 , 0  ),
  INST(Ldrh            , undefined              , undefined                 , 346 , 0  ),
  INST(Ldrht           , undefined              , undefined                 , 351 , 0  ),
  INST(Ldrsb           , undefined              , undefined                 , 357 , 0  ),
  INST(Ldrsbt          , undefined              , undefined                 , 363 , 0  ),
  INST(Ldrsh           , undefined              , undefined                 , 370 , 0  ),
  INST(Ldrsht          , undefined              , undefined                 , 376 , 0  ),
  INST(Ldrt            , undefined              , undefined                 , 383 , 0  ),
  INST(Lsl             , undefined              , undefined                 , 1929, 0  ),
  INST(Lsls            , undefined              , undefined                 , 388 , 0  ),
  INST(Lsr             , undefined              , undefined                 , 946 , 0  ),
  INST(Lsrs            , undefined              , undefined                 , 393 , 0  ),
  INST(Mcr             , undefined              , undefined                 , 398 , 0  ),
  INST(Mcr2            , undefined              , undefined                 , 402 , 0  ),
  INST(Mcrr            , undefined              , undefined                 , 407 , 0  ),
  INST(Mcrr2           , undefined              , undefined                 , 412 , 0  ),
  INST(Mla             , undefined              , undefined                 , 926 , 0  ),
  INST(Mlas            , undefined              , undefined                 , 418 , 0  ),
  INST(Mls             , undefined              , undefined                 , 939 , 0  ),
  INST(Mov             , undefined              , undefined                 , 1808, 0  ),
  INST(Movs            , undefined              , undefined                 , 423 , 0  ),
  INST(Movt            , undefined              , undefined                 , 428 , 0  ),
  INST(Movw            , undefined              , undefined                 , 433 , 0  ),
  INST(Mrc             , undefined              , undefined                 , 438 , 0  ),
  INST(Mrc2            , undefined              , undefined                 , 442 , 0  ),
  INST(Mrrc            , undefined              , undefined                 , 447 , 0  ),
  INST(Mrrc2           , undefined              , undefined                 , 452 , 0  ),
  INST(Mrs             , undefined              , undefined                 , 458 , 0  ),
  INST(Msr             , undefined              , undefined                 , 462 , 0  ),
  INST(Mul             , undefined              , undefined                 , 952 , 0  ),
  INST(Muls            , undefined              , undefined                 , 466 , 0  ),
  INST(Mvn             , undefined              , undefined                 , 1836, 0  ),
  INST(Mvns            , undefined              , undefined                 , 471 , 0  ),
  INST(Nop             , undefined              , undefined                 , 476 , 0  ),
  INST(Orn             , undefined              , undefined                 , 1864, 0  ),
  INST(Orns            , undefined              , undefined                 , 480 , 0  ),
  INST(Orr             , undefined              , undefined                 , 1869, 0  ),
  INST(Orrs            , undefined              , undefined                 , 485 , 0  ),
  INST(Pkhbt           , undefined              , undefined                 , 490 , 0  ),
  INST(Pkhtb           , undefined              , undefined                 , 496 , 0  ),
  INST(Pld             , undefined              , undefined                 , 502 , 0  ),
  INST(Pldw            , undefined              , undefined                 , 506 , 0  ),
  INST(Pli             , undefined              , undefined                 , 511 , 0  ),
  INST(Pop             , undefined              , undefined                 , 515 , 0  ),
  INST(Push            , undefined              , undefined                 , 519 , 0  ),
  INST(Qadd            , undefined              , undefined                 , 1912, 0  ),
  INST(Qadd16          , undefined              , undefined                 , 1388, 0  ),
  INST(Qadd8           , undefined              , undefined                 , 1396, 0  ),
  INST(Qasx            , undefined              , undefined                 , 1403, 0  ),
  INST(Qdadd           , undefined              , undefined                 , 524 , 0  ),
  INST(Qdsub           , undefined              , undefined                 , 530 , 0  ),
  INST(Qsax            , undefined              , undefined                 , 1409, 0  ),
  INST(Qsub            , undefined              , undefined                 , 2032, 0  ),
  INST(Qsub16          , undefined              , undefined                 , 1415, 0  ),
  INST(Qsub8           , undefined              , undefined                 , 1423, 0  ),
  INST(Rbit            , undefined              , undefined                 , 536 , 0  ),
  INST(Rev             , undefined              , undefined                 , 541 , 0  ),
  INST(Rev16           , undefined              , undefined                 , 2060, 0  ),
  INST(Revsh           , undefined              , undefined                 , 545 , 0  ),
  INST(Rfe             , undefined              , undefined                 , 551 , 0  ),
  INST(Rfeda           , undefined              , undefined                 , 555 , 0  ),
  INST(Rfedb           , undefined              , undefined                 , 561 , 0  ),
  INST(Rfeib           , undefined              , undefined                 , 567 , 0  ),
  INST(Ror             , undefined              , undefined                 , 573 , 0  ),
  INST(Rors            , undefined              , undefined                 , 577 , 0  ),
  INST(Rrx             , undefined              , undefined                 , 582 , 0  ),
  INST(Rrxs            , undefined              , undefined                 , 586 , 0  ),
  INST(Rsb             , undefined              , undefined                 , 359 , 0  ),
  INST(Rsbs            , undefined              , undefined                 , 591 , 0  ),
  INST(Rsc             , undefined              , undefined                 , 596 , 0  ),
  INST(Rscs            , undefined              , undefined                 , 600 , 0  ),
  INST(Sadd16          , undefined              , undefined                 , 605 , 0  ),
  INST(Sadd8           , undefined              , undefined                 , 612 , 0  ),
  INST(Sasx            , undefined              , undefined                 , 618 , 0  ),
  INST(Sbc             , undefined              , undefined                 , 623 , 0  ),
  INST(Sbcs            , undefined              , undefined                 , 627 , 0  ),
  INST(Sbfx            , undefined              , undefined                 , 632 , 0  ),
  INST(Sdiv            , undefined              , undefined                 , 637 , 0  ),
  INST(Sel             , undefined              , undefined                 , 642 , 0  ),
  INST(Setend          , undefined              , undefined                 , 646 , 0  ),
  INST(Sev             , undefined              , undefined                 , 653 , 0  ),
  INST(Sevl            , undefined              , undefined                 , 657 , 0  ),
  INST(Sha1c           , undefined              , undefined                 , 662 , 0  ),
  INST(Sha1h           , undefined              , undefined                 , 668 , 0  ),
  INST(Sha1m           , undefined              , undefined                 , 674 , 0  ),
  INST(Sha1p           , undefined              , undefined                 , 680 , 0  ),
  INST(Sha1su0         , undefined              , undefined                 , 686 , 0  ),
  INST(Sha1su1         , undefined              , undefined                 , 694 , 0  ),
  INST(Sha256h         , undefined              , undefined                 , 702 , 0  ),
  INST(Sha256h2        , undefined              , undefined                 , 710 , 0  ),
  INST(Sha256su0       , undefined              , undefined                 , 719 , 0  ),
  INST(Sha256su1       , undefined              , undefined                 , 729 , 0  ),
  INST(Shadd16         , undefined              , undefined                 , 739 , 0  ),
  INST(Shadd8          , undefined              , undefined                 , 747 , 0  ),
  INST(Shasx           , undefined              , undefined                 , 754 , 0  ),
  INST(Shsax           , undefined              , undefined                 , 760 , 0  ),
  INST(Shsub16         , undefined              , undefined                 , 766 , 0  ),
  INST(Shsub8          , undefined              , undefined                 , 774 , 0  ),
  INST(Smc             , undefined              , undefined                 , 38  , 0  ),
  INST(Smlabb          , undefined              , undefined                 , 781 , 0  ),
  INST(Smlabt          , undefined              , undefined                 , 788 , 0  ),
  INST(Smlad           , undefined              , undefined                 , 795 , 0  ),
  INST(Smladx          , undefined              , undefined                 , 801 , 0  ),
  INST(Smlal           , undefined              , undefined                 , 808 , 0  ),
  INST(Smlalbb         , undefined              , undefined                 , 814 , 0  ),
  INST(Smlalbt         , undefined              , undefined                 , 822 , 0  ),
  INST(Smlald          , undefined              , undefined                 , 830 , 0  ),
  INST(Smlaldx         , undefined              , undefined                 , 837 , 0  ),
  INST(Smlals          , undefined              , undefined                 , 845 , 0  ),
  INST(Smlaltb         , undefined              , undefined                 , 852 , 0  ),
  INST(Smlaltt         , undefined              , undefined                 , 860 , 0  ),
  INST(Smlatb          , undefined              , undefined                 , 868 , 0  ),
  INST(Smlatt          , undefined              , undefined                 , 875 , 0  ),
  INST(Smlawb          , undefined              , undefined                 , 882 , 0  ),
  INST(Smlawt          , undefined              , undefined                 , 889 , 0  ),
  INST(Smlsd           , undefined              , undefined                 , 896 , 0  ),
  INST(Smlsdx          , undefined              , undefined                 , 902 , 0  ),
  INST(Smlsld          , undefined              , undefined                 , 909 , 0  ),
  INST(Smlsldx         , undefined              , undefined                 , 916 , 0  ),
  INST(Smmla           , undefined              , undefined                 , 924 , 0  ),
  INST(Smmlar          , undefined              , undefined                 , 930 , 0  ),
  INST(Smmls           , undefined              , undefined                 , 937 , 0  ),
  INST(Smmlsr          , undefined              , undefined                 , 943 , 0  ),
  INST(Smmul           , undefined              , undefined                 , 950 , 0  ),
  INST(Smmulr          , undefined              , undefined                 , 956 , 0  ),
  INST(Smuad           , undefined              , undefined                 , 963 , 0  ),
  INST(Smuadx          , undefined              , undefined                 , 969 , 0  ),
  INST(Smulbb          , undefined              , undefined                 , 976 , 0  ),
  INST(Smulbt          , undefined              , undefined                 , 983 , 0  ),
  INST(Smull           , undefined              , undefined                 , 990 , 0  ),
  INST(Smulls          , undefined              , undefined                 , 996 , 0  ),
  INST(Smultb          , undefined              , undefined                 , 1003, 0  ),
  INST(Smultt          , undefined              , undefined                 , 1010, 0  ),
  INST(Smulwb          , undefined              , undefined                 , 1017, 0  ),
  INST(Smulwt          , undefined              , undefined                 , 1024, 0  ),
  INST(Smusd           , undefined              , undefined                 , 1031, 0  ),
  INST(Smusdx          , undefined              , undefined                 , 1037, 0  ),
  INST(Srs             , undefined              , undefined                 , 52  , 0  ),
  INST(Srsda           , undefined              , undefined                 , 1044, 0  ),
  INST(Srsdb           , undefined              , undefined                 , 1050, 0  ),
  INST(Srsib           , undefined              , undefined                 , 1056, 0  ),
  INST(Ssat            , undefined              , undefined                 , 1062, 0  ),
  INST(Ssat16          , undefined              , undefined                 , 1067, 0  ),
  INST(Ssax            , undefined              , undefined                 , 1074, 0  ),
  INST(Ssub16          , undefined              , undefined                 , 1079, 0  ),
  INST(Ssub8           , undefined              , undefined                 , 1086, 0  ),
  INST(Stl             , undefined              , undefined                 , 1092, 0  ),
  INST(Stlb            , undefined              , undefined                 , 1096, 0  ),
  INST(Stlex           , undefined              , undefined                 , 1101, 0  ),
  INST(Stlexb          , undefined              , undefined                 , 1107, 0  ),
  INST(Stlexd          , undefined              , undefined                 , 1114, 0  ),
  INST(Stlexh          , undefined              , undefined                 , 1121, 0  ),
  INST(Stlh            , undefined              , undefined                 , 1128, 0  ),
  INST(Stm             , undefined              , undefined                 , 1133, 0  ),
  INST(Stmda           , undefined              , undefined                 , 1137, 0  ),
  INST(Stmdb           , undefined              , undefined                 , 1143, 0  ),
  INST(Stmib           , undefined              , undefined                 , 1149, 0  ),
  INST(Str             , undefined              , undefined                 , 1155, 0  ),
  INST(Strb            , undefined              , undefined                 , 1159, 0  ),
  INST(Strbt           , undefined              , undefined                 , 1164, 0  ),
  INST(Strd            , undefined              , undefined                 , 1170, 0  ),
  INST(Strex           , undefined              , undefined                 , 1175, 0  ),
  INST(Strexb          , undefined              , undefined                 , 1181, 0  ),
  INST(Strexd          , undefined              , undefined                 , 1188, 0  ),
  INST(Strexh          , undefined              , undefined                 , 1195, 0  ),
  INST(Strh            , undefined              , undefined                 , 1202, 0  ),
  INST(Strht           , undefined              , undefined                 , 1207, 0  ),
  INST(Strt            , undefined              , undefined                 , 1213, 0  ),
  INST(Sub             , undefined              , undefined                 , 532 , 0  ),
  INST(Subs            , undefined              , undefined                 , 1218, 0  ),
  INST(Svc             , undefined              , undefined                 , 1223, 0  ),
  INST(Swp             , undefined              , undefined                 , 2281, 0  ),
  INST(Swpb            , undefined              , undefined                 , 1227, 0  ),
  INST(Sxtab           , undefined              , undefined                 , 1232, 0  ),
  INST(Sxtab16         , undefined              , undefined                 , 1238, 0  ),
  INST(Sxtah           , undefined              , undefined                 , 1246, 0  ),
  INST(Sxtb            , undefined              , undefined                 , 1252, 0  ),
  INST(Sxtb16          , undefined              , undefined                 , 1257, 0  ),
  INST(Sxth            , undefined              , undefined                 , 1264, 0  ),
  INST(Tbb             , undefined              , undefined                 , 1269, 0  ),
  INST(Tbh             , undefined              , undefined                 , 1273, 0  ),
  INST(Teq             , undefined              , undefined                 , 1277, 0  ),
  INST(Tst             , undefined              , undefined                 , 2301, 0  ),
  INST(Uadd16          , undefined              , undefined                 , 1281, 0  ),
  INST(Uadd8           , undefined              , undefined                 , 1288, 0  ),
  INST(Uasx            , undefined              , undefined                 , 1294, 0  ),
  INST(Ubfx            , undefined              , undefined                 , 1299, 0  ),
  INST(Udf             , undefined              , undefined                 , 1304, 0  ),
  INST(Udiv            , undefined              , undefined                 , 1308, 0  ),
  INST(Uhadd16         , undefined              , undefined                 , 1313, 0  ),
  INST(Uhadd8          , undefined              , undefined                 , 1321, 0  ),
  INST(Uhasx           , undefined              , undefined                 , 1328, 0  ),
  INST(Uhsax           , undefined              , undefined                 , 1334, 0  ),
  INST(Uhsub16         , undefined              , undefined                 , 1340, 0  ),
  INST(Uhsub8          , undefined              , undefined                 , 1348, 0  ),
  INST(Umaal           , undefined              , undefined                 , 1355, 0  ),
  INST(Umlal           , undefined              , undefined                 , 1361, 0  ),
  INST(Umlals          , undefined              , undefined                 , 1367, 0  ),
  INST(Umull           , undefined              , undefined                 , 1374, 0  ),
  INST(Umulls          , undefined              , undefined                 , 1380, 0  ),
  INST(Uqadd16         , undefined              , undefined                 , 1387, 0  ),
  INST(Uqadd8          , undefined              , undefined                 , 1395, 0  ),
  INST(Uqasx           , undefined              , undefined                 , 1402, 0  ),
  INST(Uqsax           , undefined              , undefined                 , 1408, 0  ),
  INST(Uqsub16         , undefined              , undefined                 , 1414, 0  ),
  INST(Uqsub8          , undefined              , undefined                 , 1422, 0  ),
  INST(Usad8           , undefined              , undefined                 , 1429, 0  ),
  INST(Usada8          , undefined              , undefined                 , 1435, 0  ),
  INST(Usat            , undefined              , undefined                 , 1442, 0  ),
  INST(Usat16          , undefined              , undefined                 , 1447, 0  ),
  INST(Usax            , undefined              , undefined                 , 1454, 0  ),
  INST(Usub16          , undefined              , undefined                 , 1459, 0  ),
  INST(Usub8           , undefined              , undefined                 , 1466, 0  ),
  INST(Uxtab           , undefined              , undefined                 , 1472, 0  ),
  INST(Uxtab16         , undefined              , undefined                 , 1478, 0  ),
  INST(Uxtah           , undefined              , undefined                 , 1486, 0  ),
  INST(Uxtb            , undefined              , undefined                 , 1492, 0  ),
  INST(Uxtb16          , undefined              , undefined                 , 1497, 0  ),
  INST(Uxth            , undefined              , undefined                 , 1504, 0  ),
  INST(Vaba            , undefined              , undefined                 , 1509, 0  ),
  INST(Vabal           , undefined              , undefined                 , 1514, 0  ),
  INST(Vabd            , undefined              , undefined                 , 1520, 0  ),
  INST(Vabdl           , undefined              , undefined                 , 1525, 0  ),
  INST(Vabs            , undefined              , undefined                 , 1531, 0  ),
  INST(Vacge           , undefined              , undefined                 , 1536, 0  ),
  INST(Vacgt           , undefined              , undefined                 , 1542, 0  ),
  INST(Vacle           , undefined              , undefined                 , 1548, 0  ),
  INST(Vaclt           , undefined              , undefined                 , 1554, 0  ),
  INST(Vadd            , undefined              , undefined                 , 1560, 0  ),
  INST(Vaddhn          , undefined              , undefined                 , 1565, 0  ),
  INST(Vaddl           , undefined              , undefined                 , 1572, 0  ),
  INST(Vaddw           , undefined              , undefined                 , 1578, 0  ),
  INST(Vand            , undefined              , undefined                 , 1584, 0  ),
  INST(Vbic            , undefined              , undefined                 , 1589, 0  ),
  INST(Vbif            , undefined              , undefined                 , 1594, 0  ),
  INST(Vbit            , undefined              , undefined                 , 1599, 0  ),
  INST(Vbsl            , undefined              , undefined                 , 1604, 0  ),
  INST(Vceq            , undefined              , undefined                 , 1609, 0  ),
  INST(Vcge            , undefined              , undefined                 , 1614, 0  ),
  INST(Vcgt            , undefined              , undefined                 , 1619, 0  ),
  INST(Vcle            , undefined              , undefined                 , 1624, 0  ),
  INST(Vcls            , undefined              , undefined                 , 1629, 0  ),
  INST(Vclt            , undefined              , undefined                 , 1634, 0  ),
  INST(Vclz            , undefined              , undefined                 , 1639, 0  ),
  INST(Vcmp            , undefined              , undefined                 , 1644, 0  ),
  INST(Vcmpe           , undefined              , undefined                 , 1649, 0  ),
  INST(Vcnt            , undefined              , undefined                 , 1655, 0  ),
  INST(Vcvt            , undefined              , undefined                 , 1660, 0  ),
  INST(Vcvta           , undefined              , undefined                 , 1665, 0  ),
  INST(Vcvtb           , undefined              , undefined                 , 1671, 0  ),
  INST(Vcvtm           , undefined              , undefined                 , 1677, 0  ),
  INST(Vcvtn           , undefined              , undefined                 , 1683, 0  ),
  INST(Vcvtp           , undefined              , undefined                 , 1689, 0  ),
  INST(Vcvtr           , undefined              , undefined                 , 1695, 0  ),
  INST(Vcvtt           , undefined              , undefined                 , 1701, 0  ),
  INST(Vdiv            , undefined              , undefined                 , 1707, 0  ),
  INST(Vdup            , undefined              , undefined                 , 1712, 0  ),
  INST(Veor            , undefined              , undefined                 , 1717, 0  ),
  INST(Vext            , undefined              , undefined                 , 1722, 0  ),
  INST(Vfma            , undefined              , undefined                 , 1727, 0  ),
  INST(Vfms            , undefined              , undefined                 , 1732, 0  ),
  INST(Vfnma           , undefined              , undefined                 , 1737, 0  ),
  INST(Vfnms           , undefined              , undefined                 , 1743, 0  ),
  INST(Vhadd           , undefined              , undefined                 , 1749, 0  ),
  INST(Vhsub           , undefined              , undefined                 , 1755, 0  ),
  INST(Vmax            , undefined              , undefined                 , 1761, 0  ),
  INST(Vmaxnm          , undefined              , undefined                 , 1766, 0  ),
  INST(Vmin            , undefined              , undefined                 , 1773, 0  ),
  INST(Vminnm          , undefined              , undefined                 , 1778, 0  ),
  INST(Vmla            , undefined              , undefined                 , 1785, 0  ),
  INST(Vmlal           , undefined              , undefined                 , 1790, 0  ),
  INST(Vmls            , undefined              , undefined                 , 1796, 0  ),
  INST(Vmlsl           , undefined              , undefined                 , 1801, 0  ),
  INST(Vmov            , undefined              , undefined                 , 1807, 0  ),
  INST(Vmovl           , undefined              , undefined                 , 1812, 0  ),
  INST(Vmovn           , undefined              , undefined                 , 1818, 0  ),
  INST(Vmul            , undefined              , undefined                 , 1824, 0  ),
  INST(Vmull           , undefined              , undefined                 , 1829, 0  ),
  INST(Vmvn            , undefined              , undefined                 , 1835, 0  ),
  INST(Vneg            , undefined              , undefined                 , 1840, 0  ),
  INST(Vnmla           , undefined              , undefined                 , 1845, 0  ),
  INST(Vnmls           , undefined              , undefined                 , 1851, 0  ),
  INST(Vnmul           , undefined              , undefined                 , 1857, 0  ),
  INST(Vorn            , undefined              , undefined                 , 1863, 0  ),
  INST(Vorr            , undefined              , undefined                 , 1868, 0  ),
  INST(Vpadal          , undefined              , undefined                 , 1873, 0  ),
  INST(Vpadd           , undefined              , undefined                 , 1880, 0  ),
  INST(Vpaddl          , undefined              , undefined                 , 1886, 0  ),
  INST(Vpmax           , undefined              , undefined                 , 1893, 0  ),
  INST(Vpmin           , undefined              , undefined                 , 1899, 0  ),
  INST(Vqabs           , undefined              , undefined                 , 1905, 0  ),
  INST(Vqadd           , undefined              , undefined                 , 1911, 0  ),
  INST(Vqdmlal         , undefined              , undefined                 , 1917, 0  ),
  INST(Vqdmlsl         , undefined              , undefined                 , 1925, 0  ),
  INST(Vqdmulh         , undefined              , undefined                 , 1933, 0  ),
  INST(Vqdmull         , undefined              , undefined                 , 1941, 0  ),
  INST(Vqmovn          , undefined              , undefined                 , 1949, 0  ),
  INST(Vqmovun         , undefined              , undefined                 , 1956, 0  ),
  INST(Vqneg           , undefined              , undefined                 , 1964, 0  ),
  INST(Vqrdmulh        , undefined              , undefined                 , 1970, 0  ),
  INST(Vqrshl          , undefined              , undefined                 , 1979, 0  ),
  INST(Vqrshrn         , undefined              , undefined                 , 1986, 0  ),
  INST(Vqrshrun        , undefined              , undefined                 , 1994, 0  ),
  INST(Vqshl           , undefined              , undefined                 , 2003, 0  ),
  INST(Vqshlu          , undefined              , undefined                 , 2009, 0  ),
  INST(Vqshrn          , undefined              , undefined                 , 2016, 0  ),
  INST(Vqshrun         , undefined              , undefined                 , 2023, 0  ),
  INST(Vqsub           , undefined              , undefined                 , 2031, 0  ),
  INST(Vraddhn         , undefined              , undefined                 , 2037, 0  ),
  INST(Vrecpe          , undefined              , undefined                 , 2045, 0  ),
  INST(Vrecps          , undefined              , undefined                 , 2052, 0  ),
  INST(Vrev16          , undefined              , undefined                 , 2059, 0  ),
  INST(Vrev32          , undefined              , undefined                 , 2066, 0  ),
  INST(Vrev64          , undefined              , undefined                 , 2073, 0  ),
  INST(Vrhadd          , undefined              , undefined                 , 2080, 0  ),
  INST(Vrinta          , undefined              , undefined                 , 2087, 0  ),
  INST(Vrintm          , undefined              , undefined                 , 2094, 0  ),
  INST(Vrintn          , undefined              , undefined                 , 2101, 0  ),
  INST(Vrintp          , undefined              , undefined                 , 2108, 0  ),
  INST(Vrintr          , undefined              , undefined                 , 2115, 0  ),
  INST(Vrintx          , undefined              , undefined                 , 2122, 0  ),
  INST(Vrintz          , undefined              , undefined                 , 2129, 0  ),
  INST(Vrshl           , undefined              , undefined                 , 2136, 0  ),
  INST(Vrshr           , undefined              , undefined                 , 2142, 0  ),
  INST(Vrshrn          , undefined              , undefined                 , 2148, 0  ),
  INST(Vrsqrte         , undefined              , undefined                 , 2155, 0  ),
  INST(Vrsqrts         , undefined              , undefined                 , 2163, 0  ),
  INST(Vrsra           , undefined              , undefined                 , 2171, 0  ),
  INST(Vrsubhn         , undefined              , undefined                 , 2177, 0  ),
  INST(Vseleq          , undefined              , undefined                 , 2185, 0  ),
  INST(Vselge          , undefined              , undefined                 , 2192, 0  ),
  INST(Vselgt          , undefined              , undefined                 , 2199, 0  ),
  INST(Vselvs          , undefined              , undefined                 , 2206, 0  ),
  INST(Vshl            , undefined              , undefined                 , 2213, 0  ),
  INST(Vshll           , undefined              , undefined                 , 2218, 0  ),
  INST(Vshr            , undefined              , undefined                 , 2224, 0  ),
  INST(Vshrl           , undefined              , undefined                 , 2229, 0  ),
  INST(Vsli            , undefined              , undefined                 , 2235, 0  ),
  INST(Vsqrt           , undefined              , undefined                 , 2240, 0  ),
  INST(Vsra            , undefined              , undefined                 , 2246, 0  ),
  INST(Vsri            , undefined              , undefined                 , 2251, 0  ),
  INST(Vsub            , undefined              , undefined                 , 2256, 0  ),
  INST(Vsubhn          , undefined              , undefined                 , 2261, 0  ),
  INST(Vsubl           , undefined              , undefined                 , 2268, 0  ),
  INST(Vsubw           , undefined              , undefined                 , 2274, 0  ),
  INST(Vswp            , undefined              , undefined                 , 2280, 0  ),
  INST(Vtbl            , undefined              , undefined                 , 2285, 0  ),
  INST(Vtbx            , undefined              , undefined                 , 2290, 0  ),
  INST(Vtrn            , undefined              , undefined                 , 2295, 0  ),
  INST(Vtst            , undefined              , undefined                 , 2300, 0  ),
  INST(Vuzp            , undefined              , undefined                 , 2305, 0  ),
  INST(Vzip            , undefined              , undefined                 , 2310, 0  ),
  INST(Wfe             , undefined              , undefined                 , 2315, 0  ),
  INST(Wfi             , undefined              , undefined                 , 2319, 0  ),
  INST(Yield           , undefined              , undefined                 , 2323, 0  )
  // ${instData:End}
};

#undef NAME_DATA_INDEX
#undef INST

// ${commonData:Begin}
// ------------------- Automatically generated, do not edit -------------------
const ArmInst::CommonData ArmInstDB::commonData[] = {
  { 0}  // #0
};
// ----------------------------------------------------------------------------
// ${commonData:End}

// ============================================================================
// [asmjit::ArmUtil - Id <-> Name]
// ============================================================================

#if !defined(ASMJIT_DISABLE_TEXT)
// ${nameData:Begin}
// ------------------- Automatically generated, do not edit -------------------
const char ArmInstDB::nameData[] =
  "\0" "adc\0" "adcs\0" "adds\0" "adr\0" "aesd\0" "aese\0" "aesimc\0" "aesmc\0"
  "ands\0" "asr\0" "asrs\0" "bfc\0" "bfi\0" "bics\0" "bkpt\0" "blx\0" "bxj\0"
  "cbnz\0" "cbz\0" "clrex\0" "cmn\0" "cpsid\0" "cpsie\0" "crc32b\0" "crc32cb\0"
  "crc32ch\0" "crc32cw\0" "crc32h\0" "crc32w\0" "dbg\0" "dmb\0" "dsb\0"
  "eors\0" "eret\0" "fldmdbx\0" "fldmiax\0" "fstmdbx\0" "fstmiax\0" "hlt\0"
  "hvc\0" "isb\0" "it{x}{y}{z}\0" "lda\0" "ldab\0" "ldaex\0" "ldaexb\0"
  "ldaexd\0" "ldaexh\0" "ldah\0" "ldm\0" "ldmda\0" "ldmdb\0" "ldmib\0" "ldr\0"
  "ldrb\0" "ldrbt\0" "ldrd\0" "ldrex\0" "ldrexb\0" "ldrexd\0" "ldrexh\0"
  "ldrh\0" "ldrht\0" "ldrsb\0" "ldrsbt\0" "ldrsh\0" "ldrsht\0" "ldrt\0"
  "lsls\0" "lsrs\0" "mcr\0" "mcr2\0" "mcrr\0" "mcrr2\0" "mlas\0" "movs\0"
  "movt\0" "movw\0" "mrc\0" "mrc2\0" "mrrc\0" "mrrc2\0" "mrs\0" "msr\0"
  "muls\0" "mvns\0" "nop\0" "orns\0" "orrs\0" "pkhbt\0" "pkhtb\0" "pld\0"
  "pldw\0" "pli\0" "pop\0" "push\0" "qdadd\0" "qdsub\0" "rbit\0" "rev\0"
  "revsh\0" "rfe\0" "rfeda\0" "rfedb\0" "rfeib\0" "ror\0" "rors\0" "rrx\0"
  "rrxs\0" "rsbs\0" "rsc\0" "rscs\0" "sadd16\0" "sadd8\0" "sasx\0" "sbc\0"
  "sbcs\0" "sbfx\0" "sdiv\0" "sel\0" "setend\0" "sev\0" "sevl\0" "sha1c\0"
  "sha1h\0" "sha1m\0" "sha1p\0" "sha1su0\0" "sha1su1\0" "sha256h\0"
  "sha256h2\0" "sha256su0\0" "sha256su1\0" "shadd16\0" "shadd8\0" "shasx\0"
  "shsax\0" "shsub16\0" "shsub8\0" "smlabb\0" "smlabt\0" "smlad\0" "smladx\0"
  "smlal\0" "smlalbb\0" "smlalbt\0" "smlald\0" "smlaldx\0" "smlals\0"
  "smlaltb\0" "smlaltt\0" "smlatb\0" "smlatt\0" "smlawb\0" "smlawt\0" "smlsd\0"
  "smlsdx\0" "smlsld\0" "smlsldx\0" "smmla\0" "smmlar\0" "smmls\0" "smmlsr\0"
  "smmul\0" "smmulr\0" "smuad\0" "smuadx\0" "smulbb\0" "smulbt\0" "smull\0"
  "smulls\0" "smultb\0" "smultt\0" "smulwb\0" "smulwt\0" "smusd\0" "smusdx\0"
  "srsda\0" "srsdb\0" "srsib\0" "ssat\0" "ssat16\0" "ssax\0" "ssub16\0"
  "ssub8\0" "stl\0" "stlb\0" "stlex\0" "stlexb\0" "stlexd\0" "stlexh\0"
  "stlh\0" "stm\0" "stmda\0" "stmdb\0" "stmib\0" "str\0" "strb\0" "strbt\0"
  "strd\0" "strex\0" "strexb\0" "strexd\0" "strexh\0" "strh\0" "strht\0"
  "strt\0" "subs\0" "svc\0" "swpb\0" "sxtab\0" "sxtab16\0" "sxtah\0" "sxtb\0"
  "sxtb16\0" "sxth\0" "tbb\0" "tbh\0" "teq\0" "uadd16\0" "uadd8\0" "uasx\0"
  "ubfx\0" "udf\0" "udiv\0" "uhadd16\0" "uhadd8\0" "uhasx\0" "uhsax\0"
  "uhsub16\0" "uhsub8\0" "umaal\0" "umlal\0" "umlals\0" "umull\0" "umulls\0"
  "uqadd16\0" "uqadd8\0" "uqasx\0" "uqsax\0" "uqsub16\0" "uqsub8\0" "usad8\0"
  "usada8\0" "usat\0" "usat16\0" "usax\0" "usub16\0" "usub8\0" "uxtab\0"
  "uxtab16\0" "uxtah\0" "uxtb\0" "uxtb16\0" "uxth\0" "vaba\0" "vabal\0"
  "vabd\0" "vabdl\0" "vabs\0" "vacge\0" "vacgt\0" "vacle\0" "vaclt\0" "vadd\0"
  "vaddhn\0" "vaddl\0" "vaddw\0" "vand\0" "vbic\0" "vbif\0" "vbit\0" "vbsl\0"
  "vceq\0" "vcge\0" "vcgt\0" "vcle\0" "vcls\0" "vclt\0" "vclz\0" "vcmp\0"
  "vcmpe\0" "vcnt\0" "vcvt\0" "vcvta\0" "vcvtb\0" "vcvtm\0" "vcvtn\0" "vcvtp\0"
  "vcvtr\0" "vcvtt\0" "vdiv\0" "vdup\0" "veor\0" "vext\0" "vfma\0" "vfms\0"
  "vfnma\0" "vfnms\0" "vhadd\0" "vhsub\0" "vmax\0" "vmaxnm\0" "vmin\0"
  "vminnm\0" "vmla\0" "vmlal\0" "vmls\0" "vmlsl\0" "vmov\0" "vmovl\0" "vmovn\0"
  "vmul\0" "vmull\0" "vmvn\0" "vneg\0" "vnmla\0" "vnmls\0" "vnmul\0" "vorn\0"
  "vorr\0" "vpadal\0" "vpadd\0" "vpaddl\0" "vpmax\0" "vpmin\0" "vqabs\0"
  "vqadd\0" "vqdmlal\0" "vqdmlsl\0" "vqdmulh\0" "vqdmull\0" "vqmovn\0"
  "vqmovun\0" "vqneg\0" "vqrdmulh\0" "vqrshl\0" "vqrshrn\0" "vqrshrun\0"
  "vqshl\0" "vqshlu\0" "vqshrn\0" "vqshrun\0" "vqsub\0" "vraddhn\0" "vrecpe\0"
  "vrecps\0" "vrev16\0" "vrev32\0" "vrev64\0" "vrhadd\0" "vrinta\0" "vrintm\0"
  "vrintn\0" "vrintp\0" "vrintr\0" "vrintx\0" "vrintz\0" "vrshl\0" "vrshr\0"
  "vrshrn\0" "vrsqrte\0" "vrsqrts\0" "vrsra\0" "vrsubhn\0" "vseleq\0"
  "vselge\0" "vselgt\0" "vselvs\0" "vshl\0" "vshll\0" "vshr\0" "vshrl\0"
  "vsli\0" "vsqrt\0" "vsra\0" "vsri\0" "vsub\0" "vsubhn\0" "vsubl\0" "vsubw\0"
  "vswp\0" "vtbl\0" "vtbx\0" "vtrn\0" "vtst\0" "vuzp\0" "vzip\0" "wfe\0"
  "wfi\0" "yield";

enum {
  kArmInstMaxLength = 11
};

struct InstNameAZ {
  uint16_t start;
  uint16_t end;
};

static const InstNameAZ ArmInstNameAZ[26] = {
  { ArmInst::kIdAdc       , ArmInst::kIdAsrs       + 1 },
  { ArmInst::kIdB         , ArmInst::kIdBxj        + 1 },
  { ArmInst::kIdCbnz      , ArmInst::kIdCrc32w     + 1 },
  { ArmInst::kIdDbg       , ArmInst::kIdDsb        + 1 },
  { ArmInst::kIdEor       , ArmInst::kIdEret       + 1 },
  { ArmInst::kIdFldmdbx   , ArmInst::kIdFstmiax    + 1 },
  { ArmInst::kIdNone      , ArmInst::kIdNone       + 1 },
  { ArmInst::kIdHlt       , ArmInst::kIdHvc        + 1 },
  { ArmInst::kIdIsb       , ArmInst::kIdIt{x}{y}{z} + 1 },
  { ArmInst::kIdNone      , ArmInst::kIdNone       + 1 },
  { ArmInst::kIdNone      , ArmInst::kIdNone       + 1 },
  { ArmInst::kIdLda       , ArmInst::kIdLsrs       + 1 },
  { ArmInst::kIdMcr       , ArmInst::kIdMvns       + 1 },
  { ArmInst::kIdNop       , ArmInst::kIdNop        + 1 },
  { ArmInst::kIdOrn       , ArmInst::kIdOrrs       + 1 },
  { ArmInst::kIdPkhbt     , ArmInst::kIdPush       + 1 },
  { ArmInst::kIdQadd      , ArmInst::kIdQsub8      + 1 },
  { ArmInst::kIdRbit      , ArmInst::kIdRscs       + 1 },
  { ArmInst::kIdSadd16    , ArmInst::kIdSxth       + 1 },
  { ArmInst::kIdTbb       , ArmInst::kIdTst        + 1 },
  { ArmInst::kIdUadd16    , ArmInst::kIdUxth       + 1 },
  { ArmInst::kIdVaba      , ArmInst::kIdVzip       + 1 },
  { ArmInst::kIdWfe       , ArmInst::kIdWfi        + 1 },
  { ArmInst::kIdNone      , ArmInst::kIdNone       + 1 },
  { ArmInst::kIdYield     , ArmInst::kIdYield      + 1 },
  { ArmInst::kIdNone      , ArmInst::kIdNone       + 1 }
};
// ----------------------------------------------------------------------------
// ${nameData:End}

uint32_t ArmInst::getIdByName(const char* name, size_t len) noexcept {
  if (ASMJIT_UNLIKELY(!name))
    return Globals::kInvalidInst;

  if (len == Globals::kInvalidIndex)
    len = ::strlen(name);

  if (ASMJIT_UNLIKELY(len == 0 || len > kArmInstMaxLength))
    return Globals::kInvalidInst;

  uint32_t prefix = static_cast<uint32_t>(name[0]) - kArmInstAlphaIndexFirst;
  if (ASMJIT_UNLIKELY(prefix > kArmInstAlphaIndexLast - kArmInstAlphaIndexFirst))
    return Globals::kInvalidInst;

  uint32_t index = ArmInstNameAZ[prefix].start;
  if (ASMJIT_UNLIKELY(!index))
    return Globals::kInvalidInst;

  const char* nameData = ArmInstDB::nameData;
  const ArmInst* instData = ArmInstDB::instData;

  const ArmInst* base = instData + index;
  const ArmInst* end  = instData + ArmInstNameAZ[prefix].end;

  for (size_t lim = (size_t)(end - base); lim != 0; lim >>= 1) {
    const ArmInst* cur = base + (lim >> 1);
    int result = Utils::cmpInstName(nameData + cur[0].getNameDataIndex(), name, len);

    if (result < 0) {
      base = cur + 1;
      lim--;
      continue;
    }

    if (result > 0)
      continue;

    return static_cast<uint32_t>((size_t)(cur - instData));
  }

  return Globals::kInvalidInst;
}

const char* ArmInst::getNameById(uint32_t id) noexcept {
  if (ASMJIT_UNLIKELY(id >= ArmInst::_kIdCount))
    return nullptr;
  return ArmInst::getInst(id).getName();
}
#else
const char ArmInstDB::nameData[] = "";
#endif // !ASMJIT_DISABLE_TEXT

// ============================================================================
// [asmjit::ArmUtil - Validation]
// ============================================================================

#if !defined(ASMJIT_DISABLE_VALIDATION)
ASMJIT_FAVOR_SIZE Error ArmInst::validate(
  uint32_t archType,
  uint32_t instId, uint32_t options,
  const Operand_& opExtra, const Operand_* opArray, uint32_t opCount) noexcept {

  // TODO: ARM.
  return kErrorOk;
}
#endif // !ASMJIT_DISABLE_VALIDATION

} // asmjit namespace

// [Api-End]
#include "../asmjit_apiend.h"

// [Guard]
#endif // ASMJIT_BUILD_ARM
