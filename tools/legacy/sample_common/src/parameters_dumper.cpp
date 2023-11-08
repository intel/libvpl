/*############################################################################
  # Copyright (C) 2005 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "parameters_dumper.h"
#include <stdio.h>
#include <fstream>
#include <string>
#include <vector>

#include "vm/strings_defs.h"

#include "mfxplugin.h"
#include "vpl/mfxjpeg.h"
#include "vpl/mfxstructures.h"
#include "vpl/mfxvideo++.h"
#include "vpl/mfxvideo.h"

#include "vpl/mfxmvc.h"
#include "vpl/mfxvp8.h"

#ifndef MFX_VERSION
    #error MFX_VERSION not defined
#endif

std::ostream& operator<<(std::ostream& stream, const mfxU8 byte) {
    stream << static_cast<unsigned int>(byte);
    return stream;
}

#define START_PROC_ARRAY(arrName)                                                            \
    for (unsigned int arrIdx = 0; arrIdx < (sizeof(info.arrName) / sizeof(info.arrName[0])); \
         arrIdx++) {
#define START_PROC_ARRAY_SIZE(arrName, numElems) \
    for (unsigned int arrIdx = 0; arrIdx < info.numElems; arrIdx++) {
#define SERIALIZE_INFO_ELEMENT(prefix, arrName, name)                       \
    {                                                                       \
        sstr << prefix << #arrName << "[" << arrIdx << "]." << #name << ":" \
             << info.arrName[arrIdx].name << std::endl;                     \
    }
#define SERIALIZE_INFO_ARRAY_ELEMENT(prefix, arrName, name)                          \
    {                                                                                \
        sstr << prefix << #arrName << "[" << arrIdx << "]." << #name << ":";         \
        for (unsigned int idx = 0;                                                   \
             idx < (sizeof(info.arrName[0].name) / sizeof(info.arrName[0].name[0])); \
             idx++) {                                                                \
            sstr << info.arrName[arrIdx].name[idx] << " ";                           \
        };                                                                           \
        sstr << std::endl;                                                           \
    }
#define END_PROC_ARRAY }
#define SERIALIZE_INFO(prefix, name) \
    { sstr << prefix << #name << ":" << info.name << std::endl; }
#define SERIALIZE_INFO_ARRAY(prefix, name)                                                    \
    {                                                                                         \
        sstr << prefix << #name << ":";                                                       \
        for (unsigned int idx = 0; idx < (sizeof(info.name) / sizeof(info.name[0])); idx++) { \
            sstr << info.name[idx] << " ";                                                    \
        };                                                                                    \
        sstr << std::endl;                                                                    \
    }
#define SERIALIZE_INFO_MEMORY(prefix, name, numElems)            \
    {                                                            \
        sstr << prefix << #name << ":";                          \
        for (unsigned int idx = 0; idx < info.numElems; idx++) { \
            sstr << info.name[idx] << " ";                       \
        };                                                       \
        sstr << std::endl;                                       \
    }

void CParametersDumper::SerializeFrameInfoStruct(std::ostream& sstr,
                                                 const char* prefix,
                                                 mfxFrameInfo& info) {
    SERIALIZE_INFO_ARRAY(prefix, reserved);
    SERIALIZE_INFO(prefix, ChannelId);

    SERIALIZE_INFO(prefix, BitDepthLuma);
    SERIALIZE_INFO(prefix, BitDepthChroma);
    SERIALIZE_INFO(prefix, Shift);

    SERIALIZE_INFO(prefix, FrameId.TemporalId);
    SERIALIZE_INFO(prefix, FrameId.PriorityId);
    SERIALIZE_INFO(prefix, FrameId.DependencyId);
    SERIALIZE_INFO(prefix, FrameId.QualityId);
    SERIALIZE_INFO(prefix, FrameId.ViewId);

    SERIALIZE_INFO(prefix, FourCC);
    char strFourCC[5] = {};
    MSDK_MEMCPY(strFourCC, sizeof(strFourCC), &info.FourCC, 4);
    sstr << prefix << "FourCC(string)"
         << ":" << strFourCC << std::endl;

    SERIALIZE_INFO(prefix, Width);
    SERIALIZE_INFO(prefix, Height);

    SERIALIZE_INFO(prefix, CropX);
    SERIALIZE_INFO(prefix, CropY);
    SERIALIZE_INFO(prefix, CropW);
    SERIALIZE_INFO(prefix, CropH);
    SERIALIZE_INFO(prefix, BufferSize);
    SERIALIZE_INFO(prefix, reserved5);

    SERIALIZE_INFO(prefix, FrameRateExtN);
    SERIALIZE_INFO(prefix, FrameRateExtD);
    SERIALIZE_INFO(prefix, reserved3);

    SERIALIZE_INFO(prefix, AspectRatioW);
    SERIALIZE_INFO(prefix, AspectRatioH);

    SERIALIZE_INFO(prefix, PicStruct);
    SERIALIZE_INFO(prefix, ChromaFormat);
    SERIALIZE_INFO(prefix, reserved2);
}

void CParametersDumper::SerializeMfxInfoMFXStruct(std::ostream& sstr,
                                                  const char* prefix,
                                                  mfxInfoMFX& info) {
    SERIALIZE_INFO_ARRAY(prefix, reserved);

    SERIALIZE_INFO(prefix, LowPower);
    SERIALIZE_INFO(prefix, BRCParamMultiplier);
    std::stringstream frameinfo_prefix;
    frameinfo_prefix << prefix << "FrameInfo.";
    SerializeFrameInfoStruct(sstr, frameinfo_prefix.str().c_str(), info.FrameInfo);
    SERIALIZE_INFO(prefix, CodecId);
    char strID[5] = {};
    MSDK_MEMCPY(strID, sizeof(strID), &info.CodecId, 4);
    sstr << prefix << "CodecID(string)"
         << ":" << strID << std::endl;

    SERIALIZE_INFO(prefix, CodecProfile);
    SERIALIZE_INFO(prefix, CodecLevel);
    SERIALIZE_INFO(prefix, NumThread);

    SERIALIZE_INFO(prefix, TargetUsage);

    SERIALIZE_INFO(prefix, GopPicSize);
    SERIALIZE_INFO(prefix, GopRefDist);
    SERIALIZE_INFO(prefix, GopOptFlag);
    SERIALIZE_INFO(prefix, IdrInterval);

    SERIALIZE_INFO(prefix, RateControlMethod);
    SERIALIZE_INFO(prefix, InitialDelayInKB);
    SERIALIZE_INFO(prefix, QPI);
    SERIALIZE_INFO(prefix, Accuracy);
    SERIALIZE_INFO(prefix, BufferSizeInKB);
    SERIALIZE_INFO(prefix, TargetKbps);
    SERIALIZE_INFO(prefix, QPP);
    SERIALIZE_INFO(prefix, ICQQuality);
    SERIALIZE_INFO(prefix, MaxKbps);
    SERIALIZE_INFO(prefix, QPB);
    SERIALIZE_INFO(prefix, Convergence);

    SERIALIZE_INFO(prefix, NumSlice);
    SERIALIZE_INFO(prefix, NumRefFrame);
    SERIALIZE_INFO(prefix, EncodedOrder);

    /* Decoding Options */
    SERIALIZE_INFO(prefix, DecodedOrder);
    SERIALIZE_INFO(prefix, ExtendedPicStruct);
    SERIALIZE_INFO(prefix, TimeStampCalc);
    SERIALIZE_INFO(prefix, SliceGroupsPresent);
    SERIALIZE_INFO(prefix, MaxDecFrameBuffering);
    SERIALIZE_INFO(prefix, EnableReallocRequest);
