/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "./mfxdefs.h"
#include "./mfxcommon.h"
#include "./mfxstructures.h"
#include "./mfxdispatcher.h"
#include "./mfximplcaps.h"
#include "./mfxjpeg.h"
#include "./mfxvideo.h"
#include "./mfxadapter.h"

#define UNUSED_PARAM(x) (void)(x)

/* mfxdispatcher.h */
mfxLoader MFX_CDECL MFXLoad()
{
    return (mfxLoader)1;
}

void MFX_CDECL MFXUnload(mfxLoader loader)
{
    UNUSED_PARAM(loader);
    return;
}
mfxConfig MFX_CDECL MFXCreateConfig(mfxLoader loader)
{
    UNUSED_PARAM(loader);
    return (mfxConfig)2;
}

mfxStatus MFX_CDECL MFXSetConfigFilterProperty(mfxConfig config, const mfxU8* name, mfxVariant value)
{
    UNUSED_PARAM(config);
    UNUSED_PARAM(name);
    UNUSED_PARAM(value);
    return MFX_ERR_NONE;
}

mfxStatus MFX_CDECL MFXEnumImplementations(mfxLoader loader, mfxU32 i, mfxImplCapsDeliveryFormat format, mfxHDL* idesc)
{
    UNUSED_PARAM(loader);
    UNUSED_PARAM(i);
    UNUSED_PARAM(format);
    UNUSED_PARAM(idesc);
    return MFX_ERR_NONE;
}

mfxStatus MFX_CDECL MFXCreateSession(mfxLoader loader, mfxU32 i, mfxSession* session)
{
    UNUSED_PARAM(loader);
    UNUSED_PARAM(i);
    UNUSED_PARAM(session);
    return MFX_ERR_NONE;
}

mfxStatus MFX_CDECL MFXDispReleaseImplDescription(mfxLoader loader, mfxHDL hdl)
{
    UNUSED_PARAM(loader);
    UNUSED_PARAM(hdl);
    return MFX_ERR_NONE;
}

/* mfximpcaps.h */

mfxHDL* MFX_CDECL MFXQueryImplsDescription(mfxImplCapsDeliveryFormat format, mfxU32 *num)
{
    UNUSED_PARAM(format);
    UNUSED_PARAM(num);
    return (mfxHDL*)(1);  //NOLINT
}

mfxStatus MFX_CDECL MFXReleaseImplDescription(mfxHDL hdl)
{
    UNUSED_PARAM(hdl);
    return MFX_ERR_NONE;
}

/* mfxsession.h */

mfxStatus MFX_CDECL MFXClose(mfxSession session)
{
    UNUSED_PARAM(session);
    return MFX_ERR_NONE;
}

/* mfxvideo.h */


mfxStatus MFX_CDECL MFXVideoCORE_SetFrameAllocator(mfxSession session, mfxFrameAllocator *allocator)
{
    UNUSED_PARAM(session);
    UNUSED_PARAM(allocator);
    return MFX_ERR_NONE;
}

mfxStatus MFX_CDECL MFXVideoCORE_SetHandle(mfxSession session, mfxHandleType type, mfxHDL hdl)
{
    UNUSED_PARAM(session);
    UNUSED_PARAM(type);
    UNUSED_PARAM(hdl);
    return MFX_ERR_NONE;
}

mfxStatus MFX_CDECL MFXVideoCORE_GetHandle(mfxSession session, mfxHandleType type, mfxHDL *hdl)
{
    UNUSED_PARAM(session);
    UNUSED_PARAM(type);
    UNUSED_PARAM(hdl);
    return MFX_ERR_NONE;
}

mfxStatus MFX_CDECL MFXVideoCORE_QueryPlatform(mfxSession session, mfxPlatform* platform)
{
    UNUSED_PARAM(session);
    UNUSED_PARAM(platform);
    return MFX_ERR_NONE;
}

mfxStatus MFX_CDECL MFXVideoCORE_SyncOperation(mfxSession session, mfxSyncPoint syncp, mfxU32 wait)
{
    UNUSED_PARAM(session);
    UNUSED_PARAM(syncp);
    UNUSED_PARAM(wait);
    return MFX_ERR_NONE;
}

mfxStatus MFX_CDECL MFXMemory_GetSurfaceForVPP(mfxSession session, mfxFrameSurface1** surface)
{
    UNUSED_PARAM(session);
    UNUSED_PARAM(surface);
    return MFX_ERR_NONE;
}

mfxStatus MFX_CDECL MFXMemory_GetSurfaceForVPPOut(mfxSession session, mfxFrameSurface1** surface)
{
    UNUSED_PARAM(session);
    UNUSED_PARAM(surface);
    return MFX_ERR_NONE;
}

mfxStatus MFX_CDECL MFXMemory_GetSurfaceForEncode(mfxSession session, mfxFrameSurface1** surface)
{
    UNUSED_PARAM(session);
    UNUSED_PARAM(surface);
    return MFX_ERR_NONE;
}

