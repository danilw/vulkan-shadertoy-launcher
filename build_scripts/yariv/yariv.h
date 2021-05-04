// The latest version of this library is available on GitHub;
// https://github.com/sheredom/yari-v

// This is free and unencumbered software released into the public domain.
//
// Anyone is free to copy, modify, publish, use, compile, sell, or
// distribute this software, either in source code form or as a compiled
// binary, for any purpose, commercial or non-commercial, and by any
// means.
//
// In jurisdictions that recognize copyright laws, the author or authors
// of this software dedicate any and all copyright interest in the
// software to the public domain. We make this dedication for the benefit
// of the public at large and to the detriment of our heirs and
// successors. We intend this dedication to be an overt act of
// relinquishment in perpetuity of all present and future rights to this
// software under copyright law.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
//
// For more information, please refer to <http://unlicense.org/>

#ifndef SHEREDOM_YARIV_H_INCLUDED
#define SHEREDOM_YARIV_H_INCLUDED

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

enum yariv_encode_flags_e {
  yariv_encode_flags_default = 0,

  // enable the SPIR-V to be stripped of all non-essential information
  yariv_encode_flags_strip = 0x1
};

#if defined(__clang__) || defined(__GNUC__)
#define yariv_nonnull __attribute__((nonnull))
#define yariv_weak __attribute__((weak))
#elif defined(_MSC_VER)
#define yariv_nonnull
#define yariv_weak __inline
#else
#error Non clang, non gcc, non MSVC compiler found!
#endif

yariv_nonnull yariv_weak size_t yariv_encode_size(uint32_t encode_flags,
                                                  const void *spirv,
                                                  size_t spirv_size);

yariv_nonnull yariv_weak int yariv_encode(uint32_t encode_flags,
                                          void *out_yariv,
                                          size_t out_yariv_size,
                                          const void *spirv, size_t spirv_size);

yariv_nonnull yariv_weak size_t yariv_decode_size(const void *yariv,
                                                  size_t yariv_size);

yariv_nonnull yariv_weak int yariv_decode(void *out_spirv,
                                          size_t out_spirv_size,
                                          const void *yariv, size_t yariv_size);

yariv_weak size_t yariv_encode_signed_varint(int32_t i, uint8_t *out);

yariv_weak size_t yariv_decode_signed_varint(const uint8_t *in, int32_t *out);

yariv_weak size_t yariv_encode_unsigned_varint(uint32_t i, uint8_t *out);

yariv_weak size_t yariv_decode_unsigned_varint(const uint8_t *in,
                                               uint32_t *out);

yariv_weak uint32_t yariv_shuffle_opcode(uint32_t opcode);

yariv_weak int yariv_opcode_has_word_count(uint32_t opcode,
                                           uint32_t *word_count);

yariv_weak int yariv_opcode_has_id(uint32_t opcode);

yariv_weak int yariv_opcode_has_type_and_id(uint32_t opcode);

yariv_weak uint32_t yariv_opcode_maybe_split(uint32_t opcode,
                                             const uint32_t *spirv,
                                             uint32_t word_count);

yariv_weak uint32_t yariv_word_count_constant_part(uint32_t opcode);

yariv_weak uint32_t yariv_word_count_to_id_delta(uint32_t opcode,
                                                 uint32_t word_count);

#undef yariv_weak
#undef yariv_nonnull

enum YarivOpcode_s {
  YarivOpNop = 0,
  YarivOpUndef = 1,
  YarivOpSourceContinued = 2,
  YarivOpSource = 3,
  YarivOpSourceExtension = 4,
  YarivOpName = 5,
  YarivOpMemberName = 6,
  YarivOpString = 7,
  YarivOpLine = 8,
  YarivOp9 = 9, // There is a gap in the SPIR-V opcode numbering here
  YarivOpExtension = 10,
  YarivOpExtInstImport = 11,
  YarivOpExtInst = 12,
  YarivOp13 = 13, // There is a gap in the SPIR-V opcode numbering here
  YarivOpMemoryModel = 14,
  YarivOpEntryPoint = 15,
  YarivOpExecutionMode = 16,
  YarivOpCapability = 17,
  YarivOp18 = 18, // There is a gap in the SPIR-V opcode numbering here
  YarivOpTypeVoid = 19,
  YarivOpTypeBool = 20,
  YarivOpTypeInt = 21,
  YarivOpTypeFloat = 22,
  YarivOpTypeVector = 23,
  YarivOpTypeMatrix = 24,
  YarivOpTypeImage = 25,
  YarivOpTypeSampler = 26,
  YarivOpTypeSampledImage = 27,
  YarivOpTypeArray = 28,
  YarivOpTypeRuntimeArray = 29,
  YarivOpTypeStruct = 30,
  YarivOpTypeOpaque = 31,
  YarivOpTypePointer = 32,
  YarivOpTypeFunction = 33,
  YarivOpTypeEvent = 34,
  YarivOpTypeDeviceEvent = 35,
  YarivOpTypeReserveId = 36,
  YarivOpTypeQueue = 37,
  YarivOpTypePipe = 38,
  YarivOpTypeForwardPointer = 39,
  YarivOp40 = 40, // There is a gap in the SPIR-V opcode numbering here
  YarivOpConstantTrue = 41,
  YarivOpConstantFalse = 42,
  YarivOpConstant = 43,
  YarivOpConstantComposite = 44,
  YarivOpConstantSampler = 45,
  YarivOpConstantNull = 46,
  YarivOp47 = 47, // There is a gap in the SPIR-V opcode numbering here
  YarivOpSpecConstantTrue = 48,
  YarivOpSpecConstantFalse = 49,
  YarivOpSpecConstant = 50,
  YarivOpSpecConstantComposite = 51,
  YarivOpSpecConstantOp = 52,
  YarivOp53 = 53, // There is a gap in the SPIR-V opcode numbering here
  YarivOpFunction = 54,
  YarivOpFunctionParameter = 55,
  YarivOpFunctionEnd = 56,
  YarivOpFunctionCall = 57,
  YarivOp58 = 58, // There is a gap in the SPIR-V opcode numbering here
  YarivOpVariable = 59,
  YarivOpImageTexelPointer = 60,
  YarivOpLoad = 61,
  YarivOpStore = 62,
  YarivOpCopyMemory = 63,
  YarivOpCopyMemorySized = 64,
  YarivOpAccessChain = 65,
  YarivOpInBoundsAccessChain = 66,
  YarivOpPtrAccessChain = 67,
  YarivOpArrayLength = 68,
  YarivOpGenericPtrMemSemantics = 69,
  YarivOpInBoundsPtrAccessChain = 70,
  YarivOpDecorate = 71,
  YarivOpMemberDecorate = 72,
  YarivOpDecorationGroup = 73,
  YarivOpGroupDecorate = 74,
  YarivOpGroupMemberDecorate = 75,
  YarivOp76 = 76, // There is a gap in the SPIR-V opcode numbering here
  YarivOpVectorExtractDynamic = 77,
  YarivOpVectorInsertDynamic = 78,
  YarivOpVectorShuffle = 79,
  YarivOpCompositeConstruct = 80,
  YarivOpCompositeExtract = 81,
  YarivOpCompositeInsert = 82,
  YarivOpCopyObject = 83,
  YarivOpTranspose = 84,
  YarivOp85 = 85, // There is a gap in the SPIR-V opcode numbering here
  YarivOpSampledImage = 86,
  YarivOpImageSampleImplicitLod = 87,
  YarivOpImageSampleExplicitLod = 88,
  YarivOpImageSampleDrefImplicitLod = 89,
  YarivOpImageSampleDrefExplicitLod = 90,
  YarivOpImageSampleProjImplicitLod = 91,
  YarivOpImageSampleProjExplicitLod = 92,
  YarivOpImageSampleProjDrefImplicitLod = 93,
  YarivOpImageSampleProjDrefExplicitLod = 94,
  YarivOpImageFetch = 95,
  YarivOpImageGather = 96,
  YarivOpImageDrefGather = 97,
  YarivOpImageRead = 98,
  YarivOpImageWrite = 99,
  YarivOpImage = 100,
  YarivOpImageQueryFormat = 101,
  YarivOpImageQueryOrder = 102,
  YarivOpImageQuerySizeLod = 103,
  YarivOpImageQuerySize = 104,
  YarivOpImageQueryLod = 105,
  YarivOpImageQueryLevels = 106,
  YarivOpImageQuerySamples = 107,
  YarivOp108 = 108, // There is a gap in the SPIR-V opcode numbering here
  YarivOpConvertFToU = 109,
  YarivOpConvertFToS = 110,
  YarivOpConvertSToF = 111,
  YarivOpConvertUToF = 112,
  YarivOpUConvert = 113,
  YarivOpSConvert = 114,
  YarivOpFConvert = 115,
  YarivOpQuantizeToF16 = 116,
  YarivOpConvertPtrToU = 117,
  YarivOpSatConvertSToU = 118,
  YarivOpSatConvertUToS = 119,
  YarivOpConvertUToPtr = 120,
  YarivOpPtrCastToGeneric = 121,
  YarivOpGenericCastToPtr = 122,
  YarivOpGenericCastToPtrExplicit = 123,
  YarivOpBitcast = 124,
  YarivOp125 = 125, // There is a gap in the SPIR-V opcode numbering here
  YarivOpSNegate = 126,
  YarivOpFNegate = 127,
  YarivOpIAdd = 128,
  YarivOpFAdd = 129,
  YarivOpISub = 130,
  YarivOpFSub = 131,
  YarivOpIMul = 132,
  YarivOpFMul = 133,
  YarivOpUDiv = 134,
  YarivOpSDiv = 135,
  YarivOpFDiv = 136,
  YarivOpUMod = 137,
  YarivOpSRem = 138,
  YarivOpSMod = 139,
  YarivOpFRem = 140,
  YarivOpFMod = 141,
  YarivOpVectorTimesScalar = 142,
  YarivOpMatrixTimesScalar = 143,
  YarivOpVectorTimesMatrix = 144,
  YarivOpMatrixTimesVector = 145,
  YarivOpMatrixTimesMatrix = 146,
  YarivOpOuterProduct = 147,
  YarivOpDot = 148,
  YarivOpIAddCarry = 149,
  YarivOpISubBorrow = 150,
  YarivOpUMulExtended = 151,
  YarivOpSMulExtended = 152,
  YarivOpAny = 154,
  YarivOpAll = 155,
  YarivOpIsNan = 156,
  YarivOpIsInf = 157,
  YarivOpIsFinite = 158,
  YarivOpIsNormal = 159,
  YarivOpSignBitSet = 160,
  YarivOpLessOrGreater = 161,
  YarivOpOrdered = 162,
  YarivOpUnordered = 163,
  YarivOpLogicalEqual = 164,
  YarivOpLogicalNotEqual = 165,
  YarivOpLogicalOr = 166,
  YarivOpLogicalAnd = 167,
  YarivOpLogicalNot = 168,
  YarivOpSelect = 169,
  YarivOpIEqual = 170,
  YarivOpINotEqual = 171,
  YarivOpUGreaterThan = 172,
  YarivOpSGreaterThan = 173,
  YarivOpUGreaterThanEqual = 174,
  YarivOpSGreaterThanEqual = 175,
  YarivOpULessThan = 176,
  YarivOpSLessThan = 177,
  YarivOpULessThanEqual = 178,
  YarivOpSLessThanEqual = 179,
  YarivOpFOrdEqual = 180,
  YarivOpFUnordEqual = 181,
  YarivOpFOrdNotEqual = 182,
  YarivOpFUnordNotEqual = 183,
  YarivOpFOrdLessThan = 184,
  YarivOpFUnordLessThan = 185,
  YarivOpFOrdGreaterThan = 186,
  YarivOpFUnordGreaterThan = 187,
  YarivOpFOrdLessThanEqual = 188,
  YarivOpFUnordLessThanEqual = 189,
  YarivOpFOrdGreaterThanEqual = 190,
  YarivOpFUnordGreaterThanEqual = 191,
  YarivOpShiftRightLogical = 194,
  YarivOpShiftRightArithmetic = 195,
  YarivOpShiftLeftLogical = 196,
  YarivOpBitwiseOr = 197,
  YarivOpBitwiseXor = 198,
  YarivOpBitwiseAnd = 199,
  YarivOpNot = 200,
  YarivOpBitFieldInsert = 201,
  YarivOpBitFieldSExtract = 202,
  YarivOpBitFieldUExtract = 203,
  YarivOpBitReverse = 204,
  YarivOpBitCount = 205,
  YarivOpDPdx = 207,
  YarivOpDPdy = 208,
  YarivOpFwidth = 209,
  YarivOpDPdxFine = 210,
  YarivOpDPdyFine = 211,
  YarivOpFwidthFine = 212,
  YarivOpDPdxCoarse = 213,
  YarivOpDPdyCoarse = 214,
  YarivOpFwidthCoarse = 215,
  YarivOpEmitVertex = 218,
  YarivOpEndPrimitive = 219,
  YarivOpEmitStreamVertex = 220,
  YarivOpEndStreamPrimitive = 221,
  YarivOpControlBarrier = 224,
  YarivOpMemoryBarrier = 225,
  YarivOpAtomicLoad = 227,
  YarivOpAtomicStore = 228,
  YarivOpAtomicExchange = 229,
  YarivOpAtomicCompareExchange = 230,
  YarivOpAtomicCompareExchangeWeak = 231,
  YarivOpAtomicIIncrement = 232,
  YarivOpAtomicIDecrement = 233,
  YarivOpAtomicIAdd = 234,
  YarivOpAtomicISub = 235,
  YarivOpAtomicSMin = 236,
  YarivOpAtomicUMin = 237,
  YarivOpAtomicSMax = 238,
  YarivOpAtomicUMax = 239,
  YarivOpAtomicAnd = 240,
  YarivOpAtomicOr = 241,
  YarivOpAtomicXor = 242,
  YarivOpPhi = 245,
  YarivOpLoopMerge = 246,
  YarivOpSelectionMerge = 247,
  YarivOpLabel = 248,
  YarivOpBranch = 249,
  YarivOpBranchConditional = 250,
  YarivOpSwitch = 251,
  YarivOpKill = 252,
  YarivOpReturn = 253,
  YarivOpReturnValue = 254,
  YarivOpUnreachable = 255,
  YarivOpLifetimeStart = 256,
  YarivOpLifetimeStop = 257,
  YarivOpGroupAsyncCopy = 259,
  YarivOpGroupWaitEvents = 260,
  YarivOpGroupAll = 261,
  YarivOpGroupAny = 262,
  YarivOpGroupBroadcast = 263,
  YarivOpGroupIAdd = 264,
  YarivOpGroupFAdd = 265,
  YarivOpGroupFMin = 266,
  YarivOpGroupUMin = 267,
  YarivOpGroupSMin = 268,
  YarivOpGroupFMax = 269,
  YarivOpGroupUMax = 270,
  YarivOpGroupSMax = 271,
  YarivOpReadPipe = 274,
  YarivOpWritePipe = 275,
  YarivOpReservedReadPipe = 276,
  YarivOpReservedWritePipe = 277,
  YarivOpReserveReadPipePackets = 278,
  YarivOpReserveWritePipePackets = 279,
  YarivOpCommitReadPipe = 280,
  YarivOpCommitWritePipe = 281,
  YarivOpIsValidReserveId = 282,
  YarivOpGetNumPipePackets = 283,
  YarivOpGetMaxPipePackets = 284,
  YarivOpGroupReserveReadPipePackets = 285,
  YarivOpGroupReserveWritePipePackets = 286,
  YarivOpGroupCommitReadPipe = 287,
  YarivOpGroupCommitWritePipe = 288,
  YarivOpEnqueueMarker = 291,
  YarivOpEnqueueKernel = 292,
  YarivOpGetKernelNDrangeSubGroupCount = 293,
  YarivOpGetKernelNDrangeMaxSubGroupSize = 294,
  YarivOpGetKernelWorkGroupSize = 295,
  YarivOpGetKernelPreferredWorkGroupSizeMultiple = 296,
  YarivOpRetainEvent = 297,
  YarivOpReleaseEvent = 298,
  YarivOpCreateUserEvent = 299,
  YarivOpIsValidEvent = 300,
  YarivOpSetUserEventStatus = 301,
  YarivOpCaptureEventProfilingInfo = 302,
  YarivOpGetDefaultQueue = 303,
  YarivOpBuildNDRange = 304,
  YarivOpImageSparseSampleImplicitLod = 305,
  YarivOpImageSparseSampleExplicitLod = 306,
  YarivOpImageSparseSampleDrefImplicitLod = 307,
  YarivOpImageSparseSampleDrefExplicitLod = 308,
  YarivOpImageSparseSampleProjImplicitLod = 309,
  YarivOpImageSparseSampleProjExplicitLod = 310,
  YarivOpImageSparseSampleProjDrefImplicitLod = 311,
  YarivOpImageSparseSampleProjDrefExplicitLod = 312,
  YarivOpImageSparseFetch = 313,
  YarivOpImageSparseGather = 314,
  YarivOpImageSparseDrefGather = 315,
  YarivOpImageSparseTexelsResident = 316,
  YarivOpNoLine = 317,
  YarivOpAtomicFlagTestAndSet = 318,
  YarivOpAtomicFlagClear = 319,
  YarivOpImageSparseRead = 320,