#if defined(MFX_VERSION_NEXT) && (MFX_VERSION >= MFX_VERSION_NEXT)
    SERIALIZE_INFO(prefix, FilmGrain);
    SERIALIZE_INFO(prefix, SkipOutput);
#endif

    /* JPEG Decoding Options */
    SERIALIZE_INFO(prefix, JPEGChromaFormat);
    SERIALIZE_INFO(prefix, Rotation);
    SERIALIZE_INFO(prefix, JPEGColorFormat);
    SERIALIZE_INFO(prefix, InterleavedDec);
    SERIALIZE_INFO_ARRAY(prefix, SamplingFactorH);
    SERIALIZE_INFO_ARRAY(prefix, SamplingFactorV);

    /* JPEG Encoding Options */
    SERIALIZE_INFO(prefix, Interleaved);
    SERIALIZE_INFO(prefix, Quality);
    SERIALIZE_INFO(prefix, RestartInterval);
}

void CParametersDumper::SerializeExtensionBuffer(std::ostream& sstr,
                                                 const char* prefix,
                                                 mfxExtBuffer* pExtBuffer) {
    char name[6] = "    .";
    MSDK_MEMCPY(name, sizeof(name), &pExtBuffer->BufferId, 4);
    std::string strName(name);
    std::string name_prefix(prefix);
    name_prefix += strName;

    // Serializing header
    {
        mfxExtBuffer& info = *pExtBuffer;
        SERIALIZE_INFO(prefix, BufferId);
        SERIALIZE_INFO(prefix, BufferSz);
    }

    // Serializing particular Ext buffer.
    switch (pExtBuffer->BufferId) {
        case MFX_EXTBUFF_THREADS_PARAM: {
            mfxExtThreadsParam& info = *(mfxExtThreadsParam*)pExtBuffer;
            SERIALIZE_INFO(prefix, NumThread);
            SERIALIZE_INFO(prefix, SchedulingType);
            SERIALIZE_INFO(prefix, Priority);
            SERIALIZE_INFO_ARRAY(prefix, reserved);
        } break;
        case MFX_EXTBUFF_JPEG_QT: {
            mfxExtJPEGQuantTables& info = *(mfxExtJPEGQuantTables*)pExtBuffer;
            SERIALIZE_INFO_ARRAY(prefix, reserved);
            SERIALIZE_INFO(prefix, NumTable);
            SERIALIZE_INFO_ARRAY(prefix, Qm[0]);
            SERIALIZE_INFO_ARRAY(prefix, Qm[1]);
            SERIALIZE_INFO_ARRAY(prefix, Qm[2]);
            SERIALIZE_INFO_ARRAY(prefix, Qm[3]);
        } break;
        case MFX_EXTBUFF_JPEG_HUFFMAN: {
            mfxExtJPEGHuffmanTables& info = *(mfxExtJPEGHuffmanTables*)pExtBuffer;
            SERIALIZE_INFO_ARRAY(prefix, reserved);
            SERIALIZE_INFO(prefix, NumDCTable);
            SERIALIZE_INFO(prefix, NumACTable);
            START_PROC_ARRAY(DCTables)
            SERIALIZE_INFO_ARRAY_ELEMENT(prefix, DCTables, Bits);
            SERIALIZE_INFO_ARRAY_ELEMENT(prefix, DCTables, Values);
            END_PROC_ARRAY
            START_PROC_ARRAY(DCTables)
            SERIALIZE_INFO_ARRAY_ELEMENT(prefix, ACTables, Bits);
            SERIALIZE_INFO_ARRAY_ELEMENT(prefix, ACTables, Values);
            END_PROC_ARRAY
        } break;
        case MFX_EXTBUFF_MVC_SEQ_DESC: {
            mfxExtMVCSeqDesc& info = *(mfxExtMVCSeqDesc*)pExtBuffer;
            SERIALIZE_INFO(prefix, NumView);
            SERIALIZE_INFO(prefix, NumViewAlloc);
            //DO_MANUALLY:     mfxMVCViewDependency *View;
            SERIALIZE_INFO(prefix, NumViewId);
            SERIALIZE_INFO(prefix, NumViewIdAlloc);
            SERIALIZE_INFO_MEMORY(prefix, ViewId, NumViewId);
            SERIALIZE_INFO(prefix, NumOP);
            SERIALIZE_INFO(prefix, NumOPAlloc);
            //DO_MANUALLY:     mfxMVCOperationPoint *OP;
            SERIALIZE_INFO(prefix, NumRefsTotal);
            SERIALIZE_INFO_ARRAY(prefix, Reserved);
        } break;
        case MFX_EXTBUFF_MVC_TARGET_VIEWS: {
            mfxExtMVCTargetViews& info = *(mfxExtMVCTargetViews*)pExtBuffer;
            SERIALIZE_INFO(prefix, TemporalId);
            SERIALIZE_INFO(prefix, NumView);
            SERIALIZE_INFO_ARRAY(prefix, ViewId);
        } break;
        case MFX_EXTBUFF_CODING_OPTION: {
            mfxExtCodingOption& info = *(mfxExtCodingOption*)pExtBuffer;
            SERIALIZE_INFO(prefix, reserved1);
            SERIALIZE_INFO(prefix, RateDistortionOpt);
            SERIALIZE_INFO(prefix, MECostType);
            SERIALIZE_INFO(prefix, MESearchType);
            SERIALIZE_INFO(prefix, MVSearchWindow.x);
            SERIALIZE_INFO(prefix, MVSearchWindow.y);
            SERIALIZE_INFO(prefix, EndOfSequence);
            SERIALIZE_INFO(prefix, FramePicture);
            SERIALIZE_INFO(prefix, CAVLC);
            SERIALIZE_INFO_ARRAY(prefix, reserved2);
            SERIALIZE_INFO(prefix, RecoveryPointSEI);
            SERIALIZE_INFO(prefix, ViewOutput);
            SERIALIZE_INFO(prefix, NalHrdConformance);
            SERIALIZE_INFO(prefix, SingleSeiNalUnit);
            SERIALIZE_INFO(prefix, VuiVclHrdParameters);
            SERIALIZE_INFO(prefix, RefPicListReordering);
            SERIALIZE_INFO(prefix, ResetRefList);
            SERIALIZE_INFO(prefix, RefPicMarkRep);
            SERIALIZE_INFO(prefix, FieldOutput);
            SERIALIZE_INFO(prefix, IntraPredBlockSize);
            SERIALIZE_INFO(prefix, InterPredBlockSize);
            SERIALIZE_INFO(prefix, MVPrecision);
            SERIALIZE_INFO(prefix, MaxDecFrameBuffering);
            SERIALIZE_INFO(prefix, AUDelimiter);
            SERIALIZE_INFO(prefix, EndOfStream);
            SERIALIZE_INFO(prefix, PicTimingSEI);
            SERIALIZE_INFO(prefix, VuiNalHrdParameters);
        } break;
        case MFX_EXTBUFF_CODING_OPTION2: {
            mfxExtCodingOption2& info = *(mfxExtCodingOption2*)pExtBuffer;
            SERIALIZE_INFO(prefix, IntRefType);
            SERIALIZE_INFO(prefix, IntRefCycleSize);
            SERIALIZE_INFO(prefix, IntRefQPDelta);
            SERIALIZE_INFO(prefix, MaxFrameSize);
            SERIALIZE_INFO(prefix, MaxSliceSize);
            SERIALIZE_INFO(prefix, BitrateLimit);
            SERIALIZE_INFO(prefix, MBBRC);
            SERIALIZE_INFO(prefix, ExtBRC);
            SERIALIZE_INFO(prefix, LookAheadDepth);
            SERIALIZE_INFO(prefix, Trellis);
            SERIALIZE_INFO(prefix, RepeatPPS);
            SERIALIZE_INFO(prefix, BRefType);
            SERIALIZE_INFO(prefix, AdaptiveI);
            SERIALIZE_INFO(prefix, AdaptiveB);
            SERIALIZE_INFO(prefix, LookAheadDS);
            SERIALIZE_INFO(prefix, NumMbPerSlice);
            SERIALIZE_INFO(prefix, SkipFrame);
            SERIALIZE_INFO(prefix, MinQPI);
            SERIALIZE_INFO(prefix, MaxQPI);
            SERIALIZE_INFO(prefix, MinQPP);
            SERIALIZE_INFO(prefix, MaxQPP);
            SERIALIZE_INFO(prefix, MinQPB);
            SERIALIZE_INFO(prefix, MaxQPB);
            SERIALIZE_INFO(prefix, FixedFrameRate);
            SERIALIZE_INFO(prefix, DisableDeblockingIdc);
            SERIALIZE_INFO(prefix, DisableVUI);
            SERIALIZE_INFO(prefix, BufferingPeriodSEI);
            SERIALIZE_INFO(prefix, EnableMAD);
            SERIALIZE_INFO(prefix, UseRawRef);
        } break;
        case MFX_EXTBUFF_CODING_OPTION3: {
            mfxExtCodingOption3& info = *(mfxExtCodingOption3*)pExtBuffer;
            SERIALIZE_INFO(prefix, NumSliceI);
            SERIALIZE_INFO(prefix, NumSliceP);
            SERIALIZE_INFO(prefix, NumSliceB);
            SERIALIZE_INFO(prefix, WinBRCMaxAvgKbps);
            SERIALIZE_INFO(prefix, WinBRCSize);
            SERIALIZE_INFO(prefix, QVBRQuality);
            SERIALIZE_INFO(prefix, EnableMBQP);
            SERIALIZE_INFO(prefix, IntRefCycleDist);
            SERIALIZE_INFO(prefix, DirectBiasAdjustment);
            SERIALIZE_INFO(prefix, GlobalMotionBiasAdjustment);
            SERIALIZE_INFO(prefix, MVCostScalingFactor);
            SERIALIZE_INFO(prefix, MBDisableSkipMap);
            SERIALIZE_INFO(prefix, WeightedPred);
            SERIALIZE_INFO(prefix, WeightedBiPred);
            SERIALIZE_INFO(prefix, AspectRatioInfoPresent);
            SERIALIZE_INFO(prefix, OverscanInfoPresent);
            SERIALIZE_INFO(prefix, OverscanAppropriate);
            SERIALIZE_INFO(prefix, TimingInfoPresent);
            SERIALIZE_INFO(prefix, BitstreamRestriction);
            SERIALIZE_INFO(prefix, LowDelayHrd);
            SERIALIZE_INFO(prefix, MotionVectorsOverPicBoundaries);
            SERIALIZE_INFO(prefix, ScenarioInfo);
            SERIALIZE_INFO(prefix, ContentInfo);
            SERIALIZE_INFO(prefix, PRefType);
            SERIALIZE_INFO(prefix, FadeDetection);
            SERIALIZE_INFO(prefix, GPB);
            SERIALIZE_INFO(prefix, MaxFrameSizeI);
            SERIALIZE_INFO(prefix, MaxFrameSizeP);
            SERIALIZE_INFO(prefix, TargetBitDepthLuma);
            SERIALIZE_INFO(prefix, TargetBitDepthChroma);
            SERIALIZE_INFO_ARRAY(prefix, reserved1);
            SERIALIZE_INFO(prefix, EnableQPOffset);
            SERIALIZE_INFO_ARRAY(prefix, QPOffset);
            SERIALIZE_INFO_ARRAY(prefix, NumRefActiveP);
            SERIALIZE_INFO_ARRAY(prefix, NumRefActiveBL0);
            SERIALIZE_INFO_ARRAY(prefix, NumRefActiveBL1);
            SERIALIZE_INFO(prefix, BRCPanicMode);
            SERIALIZE_INFO_ARRAY(prefix, reserved);
        } break;
        case MFX_EXTBUFF_VPP_DONOTUSE: {
            mfxExtVPPDoNotUse& info = *(mfxExtVPPDoNotUse*)pExtBuffer;
            SERIALIZE_INFO(prefix, NumAlg);
            SERIALIZE_INFO_MEMORY(prefix, AlgList, NumAlg);
        } break;
        case MFX_EXTBUFF_VPP_DENOISE: {
            mfxExtVPPDenoise& info = *(mfxExtVPPDenoise*)pExtBuffer;
            SERIALIZE_INFO(prefix, DenoiseFactor);
        } break;
        case MFX_EXTBUFF_VPP_DETAIL: {
            mfxExtVPPDetail& info = *(mfxExtVPPDetail*)pExtBuffer;
            SERIALIZE_INFO(prefix, DetailFactor);
        } break;
        case MFX_EXTBUFF_VPP_PROCAMP: {
            mfxExtVPPProcAmp& info = *(mfxExtVPPProcAmp*)pExtBuffer;
            SERIALIZE_INFO(prefix, Brightness);
            SERIALIZE_INFO(prefix, Contrast);
            SERIALIZE_INFO(prefix, Hue);
            SERIALIZE_INFO(prefix, Saturation);
        } break;
        case MFX_EXTBUFF_VPP_AUXDATA: {
            mfxExtVppAuxData& info = *(mfxExtVppAuxData*)pExtBuffer;
            SERIALIZE_INFO(prefix, SpatialComplexity);
            SERIALIZE_INFO(prefix, TemporalComplexity);
            SERIALIZE_INFO(prefix, PicStruct);
            SERIALIZE_INFO_ARRAY(prefix, reserved);
            SERIALIZE_INFO(prefix, SceneChangeRate);
            SERIALIZE_INFO(prefix, RepeatedFrame);
        } break;
        case MFX_EXTBUFF_CODING_OPTION_SPSPPS: {
            mfxExtCodingOptionSPSPPS& info = *(mfxExtCodingOptionSPSPPS*)pExtBuffer;
            SERIALIZE_INFO(prefix, SPSBuffer);
            SERIALIZE_INFO(prefix, PPSBuffer);
            SERIALIZE_INFO(prefix, SPSBufSize);
            SERIALIZE_INFO(prefix, PPSBufSize);
            SERIALIZE_INFO(prefix, SPSId);
            SERIALIZE_INFO(prefix, PPSId);
        } break;
        case MFX_EXTBUFF_CODING_OPTION_VPS: {
            mfxExtCodingOptionVPS& info = *(mfxExtCodingOptionVPS*)pExtBuffer;
            SERIALIZE_INFO(prefix, VPSBuffer);
            SERIALIZE_INFO(prefix, reserved1);
            SERIALIZE_INFO(prefix, VPSBufSize);
            SERIALIZE_INFO(prefix, VPSId);
            SERIALIZE_INFO_ARRAY(prefix, reserved);
        } break;
        case MFX_EXTBUFF_VPP_VIDEO_SIGNAL_INFO: {
            mfxExtVideoSignalInfo& info = *(mfxExtVideoSignalInfo*)pExtBuffer;
            SERIALIZE_INFO(prefix, VideoFormat);
            SERIALIZE_INFO(prefix, VideoFullRange);
            SERIALIZE_INFO(prefix, ColourDescriptionPresent);
            SERIALIZE_INFO(prefix, ColourPrimaries);
            SERIALIZE_INFO(prefix, TransferCharacteristics);
            SERIALIZE_INFO(prefix, MatrixCoefficients);
        } break;
        case MFX_EXTBUFF_VPP_DOUSE: {
            mfxExtVPPDoUse& info = *(mfxExtVPPDoUse*)pExtBuffer;
            SERIALIZE_INFO(prefix, NumAlg);
            SERIALIZE_INFO(prefix, AlgList);
        } break;
        case MFX_EXTBUFF_AVC_REFLIST_CTRL: {
            mfxExtAVCRefListCtrl& info = *(mfxExtAVCRefListCtrl*)pExtBuffer;
            SERIALIZE_INFO(prefix, NumRefIdxL0Active);
            SERIALIZE_INFO(prefix, NumRefIdxL1Active);
            START_PROC_ARRAY(PreferredRefList)
            SERIALIZE_INFO_ELEMENT(prefix, PreferredRefList, FrameOrder);
            SERIALIZE_INFO_ELEMENT(prefix, PreferredRefList, PicStruct);
            SERIALIZE_INFO_ELEMENT(prefix, PreferredRefList, ViewId);
            SERIALIZE_INFO_ELEMENT(prefix, PreferredRefList, LongTermIdx);
            SERIALIZE_INFO_ARRAY_ELEMENT(prefix, PreferredRefList, reserved);
            END_PROC_ARRAY

            START_PROC_ARRAY(RejectedRefList)
            SERIALIZE_INFO_ELEMENT(prefix, RejectedRefList, FrameOrder);
            SERIALIZE_INFO_ELEMENT(prefix, RejectedRefList, PicStruct);
            SERIALIZE_INFO_ELEMENT(prefix, RejectedRefList, ViewId);
            SERIALIZE_INFO_ELEMENT(prefix, RejectedRefList, LongTermIdx);
            SERIALIZE_INFO_ARRAY_ELEMENT(prefix, RejectedRefList, reserved);
            END_PROC_ARRAY

            START_PROC_ARRAY(LongTermRefList)
            SERIALIZE_INFO_ELEMENT(prefix, LongTermRefList, FrameOrder);
            SERIALIZE_INFO_ELEMENT(prefix, LongTermRefList, PicStruct);
            SERIALIZE_INFO_ELEMENT(prefix, LongTermRefList, ViewId);
            SERIALIZE_INFO_ELEMENT(prefix, LongTermRefList, LongTermIdx);
            SERIALIZE_INFO_ARRAY_ELEMENT(prefix, LongTermRefList, reserved);
            END_PROC_ARRAY

            SERIALIZE_INFO(prefix, ApplyLongTermIdx);
            SERIALIZE_INFO_ARRAY(prefix, reserved);
        } break;
        case MFX_EXTBUFF_VPP_FRAME_RATE_CONVERSION: {
            mfxExtVPPFrameRateConversion& info = *(mfxExtVPPFrameRateConversion*)pExtBuffer;
            SERIALIZE_INFO(prefix, Algorithm);
            SERIALIZE_INFO(prefix, reserved);
            SERIALIZE_INFO_ARRAY(prefix, reserved2);
        } break;
        case MFX_EXTBUFF_VPP_IMAGE_STABILIZATION: {
            mfxExtVPPImageStab& info = *(mfxExtVPPImageStab*)pExtBuffer;
            SERIALIZE_INFO(prefix, Mode);
            SERIALIZE_INFO_ARRAY(prefix, reserved);
        } break;
        case MFX_EXTBUFF_PICTURE_TIMING_SEI: {
            mfxExtPictureTimingSEI& info = *(mfxExtPictureTimingSEI*)pExtBuffer;
            SERIALIZE_INFO_ARRAY(prefix, reserved);

            SERIALIZE_INFO(prefix, TimeStamp[0].ClockTimestampFlag);
            SERIALIZE_INFO(prefix, TimeStamp[0].CtType);
            SERIALIZE_INFO(prefix, TimeStamp[0].NuitFieldBasedFlag);
            SERIALIZE_INFO(prefix, TimeStamp[0].CountingType);
            SERIALIZE_INFO(prefix, TimeStamp[0].FullTimestampFlag);
            SERIALIZE_INFO(prefix, TimeStamp[0].DiscontinuityFlag);
            SERIALIZE_INFO(prefix, TimeStamp[0].CntDroppedFlag);
            SERIALIZE_INFO(prefix, TimeStamp[0].NFrames);
            SERIALIZE_INFO(prefix, TimeStamp[0].SecondsFlag);
            SERIALIZE_INFO(prefix, TimeStamp[0].MinutesFlag);
            SERIALIZE_INFO(prefix, TimeStamp[0].HoursFlag);
            SERIALIZE_INFO(prefix, TimeStamp[0].SecondsValue);
            SERIALIZE_INFO(prefix, TimeStamp[0].MinutesValue);
            SERIALIZE_INFO(prefix, TimeStamp[0].HoursValue);
            SERIALIZE_INFO(prefix, TimeStamp[0].TimeOffset);

            SERIALIZE_INFO(prefix, TimeStamp[1].ClockTimestampFlag);
            SERIALIZE_INFO(prefix, TimeStamp[1].CtType);
            SERIALIZE_INFO(prefix, TimeStamp[1].NuitFieldBasedFlag);
            SERIALIZE_INFO(prefix, TimeStamp[1].CountingType);
            SERIALIZE_INFO(prefix, TimeStamp[1].FullTimestampFlag);
            SERIALIZE_INFO(prefix, TimeStamp[1].DiscontinuityFlag);
            SERIALIZE_INFO(prefix, TimeStamp[1].CntDroppedFlag);
            SERIALIZE_INFO(prefix, TimeStamp[1].NFrames);
            SERIALIZE_INFO(prefix, TimeStamp[1].SecondsFlag);
            SERIALIZE_INFO(prefix, TimeStamp[1].MinutesFlag);
            SERIALIZE_INFO(prefix, TimeStamp[1].HoursFlag);
            SERIALIZE_INFO(prefix, TimeStamp[1].SecondsValue);
            SERIALIZE_INFO(prefix, TimeStamp[1].MinutesValue);
            SERIALIZE_INFO(prefix, TimeStamp[1].HoursValue);
            SERIALIZE_INFO(prefix, TimeStamp[1].TimeOffset);

            SERIALIZE_INFO(prefix, TimeStamp[2].ClockTimestampFlag);
            SERIALIZE_INFO(prefix, TimeStamp[2].CtType);
            SERIALIZE_INFO(prefix, TimeStamp[2].NuitFieldBasedFlag);
            SERIALIZE_INFO(prefix, TimeStamp[2].CountingType);
            SERIALIZE_INFO(prefix, TimeStamp[2].FullTimestampFlag);
            SERIALIZE_INFO(prefix, TimeStamp[2].DiscontinuityFlag);
            SERIALIZE_INFO(prefix, TimeStamp[2].CntDroppedFlag);
            SERIALIZE_INFO(prefix, TimeStamp[2].NFrames);
            SERIALIZE_INFO(prefix, TimeStamp[2].SecondsFlag);
            SERIALIZE_INFO(prefix, TimeStamp[2].MinutesFlag);
            SERIALIZE_INFO(prefix, TimeStamp[2].HoursFlag);
            SERIALIZE_INFO(prefix, TimeStamp[2].SecondsValue);
            SERIALIZE_INFO(prefix, TimeStamp[2].MinutesValue);
            SERIALIZE_INFO(prefix, TimeStamp[2].HoursValue);
            SERIALIZE_INFO(prefix, TimeStamp[2].TimeOffset);
        } break;
        case MFX_EXTBUFF_AVC_TEMPORAL_LAYERS: {
            mfxExtAvcTemporalLayers& info = *(mfxExtAvcTemporalLayers*)pExtBuffer;
            SERIALIZE_INFO_ARRAY(prefix, reserved1);
            SERIALIZE_INFO(prefix, reserved2);
            SERIALIZE_INFO(prefix, BaseLayerPID);
            SERIALIZE_INFO(prefix, Layer[0].Scale);
            SERIALIZE_INFO_ARRAY(prefix, Layer[0].reserved);
            SERIALIZE_INFO(prefix, Layer[1].Scale);
            SERIALIZE_INFO_ARRAY(prefix, Layer[1].reserved);
            SERIALIZE_INFO(prefix, Layer[2].Scale);
            SERIALIZE_INFO_ARRAY(prefix, Layer[2].reserved);
            SERIALIZE_INFO(prefix, Layer[3].Scale);
            SERIALIZE_INFO_ARRAY(prefix, Layer[3].reserved);
            SERIALIZE_INFO(prefix, Layer[4].Scale);
            SERIALIZE_INFO_ARRAY(prefix, Layer[4].reserved);
            SERIALIZE_INFO(prefix, Layer[5].Scale);
            SERIALIZE_INFO_ARRAY(prefix, Layer[5].reserved);
            SERIALIZE_INFO(prefix, Layer[6].Scale);
            SERIALIZE_INFO_ARRAY(prefix, Layer[6].reserved);
            SERIALIZE_INFO(prefix, Layer[7].Scale);
            SERIALIZE_INFO_ARRAY(prefix, Layer[7].reserved);
        } break;
        case MFX_EXTBUFF_ENCODER_CAPABILITY: {
            mfxExtEncoderCapability& info = *(mfxExtEncoderCapability*)pExtBuffer;
            SERIALIZE_INFO(prefix, MBPerSec);
            SERIALIZE_INFO_ARRAY(prefix, reserved);
        } break;
        case MFX_EXTBUFF_ENCODER_RESET_OPTION: {
            mfxExtEncoderResetOption& info = *(mfxExtEncoderResetOption*)pExtBuffer;
            SERIALIZE_INFO(prefix, StartNewSequence);
            SERIALIZE_INFO_ARRAY(prefix, reserved);
        } break;
        case MFX_EXTBUFF_ENCODED_FRAME_INFO: {
            mfxExtAVCEncodedFrameInfo& info = *(mfxExtAVCEncodedFrameInfo*)pExtBuffer;
            SERIALIZE_INFO(prefix, FrameOrder);
            SERIALIZE_INFO(prefix, PicStruct);
            SERIALIZE_INFO(prefix, LongTermIdx);
            SERIALIZE_INFO(prefix, MAD);
            SERIALIZE_INFO(prefix, BRCPanicMode);
            SERIALIZE_INFO(prefix, QP);
            SERIALIZE_INFO(prefix, SecondFieldOffset);
            SERIALIZE_INFO_ARRAY(prefix, reserved);
            SERIALIZE_INFO(prefix, FrameOrder);
            SERIALIZE_INFO(prefix, PicStruct);
            SERIALIZE_INFO(prefix, LongTermIdx);
            SERIALIZE_INFO_ARRAY(prefix, reserved);
        } break;
        case MFX_EXTBUFF_VPP_COMPOSITE: {
            mfxExtVPPComposite& info = *(mfxExtVPPComposite*)pExtBuffer;
            SERIALIZE_INFO(prefix, Y);
            SERIALIZE_INFO(prefix, R);
            SERIALIZE_INFO(prefix, U);
            SERIALIZE_INFO(prefix, G);
            SERIALIZE_INFO(prefix, V);
            SERIALIZE_INFO(prefix, B);
            SERIALIZE_INFO_ARRAY(prefix, reserved1);
            SERIALIZE_INFO(prefix, NumInputStream);
            for (int i = 0; i < info.NumInputStream; i++) {
                std::stringstream sstr;
                sstr << name_prefix << "InputStream[" << i << "].";
                SerializeVPPCompInputStream(sstr, sstr.str().c_str(), info.InputStream[i]);
            }
        } break;
        case MFX_EXTBUFF_VIDEO_SIGNAL_INFO: {
            mfxExtVPPVideoSignalInfo& info = *(mfxExtVPPVideoSignalInfo*)pExtBuffer;
            SERIALIZE_INFO_ARRAY(prefix, reserved1);

            SERIALIZE_INFO(prefix, In.TransferMatrix);
            SERIALIZE_INFO(prefix, In.NominalRange);
            SERIALIZE_INFO_ARRAY(prefix, In.reserved2);
            SERIALIZE_INFO(prefix, Out.TransferMatrix);
            SERIALIZE_INFO(prefix, Out.NominalRange);
            SERIALIZE_INFO_ARRAY(prefix, Out.reserved2);

            SERIALIZE_INFO(prefix, TransferMatrix);
            SERIALIZE_INFO(prefix, NominalRange);
            SERIALIZE_INFO_ARRAY(prefix, reserved3);
        } break;
        case MFX_EXTBUFF_ENCODER_ROI: {
            mfxExtEncoderROI& info = *(mfxExtEncoderROI*)pExtBuffer;
            SERIALIZE_INFO(prefix, NumROI);
            SERIALIZE_INFO(prefix, ROIMode);
            SERIALIZE_INFO_ARRAY(prefix, reserved1);
            START_PROC_ARRAY_SIZE(ROI, NumROI)
            SERIALIZE_INFO_ELEMENT(prefix, ROI, Left);
            SERIALIZE_INFO_ELEMENT(prefix, ROI, Top);
            SERIALIZE_INFO_ELEMENT(prefix, ROI, Right);
            SERIALIZE_INFO_ELEMENT(prefix, ROI, Bottom);
            SERIALIZE_INFO_ELEMENT(prefix, ROI, Priority);
            SERIALIZE_INFO_ELEMENT(prefix, ROI, DeltaQP);

            SERIALIZE_INFO_ARRAY_ELEMENT(prefix, ROI, reserved2);
            END_PROC_ARRAY
        } break;
        case MFX_EXTBUFF_VPP_DEINTERLACING: {
            mfxExtVPPDeinterlacing& info = *(mfxExtVPPDeinterlacing*)pExtBuffer;
            SERIALIZE_INFO(prefix, Mode);
            SERIALIZE_INFO(prefix, TelecinePattern);
            SERIALIZE_INFO(prefix, TelecineLocation);
            SERIALIZE_INFO_ARRAY(prefix, reserved);
        } break;
        case MFX_EXTBUFF_AVC_REFLISTS: {
            mfxExtAVCRefLists& info = *(mfxExtAVCRefLists*)pExtBuffer;
            SERIALIZE_INFO(prefix, NumRefIdxL0Active);
            SERIALIZE_INFO(prefix, NumRefIdxL1Active);
            SERIALIZE_INFO_ARRAY(prefix, reserved);

            START_PROC_ARRAY(RefPicList0)
            SERIALIZE_INFO_ELEMENT(prefix, RefPicList0, FrameOrder);
            SERIALIZE_INFO_ELEMENT(prefix, RefPicList0, PicStruct);
            SERIALIZE_INFO_ELEMENT(prefix, RefPicList0, reserved);
            END_PROC_ARRAY

            START_PROC_ARRAY(RefPicList1)
            SERIALIZE_INFO_ELEMENT(prefix, RefPicList1, FrameOrder);
            SERIALIZE_INFO_ELEMENT(prefix, RefPicList1, PicStruct);
            SERIALIZE_INFO_ELEMENT(prefix, RefPicList1, reserved);
            END_PROC_ARRAY
        } break;
        case MFX_EXTBUFF_VPP_FIELD_PROCESSING: {
            mfxExtVPPFieldProcessing& info = *(mfxExtVPPFieldProcessing*)pExtBuffer;
            SERIALIZE_INFO(prefix, Mode);
            SERIALIZE_INFO(prefix, InField);
            SERIALIZE_INFO(prefix, OutField);
            SERIALIZE_INFO_ARRAY(prefix, reserved);
        } break;
        case MFX_EXTBUFF_DEC_VIDEO_PROCESSING: {
            mfxExtDecVideoProcessing& info = *(mfxExtDecVideoProcessing*)pExtBuffer;
            SERIALIZE_INFO(prefix, In.CropX);
            SERIALIZE_INFO(prefix, In.CropY);
            SERIALIZE_INFO(prefix, In.CropW);
            SERIALIZE_INFO(prefix, In.CropH);
            SERIALIZE_INFO_ARRAY(prefix, In.reserved);
            SERIALIZE_INFO(prefix, Out.FourCC);
            SERIALIZE_INFO(prefix, Out.ChromaFormat);
            SERIALIZE_INFO(prefix, Out.Width);
            SERIALIZE_INFO(prefix, Out.Height);
            SERIALIZE_INFO(prefix, Out.CropX);
            SERIALIZE_INFO(prefix, Out.CropY);
            SERIALIZE_INFO(prefix, Out.CropW);
            SERIALIZE_INFO(prefix, Out.CropH);
            SERIALIZE_INFO_ARRAY(prefix, Out.reserved);
        } break;
        case MFX_EXTBUFF_CHROMA_LOC_INFO: {
            mfxExtChromaLocInfo& info = *(mfxExtChromaLocInfo*)pExtBuffer;
            SERIALIZE_INFO(prefix, ChromaLocInfoPresentFlag);
            SERIALIZE_INFO(prefix, ChromaSampleLocTypeTopField);
            SERIALIZE_INFO(prefix, ChromaSampleLocTypeBottomField);
            SERIALIZE_INFO_ARRAY(prefix, reserved);
        } break;
        case MFX_EXTBUFF_MBQP: {
            mfxExtMBQP& info = *(mfxExtMBQP*)pExtBuffer;
            SERIALIZE_INFO_ARRAY(prefix, reserved);
            SERIALIZE_INFO(prefix, NumQPAlloc);
            SERIALIZE_INFO_MEMORY(prefix, QP, NumQPAlloc);
            SERIALIZE_INFO(prefix, reserved2);
        } break;
        case MFX_EXTBUFF_HEVC_TILES: {
            mfxExtHEVCTiles& info = *(mfxExtHEVCTiles*)pExtBuffer;
            SERIALIZE_INFO(prefix, NumTileRows);
            SERIALIZE_INFO(prefix, NumTileColumns);
            SERIALIZE_INFO_ARRAY(prefix, reserved);
        } break;
        case MFX_EXTBUFF_MB_DISABLE_SKIP_MAP: {
            mfxExtMBDisableSkipMap& info = *(mfxExtMBDisableSkipMap*)pExtBuffer;
            SERIALIZE_INFO_ARRAY(prefix, reserved);
            SERIALIZE_INFO(prefix, MapSize);
            SERIALIZE_INFO_MEMORY(prefix, Map, MapSize);
            SERIALIZE_INFO(prefix, reserved2);
        } break;
        case MFX_EXTBUFF_HEVC_PARAM: {
            mfxExtHEVCParam& info = *(mfxExtHEVCParam*)pExtBuffer;
            SERIALIZE_INFO(prefix, PicWidthInLumaSamples);
            SERIALIZE_INFO(prefix, PicHeightInLumaSamples);
            SERIALIZE_INFO(prefix, GeneralConstraintFlags);
            SERIALIZE_INFO_ARRAY(prefix, reserved);
        } break;
        case MFX_EXTBUFF_DECODED_FRAME_INFO: {
            mfxExtDecodedFrameInfo& info = *(mfxExtDecodedFrameInfo*)pExtBuffer;
            SERIALIZE_INFO(prefix, FrameType);
            SERIALIZE_INFO_ARRAY(prefix, reserved);
        } break;
        case MFX_EXTBUFF_TIME_CODE: {
            mfxExtTimeCode& info = *(mfxExtTimeCode*)pExtBuffer;
            SERIALIZE_INFO(prefix, DropFrameFlag);
            SERIALIZE_INFO(prefix, TimeCodeHours);
            SERIALIZE_INFO(prefix, TimeCodeMinutes);
            SERIALIZE_INFO(prefix, TimeCodeSeconds);
            SERIALIZE_INFO(prefix, TimeCodePictures);
            SERIALIZE_INFO_ARRAY(prefix, reserved);
        } break;
        case MFX_EXTBUFF_HEVC_REGION: {
            mfxExtHEVCRegion& info = *(mfxExtHEVCRegion*)pExtBuffer;
            SERIALIZE_INFO(prefix, RegionId);
            SERIALIZE_INFO(prefix, RegionType);
            SERIALIZE_INFO(prefix, RegionEncoding);
            SERIALIZE_INFO_ARRAY(prefix, reserved);
        } break;
        case MFX_EXTBUFF_PRED_WEIGHT_TABLE: {
            mfxExtPredWeightTable& info = *(mfxExtPredWeightTable*)pExtBuffer;
            SERIALIZE_INFO(prefix, LumaLog2WeightDenom);
            SERIALIZE_INFO(prefix, ChromaLog2WeightDenom);
            SERIALIZE_INFO_ARRAY(prefix, LumaWeightFlag[0]);
            SERIALIZE_INFO_ARRAY(prefix, LumaWeightFlag[1]);
            SERIALIZE_INFO_ARRAY(prefix, ChromaWeightFlag[0]);
            SERIALIZE_INFO_ARRAY(prefix, ChromaWeightFlag[1]);
            //DO_MANUALLY: Weights[2][32][3][2];
            SERIALIZE_INFO_ARRAY(prefix, reserved);
        } break;
        case MFX_EXTBUFF_DIRTY_RECTANGLES: {
            mfxExtDirtyRect& info = *(mfxExtDirtyRect*)pExtBuffer;
            SERIALIZE_INFO(prefix, NumRect);
            SERIALIZE_INFO_ARRAY(prefix, reserved1);

            START_PROC_ARRAY_SIZE(Rect, NumRect)
            SERIALIZE_INFO_ELEMENT(prefix, Rect, Left);
            SERIALIZE_INFO_ELEMENT(prefix, Rect, Top);
            SERIALIZE_INFO_ELEMENT(prefix, Rect, Right);
            SERIALIZE_INFO_ELEMENT(prefix, Rect, Bottom);
            SERIALIZE_INFO_ARRAY_ELEMENT(prefix, Rect, reserved2);
            END_PROC_ARRAY
        } break;
        case MFX_EXTBUFF_MOVING_RECTANGLES: {
            mfxExtMoveRect& info = *(mfxExtMoveRect*)pExtBuffer;
            SERIALIZE_INFO(prefix, NumRect);
            SERIALIZE_INFO_ARRAY(prefix, reserved1);

            START_PROC_ARRAY_SIZE(Rect, NumRect)
            SERIALIZE_INFO_ELEMENT(prefix, Rect, DestLeft);
            SERIALIZE_INFO_ELEMENT(prefix, Rect, DestTop);
            SERIALIZE_INFO_ELEMENT(prefix, Rect, DestRight);
            SERIALIZE_INFO_ELEMENT(prefix, Rect, DestBottom);
            SERIALIZE_INFO_ELEMENT(prefix, Rect, SourceLeft);
            SERIALIZE_INFO_ELEMENT(prefix, Rect, SourceTop);
            SERIALIZE_INFO_ARRAY_ELEMENT(prefix, Rect, reserved2);
            END_PROC_ARRAY
        } break;
        case MFX_EXTBUFF_VPP_ROTATION: {
            mfxExtVPPRotation& info = *(mfxExtVPPRotation*)pExtBuffer;
            SERIALIZE_INFO(prefix, Angle);
            SERIALIZE_INFO_ARRAY(prefix, reserved);
        } break;
        case MFX_EXTBUFF_ENCODED_SLICES_INFO: {
            mfxExtEncodedSlicesInfo& info = *(mfxExtEncodedSlicesInfo*)pExtBuffer;
            SERIALIZE_INFO(prefix, SliceSizeOverflow);
            SERIALIZE_INFO(prefix, NumSliceNonCopliant);
            SERIALIZE_INFO(prefix, NumEncodedSlice);
            SERIALIZE_INFO(prefix, NumSliceSizeAlloc);
            SERIALIZE_INFO_MEMORY(prefix, SliceSize, NumSliceSizeAlloc);
            SERIALIZE_INFO(prefix, reserved1);
            SERIALIZE_INFO_ARRAY(prefix, reserved);
        } break;
        case MFX_EXTBUFF_VPP_SCALING: {
            mfxExtVPPScaling& info = *(mfxExtVPPScaling*)pExtBuffer;
            SERIALIZE_INFO(prefix, ScalingMode);
            SERIALIZE_INFO_ARRAY(prefix, reserved);
        } break;
        case MFX_EXTBUFF_VPP_MIRRORING: {
            mfxExtVPPMirroring& info = *(mfxExtVPPMirroring*)pExtBuffer;
            SERIALIZE_INFO(prefix, Type);
            SERIALIZE_INFO_ARRAY(prefix, reserved);
        } break;
        case MFX_EXTBUFF_MV_OVER_PIC_BOUNDARIES: {
            mfxExtMVOverPicBoundaries& info = *(mfxExtMVOverPicBoundaries*)pExtBuffer;
            SERIALIZE_INFO(prefix, StickTop);
            SERIALIZE_INFO(prefix, StickBottom);
            SERIALIZE_INFO(prefix, StickLeft);
            SERIALIZE_INFO(prefix, StickRight);
            SERIALIZE_INFO_ARRAY(prefix, reserved);
        } break;
        case MFX_EXTBUFF_VPP_COLORFILL: {
            mfxExtVPPColorFill& info = *(mfxExtVPPColorFill*)pExtBuffer;
            SERIALIZE_INFO(prefix, Enable);
            SERIALIZE_INFO_ARRAY(prefix, reserved);
        } break;
        case MFX_EXTBUFF_VP8_CODING_OPTION: {
            mfxExtVP8CodingOption& info = *(mfxExtVP8CodingOption*)pExtBuffer;
            SERIALIZE_INFO(prefix, Version);
            SERIALIZE_INFO(prefix, EnableMultipleSegments);
            SERIALIZE_INFO(prefix, LoopFilterType);
            SERIALIZE_INFO_ARRAY(prefix, LoopFilterLevel);
            SERIALIZE_INFO(prefix, SharpnessLevel);
            SERIALIZE_INFO(prefix, NumTokenPartitions);
            SERIALIZE_INFO_ARRAY(prefix, LoopFilterRefTypeDelta);
            SERIALIZE_INFO_ARRAY(prefix, LoopFilterMbModeDelta);
            SERIALIZE_INFO_ARRAY(prefix, SegmentQPDelta);
            SERIALIZE_INFO_ARRAY(prefix, CoeffTypeQPDelta);
            SERIALIZE_INFO(prefix, WriteIVFHeaders);
            SERIALIZE_INFO(prefix, NumFramesForIVFHeader);
            SERIALIZE_INFO_ARRAY(prefix, reserved);
        } break;
    }
    // End of autogenerated code
}