mfxStatus MFX_CDECL MFXMemory_GetSurfaceForDecode(mfxSession session, mfxFrameSurface1** surface)
{
    UNUSED_PARAM(session);
    UNUSED_PARAM(surface);
    return MFX_ERR_NONE;
}

mfxStatus MFX_CDECL MFXVideoENCODE_Query(mfxSession session, mfxVideoParam *in, mfxVideoParam *out)
{
    UNUSED_PARAM(session);
    UNUSED_PARAM(in);
    UNUSED_PARAM(out);
    return MFX_ERR_NONE;
}

mfxStatus MFX_CDECL MFXVideoENCODE_QueryIOSurf(mfxSession session, mfxVideoParam *par, mfxFrameAllocRequest *request)
{
    UNUSED_PARAM(session);
    UNUSED_PARAM(par);
    UNUSED_PARAM(request);
    return MFX_ERR_NONE;
}

mfxStatus MFX_CDECL MFXVideoENCODE_Init(mfxSession session, mfxVideoParam *par)
{
    UNUSED_PARAM(session);
    UNUSED_PARAM(par);
    return MFX_ERR_NONE;
}

mfxStatus MFX_CDECL MFXVideoENCODE_Reset(mfxSession session, mfxVideoParam *par)
{
    UNUSED_PARAM(session);
    UNUSED_PARAM(par);
    return MFX_ERR_NONE;
}

mfxStatus MFX_CDECL MFXVideoENCODE_Close(mfxSession session)
{
    UNUSED_PARAM(session);
    return MFX_ERR_NONE;
}

mfxStatus MFX_CDECL MFXVideoENCODE_GetVideoParam(mfxSession session, mfxVideoParam *par)
{
    UNUSED_PARAM(session);
    UNUSED_PARAM(par);
    return MFX_ERR_NONE;
}

mfxStatus MFX_CDECL MFXVideoENCODE_GetEncodeStat(mfxSession session, mfxEncodeStat *stat)
{
    UNUSED_PARAM(session);
    UNUSED_PARAM(stat);
    return MFX_ERR_NONE;
}

mfxStatus MFX_CDECL MFXVideoENCODE_EncodeFrameAsync(mfxSession session, mfxEncodeCtrl *ctrl, mfxFrameSurface1 *surface, mfxBitstream *bs, mfxSyncPoint *syncp)
{
    UNUSED_PARAM(session);
    UNUSED_PARAM(ctrl);
    UNUSED_PARAM(surface);
    UNUSED_PARAM(bs);
    UNUSED_PARAM(syncp);
    return MFX_ERR_NONE;
}

mfxStatus MFX_CDECL MFXVideoDECODE_Query(mfxSession session, mfxVideoParam *in, mfxVideoParam *out)
{
    UNUSED_PARAM(session);
    UNUSED_PARAM(in);
    UNUSED_PARAM(out);
    return MFX_ERR_NONE;
}
   
mfxStatus MFX_CDECL MFXVideoDECODE_DecodeHeader(mfxSession session, mfxBitstream *bs, mfxVideoParam *par)
{
    UNUSED_PARAM(session);
    UNUSED_PARAM(bs);
    UNUSED_PARAM(par);
    return MFX_ERR_NONE;
}

mfxStatus MFX_CDECL MFXVideoDECODE_QueryIOSurf(mfxSession session, mfxVideoParam *par, mfxFrameAllocRequest *request)
{
    UNUSED_PARAM(session);
    UNUSED_PARAM(par);
    UNUSED_PARAM(request);
    return MFX_ERR_NONE;
}

mfxStatus MFX_CDECL MFXVideoDECODE_Init(mfxSession session, mfxVideoParam *par)
{
    UNUSED_PARAM(session);
    UNUSED_PARAM(par);
    return MFX_ERR_NONE;
}

mfxStatus MFX_CDECL MFXVideoDECODE_Reset(mfxSession session, mfxVideoParam *par)
{
    UNUSED_PARAM(session);
    UNUSED_PARAM(par);
    return MFX_ERR_NONE;
}

mfxStatus MFX_CDECL MFXVideoDECODE_Close(mfxSession session)
{
    UNUSED_PARAM(session);
    return MFX_ERR_NONE;
}

mfxStatus MFX_CDECL MFXVideoDECODE_GetVideoParam(mfxSession session, mfxVideoParam *par)
{
    UNUSED_PARAM(session);
    UNUSED_PARAM(par);
    return MFX_ERR_NONE;
}

mfxStatus MFX_CDECL MFXVideoDECODE_GetDecodeStat(mfxSession session, mfxDecodeStat *stat)
{
    UNUSED_PARAM(session);
    UNUSED_PARAM(stat);
    return MFX_ERR_NONE;
}

mfxStatus MFX_CDECL MFXVideoDECODE_SetSkipMode(mfxSession session, mfxSkipMode mode)
{
    UNUSED_PARAM(session);
    UNUSED_PARAM(mode);
    return MFX_ERR_NONE;
}

mfxStatus MFX_CDECL MFXVideoDECODE_GetPayload(mfxSession session, mfxU64 *ts, mfxPayload *payload)
{
    UNUSED_PARAM(session);
    UNUSED_PARAM(ts);
    UNUSED_PARAM(payload);
    return MFX_ERR_NONE;
}

