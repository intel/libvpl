/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "va/va.h"

#include "mfxdefs.h"
#include "mfxvideo.h"
#include "mfxmvc.h"
#ifdef ONEVPL_EXPERIMENTAL
#include "mfxcamera.h"
#endif

/* These macro required for code compilation. */
#define INFINITE 0x7FFFFFFF
#define UNUSED_PARAM(x) (void)(x)

mfxSession session;
mfxVideoParam init_param;
mfxFrameAllocRequest response[2];
mfxStatus sts;
mfxFrameSurface1 **in_pool, **out_pool;
mfxSyncPoint syncp;

mfxFrameSurface1 *in, *out;


static void allocate_pool_of_surfaces(mfxFrameSurface1 **pool, int nFrames)
{
    UNUSED_PARAM(pool);
    UNUSED_PARAM(nFrames);
    return;
}

static mfxFrameSurface1* find_unlocked_surface_and_fill_content(mfxFrameSurface1 **pool)
{
    UNUSED_PARAM(pool);
    return pool[0];
}

static mfxFrameSurface1* find_unlocked_surface_from_the_pool(mfxFrameSurface1 **pool)
{
    UNUSED_PARAM(pool);
    return pool[0];
}

static void process_output_frame(mfxFrameSurface1 *surf)
{
    UNUSED_PARAM(surf);
    return;
}

static void fill_content_for_video_processing(mfxFrameSurface1 *surf)
{
    UNUSED_PARAM(surf);
    return;
}

static int end_of_stream()
{
    return 1;
}

static void free_pool_of_surfaces(mfxFrameSurface1 **in_pool)
{
    UNUSED_PARAM(in_pool);
    return;
}
/* end of internal stuff */


static void prg_vpp1 () {
/*beg1*/
MFXVideoVPP_QueryIOSurf(session, &init_param, response);
allocate_pool_of_surfaces(in_pool, response[0].NumFrameSuggested);
allocate_pool_of_surfaces(out_pool, response[1].NumFrameSuggested);
MFXVideoVPP_Init(session, &init_param);
mfxFrameSurface1 *in=find_unlocked_surface_and_fill_content(in_pool);
mfxFrameSurface1 *out=find_unlocked_surface_from_the_pool(out_pool);
for (;;) {
   sts=MFXVideoVPP_RunFrameVPPAsync(session,in,out,NULL,&syncp);
   if (sts==MFX_ERR_MORE_SURFACE || sts==MFX_ERR_NONE) {
      MFXVideoCORE_SyncOperation(session,syncp,INFINITE);
      process_output_frame(out);
      out=find_unlocked_surface_from_the_pool(out_pool);
   }
   if (sts==MFX_ERR_MORE_DATA && in==NULL)
      break;
   if (sts==MFX_ERR_NONE || sts==MFX_ERR_MORE_DATA) {
      in=find_unlocked_surface_from_the_pool(in_pool);
      fill_content_for_video_processing(in);
      if (end_of_stream())
         in=NULL;
   }
}
MFXVideoVPP_Close(session);
free_pool_of_surfaces(in_pool);
free_pool_of_surfaces(out_pool);
/*end1*/
}

static void prg_vpp2 () {
/*beg2*/
/* enable image stabilization filter with default settings */
mfxExtVPPDoUse du;
mfxU32 al=MFX_EXTBUFF_VPP_IMAGE_STABILIZATION;

du.Header.BufferId=MFX_EXTBUFF_VPP_DOUSE;
du.Header.BufferSz=sizeof(mfxExtVPPDoUse);
du.NumAlg=1;
du.AlgList=&al;

/* configure the mfxVideoParam structure */
mfxVideoParam conf;
mfxExtBuffer *eb=(mfxExtBuffer *)&du;

memset(&conf,0,sizeof(conf));
conf.IOPattern=MFX_IOPATTERN_IN_SYSTEM_MEMORY | MFX_IOPATTERN_OUT_SYSTEM_MEMORY;
conf.NumExtParam=1;
conf.ExtParam=&eb;

conf.vpp.In.FourCC=MFX_FOURCC_YV12;
conf.vpp.Out.FourCC=MFX_FOURCC_NV12;
conf.vpp.In.Width=conf.vpp.Out.Width=1920;
conf.vpp.In.Height=conf.vpp.Out.Height=1088;

/* video processing initialization */
MFXVideoVPP_Init(session, &conf);
/*end2*/
}