  // the following are not actual SPIR-V opcodes, but I want to generate them
  // and treat them as such in the encoder/decoder
  YarivOpLoadWithMemoryAccess = 500,
  YarivOpStoreWithMemoryAccess = 501,
  YarivOpDecorateWithNoLiterals = 502,
  YarivOpDecorateWithOneLiteral = 503,
  YarivOpMemberDecorateWithNoLiterals = 504,
  YarivOpMemberDecorateWithOneLiteral = 505,
  YarivOpVariableWithInitializer = 506,
  YarivOpConstantWithManyLiterals = 507,
  YarivOpAccessChainWithOneIndex = 508,
  YarivOpAccessChainWithTwoIndices = 509,
  YarivOpAccessChainWithThreeIndices = 510,
  YarivOpVectorShuffleTwoLiterals = 511,
  YarivOpVectorShuffleThreeLiterals = 512,
  YarivOpVectorShuffleFourLiterals = 513,
  YarivOp514 = 514,
  YarivOp515 = 515,
  YarivOp516 = 516,
  YarivOpVectorSwizzleTwoLiterals = 517,
  YarivOpVectorSwizzleThreeLiterals = 518,
  YarivOpVectorSwizzleFourLiterals = 519,
  YarivOpCompositeExtractWithLiteralZero = 520,
  YarivOpCompositeExtractWithLiteralOne = 521,
  YarivOpCompositeExtractWithLiteralTwo = 522,
  YarivOpCompositeExtractWithLiteralThree = 523,
  YarivOpCompositeConstructOneConstituent = 524,
  YarivOpCompositeConstructTwoConstituents = 525,
  YarivOpCompositeConstructThreeConstituents = 526,
  YarivOpDecorateRelaxedPrecision = 527,
  YarivOpMemberDecorateOffset = 528,
  YarivOpConstantWithOneFloatLiteral = 529,
  YarivOpDecorateLocation = 530
};

#if defined(__cplusplus)
#define YARIV_CAST(type, x) (static_cast<type>(x))
#else
#define YARIV_CAST(type, x) ((type)(x))
#endif

size_t yariv_encode_size(uint32_t encode_flags, const void *spirv,
                         size_t spirv_size) {
  size_t yariv_size = 0;
  size_t i, e;
  int32_t last_id = 1;
  const uint32_t type_offset = 1;

  // for the SPIR-V header
  // * we skip encoding SPIR-V's magic number at the beginning of the binary
  // * we use two bytes to encode the version number
  yariv_size += 2;
  // * we use varint encoding for the generators magic number
  yariv_size +=
      yariv_encode_unsigned_varint(YARIV_CAST(const uint32_t *, spirv)[2], 0);
  // * we use varint encoding for the bound (maximum ID used)
  yariv_size +=
      yariv_encode_unsigned_varint(YARIV_CAST(const uint32_t *, spirv)[3], 0);
  // * we skip encoding the instruction schema

  // iterate through all the opcodes
  for (i = 5, e = spirv_size / 4; i < e;) {
    const uint32_t *from = YARIV_CAST(const uint32_t *, spirv) + i;
    const uint32_t word_count = (*from) >> 16;
    uint32_t opcode = (*from) & 0xFFFFu;
    uint32_t k;
    int32_t maybe_update_id = last_id;

    // if we can strip opcodes
    if (yariv_encode_flags_strip & encode_flags) {
      switch (opcode) {
      default:
        break;
      case YarivOpNop:
      case YarivOpSourceContinued:
      case YarivOpSource:
      case YarivOpSourceExtension:
      case YarivOpName:
      case YarivOpMemberName:
      case YarivOpString:
      case YarivOpLine:
      case YarivOpNoLine:
        // all of these opcodes can be stripped, so we skip them!
        i += word_count;
        continue;
      }
    }

    opcode = yariv_opcode_maybe_split(opcode, from, word_count);

    // we encode the opcode first as a varint
    yariv_size += yariv_encode_unsigned_varint(yariv_shuffle_opcode(opcode), 0);

    // followed by the word_count as a varint
    if (!yariv_opcode_has_word_count(opcode, 0)) {
      // we encode word count minus the constant part of the word count (derived
      // from the opcode)
      yariv_size += yariv_encode_unsigned_varint(
          word_count - yariv_word_count_constant_part(opcode), 0);
    }

    // by default, we use unsigned varint encoding from the word immediately
    // following the opcode/word-count word
    k = 1;

    if (yariv_opcode_has_id(opcode)) {
      // if our opcode has an id, we want to use signed varint encoding
      // relative to the last id we used
      const int32_t id = YARIV_CAST(const int32_t *, spirv)[i + 1];
      yariv_size += yariv_encode_signed_varint(id - last_id, 0);
      maybe_update_id = id;

      // and since we have already encoded the first word, k should skip
      // another word for the remaining unsigned varint encoding
      k += 1;
    } else if (yariv_opcode_has_type_and_id(opcode)) {
      // if our opcode has a type and id, we want to use signed varint
      // encoding for the id relative to the last id we used
      const uint32_t type = YARIV_CAST(const uint32_t *, spirv)[i + 1];
      const int32_t id = YARIV_CAST(const int32_t *, spirv)[i + 2];

      yariv_size += yariv_encode_unsigned_varint(type - type_offset, 0);
      yariv_size += yariv_encode_signed_varint(id - last_id, 0);
      maybe_update_id = id;

      // and since we have already encoded the first two words, k should skip
      // another two words for the remaining unsigned varint encoding
      k += 2;
    }

    switch (opcode) {
    default:
      // then we encode some words as signed varints
      for (; k < yariv_word_count_to_id_delta(opcode, word_count); k++) {
        const int32_t id = YARIV_CAST(const int32_t *, spirv)[i + k];
        yariv_size += yariv_encode_signed_varint(id - last_id, 0);
      }

      // then we encode the remaining words as unsigned varints
      for (; k < word_count; k++) {
        yariv_size += yariv_encode_unsigned_varint(
            YARIV_CAST(const uint32_t *, spirv)[i + k], 0);
      }
      break;
    case YarivOpExtInstImport:
      // our literal string can just be memcpy'ed over
      yariv_size += sizeof(uint32_t) * (word_count - k);
      break;
    case YarivOpVectorShuffleTwoLiterals:
    case YarivOpVectorShuffleThreeLiterals:
    case YarivOpVectorShuffleFourLiterals: {
      const int32_t vecid1 = YARIV_CAST(const int32_t *, spirv)[i + k++];
      const int32_t vecid2 = YARIV_CAST(const int32_t *, spirv)[i + k++];

      // we use the signed varint encoding relative to last id for the two
      // vectors we are shuffling (but we don't update last id crucially)
      yariv_size += yariv_encode_signed_varint(vecid1 - last_id, 0);
      yariv_size += yariv_encode_signed_varint(vecid2 - last_id, 0);

      // our literals are at most 3 bits each, and we have n of them
      if (YarivOpVectorShuffleTwoLiterals == opcode) {
        yariv_size += sizeof(uint8_t);
      } else {
        yariv_size += sizeof(uint8_t) * 2;
      }
    } break;
    case YarivOpVectorSwizzleTwoLiterals:
    case YarivOpVectorSwizzleThreeLiterals:
    case YarivOpVectorSwizzleFourLiterals: {
      // both our IDs are the same, so we output it only once
      const int32_t vecid1 = YARIV_CAST(const int32_t *, spirv)[i + k++];
      // skip the ID that is the same
      k++;

      // we use the signed varint encoding relative to last id for the two
      // vectors we are shuffling (but we don't update last id crucially)
      yariv_size += yariv_encode_signed_varint(vecid1 - last_id, 0);

      // our literals are at most 2 bits each, and we have n of them
      yariv_size += sizeof(uint8_t);
    } break;
    case YarivOpCompositeExtractWithLiteralZero:
    case YarivOpCompositeExtractWithLiteralOne:
    case YarivOpCompositeExtractWithLiteralTwo:
    case YarivOpCompositeExtractWithLiteralThree: {
      // the id of the composite we are extracting from
      const int32_t composite = YARIV_CAST(const int32_t *, spirv)[i + k++];

      // we use the signed varint encoding relative to last id for the two
      // vectors we are shuffling (but we don't update last id crucially)
      yariv_size += yariv_encode_signed_varint(composite - last_id, 0);

      // skip the literal that we are not going to encode
      k++;
    } break;
    case YarivOpDecorateRelaxedPrecision:
      // skip the decoration for relaxed precision that we are not going to
      // encode
      k++;
      break;
    case YarivOpMemberDecorateOffset:
      // encode the member literal as a varint
      yariv_size += yariv_encode_unsigned_varint(
          YARIV_CAST(const uint32_t *, spirv)[i + k++], 0);

      // skip the decoration for offset that we are not going to encode
      k++;

      // encode the byte offset as a varint
      yariv_size += yariv_encode_unsigned_varint(
          YARIV_CAST(const uint32_t *, spirv)[i + k++], 0);

      break;
    case YarivOpConstantWithOneFloatLiteral:
      // our literal is a float, so we just encode it using 4 bytes
      yariv_size += sizeof(uint32_t);
      break;
    case YarivOpDecorateLocation:
      // skip the decoration for location that we are not going to encode
      k++;

      // encode the location as a varint
      yariv_size += yariv_encode_unsigned_varint(
          YARIV_CAST(const uint32_t *, spirv)[i + k++], 0);

      break;
    }

    // bump i along
    i += word_count;

    // set last id to be the one we (might) have set
    last_id = maybe_update_id;
  }

  return yariv_size;
}