void CParametersDumper::SerializeVPPCompInputStream(std::ostream& sstr,
                                                    const char* prefix,
                                                    mfxVPPCompInputStream& info) {
    SERIALIZE_INFO(prefix, DstX);
    SERIALIZE_INFO(prefix, DstY);
    SERIALIZE_INFO(prefix, DstW);
    SERIALIZE_INFO(prefix, DstH);

    SERIALIZE_INFO(prefix, LumaKeyEnable);
    SERIALIZE_INFO(prefix, LumaKeyMin);
    SERIALIZE_INFO(prefix, LumaKeyMax);

    SERIALIZE_INFO(prefix, GlobalAlphaEnable);
    SERIALIZE_INFO(prefix, GlobalAlpha);

    SERIALIZE_INFO(prefix, PixelAlphaEnable);

    SERIALIZE_INFO_ARRAY(prefix, reserved2);
}

void CParametersDumper::SerializeVideoParamStruct(std::ostream& sstr,
                                                  const char* sectionName,
                                                  mfxVideoParam& info,
                                                  bool shouldUseVPPSection) {
    std::string prefix("");

    sstr << sectionName << std::endl;
    SERIALIZE_INFO(prefix, AllocId);
    SERIALIZE_INFO_ARRAY(prefix, reserved);
    SERIALIZE_INFO(prefix, reserved3);
    SERIALIZE_INFO(prefix, AsyncDepth);

    if (shouldUseVPPSection) {
        SERIALIZE_INFO_ARRAY(prefix, vpp.reserved);
        SerializeFrameInfoStruct(sstr, "vpp.In.", info.vpp.In);
        SerializeFrameInfoStruct(sstr, "vpp.Out.", info.vpp.Out);
    }
    else {
        SerializeMfxInfoMFXStruct(sstr, "mfx.", info.mfx);
    }

    SERIALIZE_INFO(prefix, Protected);
    SERIALIZE_INFO(prefix, IOPattern);
    for (int i = 0; i < info.NumExtParam; i++) {
        SerializeExtensionBuffer(sstr, "ext.", info.ExtParam[i]);
    }
    SERIALIZE_INFO(prefix, reserved2);
}