static void prg_vpp3 () {
/*beg3*/
mfxExtBuffer *eb;
mfxExtMVCSeqDesc  seq_desc;
mfxVideoParam init_param;

init_param.ExtParam = &eb;
init_param.NumExtParam=1;
eb=(mfxExtBuffer *)&seq_desc;

/* init VPP */
MFXVideoVPP_Init(session, &init_param);

/* perform processing */
for (;;) {
    MFXVideoVPP_RunFrameVPPAsync(session,in,out,NULL,&syncp);
    MFXVideoCORE_SyncOperation(session,syncp,INFINITE);
}

/* close VPP */
MFXVideoVPP_Close(session);
/*end3*/
}

static void prg_vpp4 () {
/*beg4*/
VADisplay va_dpy = 0;
VASurfaceID surface_id = 0;

vaInitialize(va_dpy, NULL, NULL);

// MFX_3DLUT_MEMORY_LAYOUT_INTEL_65LUT indicate 65*65*128*8bytes.
mfxU32 seg_size = 65, mul_size = 128;
mfxMemId memId = 0;

// create 3DLUT surface (MFX_3DLUT_MEMORY_LAYOUT_INTEL_65LUT)
VASurfaceAttrib    surface_attrib = {};
surface_attrib.type =  VASurfaceAttribPixelFormat;
surface_attrib.flags = VA_SURFACE_ATTRIB_SETTABLE;
surface_attrib.value.type = VAGenericValueTypeInteger;
surface_attrib.value.value.i = VA_FOURCC_RGBA;

vaCreateSurfaces(va_dpy,
                 VA_RT_FORMAT_RGB32,   // 4 bytes
                 seg_size * mul_size,  // 65*128
                 seg_size * 2,         // 65*2
                 &surface_id,
                 1,
                 &surface_attrib,
                 1);

*((VASurfaceID*)memId) = surface_id;

// configure 3DLUT parameters
mfxExtVPP3DLut lut3DConfig;
memset(&lut3DConfig, 0, sizeof(lut3DConfig));
lut3DConfig.Header.BufferId         = MFX_EXTBUFF_VPP_3DLUT;
lut3DConfig.Header.BufferSz         = sizeof(mfxExtVPP3DLut);
lut3DConfig.ChannelMapping          = MFX_3DLUT_CHANNEL_MAPPING_RGB_RGB;    
lut3DConfig.BufferType              = MFX_RESOURCE_VA_SURFACE;
lut3DConfig.VideoBuffer.DataType    = MFX_DATA_TYPE_U16;
lut3DConfig.VideoBuffer.MemLayout   = MFX_3DLUT_MEMORY_LAYOUT_INTEL_65LUT;
lut3DConfig.VideoBuffer.MemId       = memId;

// release 3DLUT surface
vaDestroySurfaces(va_dpy, &surface_id, 1);
/*end4*/
}

static void prg_vpp5 () {
/*beg5*/

// 64 size 3DLUT(3 dimension look up table)
// The buffer size(in bytes) for every channel is 64*64*64*sizeof(DataType)
mfxU16 dataR[64*64*64], dataG[64*64*64], dataB[64*64*64];
mfxChannel channelR, channelG, channelB;
channelR.DataType = MFX_DATA_TYPE_U16;
channelR.Size = 64;
channelR.Data16 = dataR;
channelG.DataType = MFX_DATA_TYPE_U16;
channelG.Size = 64;
channelG.Data16 = dataG;
channelB.DataType = MFX_DATA_TYPE_U16;
channelB.Size = 64;
channelB.Data16 = dataB;

// configure 3DLUT parameters
mfxExtVPP3DLut lut3DConfig;
memset(&lut3DConfig, 0, sizeof(lut3DConfig));
lut3DConfig.Header.BufferId         = MFX_EXTBUFF_VPP_3DLUT;
lut3DConfig.Header.BufferSz         = sizeof(mfxExtVPP3DLut);
lut3DConfig.ChannelMapping          = MFX_3DLUT_CHANNEL_MAPPING_RGB_RGB;    
lut3DConfig.BufferType              = MFX_RESOURCE_SYSTEM_SURFACE;
lut3DConfig.SystemBuffer.Channel[0] = channelR;
lut3DConfig.SystemBuffer.Channel[1] = channelG;
lut3DConfig.SystemBuffer.Channel[2] = channelB;

/*end5*/
}