int yariv_encode(uint32_t encode_flags, void *out_yariv, size_t out_yariv_size,
                 const void *spirv, size_t spirv_size) {
  size_t i, e;
  int32_t last_id = 1;
  const uint32_t type_offset = 1;
  uint8_t *yariv = YARIV_CAST(uint8_t *, out_yariv);

  // FIXME: I should really check as I go that we don't try and write into
  // our_yariv beyond out_yariv_size!
  (void)out_yariv_size;

  // for the SPIR-V header
  // * encode the two bytes of the version number
  *yariv++ =
      YARIV_CAST(uint8_t, (YARIV_CAST(const uint32_t *, spirv)[1] >> 16));
  *yariv++ = YARIV_CAST(uint8_t, (YARIV_CAST(const uint32_t *, spirv)[1] >> 8));
  // * encode the varint for the generators magic number
  yariv += yariv_encode_unsigned_varint(YARIV_CAST(const uint32_t *, spirv)[2],
                                        yariv);
  // * encode the varint for the bound
  yariv += yariv_encode_unsigned_varint(YARIV_CAST(const uint32_t *, spirv)[3],
                                        yariv);

  // iterate through all the opcodes
  for (i = 5, e = spirv_size / 4; i < e;) {
    const uint32_t *from = YARIV_CAST(const uint32_t *, spirv) + i;
    const uint32_t word_count = (*from) >> 16;
    uint32_t opcode = (*from) & 0xFFFFu;
    uint32_t k;
    int32_t maybe_update_id = last_id;

    // if we can strip opcodes
    if (yariv_encode_flags_strip & encode_flags) {
      switch (opcode) {
      default:
        break;
      case YarivOpNop:
      case YarivOpSourceContinued:
      case YarivOpSource:
      case YarivOpSourceExtension:
      case YarivOpName:
      case YarivOpMemberName:
      case YarivOpString:
      case YarivOpLine:
      case YarivOpNoLine:
        // all of these opcodes can be stripped, so we skip them!
        i += word_count;
        continue;
      }
    }

    opcode = yariv_opcode_maybe_split(opcode, from, word_count);

    // for all opcodes we encode the opcode first as a varint
    yariv += yariv_encode_unsigned_varint(yariv_shuffle_opcode(opcode), yariv);

    if (!yariv_opcode_has_word_count(opcode, 0)) {
      // followed by the word count as a varint (removing the known constant
      // part of word count)
      yariv += yariv_encode_unsigned_varint(
          word_count - yariv_word_count_constant_part(opcode), yariv);
    }

    // by default, we use unsigned varint encoding from the word immediately
    // following the opcode/word-count word
    k = 1;

    if (yariv_opcode_has_id(opcode)) {
      // if our opcode has an id, we want to use signed varint encoding
      // relative to the last id we used
      const int32_t id = YARIV_CAST(const int32_t *, spirv)[i + 1];
      yariv += yariv_encode_signed_varint(id - last_id, yariv);
      maybe_update_id = id;

      // and since we have already encoded the first word, k should skip
      // another word for the remaining unsigned varint encoding
      k += 1;
    } else if (yariv_opcode_has_type_and_id(opcode)) {
      // if our opcode has a type and id, we want to use signed varint
      // encoding for the id relative to the last id we used
      const uint32_t type = YARIV_CAST(const uint32_t *, spirv)[i + 1];
      const int32_t id = YARIV_CAST(const int32_t *, spirv)[i + 2];

      yariv += yariv_encode_unsigned_varint(type - type_offset, yariv);
      yariv += yariv_encode_signed_varint(id - last_id, yariv);
      maybe_update_id = id;

      // and since we have already encoded the first two words, k should skip
      // another two words for the remaining unsigned varint encoding
      k += 2;
    }

    switch (opcode) {
    default:
      // then we encode some words as signed varints
      for (; k < yariv_word_count_to_id_delta(opcode, word_count); k++) {
        const int32_t id = YARIV_CAST(const int32_t *, spirv)[i + k];
        yariv += yariv_encode_signed_varint(id - last_id, yariv);
      }

      // then we encode the remaining words as unsigned varints
      for (; k < word_count; k++) {
        yariv += yariv_encode_unsigned_varint(
            YARIV_CAST(const uint32_t *, spirv)[i + k], yariv);
      }
      break;
    case YarivOpExtInstImport:
      // our literal string can just be memcpy'ed over
      memcpy(yariv, YARIV_CAST(const uint32_t *, spirv) + i + k,
             sizeof(uint32_t) * (word_count - k));
      yariv += sizeof(uint32_t) * (word_count - k);
      break;
    case YarivOpVectorShuffleTwoLiterals:
    case YarivOpVectorShuffleThreeLiterals:
    case YarivOpVectorShuffleFourLiterals: {
      const int32_t vecid1 = YARIV_CAST(const int32_t *, spirv)[i + k++];
      const int32_t vecid2 = YARIV_CAST(const int32_t *, spirv)[i + k++];

      // we use the signed varint encoding relative to last id for the two
      // vectors we are shuffling (but we don't update last id crucially)
      yariv += yariv_encode_signed_varint(vecid1 - last_id, yariv);
      yariv += yariv_encode_signed_varint(vecid2 - last_id, yariv);

      // our literals are at most 3 bits each, and we have n of them
      if (YarivOpVectorShuffleTwoLiterals == opcode) {
        const int32_t component0 = YARIV_CAST(const int32_t *, spirv)[i + k++];
        const int32_t component1 = YARIV_CAST(const int32_t *, spirv)[i + k++];

        *yariv++ = YARIV_CAST(uint8_t, ((component0 << 3) | component1) & 0xFF);
      } else if (YarivOpVectorShuffleThreeLiterals == opcode) {
        const int32_t component0 = YARIV_CAST(const int32_t *, spirv)[i + k++];
        const int32_t component1 = YARIV_CAST(const int32_t *, spirv)[i + k++];
        const int32_t component2 = YARIV_CAST(const int32_t *, spirv)[i + k++];
        *yariv++ = YARIV_CAST(uint8_t, ((component0 << 3) | component1) & 0xFF);
        *yariv++ = YARIV_CAST(uint8_t, (component2)&0xFF);
      } else if (YarivOpVectorShuffleFourLiterals == opcode) {
        const int32_t component0 = YARIV_CAST(const int32_t *, spirv)[i + k++];
        const int32_t component1 = YARIV_CAST(const int32_t *, spirv)[i + k++];
        const int32_t component2 = YARIV_CAST(const int32_t *, spirv)[i + k++];
        const int32_t component3 = YARIV_CAST(const int32_t *, spirv)[i + k++];
        *yariv++ = YARIV_CAST(uint8_t, ((component0 << 3) | component1) & 0xFF);
        *yariv++ = YARIV_CAST(uint8_t, ((component2 << 3) | component3) & 0xFF);
      }
    } break;
    case YarivOpVectorSwizzleTwoLiterals:
    case YarivOpVectorSwizzleThreeLiterals:
    case YarivOpVectorSwizzleFourLiterals: {
      // both our IDs are the same, so we output it only once
      const int32_t vecid1 = YARIV_CAST(const int32_t *, spirv)[i + k++];
      // skip the ID that is the same
      k++;

      // we use the signed varint encoding relative to last id for the one
      // vector we are shuffling (but we don't update last id crucially)
      yariv += yariv_encode_signed_varint(vecid1 - last_id, yariv);

      // our literals are at most 2 bits each, and we have n of them
      if (YarivOpVectorSwizzleTwoLiterals == opcode) {
        const int32_t component0 = YARIV_CAST(const int32_t *, spirv)[i + k++];
        const int32_t component1 = YARIV_CAST(const int32_t *, spirv)[i + k++];

        *yariv++ = YARIV_CAST(uint8_t, ((component0 << 2) | component1) & 0xFF);
      } else if (YarivOpVectorSwizzleThreeLiterals == opcode) {
        const int32_t component0 = YARIV_CAST(const int32_t *, spirv)[i + k++];
        const int32_t component1 = YARIV_CAST(const int32_t *, spirv)[i + k++];
        const int32_t component2 = YARIV_CAST(const int32_t *, spirv)[i + k++];
        *yariv++ = YARIV_CAST(
            uint8_t,
            ((component0 << 4) | (component1 << 2) | component2) & 0xFF);
      } else if (YarivOpVectorSwizzleFourLiterals == opcode) {
        const int32_t component0 = YARIV_CAST(const int32_t *, spirv)[i + k++];
        const int32_t component1 = YARIV_CAST(const int32_t *, spirv)[i + k++];
        const int32_t component2 = YARIV_CAST(const int32_t *, spirv)[i + k++];
        const int32_t component3 = YARIV_CAST(const int32_t *, spirv)[i + k++];
        *yariv++ = YARIV_CAST(uint8_t, ((component0 << 6) | (component1 << 4) |
                                        (component2 << 2) | component3) &
                                           0xFF);
      }
    } break;
    case YarivOpCompositeExtractWithLiteralZero:
    case YarivOpCompositeExtractWithLiteralOne:
    case YarivOpCompositeExtractWithLiteralTwo:
    case YarivOpCompositeExtractWithLiteralThree: {
      // the id of the composite we are extracting from
      const int32_t composite = YARIV_CAST(const int32_t *, spirv)[i + k++];

      // we use the signed varint encoding relative to last id for the two
      // vectors we are shuffling (but we don't update last id crucially)
      yariv += yariv_encode_signed_varint(composite - last_id, yariv);

      // skip the literal that we are not going to encode
      k++;
    } break;
    case YarivOpDecorateRelaxedPrecision:
      // skip the decoration for relaxed precision that we are not going to
      // encode
      k++;
      break;
    case YarivOpMemberDecorateOffset:
      // encode the member literal as a varint
      yariv += yariv_encode_unsigned_varint(
          YARIV_CAST(const uint32_t *, spirv)[i + k++], yariv);

      // skip the decoration for offset that we are not going to encode
      k++;

      // encode the byte offset as a varint
      yariv += yariv_encode_unsigned_varint(
          YARIV_CAST(const uint32_t *, spirv)[i + k++], yariv);

      break;
    case YarivOpConstantWithOneFloatLiteral:
      // our literal is a float, so we just encode it using 4 bytes
      memcpy(yariv, YARIV_CAST(const uint32_t *, spirv) + i + k++,
             sizeof(uint32_t));
      yariv += sizeof(uint32_t);
      break;
    case YarivOpDecorateLocation:
      // skip the decoration for location that we are not going to encode
      k++;

      // encode the location as a varint
      yariv += yariv_encode_unsigned_varint(
          YARIV_CAST(const uint32_t *, spirv)[i + k++], yariv);

      break;
    }

    // and move i along to the next opcode
    i += word_count;

    // and (maybe) update last_id too!
    last_id = maybe_update_id;
  }

  return 0;
}