mfxStatus CParametersDumper::DumpLibraryConfiguration(std::string fileName,
                                                      MFXVideoDECODE* pMfxDec,
                                                      MFXVideoVPP* pMfxVPP,
                                                      MFXVideoENCODE* pMfxEnc,
                                                      const mfxVideoParam* pDecoderPresetParams,
                                                      const mfxVideoParam* pVPPPresetParams,
                                                      const mfxVideoParam* pEncoderPresetParams) {
    try {
        std::stringstream sstr;
        sstr << "Configuration settings (fields from API " << MFX_VERSION_MAJOR << "."
             << MFX_VERSION_MINOR << ")\n";

        mfxVideoParam params;
        if (pMfxDec) {
            if (GetUnitParams(pMfxDec, pDecoderPresetParams, &params) == MFX_ERR_NONE) {
                SerializeVideoParamStruct(sstr, "*** Decoder ***", params, false);
                ClearExtBuffs(&params);
            }
        }
        if (pMfxVPP) {
            if (GetUnitParams(pMfxVPP, pVPPPresetParams, &params) == MFX_ERR_NONE) {
                SerializeVideoParamStruct(sstr, "*** VPP ***", params, true);
                ClearExtBuffs(&params);
            }
        }
        if (pMfxEnc) {
            if (GetUnitParams(pMfxEnc, pEncoderPresetParams, &params) == MFX_ERR_NONE) {
                SerializeVideoParamStruct(sstr, "*** Encoder ***", params, false);
                ClearExtBuffs(&params);
            }
        }
        std::fstream dump_fstr(fileName.c_str(), std::fstream::out);
        dump_fstr << sstr.str();
        dump_fstr.close();
    }
    catch (...) {
        printf("Cannot save library settings into file.\n");
        return MFX_ERR_NULL_PTR;
    }
    return MFX_ERR_NONE;
}

void CParametersDumper::ShowConfigurationDiff(std::ostream& str1, std::ostream& str2) {
    std::stringstream ss1;
    ss1 << str1.rdbuf();
    std::stringstream ss2;
    ss2 << str2.rdbuf();

    std::string l, r;
    while (ss1 >> l && ss2 >> r) {
        if (l != r) {
            printf("%s changed to %s \n", l.c_str(), r.c_str());
        }
        else {
            continue;
        }
    }
}