static void prg_vpp6() {
/*beg6*/

// HDR to SDR (e.g P010 HDR signal -> NV12 SDR signal) in transcoding pipeline
// Attach input external buffers as the below for HDR input. SDR is by default, hence no
// extra output external buffer.
// The input Video Signal Information
mfxExtVideoSignalInfo inSignalInfo   = {};
inSignalInfo.Header.BufferId         = MFX_EXTBUFF_VIDEO_SIGNAL_INFO_IN;
inSignalInfo.Header.BufferSz         = sizeof(mfxExtVideoSignalInfo);
inSignalInfo.VideoFullRange          = 0; // Limited range P010
inSignalInfo.ColourPrimaries         = 9; // BT.2020
inSignalInfo.TransferCharacteristics = 16; // ST2084

// The content Light Level Information
mfxExtContentLightLevelInfo inContentLight = {};
inContentLight.Header.BufferId         = MFX_EXTBUFF_CONTENT_LIGHT_LEVEL_INFO;
inContentLight.Header.BufferSz         = sizeof(mfxExtContentLightLevelInfo);
inContentLight.MaxContentLightLevel    = 4000; // nits
inContentLight.MaxPicAverageLightLevel = 1000; // nits

// The mastering display colour volume
mfxExtMasteringDisplayColourVolume inColourVolume = {};
inColourVolume.Header.BufferId = MFX_EXTBUFF_MASTERING_DISPLAY_COLOUR_VOLUME_IN;
inColourVolume.Header.BufferSz = sizeof(mfxExtMasteringDisplayColourVolume);
// Based on the needs, Please set DisplayPrimaryX/Y[3], WhitePointX/Y, and MaxDisplayMasteringLuminance,
// MinDisplayMasteringLuminance

mfxExtBuffer *ExtBufferIn[3];
ExtBufferIn[0] = (mfxExtBuffer *)&inSignalInfo;
ExtBufferIn[1] = (mfxExtBuffer *)&inContentLight;
ExtBufferIn[2] = (mfxExtBuffer *)&inColourVolume;

mfxSession session      = (mfxSession)0;
mfxVideoParam VPPParams = {};
VPPParams.NumExtParam   = 3;
VPPParams.ExtParam      = (mfxExtBuffer **)&ExtBufferIn[0];
MFXVideoVPP_Init(session, &VPPParams);

/*end6*/
}

static void prg_vpp7() {
/*beg7*/

// SDR to HDR (e.g NV12 SDR signal -> P010 HDR signal) in transcoding pipeline
// Attach output external buffers as the below for HDR output. SDR is by default, hence no
// extra input external buffer.
// The output Video Signal Information
mfxExtVideoSignalInfo outSignalInfo   = {};
outSignalInfo.Header.BufferId        = MFX_EXTBUFF_VIDEO_SIGNAL_INFO_OUT;
outSignalInfo.Header.BufferSz         = sizeof(mfxExtVideoSignalInfo);
outSignalInfo.VideoFullRange          = 0; // Limited range P010
outSignalInfo.ColourPrimaries         = 9; // BT.2020
outSignalInfo.TransferCharacteristics = 16; // ST2084

// The mastering display colour volume
mfxExtMasteringDisplayColourVolume outColourVolume = {};
outColourVolume.Header.BufferId = MFX_EXTBUFF_MASTERING_DISPLAY_COLOUR_VOLUME_OUT;
outColourVolume.Header.BufferSz = sizeof(mfxExtMasteringDisplayColourVolume);
// Based on the needs, Please set DisplayPrimaryX/Y[3], WhitePointX/Y, and MaxDisplayMasteringLuminance,
// MinDisplayMasteringLuminance

mfxExtBuffer *ExtBufferOut[2];
ExtBufferOut[0] = (mfxExtBuffer *)&outSignalInfo;
ExtBufferOut[2] = (mfxExtBuffer *)&outColourVolume;

mfxSession session      = (mfxSession)0;
mfxVideoParam VPPParams = {};
VPPParams.NumExtParam   = 2;
VPPParams.ExtParam      = (mfxExtBuffer **)&ExtBufferOut[0];
MFXVideoVPP_Init(session, &VPPParams);
/*end7*/
}

