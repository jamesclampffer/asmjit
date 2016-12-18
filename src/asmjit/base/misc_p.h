// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_BASE_MISC_P_H
#define _ASMJIT_BASE_MISC_P_H

// [Dependencies]
#include "../asmjit_build.h"

// [Api-Begin]
#include "../asmjit_apibegin.h"

namespace asmjit {

//! \addtogroup asmjit_base
//! \{

//! \internal
//!
//! Macro used to populate a table with 16 elements starting at `I`.
#define ASMJIT_TABLE_16(DEF, I) DEF((I*16) +  0), DEF((I*16) +  1), DEF((I*16) +  2), DEF((I*16) +  3), \
                                DEF((I*16) +  4), DEF((I*16) +  5), DEF((I*16) +  6), DEF((I*16) +  7), \
                                DEF((I*16) +  8), DEF((I*16) +  9), DEF((I*16) + 10), DEF((I*16) + 11), \
                                DEF((I*16) + 12), DEF((I*16) + 13), DEF((I*16) + 14), DEF((I*16) + 15)

//! \}

} // asmjit namespace

//! \}

// [Api-End]
#include "../asmjit_apiend.h"

// [Guard]
#endif // _ASMJIT_BASE_MISC_P_H