size_t yariv_decode_size(const void *yariv, size_t yariv_size) {
  // the SPIR-V header is 5 words
  size_t spirv_size = 5 * sizeof(uint32_t);
  size_t i = 2;

  // we need to decode the varints in the header
  // * the first varint is for our generators magic number
  i += yariv_decode_unsigned_varint(YARIV_CAST(const uint8_t *, yariv) + i, 0);
  // * the second varint is for our bound
  i += yariv_decode_unsigned_varint(YARIV_CAST(const uint8_t *, yariv) + i, 0);

  // iterate through all the opcodes
  while (i < yariv_size) {
    uint32_t opcode, word_count, k;
    i += yariv_decode_unsigned_varint(YARIV_CAST(const uint8_t *, yariv) + i,
                                      &opcode);

    // shuffle the opcode back to its actual opcode
    opcode = yariv_shuffle_opcode(opcode);

    // our opcode/word-count pair takes up at least one word
    spirv_size += sizeof(uint32_t);

    if (!yariv_opcode_has_word_count(opcode, &word_count)) {
      // if the word count wasn't constant for the opcode, we encoded it as a
      // varint
      i += yariv_decode_unsigned_varint(YARIV_CAST(const uint8_t *, yariv) + i,
                                        &word_count);

      // when we encoded the word count, we removed the known constant part of
      // the word count (derived from the opcode) we need to re-add this as we
      // re-encode word count
      word_count += yariv_word_count_constant_part(opcode);
    }

    // by default, we use unsigned varint encoding from the word immediately
    // following the opcode/word-count word
    k = 1;

    if (yariv_opcode_has_id(opcode)) {
      // if our opcode has an id, we used a signed varint encoding relative to
      // the last id
      i +=
          yariv_decode_signed_varint(YARIV_CAST(const uint8_t *, yariv) + i, 0);
      spirv_size += sizeof(uint32_t);

      // and since we have already decoded the first word, k should skip
      // another word for the remaining unsigned varint decoding
      k += 1;
    } else if (yariv_opcode_has_type_and_id(opcode)) {
      // if our opcode has a type and id, we used a signed varint encoding
      // for the id relative to the last id
      i += yariv_decode_unsigned_varint(YARIV_CAST(const uint8_t *, yariv) + i,
                                        0);
      spirv_size += sizeof(uint32_t);

      i +=
          yariv_decode_signed_varint(YARIV_CAST(const uint8_t *, yariv) + i, 0);
      spirv_size += sizeof(uint32_t);

      // and since we have already decoded the first two words, k should skip
      // two words for the remaining unsigned varint decoding
      k += 2;
    }

    switch (opcode) {
    default:
      // then we decode some words as signed varints
      for (; k < yariv_word_count_to_id_delta(opcode, word_count); k++) {
        i += yariv_decode_signed_varint(YARIV_CAST(const uint8_t *, yariv) + i,
                                        0);
        spirv_size += sizeof(uint32_t);
      }

      // then we decode the remaining words as unsigned varints
      for (; k < word_count; k++) {
        i += yariv_decode_unsigned_varint(
            YARIV_CAST(const uint8_t *, yariv) + i, 0);

        spirv_size += sizeof(uint32_t);
      }
      break;
    case YarivOpExtInstImport:
      // our literal string was just be memcpy'ed over
      spirv_size += sizeof(uint32_t) * (word_count - k);
      i += sizeof(uint32_t) * (word_count - k);
      break;
    case YarivOpVectorShuffleTwoLiterals:
    case YarivOpVectorShuffleThreeLiterals:
    case YarivOpVectorShuffleFourLiterals:
      // we use the signed varint encoding relative to last id for the two
      // vectors we are shuffling (but we don't update last id crucially)
      i +=
          yariv_decode_signed_varint(YARIV_CAST(const uint8_t *, yariv) + i, 0);
      spirv_size += sizeof(uint32_t);
      i +=
          yariv_decode_signed_varint(YARIV_CAST(const uint8_t *, yariv) + i, 0);
      spirv_size += sizeof(uint32_t);

      // our literals are at most 3 bits each, and we have n of them
      if (YarivOpVectorShuffleTwoLiterals == opcode) {
        spirv_size += 2 * sizeof(uint32_t);
        i += sizeof(uint8_t);
      } else if (YarivOpVectorShuffleThreeLiterals == opcode) {
        spirv_size += 3 * sizeof(uint32_t);
        i += sizeof(uint8_t) * 2;
      } else if (YarivOpVectorShuffleFourLiterals == opcode) {
        spirv_size += 4 * sizeof(uint32_t);
        i += sizeof(uint8_t) * 2;
      }

      break;
    case YarivOpVectorSwizzleTwoLiterals:
    case YarivOpVectorSwizzleThreeLiterals:
    case YarivOpVectorSwizzleFourLiterals:
      // we use the signed varint encoding relative to last id for the one
      // vector we are shuffling (but we don't update last id crucially)
      i +=
          yariv_decode_signed_varint(YARIV_CAST(const uint8_t *, yariv) + i, 0);
      spirv_size += sizeof(uint32_t);
      spirv_size += sizeof(uint32_t);

      // our literals are at most 2 bits each, and we have n of them
      i += sizeof(uint8_t);

      if (YarivOpVectorSwizzleTwoLiterals == opcode) {
        spirv_size += 2 * sizeof(uint32_t);
      } else if (YarivOpVectorSwizzleThreeLiterals == opcode) {
        spirv_size += 3 * sizeof(uint32_t);
      } else if (YarivOpVectorSwizzleFourLiterals == opcode) {
        spirv_size += 4 * sizeof(uint32_t);
      }

      break;
    case YarivOpCompositeExtractWithLiteralZero:
    case YarivOpCompositeExtractWithLiteralOne:
    case YarivOpCompositeExtractWithLiteralTwo:
    case YarivOpCompositeExtractWithLiteralThree:
      // we use the signed varint encoding relative to last id for the one
      // vector we are shuffling (but we don't update last id crucially)
      i +=
          yariv_decode_signed_varint(YARIV_CAST(const uint8_t *, yariv) + i, 0);
      spirv_size += sizeof(uint32_t);

      // we skipped a literal that we need to re-encode
      spirv_size += sizeof(uint32_t);

      break;
    case YarivOpDecorateRelaxedPrecision:
      // we skipped a decoration that we need to re-encode
      spirv_size += sizeof(uint32_t);
      break;
    case YarivOpMemberDecorateOffset:
      // we encoded the member literal as a varint
      i += yariv_decode_unsigned_varint(YARIV_CAST(const uint8_t *, yariv) + i,
                                        0);
      spirv_size += sizeof(uint32_t);

      // we skipped a decoration that we need to re-encode
      spirv_size += sizeof(uint32_t);

      // we encoded the offset as a varint
      i += yariv_decode_unsigned_varint(YARIV_CAST(const uint8_t *, yariv) + i,
                                        0);
      spirv_size += sizeof(uint32_t);
      break;
    case YarivOpConstantWithOneFloatLiteral:
      // we encoded our literal as a 4 byte value
      spirv_size += sizeof(uint32_t);
      i += sizeof(uint32_t);
      break;
    case YarivOpDecorateLocation:
      // we skipped a decoration that we need to re-encode
      spirv_size += sizeof(uint32_t);

      // we encoded the offset as a varint
      i += yariv_decode_unsigned_varint(YARIV_CAST(const uint8_t *, yariv) + i,
                                        0);
      spirv_size += sizeof(uint32_t);
      break;
    }
  }

  return spirv_size;
}