static void prg_vpp8() {
/*beg8*/

// HDR to HDR (e.g P010 HDR signal -> P010 HDR signal) in transcoding pipeline
// Attach in/output external buffers as the below for HDR input/output. 
// The input Video Signal Information
mfxExtVideoSignalInfo inSignalInfo   = {};
inSignalInfo.Header.BufferId         = MFX_EXTBUFF_VIDEO_SIGNAL_INFO_IN;
inSignalInfo.Header.BufferSz         = sizeof(mfxExtVideoSignalInfo);
inSignalInfo.VideoFullRange          = 0; // Limited range P010
inSignalInfo.ColourPrimaries         = 9; // BT.2020
inSignalInfo.TransferCharacteristics = 16; // ST2084

// The content Light Level Information
mfxExtContentLightLevelInfo inContentLight = {};
inContentLight.Header.BufferId         = MFX_EXTBUFF_CONTENT_LIGHT_LEVEL_INFO;
inContentLight.Header.BufferSz         = sizeof(mfxExtContentLightLevelInfo);
inContentLight.MaxContentLightLevel    = 4000; // nits
inContentLight.MaxPicAverageLightLevel = 1000; // nits

// The mastering display colour volume
mfxExtMasteringDisplayColourVolume inColourVolume = {};
inColourVolume.Header.BufferId = MFX_EXTBUFF_MASTERING_DISPLAY_COLOUR_VOLUME_IN;
inColourVolume.Header.BufferSz = sizeof(mfxExtMasteringDisplayColourVolume);
// Based on the needs, Please set DisplayPrimaryX/Y[3], WhitePointX/Y, and MaxDisplayMasteringLuminance,
// MinDisplayMasteringLuminance

mfxExtVideoSignalInfo outSignalInfo   = {};
outSignalInfo.Header.BufferId         = MFX_EXTBUFF_VIDEO_SIGNAL_INFO_OUT;
outSignalInfo.Header.BufferSz         = sizeof(mfxExtVideoSignalInfo);
outSignalInfo.VideoFullRange          = 0; // Limited range P010
outSignalInfo.ColourPrimaries         = 9; // BT.2020
outSignalInfo.TransferCharacteristics = 16; // ST2084

// The mastering display colour volume
mfxExtMasteringDisplayColourVolume outColourVolume = {};
outColourVolume.Header.BufferId = MFX_EXTBUFF_MASTERING_DISPLAY_COLOUR_VOLUME_OUT;
outColourVolume.Header.BufferSz = sizeof(mfxExtMasteringDisplayColourVolume);
// Based on the needs, Please set DisplayPrimaryX/Y[3], WhitePointX/Y, and MaxDisplayMasteringLuminance,
// MinDisplayMasteringLuminance

mfxExtBuffer *ExtBuffer[5];
ExtBuffer[0] = (mfxExtBuffer *)&inSignalInfo;
ExtBuffer[1] = (mfxExtBuffer *)&inContentLight;
ExtBuffer[2] = (mfxExtBuffer *)&inColourVolume;
ExtBuffer[3] = (mfxExtBuffer *)&outSignalInfo;
ExtBuffer[4] = (mfxExtBuffer *)&outColourVolume;

mfxSession session      = (mfxSession)0;
mfxVideoParam VPPParams = {};
VPPParams.NumExtParam   = 5;
VPPParams.ExtParam      = (mfxExtBuffer **)&ExtBuffer[0];
MFXVideoVPP_Init(session, &VPPParams);

/*end8*/
}

static void prg_vpp9() {
/*beg9*/
#ifdef ONEVPL_EXPERIMENTAL
// Camera Raw Format
mfxExtCamPipeControl pipeControl = {};
pipeControl.Header.BufferId      = MFX_EXTBUF_CAM_PIPECONTROL;
pipeControl.Header.BufferSz      = sizeof(mfxExtCamPipeControl);
pipeControl.RawFormat            = (mfxU16)MFX_CAM_BAYER_BGGR;

// Black level correction
mfxExtCamBlackLevelCorrection blackLevelCorrection   = {};
blackLevelCorrection.Header.BufferId                 = MFX_EXTBUF_CAM_BLACK_LEVEL_CORRECTION;
blackLevelCorrection.Header.BufferSz                 = sizeof(mfxExtCamBlackLevelCorrection);
mfxU16 black_level_B = 16, black_level_G0 = 16, black_level_G1 = 16, black_level_R = 16;
// Initialize the value for black level B, G0, G1, R as needed
blackLevelCorrection.B  = black_level_B;
blackLevelCorrection.G0 = black_level_G0;
blackLevelCorrection.G1 = black_level_G1;
blackLevelCorrection.R  = black_level_R;

mfxExtBuffer *ExtBufferIn[2];
ExtBufferIn[0] = (mfxExtBuffer *)&pipeControl;
ExtBufferIn[1] = (mfxExtBuffer *)&blackLevelCorrection;

mfxSession session      = (mfxSession)0;
mfxVideoParam VPPParams = {};
VPPParams.NumExtParam   = 2;
VPPParams.ExtParam      = (mfxExtBuffer **)&ExtBufferIn[0];
MFXVideoVPP_Init(session, &VPPParams);
#endif
/*end9*/
}