mfxStatus MFX_CDECL MFXVideoDECODE_DecodeFrameAsync(mfxSession session, mfxBitstream *bs, mfxFrameSurface1 *surface_work, mfxFrameSurface1 **surface_out, mfxSyncPoint *syncp)
{
    UNUSED_PARAM(session);
    UNUSED_PARAM(bs);
    UNUSED_PARAM(surface_work);
    UNUSED_PARAM(surface_out);
    UNUSED_PARAM(syncp);
    return MFX_ERR_NONE;
}
 
mfxStatus MFX_CDECL MFXVideoVPP_Query(mfxSession session, mfxVideoParam *in, mfxVideoParam *out)
{
    UNUSED_PARAM(session);
    UNUSED_PARAM(in);
    UNUSED_PARAM(out);
    return MFX_ERR_NONE;
}

mfxStatus MFX_CDECL MFXVideoVPP_QueryIOSurf(mfxSession session, mfxVideoParam *par, mfxFrameAllocRequest request[2])
{
    UNUSED_PARAM(session);
    UNUSED_PARAM(par);
    UNUSED_PARAM(request);
    return MFX_ERR_NONE;
}

mfxStatus MFX_CDECL MFXVideoVPP_Init(mfxSession session, mfxVideoParam *par)
{
    UNUSED_PARAM(session);
    UNUSED_PARAM(par);
    return MFX_ERR_NONE;
}

mfxStatus MFX_CDECL MFXVideoVPP_Reset(mfxSession session, mfxVideoParam *par)
{
    UNUSED_PARAM(session);
    UNUSED_PARAM(par);
    return MFX_ERR_NONE;
}

mfxStatus MFX_CDECL MFXVideoVPP_Close(mfxSession session)
{
    UNUSED_PARAM(session);
    return MFX_ERR_NONE;
}

mfxStatus MFX_CDECL MFXVideoVPP_GetVideoParam(mfxSession session, mfxVideoParam *par)
{
    UNUSED_PARAM(session);
    UNUSED_PARAM(par);
    return MFX_ERR_NONE;
}

mfxStatus MFX_CDECL MFXVideoVPP_GetVPPStat(mfxSession session, mfxVPPStat *stat)
{
    UNUSED_PARAM(session);
    UNUSED_PARAM(stat);
    return MFX_ERR_NONE;
}

mfxStatus MFX_CDECL MFXVideoVPP_RunFrameVPPAsync(mfxSession session, mfxFrameSurface1 *in, mfxFrameSurface1 *out, mfxExtVppAuxData *aux, mfxSyncPoint *syncp)
{
    UNUSED_PARAM(session);
    UNUSED_PARAM(in);
    UNUSED_PARAM(out);
    UNUSED_PARAM(aux);
    UNUSED_PARAM(syncp);
    return MFX_ERR_NONE;
}

/* mfxadapter.h */

mfxStatus MFX_CDECL MFXQueryAdapters(mfxComponentInfo* input_info, mfxAdaptersInfo* adapters)
{
    UNUSED_PARAM(input_info);
    UNUSED_PARAM(adapters);
    return MFX_ERR_NONE;
}

mfxStatus MFX_CDECL MFXQueryAdaptersDecode(mfxBitstream* bitstream, mfxU32 codec_id, mfxAdaptersInfo* adapters)
{
    UNUSED_PARAM(bitstream);
    UNUSED_PARAM(codec_id);
    UNUSED_PARAM(adapters);
    return MFX_ERR_NONE;
}

mfxStatus MFX_CDECL MFXQueryAdaptersNumber(mfxU32* num_adapters)
{
    UNUSED_PARAM(num_adapters);
    return MFX_ERR_NONE;
}

mfxStatus  MFX_CDECL MFXVideoDECODE_VPP_Init(mfxSession session, mfxVideoParam* decode_par, mfxVideoChannelParam** vpp_par_array, mfxU32 num_channel_par)
{
    UNUSED_PARAM(session);
    UNUSED_PARAM(decode_par);
    UNUSED_PARAM(vpp_par_array);
    UNUSED_PARAM(num_channel_par);
    return MFX_ERR_NONE;
}

mfxStatus  MFX_CDECL MFXVideoDECODE_VPP_DecodeFrameAsync(mfxSession session, mfxBitstream *bs, mfxU32* skip_channels, mfxU32 num_skip_channels, mfxSurfaceArray **surf_array_out)
{
    UNUSED_PARAM(session);
    UNUSED_PARAM(bs);
    UNUSED_PARAM(skip_channels);
    UNUSED_PARAM(num_skip_channels);
    UNUSED_PARAM(surf_array_out);
    return MFX_ERR_NONE;
}

mfxStatus MFXVideoVPP_ProcessFrameAsync(mfxSession session, mfxFrameSurface1 *in, mfxFrameSurface1 **out)
{
    UNUSED_PARAM(session);
    UNUSED_PARAM(in);
    UNUSED_PARAM(out);
    return MFX_ERR_NONE;
}