int yariv_decode(void *out_spirv, size_t out_spirv_size, const void *yariv,
                 size_t yariv_size) {
  size_t i;
  int32_t last_id = 1, temp_id, maybe_update_id;
  const uint32_t type_offset = 1;
  uint32_t *spirv = YARIV_CAST(uint32_t *, out_spirv);

  // FIXME: I should really check we don't try and write beyond out_spirv_size
  // in out_spirv!
  (void)out_spirv_size;

  // for the SPIR-V header
  // * we re-encode SPIR-V's magic number
  *spirv++ = 0x07230203u;
  // * we re-encode the version number
  *spirv++ =
      (YARIV_CAST(uint32_t, YARIV_CAST(const uint8_t *, yariv)[0]) << 16) |
      (YARIV_CAST(uint32_t, YARIV_CAST(const uint8_t *, yariv)[1]) << 8);
  i = 2;
  // * we used varint encoding for the generators magic number
  i += yariv_decode_unsigned_varint(YARIV_CAST(const uint8_t *, yariv) + i,
                                    spirv++);
  // * we used varint encoding for the bound
  i += yariv_decode_unsigned_varint(YARIV_CAST(const uint8_t *, yariv) + i,
                                    spirv++);
  // * we re-encode the instruction schema (which is zero)
  *spirv++ = 0;

  while (i < yariv_size) {
    uint32_t opcode, word_count, word, k;

    maybe_update_id = last_id;

    i += yariv_decode_unsigned_varint(YARIV_CAST(const uint8_t *, yariv) + i,
                                      &opcode);

    // shuffle the opcode back to its actual opcode
    opcode = yariv_shuffle_opcode(opcode);

    if (!yariv_opcode_has_word_count(opcode, &word_count)) {
      // if the word count wasn't constant for the opcode, we encoded it as a
      // varint
      i += yariv_decode_unsigned_varint(YARIV_CAST(const uint8_t *, yariv) + i,
                                        &word_count);

      // when we encoded the word count, we removed the known constant part of
      // the word count (derived from the opcode) we need to re-add this as we
      // re-encode word count
      word_count += yariv_word_count_constant_part(opcode);
    }

    switch (opcode) {
    default:
      *spirv++ = (word_count << 16) | opcode;
      break;
    case YarivOpStoreWithMemoryAccess:
      // we added this fake opcode during encoding to allow us to compress
      // some more, so now we have to undo this
      *spirv++ = (word_count << 16) | YarivOpStore;
      break;
    case YarivOpLoadWithMemoryAccess:
      // we added this fake opcode during encoding to allow us to compress
      // some more, so now we have to undo this
      *spirv++ = (word_count << 16) | YarivOpLoad;
      break;
    case YarivOpDecorateRelaxedPrecision:
    case YarivOpDecorateWithNoLiterals:
    case YarivOpDecorateWithOneLiteral:
    case YarivOpDecorateLocation:
      // we added this fake opcode during encoding to allow us to compress
      // some more, so now we have to undo this
      *spirv++ = (word_count << 16) | YarivOpDecorate;
      break;
    case YarivOpMemberDecorateWithNoLiterals:
    case YarivOpMemberDecorateWithOneLiteral:
    case YarivOpMemberDecorateOffset:
      // we added this fake opcode during encoding to allow us to compress
      // some more, so now we have to undo this
      *spirv++ = (word_count << 16) | YarivOpMemberDecorate;
      break;
    case YarivOpVariableWithInitializer:
      // we added this fake opcode during encoding to allow us to compress
      // some more, so now we have to undo this
      *spirv++ = (word_count << 16) | YarivOpVariable;
      break;
    case YarivOpConstantWithManyLiterals:
    case YarivOpConstantWithOneFloatLiteral:
      // we added this fake opcode during encoding to allow us to compress
      // some more, so now we have to undo this
      *spirv++ = (word_count << 16) | YarivOpConstant;
      break;
    case YarivOpAccessChainWithOneIndex:
    case YarivOpAccessChainWithTwoIndices:
    case YarivOpAccessChainWithThreeIndices:
      // we added this fake opcode during encoding to allow us to compress
      // some more, so now we have to undo this
      *spirv++ = (word_count << 16) | YarivOpAccessChain;
      break;
    case YarivOpVectorShuffleTwoLiterals:
    case YarivOpVectorShuffleThreeLiterals:
    case YarivOpVectorShuffleFourLiterals:
    case YarivOpVectorSwizzleTwoLiterals:
    case YarivOpVectorSwizzleThreeLiterals:
    case YarivOpVectorSwizzleFourLiterals:
      // we added this fake opcode during encoding to allow us to compress
      // some more, so now we have to undo this
      *spirv++ = (word_count << 16) | YarivOpVectorShuffle;
      break;
    case YarivOpCompositeExtractWithLiteralZero:
    case YarivOpCompositeExtractWithLiteralOne:
    case YarivOpCompositeExtractWithLiteralTwo:
    case YarivOpCompositeExtractWithLiteralThree:
      // we added this fake opcode during encoding to allow us to compress
      // some more, so now we have to undo this
      *spirv++ = (word_count << 16) | YarivOpCompositeExtract;
      break;
    case YarivOpCompositeConstructOneConstituent:
    case YarivOpCompositeConstructTwoConstituents:
    case YarivOpCompositeConstructThreeConstituents:
      // we added this fake opcode during encoding to allow us to compress
      // some more, so now we have to undo this
      *spirv++ = (word_count << 16) | YarivOpCompositeConstruct;
      break;
    }

    // by default, we use unsigned varint encoding from the word immediately
    // following the opcode/word-count word
    k = 1;

    if (yariv_opcode_has_id(opcode)) {
      // if our opcode has an id, we used a signed varint encoding relative to
      // the last id
      int32_t id;
      i += yariv_decode_signed_varint(YARIV_CAST(const uint8_t *, yariv) + i,
                                      &id);
      maybe_update_id += id;
      *spirv++ = YARIV_CAST(uint32_t, maybe_update_id);

      // and since we have already decoded the first word, k should skip
      // another word for the remaining unsigned varint decoding
      k += 1;
    } else if (yariv_opcode_has_type_and_id(opcode)) {
      // if our opcode has a type and id, we used a signed varint encoding
      // relative for the id to the last id
      uint32_t type;
      int32_t id;

      i += yariv_decode_unsigned_varint(YARIV_CAST(const uint8_t *, yariv) + i,
                                        &type);
      *spirv++ = YARIV_CAST(uint32_t, type + type_offset);

      i += yariv_decode_signed_varint(YARIV_CAST(const uint8_t *, yariv) + i,
                                      &id);
      maybe_update_id += id;
      *spirv++ = YARIV_CAST(uint32_t, maybe_update_id);

      // and since we have already decoded the first two words, k should skip
      // two words for the remaining unsigned varint decoding
      k += 2;
    }

    switch (opcode) {
    default:
      // then we decode some words as signed varints
      for (; k < yariv_word_count_to_id_delta(opcode, word_count); k++) {
        i += yariv_decode_signed_varint(YARIV_CAST(const uint8_t *, yariv) + i,
                                        &temp_id);
        *spirv++ = YARIV_CAST(uint32_t, temp_id + last_id);
      }

      // followed by all remaining words as varints
      for (; k < word_count; k++) {
        i += yariv_decode_unsigned_varint(
            YARIV_CAST(const uint8_t *, yariv) + i, &word);
        *spirv++ = word;
      }
      break;
    case YarivOpExtInstImport:
      // our literal string was just be memcpy'ed over
      memcpy(spirv, YARIV_CAST(const uint8_t *, yariv) + i,
             sizeof(uint32_t) * (word_count - k));
      spirv += word_count - k;
      i += sizeof(uint32_t) * (word_count - k);
      break;
    case YarivOpVectorShuffleTwoLiterals:
    case YarivOpVectorShuffleThreeLiterals:
    case YarivOpVectorShuffleFourLiterals:
      // we use the signed varint encoding relative to last id for the two
      // vectors we are shuffling (but we don't update last id crucially)
      i += yariv_decode_signed_varint(YARIV_CAST(const uint8_t *, yariv) + i,
                                      &temp_id);
      *spirv++ = YARIV_CAST(uint32_t, last_id + temp_id);
      i += yariv_decode_signed_varint(YARIV_CAST(const uint8_t *, yariv) + i,
                                      &temp_id);
      *spirv++ = YARIV_CAST(uint32_t, last_id + temp_id);

      // our literals were at most 3 bits each, and we have n of them
      if (YarivOpVectorShuffleTwoLiterals == opcode) {
        const uint32_t component0 = YARIV_CAST(const uint8_t *, yariv)[i++];
        *spirv++ = component0 >> 3;
        *spirv++ = component0 & 0x7u;
      } else if (YarivOpVectorShuffleThreeLiterals == opcode) {
        const uint32_t component0 = YARIV_CAST(const uint8_t *, yariv)[i++];
        const uint32_t component1 = YARIV_CAST(const uint8_t *, yariv)[i++];
        *spirv++ = component0 >> 3;
        *spirv++ = component0 & 0x7u;
        *spirv++ = component1;
      } else if (YarivOpVectorShuffleFourLiterals == opcode) {
        const uint32_t component0 = YARIV_CAST(const uint8_t *, yariv)[i++];
        const uint32_t component1 = YARIV_CAST(const uint8_t *, yariv)[i++];
        *spirv++ = component0 >> 3;
        *spirv++ = component0 & 0x7u;
        *spirv++ = component1 >> 3;
        *spirv++ = component1 & 0x7u;
      }

      break;
    case YarivOpVectorSwizzleTwoLiterals:
    case YarivOpVectorSwizzleThreeLiterals:
    case YarivOpVectorSwizzleFourLiterals:
      // we use the signed varint encoding relative to last id for the one
      // vector we are shuffling (but we don't update last id crucially)
      i += yariv_decode_signed_varint(YARIV_CAST(const uint8_t *, yariv) + i,
                                      &temp_id);
      *spirv++ = YARIV_CAST(uint32_t, last_id + temp_id);
      *spirv++ = YARIV_CAST(uint32_t, last_id + temp_id);

      // our literals were at most 2 bits each, and we have n of them
      if (YarivOpVectorSwizzleTwoLiterals == opcode) {
        const uint32_t component = YARIV_CAST(const uint8_t *, yariv)[i++];
        *spirv++ = component >> 2;
        *spirv++ = component & 0x3u;
      } else if (YarivOpVectorSwizzleThreeLiterals == opcode) {
        const uint32_t component = YARIV_CAST(const uint8_t *, yariv)[i++];
        *spirv++ = component >> 4;
        *spirv++ = (component >> 2) & 0x3u;
        *spirv++ = component & 0x3u;
      } else if (YarivOpVectorSwizzleFourLiterals == opcode) {
        const uint32_t component = YARIV_CAST(const uint8_t *, yariv)[i++];
        *spirv++ = component >> 6;
        *spirv++ = (component >> 4) & 0x3u;
        *spirv++ = (component >> 2) & 0x3u;
        *spirv++ = component & 0x3u;
      }

      break;
    case YarivOpCompositeExtractWithLiteralZero:
    case YarivOpCompositeExtractWithLiteralOne:
    case YarivOpCompositeExtractWithLiteralTwo:
    case YarivOpCompositeExtractWithLiteralThree:
      // we use the signed varint encoding relative to last id for the one
      // vector we are shuffling (but we don't update last id crucially)
      i += yariv_decode_signed_varint(YARIV_CAST(const uint8_t *, yariv) + i,
                                      &temp_id);
      *spirv++ = YARIV_CAST(uint32_t, last_id + temp_id);

      // we skipped a literal that we need to re-encode
      if (YarivOpCompositeExtractWithLiteralZero == opcode) {
        *spirv++ = 0;
      } else if (YarivOpCompositeExtractWithLiteralOne == opcode) {
        *spirv++ = 1;
      } else if (YarivOpCompositeExtractWithLiteralTwo == opcode) {
        *spirv++ = 2;
      } else if (YarivOpCompositeExtractWithLiteralThree == opcode) {
        *spirv++ = 3;
      }

      break;
    case YarivOpDecorateRelaxedPrecision:
      // we skipped a decoration that we need to re-encode
      *spirv++ = 0; // output the RelaxedPrecision decoration
      break;
    case YarivOpMemberDecorateOffset:
      // we encoded the member literal as a varint
      i += yariv_decode_unsigned_varint(YARIV_CAST(const uint8_t *, yariv) + i,
                                        &k);
      *spirv++ = k;

      // we skipped a decoration that we need to re-encode
      *spirv++ = 35; // output the Offset decoration

      // we encoded the offset as a varint
      i += yariv_decode_unsigned_varint(YARIV_CAST(const uint8_t *, yariv) + i,
                                        &k);
      *spirv++ = k;
      break;
    case YarivOpConstantWithOneFloatLiteral:
      memcpy(spirv++, YARIV_CAST(const uint8_t *, yariv) + i, sizeof(uint32_t));
      i += sizeof(uint32_t);
      break;
    case YarivOpDecorateLocation:
      // we skipped a decoration that we need to re-encode
      *spirv++ = 30; // output the Location decoration

      // we encoded the location as a varint
      i += yariv_decode_unsigned_varint(YARIV_CAST(const uint8_t *, yariv) + i,
                                        &k);
      *spirv++ = k;
      break;
    }

    // and maybe move the id along
    last_id = maybe_update_id;
  }

  return 0;
}

uint32_t yariv_word_count_to_id_delta(uint32_t opcode, uint32_t word_count) {
  (void)word_count;
  switch (opcode) {
  default:
    // by default we want no IDs to be delta encoded
    return 0;
  case YarivOpAccessChain:
  case YarivOpStore:
  case YarivOpLoad:
  case YarivOpFMul:
  case YarivOpFAdd:
  case YarivOpFSub:
  case YarivOpConstantComposite:
  case YarivOpCompositeConstruct:
  case YarivOpCompositeConstructOneConstituent:
  case YarivOpCompositeConstructTwoConstituents:
  case YarivOpCompositeConstructThreeConstituents:
  case YarivOpDot:
  case YarivOpVectorTimesScalar:
  case YarivOpFNegate:
  case YarivOpFDiv:
  case YarivOpFOrdEqual:
  case YarivOpFOrdLessThan:
  case YarivOpFOrdLessThanEqual:
  case YarivOpFOrdGreaterThan:
  case YarivOpFOrdGreaterThanEqual:
  case YarivOpFOrdNotEqual:
  case YarivOpIEqual:
  case YarivOpIAdd:
  case YarivOpFunctionCall:
  case YarivOpSDiv:
  case YarivOpUDiv:
  case YarivOpSLessThan:
  case YarivOpSLessThanEqual:
  case YarivOpPhi:
  case YarivOpLogicalAnd:
  case YarivOpVectorTimesMatrix:
  case YarivOpMatrixTimesVector:
  case YarivOpLogicalNot:
  case YarivOpIMul:
  case YarivOpISub:
  case YarivOpConvertSToF:
  case YarivOpConvertFToS:
  case YarivOpConvertUToF:
  case YarivOpConvertFToU:
  case YarivOpFMod:
  case YarivOpLogicalOr:
  case YarivOpBitcast:
  case YarivOpShiftLeftLogical:
  case YarivOpShiftRightArithmetic:
  case YarivOpShiftRightLogical:
  case YarivOpMatrixTimesScalar:
  case YarivOpMatrixTimesMatrix:
  case YarivOpSelect:
  case YarivOpTranspose:
    return word_count;
  case YarivOpBranchConditional:
    return 4;
  case YarivOpVectorShuffle:
    return 5;
  case YarivOpImageSampleImplicitLod:
  case YarivOpImageSampleExplicitLod:
    return 5;
  case YarivOpImageSampleDrefImplicitLod:
  case YarivOpImageSampleDrefExplicitLod:
    return 6;
  }
}