static void prg_vpp11() {
/*beg11*/
#ifdef ONEVPL_EXPERIMENTAL
// configure 3DLUT parameters
mfxExtVPP3DLut lut3DConfig;
memset(&lut3DConfig, 0, sizeof(lut3DConfig));
lut3DConfig.Header.BufferId         = MFX_EXTBUFF_VPP_3DLUT;
lut3DConfig.Header.BufferSz         = sizeof(mfxExtVPP3DLut);
lut3DConfig.ChannelMapping          = MFX_3DLUT_CHANNEL_MAPPING_RGB_RGB;
lut3DConfig.BufferType              = MFX_RESOURCE_SYSTEM_SURFACE;
lut3DConfig.InterpolationMethod     = MFX_3DLUT_INTERPOLATION_TETRAHEDRAL;
#endif
/*end11*/
}

static void prg_vpp12() {
/*beg12*/
// configure AI super resolution vpp filter
mfxExtVPPAISuperResolution aiSuperResolution = {};
aiSuperResolution.Header.BufferId      = MFX_EXTBUFF_VPP_AI_SUPER_RESOLUTION;
aiSuperResolution.Header.BufferSz      = sizeof(mfxExtVPPAISuperResolution);
aiSuperResolution.SRMode               = MFX_AI_SUPER_RESOLUTION_MODE_DEFAULT;

mfxExtBuffer * ExtParam[1] = { (mfxExtBuffer *)&aiSuperResolution };

mfxSession session      = (mfxSession)0;
mfxVideoParam VPPParams = {};
VPPParams.NumExtParam   = 1;
VPPParams.ExtParam      = ExtParam;
MFXVideoVPP_Init(session, &VPPParams);
/*end12*/
}

static void prg_vpp13() {
/*beg13*/
// configure AI frame interpolation vpp filter
mfxExtVPPAIFrameInterpolation aiFrameInterpolation = {};
aiFrameInterpolation.Header.BufferId      = MFX_EXTBUFF_VPP_AI_FRAME_INTERPOLATION;
aiFrameInterpolation.Header.BufferSz      = sizeof(mfxExtVPPAIFrameInterpolation);
aiFrameInterpolation.FIMode               = MFX_AI_FRAME_INTERPOLATION_MODE_DEFAULT;
aiFrameInterpolation.EnableScd            = 1;
mfxExtBuffer * ExtParam[1] = { (mfxExtBuffer *)&aiFrameInterpolation };

init_param.NumExtParam   = 1;
init_param.ExtParam      = ExtParam;
init_param.vpp.In.FrameRateExtN = 30;
init_param.vpp.In.FrameRateExtD = 1;
init_param.vpp.Out.FrameRateExtN = 60;
init_param.vpp.Out.FrameRateExtD = 1;
sts = MFXVideoVPP_QueryIOSurf(session, &init_param, response);
sts = MFXVideoVPP_Init(session, &init_param);

// The below code follows the video processing procedure, not specific to AI frame interpolation.
allocate_pool_of_surfaces(in_pool, response[0].NumFrameSuggested);
allocate_pool_of_surfaces(out_pool, response[1].NumFrameSuggested);
mfxFrameSurface1 *in=find_unlocked_surface_and_fill_content(in_pool);
mfxFrameSurface1 *out=find_unlocked_surface_from_the_pool(out_pool);
for (;;) {
   sts=MFXVideoVPP_RunFrameVPPAsync(session,in,out,NULL,&syncp);
   if (sts==MFX_ERR_MORE_SURFACE || sts==MFX_ERR_NONE) {
      MFXVideoCORE_SyncOperation(session,syncp,INFINITE);
      process_output_frame(out);
      out=find_unlocked_surface_from_the_pool(out_pool);
   }
   if (sts==MFX_ERR_MORE_DATA && in==NULL)
      break;
   if (sts==MFX_ERR_NONE || sts==MFX_ERR_MORE_DATA) {
      in=find_unlocked_surface_from_the_pool(in_pool);
      fill_content_for_video_processing(in);
      if (end_of_stream())
         in=NULL;
   }
}
MFXVideoVPP_Close(session);
free_pool_of_surfaces(in_pool);
free_pool_of_surfaces(out_pool);
/*end13*/
}

