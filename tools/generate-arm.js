// [Generate-ARM]
//
// NOTE: This script relies on 'asmdb' package. Either install it by using
// node.js package manager (npm) or by copying/symlinking the whole asmdb
// directory as [asmjit]/tools/asmdb.
"use strict";

const base = require("./generate-base.js");
const hasOwn = Object.prototype.hasOwnProperty;
const kIndent = base.kIndent;
const StringUtils = base.StringUtils;

// ----------------------------------------------------------------------------
// [ArmDB]
// ----------------------------------------------------------------------------

// Create the ARM database.
const arm = base.asmdb.arm;
const armdb = new arm.DB().addDefault();

console.log(JSON.stringify(armdb.map, null, 2));

// ----------------------------------------------------------------------------
// [GenUtils]
// ----------------------------------------------------------------------------

class GenUtils {
  // Get group of instructions having the same name as understood by AsmJit.
  static groupOf(name) {
    return armdb.getGroup(name);
  }

  static archOf(group) {
    var t16Arch = false;
    var t32Arch = false;
    var a32Arch = false;
    var a64Arch = false;

    for (var i = 0; i < group.length; i++) {
      const inst = group[i];
      if (inst.encoding === "T16") t16Arch = true;
      if (inst.encoding === "T32") t32Arch = true;
      if (inst.encoding === "A32") a32Arch = true;
      if (inst.encoding === "A64") a64Arch = true;
    }

    var s = "";
    s += (!t16Arch && !t32Arch) ? " " : "T";
    s += (a32Arch) ? "A" : " ";
    return `[${s}]`;
  }
}

// ----------------------------------------------------------------------------
// [ArmGenerator]
// ----------------------------------------------------------------------------

class ArmGenerator extends base.BaseGenerator {
  constructor() {
    super("Arm");

    this.load([
      "src/asmjit/arm/arminst.cpp",
      "src/asmjit/arm/arminst.h"
    ]);
  }

  // --------------------------------------------------------------------------
  // [Parse]
  // --------------------------------------------------------------------------

  parse() {
    this.addInst({
      id: 0,
      name: "",
      enum: "None"
    });

    var names = armdb.getInstructionNames();
    for (var i = 0; i < names.length; i++) {
      const name = names[i];
      const group = GenUtils.groupOf(names[i]);

      const enum_ = StringUtils.upFirst(name);

      this.addInst({
        id            : 0,             // Instruction id (numeric value).
        name          : name,          // Instruction name.
        enum          : enum_,         // Instruction enum without `kId` prefix.

        familyType    : "kFamilyNone", // Family type.
        familyIndex   : 0,             // Index to a family-specific data.

        nameIndex     : -1,            // Instruction name-index.
        commonIndex   : -1
      });
    }

    console.log("Number of Instructions: " + this.instArray.length);
  }

  // --------------------------------------------------------------------------
  // [Generate]
  // --------------------------------------------------------------------------

  generate() {
    // Order doesn't matter here.
    this.generateIdData();
    this.generateNameData();

    // These must be last, and order matters.
    this.generateCommonData();
    this.generateInstData();

    return this;
  }

  // --------------------------------------------------------------------------
  // [Generate - CommonData]
  // --------------------------------------------------------------------------

  generateCommonData() {
    const table = new base.IndexedArray();
    for (var i = 0; i < this.instArray.length; i++) {
      const inst = this.instArray[i];

      const item = "{ " + StringUtils.padLeft("0", 1) + "}";
      inst.commonIndex = table.addIndexed(item);
    }

    var s = `const ArmInst::CommonData ArmInstDB::commonData[] = {\n${StringUtils.format(table, kIndent, true)}\n};\n`;
    return this.inject("commonData", StringUtils.disclaimer(s), table.length * 12);
  }

  // --------------------------------------------------------------------------
  // [Generate - InstData]
  // --------------------------------------------------------------------------

  generateInstData() {
    var s = StringUtils.format(this.instArray, "", false, function(inst) {
      return "INST(" +
        StringUtils.padLeft(inst.enum       , 16) + ", " +
        StringUtils.padLeft(inst.encoding   , 23) + ", " +
        StringUtils.padLeft(inst.opcode0    , 26) + ", " +
        StringUtils.padLeft(inst.nameIndex  ,  4) + ", " +
        StringUtils.padLeft(inst.commonIndex,  3) + ")";
    }) + "\n";
    return this.inject("instData", s, this.instArray.length * 12);
  }

  // --------------------------------------------------------------------------
  // [Reimplement]
  // --------------------------------------------------------------------------

  getCommentOf(name) {
    var group = GenUtils.groupOf(name);
    if (!group) return "";

    //var features = GenUtils.featuresOf(group);
    var comment = GenUtils.archOf(group);
    /*
    if (features.length) {
      if (comment === "ANY")
        comment = "";
      else
        comment += " ";

      const vl = features.indexOf("AVX512VL");
      if (vl !== -1) features.splice(vl, 1);
      comment += features.join("|");
      if (vl !== -1) comment += " (VL)";
    }
    */

    return comment;
  }
}

// ----------------------------------------------------------------------------
// [Main]
// ----------------------------------------------------------------------------

function main() {
  const gen = new ArmGenerator();

  gen.parse();
  gen.generate();
  gen.dumpTableSizes();
  gen.save();
}
main();