uint32_t yariv_word_count_constant_part(uint32_t opcode) {
  switch (opcode) {
  default:
    // by default we can assume we have a word count of at least one, because
    // the opcode itself must reside in a word count
    return 1;
  case YarivOpSourceContinued:
  case YarivOpSourceExtension:
  case YarivOpGroupDecorate:
  case YarivOpGroupMemberDecorate:
  case YarivOpExtension:
  case YarivOpTypeStruct:
    return 2;
  case YarivOpSource:
  case YarivOpName:
  case YarivOpString:
  case YarivOpDecorate:
  case YarivOpExtInstImport:
  case YarivOpExecutionMode:
  case YarivOpTypeOpaque:
  case YarivOpTypeFunction:
  case YarivOpConstant:
  case YarivOpConstantComposite:
  case YarivOpSpecConstant:
  case YarivOpSpecConstantComposite:
  case YarivOpStore:
  case YarivOpCopyMemory:
  case YarivOpCompositeConstruct:
  case YarivOpPhi:
  case YarivOpSwitch:
    return 3;
  case YarivOpMemberName:
  case YarivOpMemberDecorate:
  case YarivOpEntryPoint:
  case YarivOpSpecConstantOp:
  case YarivOpVariable:
  case YarivOpLoad:
  case YarivOpCopyMemorySized:
  case YarivOpAccessChain:
  case YarivOpInBoundsAccessChain:
  case YarivOpFunctionCall:
  case YarivOpImageWrite:
  case YarivOpCompositeExtract:
  case YarivOpBranchConditional:
    return 4;
  case YarivOpExtInst:
  case YarivOpPtrAccessChain:
  case YarivOpInBoundsPtrAccessChain:
  case YarivOpImageSampleImplicitLod:
  case YarivOpImageSampleProjImplicitLod:
  case YarivOpImageFetch:
  case YarivOpImageRead:
  case YarivOpImageSparseSampleImplicitLod:
  case YarivOpImageSparseSampleProjImplicitLod:
  case YarivOpImageSparseFetch:
  case YarivOpImageSparseRead:
  case YarivOpVectorShuffle:
  case YarivOpCompositeInsert:
    return 5;
  case YarivOpImageSparseSampleProjDrefImplicitLod:
    return 6;
  case YarivOpImageSparseSampleProjExplicitLod:
    return 7;
  }
}

uint32_t yariv_opcode_maybe_split(uint32_t opcode, const uint32_t *spirv,
                                  uint32_t word_count) {
  uint32_t k;
  switch (opcode) {
  default:
    break;
  case YarivOpStore:
    // if the store has a memory access, we will encode it as a different
    // opcode instead to allow us to compress load some more
    if (4 == word_count) {
      opcode = YarivOpStoreWithMemoryAccess;
    }
    break;
  case YarivOpLoad:
    // if the load have a memory access, we will encode it as a different
    // opcode instead to allow us to compress load some more
    if (5 == word_count) {
      opcode = YarivOpLoadWithMemoryAccess;
    }
    break;
  case YarivOpDecorate:
    // if the decoration has 0 or 1 literals, we will encode it as a different
    // opcode instead to allow us to compress decorate some more
    if (3 == word_count) {
      if (0 == spirv[2]) {
        opcode = YarivOpDecorateRelaxedPrecision;
      } else {
        opcode = YarivOpDecorateWithNoLiterals;
      }
    } else if (4 == word_count) {
      if (30 == spirv[2]) {
        opcode = YarivOpDecorateLocation;
      } else {
        opcode = YarivOpDecorateWithOneLiteral;
      }
    }
    break;
  case YarivOpMemberDecorate:
    // if the decoration has 0 or 1 literals, we will encode it as a different
    // opcode instead to allow us to compress decorate some more
    if (4 == word_count) {
      opcode = YarivOpMemberDecorateWithNoLiterals;
    } else if (5 == word_count) {
      if (35 == spirv[3]) {
        opcode = YarivOpMemberDecorateOffset;
      } else {
        opcode = YarivOpMemberDecorateWithOneLiteral;
      }
    }
    break;
  case YarivOpVariable:
    if (5 == word_count) {
      opcode = YarivOpVariableWithInitializer;
    }
    break;
  case YarivOpConstant:
    if (4 < word_count) {
      opcode = YarivOpConstantWithManyLiterals;
    } else if (0xFF800000u & spirv[3]) {
      opcode = YarivOpConstantWithOneFloatLiteral;
    }
    break;
  case YarivOpAccessChain:
    // If the access chain had one, two or three indices, we will encode it as a
    // different opcode instead to allow us to compress it some more
    if (5 == word_count) {
      opcode = YarivOpAccessChainWithOneIndex;
    } else if (6 == word_count) {
      opcode = YarivOpAccessChainWithTwoIndices;
    } else if (7 == word_count) {
      opcode = YarivOpAccessChainWithThreeIndices;
    }
    break;
  case YarivOpVectorShuffle: {
    const int haveBothIdsTheSame = spirv[3] == spirv[4];
    int hasUndef = 0;
    int hasLiteralsLessThan4 = 1;
    int hasLiteralsLessThan8 = 1;

    // If the vector shuffle has no undef literals (0xFFFFFFFu), and it is
    // requesting a result vector of two, three or four width, we will encode it
    // as a different opcode instead to allow us to compress it some more
    for (k = 5; k < word_count; k++) {
      if (0xFFFFFFFFu == spirv[k]) {
        // we are using the undef literal so we can't encode the shuffle more
        // efficiently!
        hasUndef = 1;
        break;
      } else {
        if (3 < spirv[k]) {
          // we are using vector types greater than vec4's so we can't encode
          // the
          // shuffle more efficiently!
          hasLiteralsLessThan4 = 0;
        }
        if (7 < spirv[k]) {
          // we are using vector types greater than vec4's so we can't encode
          // the
          // shuffle more efficiently!
          hasLiteralsLessThan8 = 0;
        }
      }
    }

    // we found an undef, so we can't encode the opcode differently
    if (hasUndef) {
      break;
    }

    if (hasLiteralsLessThan4) {
      if (haveBothIdsTheSame) {
        if (7 == word_count) {
          opcode = YarivOpVectorSwizzleTwoLiterals;
        } else if (8 == word_count) {
          opcode = YarivOpVectorSwizzleThreeLiterals;
        } else if (9 == word_count) {
          opcode = YarivOpVectorSwizzleFourLiterals;
        }
      }
    } else if (hasLiteralsLessThan8) {
      if (7 == word_count) {
        opcode = YarivOpVectorShuffleTwoLiterals;
      } else if (8 == word_count) {
        opcode = YarivOpVectorShuffleThreeLiterals;
      } else if (9 == word_count) {
        opcode = YarivOpVectorShuffleFourLiterals;
      }
    }
  } break;
  case YarivOpCompositeExtract:
    if (5 == word_count) {
      switch (spirv[4]) {
      default:
        break;
      case 0:
        opcode = YarivOpCompositeExtractWithLiteralZero;
        break;
      case 1:
        opcode = YarivOpCompositeExtractWithLiteralOne;
        break;
      case 2:
        opcode = YarivOpCompositeExtractWithLiteralTwo;
        break;
      case 3:
        opcode = YarivOpCompositeExtractWithLiteralThree;
        break;
      }
    }
    break;
  case YarivOpCompositeConstruct:
    if (4 == word_count) {
      opcode = YarivOpCompositeConstructOneConstituent;
    } else if (5 == word_count) {
      opcode = YarivOpCompositeConstructTwoConstituents;
    } else if (6 == word_count) {
      opcode = YarivOpCompositeConstructThreeConstituents;
    }
    break;
  }

  return opcode;
}

int yariv_opcode_has_id(uint32_t opcode) {
  switch (opcode) {
  default:
    // by default we assume the opcode does not have an id
    return 0;
  case YarivOpName:
  case YarivOpMemberName:
  // technically member name has a type and no id, but it behaves like
  // an ID for the context we require
  case YarivOpString:
  case YarivOpLine:
  case YarivOpDecorate:
  case YarivOpMemberDecorate:
  // technically member decorate has a type and no id, but it behaves like
  // an ID for the context we require
  case YarivOpDecorationGroup:
  case YarivOpGroupDecorate:
  case YarivOpGroupMemberDecorate:
  case YarivOpExtInstImport:
  case YarivOpExtInst:
  case YarivOpExecutionMode:
  case YarivOpStore:
  case YarivOpCopyMemory:
  case YarivOpCopyMemorySized:
  case YarivOpLoopMerge:
  case YarivOpSelectionMerge:
  case YarivOpLabel:
  case YarivOpBranch:
  case YarivOpBranchConditional:
  case YarivOpSwitch:
  case YarivOpReturnValue:
  case YarivOpLifetimeStart:
  case YarivOpLifetimeStop:
  case YarivOpAtomicFlagClear:
  case YarivOpEmitStreamVertex:
  case YarivOpEndStreamPrimitive:
  case YarivOpControlBarrier:
  case YarivOpMemoryBarrier:
  case YarivOpRetainEvent:
  case YarivOpReleaseEvent:
  case YarivOpSetUserEventStatus:
  case YarivOpCaptureEventProfilingInfo:
  case YarivOpDecorateWithNoLiterals:
  case YarivOpDecorateWithOneLiteral:
  case YarivOpMemberDecorateWithNoLiterals:
  case YarivOpMemberDecorateWithOneLiteral:
  case YarivOpDecorateRelaxedPrecision:
  case YarivOpMemberDecorateOffset:
  case YarivOpDecorateLocation:
    return 1;
  }
}

int yariv_opcode_has_type_and_id(uint32_t opcode) {
  switch (opcode) {
  default:
    // by default we assume the opcode does not have a type and id
    return 0;
  case YarivOpUndef:
  case YarivOpExtInst:
  case YarivOpConstantTrue:
  case YarivOpConstantFalse:
  case YarivOpConstant:
  case YarivOpConstantComposite:
  case YarivOpConstantSampler:
  case YarivOpConstantNull:
  case YarivOpSpecConstantTrue:
  case YarivOpSpecConstantFalse:
  case YarivOpSpecConstant:
  case YarivOpSpecConstantComposite:
  case YarivOpSpecConstantOp:
  case YarivOpVariable:
  case YarivOpImageTexelPointer:
  case YarivOpLoad:
  case YarivOpAccessChain:
  case YarivOpInBoundsAccessChain:
  case YarivOpPtrAccessChain:
  case YarivOpArrayLength:
  case YarivOpGenericPtrMemSemantics:
  case YarivOpInBoundsPtrAccessChain:
  case YarivOpFunction:
  case YarivOpFunctionParameter:
  case YarivOpFunctionCall:
  case YarivOpSampledImage:
  case YarivOpImageSampleImplicitLod:
  case YarivOpImageSampleExplicitLod:
  case YarivOpImageSampleDrefImplicitLod:
  case YarivOpImageSampleDrefExplicitLod:
  case YarivOpImageSampleProjImplicitLod:
  case YarivOpImageSampleProjExplicitLod:
  case YarivOpImageSampleProjDrefImplicitLod:
  case YarivOpImageSampleProjDrefExplicitLod:
  case YarivOpImageFetch:
  case YarivOpImageGather:
  case YarivOpImageDrefGather:
  case YarivOpImageRead:
  case YarivOpImageWrite:
  case YarivOpImage:
  case YarivOpImageQueryFormat:
  case YarivOpImageQueryOrder:
  case YarivOpImageQuerySizeLod:
  case YarivOpImageQuerySize:
  case YarivOpImageQueryLod:
  case YarivOpImageQueryLevels:
  case YarivOpImageQuerySamples:
  case YarivOpImageSparseSampleImplicitLod:
  case YarivOpImageSparseSampleExplicitLod:
  case YarivOpImageSparseSampleDrefImplicitLod:
  case YarivOpImageSparseSampleDrefExplicitLod:
  case YarivOpImageSparseSampleProjImplicitLod:
  case YarivOpImageSparseSampleProjExplicitLod:
  case YarivOpImageSparseSampleProjDrefImplicitLod:
  case YarivOpImageSparseSampleProjDrefExplicitLod:
  case YarivOpImageSparseFetch:
  case YarivOpImageSparseGather:
  case YarivOpImageSparseDrefGather:
  case YarivOpImageSparseTexelsResident:
  case YarivOpImageSparseRead:
  case YarivOpConvertFToU:
  case YarivOpConvertFToS:
  case YarivOpConvertSToF:
  case YarivOpConvertUToF:
  case YarivOpUConvert:
  case YarivOpSConvert:
  case YarivOpFConvert:
  case YarivOpQuantizeToF16:
  case YarivOpConvertPtrToU:
  case YarivOpSatConvertSToU:
  case YarivOpSatConvertUToS:
  case YarivOpConvertUToPtr:
  case YarivOpPtrCastToGeneric:
  case YarivOpGenericCastToPtr:
  case YarivOpGenericCastToPtrExplicit:
  case YarivOpBitcast:
  case YarivOpVectorExtractDynamic:
  case YarivOpVectorInsertDynamic:
  case YarivOpVectorShuffle:
  case YarivOpCompositeConstruct:
  case YarivOpCompositeExtract:
  case YarivOpCompositeInsert:
  case YarivOpCopyObject:
  case YarivOpTranspose:
  case YarivOpSNegate:
  case YarivOpFNegate:
  case YarivOpIAdd:
  case YarivOpFAdd:
  case YarivOpISub:
  case YarivOpFSub:
  case YarivOpIMul:
  case YarivOpFMul:
  case YarivOpUDiv:
  case YarivOpSDiv:
  case YarivOpFDiv:
  case YarivOpUMod:
  case YarivOpSRem:
  case YarivOpSMod:
  case YarivOpFRem:
  case YarivOpFMod:
  case YarivOpVectorTimesScalar:
  case YarivOpMatrixTimesScalar:
  case YarivOpVectorTimesMatrix:
  case YarivOpMatrixTimesVector:
  case YarivOpMatrixTimesMatrix:
  case YarivOpOuterProduct:
  case YarivOpDot:
  case YarivOpIAddCarry:
  case YarivOpISubBorrow:
  case YarivOpUMulExtended:
  case YarivOpSMulExtended:
  case YarivOpShiftRightLogical:
  case YarivOpShiftRightArithmetic:
  case YarivOpShiftLeftLogical:
  case YarivOpBitwiseOr:
  case YarivOpBitwiseXor:
  case YarivOpBitwiseAnd:
  case YarivOpNot:
  case YarivOpBitFieldInsert:
  case YarivOpBitFieldSExtract:
  case YarivOpBitFieldUExtract:
  case YarivOpBitReverse:
  case YarivOpBitCount:
  case YarivOpAny:
  case YarivOpAll:
  case YarivOpIsNan:
  case YarivOpIsInf:
  case YarivOpIsFinite:
  case YarivOpIsNormal:
  case YarivOpSignBitSet:
  case YarivOpLessOrGreater:
  case YarivOpOrdered:
  case YarivOpUnordered:
  case YarivOpLogicalEqual:
  case YarivOpLogicalNotEqual:
  case YarivOpLogicalOr:
  case YarivOpLogicalAnd:
  case YarivOpLogicalNot:
  case YarivOpSelect:
  case YarivOpIEqual:
  case YarivOpINotEqual:
  case YarivOpUGreaterThan:
  case YarivOpSGreaterThan:
  case YarivOpUGreaterThanEqual:
  case YarivOpSGreaterThanEqual:
  case YarivOpULessThan:
  case YarivOpSLessThan:
  case YarivOpULessThanEqual:
  case YarivOpSLessThanEqual:
  case YarivOpFOrdEqual:
  case YarivOpFUnordEqual:
  case YarivOpFOrdNotEqual:
  case YarivOpFUnordNotEqual:
  case YarivOpFOrdLessThan:
  case YarivOpFUnordLessThan:
  case YarivOpFOrdGreaterThan:
  case YarivOpFUnordGreaterThan:
  case YarivOpFOrdLessThanEqual:
  case YarivOpFUnordLessThanEqual:
  case YarivOpFOrdGreaterThanEqual:
  case YarivOpFUnordGreaterThanEqual:
  case YarivOpDPdx:
  case YarivOpDPdy:
  case YarivOpFwidth:
  case YarivOpDPdxFine:
  case YarivOpDPdyFine:
  case YarivOpFwidthFine:
  case YarivOpDPdxCoarse:
  case YarivOpDPdyCoarse:
  case YarivOpFwidthCoarse:
  case YarivOpPhi:
  case YarivOpAtomicLoad:
  case YarivOpAtomicStore:
  case YarivOpAtomicExchange:
  case YarivOpAtomicCompareExchange:
  case YarivOpAtomicCompareExchangeWeak:
  case YarivOpAtomicIIncrement:
  case YarivOpAtomicIDecrement:
  case YarivOpAtomicIAdd:
  case YarivOpAtomicISub:
  case YarivOpAtomicSMin:
  case YarivOpAtomicUMin:
  case YarivOpAtomicSMax:
  case YarivOpAtomicUMax:
  case YarivOpAtomicAnd:
  case YarivOpAtomicOr:
  case YarivOpAtomicXor:
  case YarivOpAtomicFlagTestAndSet:
  case YarivOpLoadWithMemoryAccess:
  case YarivOpStoreWithMemoryAccess:
  case YarivOpVariableWithInitializer:
  case YarivOpConstantWithManyLiterals:
  case YarivOpAccessChainWithOneIndex:
  case YarivOpAccessChainWithTwoIndices:
  case YarivOpAccessChainWithThreeIndices:
  case YarivOpVectorShuffleTwoLiterals:
  case YarivOpVectorShuffleThreeLiterals:
  case YarivOpVectorShuffleFourLiterals:
  case YarivOpVectorSwizzleTwoLiterals:
  case YarivOpVectorSwizzleThreeLiterals:
  case YarivOpVectorSwizzleFourLiterals:
  case YarivOpCompositeExtractWithLiteralZero:
  case YarivOpCompositeExtractWithLiteralOne:
  case YarivOpCompositeExtractWithLiteralTwo:
  case YarivOpCompositeExtractWithLiteralThree:
  case YarivOpCompositeConstructOneConstituent:
  case YarivOpCompositeConstructTwoConstituents:
  case YarivOpCompositeConstructThreeConstituents:
  case YarivOpConstantWithOneFloatLiteral:
    return 1;
  }
}

int yariv_opcode_has_word_count(uint32_t opcode, uint32_t *word_count) {
  uint32_t count;
  switch (opcode) {
  default:
    // word count was not static for this opcode, so we can't infer it!
    return 0;
  case YarivOpNop:
  case YarivOpNoLine:
  case YarivOpFunctionEnd:
  case YarivOpKill:
  case YarivOpReturn:
  case YarivOpEmitVertex:
  case YarivOpEndPrimitive:
    count = 1;
    break;
  case YarivOpDecorationGroup:
  case YarivOpCapability:
  case YarivOpTypeVoid:
  case YarivOpTypeBool:
  case YarivOpTypeSampler:
  case YarivOpTypeEvent:
  case YarivOpTypeDeviceEvent:
  case YarivOpTypeReserveId:
  case YarivOpTypeQueue:
  case YarivOpLabel:
  case YarivOpBranch:
  case YarivOpReturnValue:
  case YarivOpEmitStreamVertex:
  case YarivOpEndStreamPrimitive:
  case YarivOpRetainEvent:
  case YarivOpReleaseEvent:
    count = 2;
    break;
  case YarivOpUndef:
  case YarivOpMemoryModel:
  case YarivOpTypeFloat:
  case YarivOpTypeSampledImage:
  case YarivOpTypeRuntimeArray:
  case YarivOpTypePipe:
  case YarivOpTypeForwardPointer:
  case YarivOpConstantTrue:
  case YarivOpConstantFalse:
  case YarivOpConstantNull:
  case YarivOpSpecConstantTrue:
  case YarivOpSpecConstantFalse:
  case YarivOpFunctionParameter:
  case YarivOpSelectionMerge:
  case YarivOpLifetimeStart:
  case YarivOpLifetimeStop:
  case YarivOpMemoryBarrier:
  case YarivOpCreateUserEvent:
  case YarivOpSetUserEventStatus:
  case YarivOpGetDefaultQueue:
    count = 3;
    break;
  case YarivOpLine:
  case YarivOpTypeInt:
  case YarivOpTypeVector:
  case YarivOpTypeMatrix:
  case YarivOpTypeArray:
  case YarivOpTypePointer:
  case YarivOpGenericPtrMemSemantics:
  case YarivOpImage:
  case YarivOpImageQueryFormat:
  case YarivOpImageQueryOrder:
  case YarivOpImageQuerySize:
  case YarivOpImageQueryLevels:
  case YarivOpImageQuerySamples:
  case YarivOpImageSparseTexelsResident:
  case YarivOpConvertFToU:
  case YarivOpConvertFToS:
  case YarivOpConvertSToF:
  case YarivOpConvertUToF:
  case YarivOpUConvert:
  case YarivOpSConvert:
  case YarivOpFConvert:
  case YarivOpQuantizeToF16:
  case YarivOpConvertPtrToU:
  case YarivOpSatConvertSToU:
  case YarivOpSatConvertUToS:
  case YarivOpConvertUToPtr:
  case YarivOpPtrCastToGeneric:
  case YarivOpGenericCastToPtr:
  case YarivOpBitcast:
  case YarivOpCopyObject:
  case YarivOpTranspose:
  case YarivOpSNegate:
  case YarivOpFNegate:
  case YarivOpNot:
  case YarivOpBitReverse:
  case YarivOpBitCount:
  case YarivOpAny:
  case YarivOpAll:
  case YarivOpIsNan:
  case YarivOpIsInf:
  case YarivOpIsFinite:
  case YarivOpIsNormal:
  case YarivOpSignBitSet:
  case YarivOpLogicalNot:
  case YarivOpDPdx:
  case YarivOpDPdy:
  case YarivOpFwidth:
  case YarivOpDPdxFine:
  case YarivOpDPdyFine:
  case YarivOpFwidthFine:
  case YarivOpDPdxCoarse:
  case YarivOpDPdyCoarse:
  case YarivOpFwidthCoarse:
  case YarivOpLoopMerge:
  case YarivOpAtomicFlagClear:
  case YarivOpControlBarrier:
  case YarivOpGroupWaitEvents:
  case YarivOpIsValidEvent:
  case YarivOpCaptureEventProfilingInfo:
  case YarivOpIsValidReserveId:
    count = 4;
    break;
  case YarivOpArrayLength:
  case YarivOpFunction:
  case YarivOpSampledImage:
  case YarivOpImageQuerySizeLod:
  case YarivOpImageQueryLod:
  case YarivOpGenericCastToPtrExplicit:
  case YarivOpVectorExtractDynamic:
  case YarivOpIAdd:
  case YarivOpFAdd:
  case YarivOpISub:
  case YarivOpFSub:
  case YarivOpIMul:
  case YarivOpFMul:
  case YarivOpUDiv:
  case YarivOpSDiv:
  case YarivOpFDiv:
  case YarivOpUMod:
  case YarivOpSRem:
  case YarivOpSMod:
  case YarivOpFRem:
  case YarivOpFMod:
  case YarivOpVectorTimesScalar:
  case YarivOpMatrixTimesScalar:
  case YarivOpVectorTimesMatrix:
  case YarivOpMatrixTimesVector:
  case YarivOpMatrixTimesMatrix:
  case YarivOpOuterProduct:
  case YarivOpDot:
  case YarivOpIAddCarry:
  case YarivOpISubBorrow:
  case YarivOpUMulExtended:
  case YarivOpSMulExtended:
  case YarivOpShiftRightLogical:
  case YarivOpShiftRightArithmetic:
  case YarivOpShiftLeftLogical:
  case YarivOpBitwiseOr:
  case YarivOpBitwiseXor:
  case YarivOpBitwiseAnd:
  case YarivOpLessOrGreater:
  case YarivOpOrdered:
  case YarivOpUnordered:
  case YarivOpLogicalEqual:
  case YarivOpLogicalNotEqual:
  case YarivOpLogicalOr:
  case YarivOpLogicalAnd:
  case YarivOpIEqual:
  case YarivOpINotEqual:
  case YarivOpUGreaterThan:
  case YarivOpSGreaterThan:
  case YarivOpUGreaterThanEqual:
  case YarivOpSGreaterThanEqual:
  case YarivOpULessThan:
  case YarivOpSLessThan:
  case YarivOpULessThanEqual:
  case YarivOpSLessThanEqual:
  case YarivOpFOrdEqual:
  case YarivOpFUnordEqual:
  case YarivOpFOrdNotEqual:
  case YarivOpFUnordNotEqual:
  case YarivOpFOrdLessThan:
  case YarivOpFUnordLessThan:
  case YarivOpFOrdGreaterThan:
  case YarivOpFUnordGreaterThan:
  case YarivOpFOrdLessThanEqual:
  case YarivOpFUnordLessThanEqual:
  case YarivOpFOrdGreaterThanEqual:
  case YarivOpFUnordGreaterThanEqual:
  case YarivOpAtomicStore:
  case YarivOpGroupAll:
  case YarivOpGroupAny:
  case YarivOpCommitReadPipe:
  case YarivOpCommitWritePipe:
    count = 5;
    break;
  case YarivOpConstantSampler:
  case YarivOpImageTexelPointer:
  case YarivOpVectorInsertDynamic:
  case YarivOpBitFieldSExtract:
  case YarivOpBitFieldUExtract:
  case YarivOpSelect:
  case YarivOpAtomicLoad:
  case YarivOpAtomicIIncrement:
  case YarivOpAtomicIDecrement:
  case YarivOpAtomicFlagTestAndSet:
  case YarivOpGroupBroadcast:
  case YarivOpGroupIAdd:
  case YarivOpGroupFAdd:
  case YarivOpGroupFMin:
  case YarivOpGroupUMin:
  case YarivOpGroupSMin:
  case YarivOpGroupFMax:
  case YarivOpGroupUMax:
  case YarivOpGroupSMax:
  case YarivOpBuildNDRange:
  case YarivOpGetNumPipePackets:
  case YarivOpGetMaxPipePackets:
  case YarivOpGroupCommitReadPipe:
  case YarivOpGroupCommitWritePipe:
    count = 6;
    break;
  case YarivOpBitFieldInsert:
  case YarivOpAtomicExchange:
  case YarivOpAtomicIAdd:
  case YarivOpAtomicISub:
  case YarivOpAtomicSMin:
  case YarivOpAtomicUMin:
  case YarivOpAtomicSMax:
  case YarivOpAtomicUMax:
  case YarivOpAtomicAnd:
  case YarivOpAtomicOr:
  case YarivOpAtomicXor:
  case YarivOpEnqueueMarker:
  case YarivOpGetKernelWorkGroupSize:
  case YarivOpGetKernelPreferredWorkGroupSizeMultiple:
  case YarivOpReadPipe:
  case YarivOpWritePipe:
  case YarivOpReserveReadPipePackets:
  case YarivOpReserveWritePipePackets:
    count = 7;
    break;
  case YarivOpGetKernelNDrangeSubGroupCount:
  case YarivOpGetKernelNDrangeMaxSubGroupSize:
  case YarivOpGroupReserveReadPipePackets:
  case YarivOpGroupReserveWritePipePackets:
    count = 8;
    break;
  case YarivOpAtomicCompareExchange:
  case YarivOpAtomicCompareExchangeWeak:
  case YarivOpGroupAsyncCopy:
  case YarivOpReservedReadPipe:
  case YarivOpReservedWritePipe:
    count = 9;
    break;
  case YarivOpStore: // beware: in SPIR-V this has a non constant encoding!
  case YarivOpDecorateWithNoLiterals:   // this fake opcode has a static size
  case YarivOpDecorateRelaxedPrecision: // this fake opcode has a static size
    count = 3;
    break;
  case YarivOpConstant: // beware: in SPIR-V this has a non constant encoding!
  case YarivOpLoad:     // beware: in SPIR-V this has a non constant encoding!
  case YarivOpVariable: // beware: in SPIR-V this has a non constant encoding!
  case YarivOpStoreWithMemoryAccess:  // this fake opcode has a static size
  case YarivOpDecorateWithOneLiteral: // this fake opcode has a static size
  case YarivOpMemberDecorateWithNoLiterals:     // this fake opcode has a static
                                                // size
  case YarivOpCompositeConstructOneConstituent: // this fake opcode has a static
                                                // size
  case YarivOpConstantWithOneFloatLiteral: // this fake opcode has a static size
  case YarivOpDecorateLocation:            // this fake opcode has a static size
    count = 4;
    break;
  case YarivOpLoadWithMemoryAccess: // this fake opcode has a static size
  case YarivOpMemberDecorateWithOneLiteral: // this fake opcode has a static
                                            // size
  case YarivOpVariableWithInitializer:  // this fake opcode has a static size
  case YarivOpConstantWithManyLiterals: // this fake opcode has a static size
  case YarivOpAccessChainWithOneIndex:  // this fake opcode has a static size
  case YarivOpCompositeExtractWithLiteralZero:  // this fake opcode has a static
                                                // size
  case YarivOpCompositeExtractWithLiteralOne:   // this fake opcode has a static
                                                // size
  case YarivOpCompositeExtractWithLiteralTwo:   // this fake opcode has a static
                                                // size
  case YarivOpCompositeExtractWithLiteralThree: // this fake opcode has a static
                                                // size
  case YarivOpCompositeConstructTwoConstituents: // this fake opcode has a
                                                 // static size
  case YarivOpMemberDecorateOffset: // this fake opcode has a static size
    count = 5;
    break;
  case YarivOpAccessChainWithTwoIndices: // this fake opcode has a static size
  case YarivOpCompositeConstructThreeConstituents: // this fake opcode has a
                                                   // static size
    count = 6;
    break;
  case YarivOpAccessChainWithThreeIndices: // this fake opcode has a static size
  case YarivOpVectorShuffleTwoLiterals:    // this fake opcode has a static size
  case YarivOpVectorSwizzleTwoLiterals:    // this fake opcode has a static size
    count = 7;
    break;
  case YarivOpVectorShuffleThreeLiterals: // this fake opcode has a static size
  case YarivOpVectorSwizzleThreeLiterals: // this fake opcode has a static size
    count = 8;
    break;
  case YarivOpVectorShuffleFourLiterals: // this fake opcode has a static size
  case YarivOpVectorSwizzleFourLiterals: // this fake opcode has a static size
    count = 9;
    break;
  }

  if (word_count) {
    *word_count = count;
  }
  return 1;
}

uint32_t yariv_shuffle_opcode(uint32_t opcode) {
  switch (opcode) {
  default:
    return opcode;
#define YARIV_SWAP(x, y)                                                       \
  case (x):                                                                    \
    return (y);                                                                \
  case (y):                                                                    \
    return (x)
    YARIV_SWAP(YarivOpLabel, YarivOpNop);
    YARIV_SWAP(YarivOpFSub, YarivOpUndef);
    YARIV_SWAP(YarivOpFMul, YarivOpSourceContinued);
    YARIV_SWAP(YarivOpFAdd, YarivOpSource);
    YARIV_SWAP(YarivOpBranch, YarivOpSourceExtension);
    YARIV_SWAP(YarivOpDecorateWithNoLiterals, YarivOpLine);
    YARIV_SWAP(YarivOpDecorateWithOneLiteral, YarivOpExtension);
    YARIV_SWAP(YarivOpMemberDecorateWithNoLiterals, YarivOpExtInstImport);
    YARIV_SWAP(YarivOpMemberDecorateWithOneLiteral, YarivOpMemoryModel);
    YARIV_SWAP(YarivOpAccessChainWithOneIndex, YarivOpExecutionMode);
    YARIV_SWAP(YarivOpAccessChainWithTwoIndices, YarivOpCapability);
    YARIV_SWAP(YarivOpAccessChainWithThreeIndices, YarivOpTypeVoid);
    YARIV_SWAP(YarivOpVectorShuffleTwoLiterals, YarivOp9);
    YARIV_SWAP(YarivOpVectorShuffleThreeLiterals, YarivOp13);
    YARIV_SWAP(YarivOpVectorShuffleFourLiterals, YarivOp18);
    YARIV_SWAP(YarivOpDot, YarivOp40);
    YARIV_SWAP(YarivOpVectorTimesScalar, YarivOp47);
    YARIV_SWAP(YarivOpFDiv, YarivOp53);
    YARIV_SWAP(YarivOpVectorSwizzleTwoLiterals, YarivOp58);
    YARIV_SWAP(YarivOpVectorSwizzleThreeLiterals, YarivOp76);
    YARIV_SWAP(YarivOpVectorSwizzleFourLiterals, YarivOp85);
    YARIV_SWAP(YarivOpBranchConditional, YarivOpTypeEvent);
    YARIV_SWAP(YarivOpSelectionMerge, YarivOpTypeDeviceEvent);
    YARIV_SWAP(YarivOpCompositeExtractWithLiteralZero,
               YarivOpGenericPtrMemSemantics);
    YARIV_SWAP(YarivOpCompositeExtractWithLiteralOne, YarivOpPtrCastToGeneric);
    YARIV_SWAP(YarivOpCompositeExtractWithLiteralTwo, YarivOpGenericCastToPtr);
    YARIV_SWAP(YarivOpCompositeExtractWithLiteralThree,
               YarivOpGenericCastToPtrExplicit);
    YARIV_SWAP(YarivOpCompositeConstructOneConstituent, YarivOpTypeReserveId);
    YARIV_SWAP(YarivOpCompositeConstructTwoConstituents, YarivOpTypeQueue);
    YARIV_SWAP(YarivOpCompositeConstructThreeConstituents, YarivOpTypePipe);
    YARIV_SWAP(YarivOpDecorateRelaxedPrecision, YarivOpImageTexelPointer);
    YARIV_SWAP(YarivOpMemberDecorateOffset, YarivOp108);
    YARIV_SWAP(YarivOpConstantWithOneFloatLiteral, YarivOpTypeForwardPointer);
    YARIV_SWAP(YarivOpDecorateLocation, YarivOp125);
    YARIV_SWAP(YarivOpReturnValue, YarivOpTypeRuntimeArray);
    YARIV_SWAP(YarivOpFOrdEqual, YarivOpCompositeInsert);
    YARIV_SWAP(YarivOpFOrdLessThanEqual, YarivOpVectorExtractDynamic);
    YARIV_SWAP(YarivOpFOrdLessThan, YarivOpConstantFalse);
    YARIV_SWAP(YarivOpIEqual, YarivOpTranspose);
    YARIV_SWAP(YarivOpReturn, YarivOpSNegate);
    YARIV_SWAP(YarivOpIAdd, YarivOpCopyObject);
#undef YARIV_SWAP
  }
}

size_t yariv_encode_signed_varint(int32_t i, uint8_t *out) {
  // we use zig-zag encoding of the original integer
  return yariv_encode_unsigned_varint(
      YARIV_CAST(uint32_t, (i << 1) ^ (i >> 31)), out);
}

size_t yariv_decode_signed_varint(const uint8_t *in, int32_t *out) {
  uint32_t u;
  const size_t r = yariv_decode_unsigned_varint(in, &u);

  if (out) {
    // we use zig-zag decoding of the unsigned integer
    *out = YARIV_CAST(int32_t, u >> 1) ^ -(YARIV_CAST(int32_t, u) & 1);
  }

  return r;
}

size_t yariv_encode_unsigned_varint(uint32_t i, uint8_t *out) {
  size_t r = 0;

  while (i > 127) {
    if (out) {
      out[r] = (YARIV_CAST(uint8_t, i) & 0x7Fu) | 0x80u;
    }
    i >>= 7;
    r++;
  }

  if (out) {
    out[r] = YARIV_CAST(uint8_t, i);
  }

  return r + 1;
}

size_t yariv_decode_unsigned_varint(const uint8_t *in, uint32_t *out) {
  uint32_t r = 0;
  size_t i;

  // the maximum length of a uint32_t varint is 5 bytes
  for (i = 0; i < 5; i++) {
    // bring in the 7 significant bits of this part of the varint
    r |= (0x7Fu & in[i]) << 7 * i;

    // if we don't have the 'there is another byte in our varint' bit set
    if (0u == (0x80u & in[i])) {
      if (out) {
        *out = r;
      }
      return i + 1;
    }
  }

  return 0;
}

#undef YARIV_CAST

#ifdef __cplusplus
}
#endif

#endif // SHEREDOM_YARIV_H_INCLUDED
