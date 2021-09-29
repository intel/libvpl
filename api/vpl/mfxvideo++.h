/*###########################################################################
  # Copyright Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ###########################################################################*/

#ifndef __MFXVIDEOPLUSPLUS_H
#define __MFXVIDEOPLUSPLUS_H

#include "mfx.h"
#include <vector>

#define MFX_IMPL_BASETYPE(x)  (0x00ff & (x))
#define MFX_IMPL_ACCELMODE(x) (0xff00 & (x))

class MFXVideoSession {
public:
    MFXVideoSession(void) {
        m_session = (mfxSession)0;
        m_loader = (mfxLoader)0;
        m_initialized=false;
    }
    virtual ~MFXVideoSession(void) {
        Close();
    }

    virtual mfxStatus Init(mfxIMPL impl, mfxVersion *ver) {
        mfxInitParam par={};
        par.Implementation=impl;
        par.Version=*ver;
        return InitSession(par);
    }

    virtual mfxStatus InitEx(mfxInitParam par) {
        return InitSession(par);
    }
    
    virtual mfxStatus Close(void) {
        if (m_session) {
            mfxStatus mfxRes;
            mfxRes    = MFXClose(m_session);
            m_session = (mfxSession)0;
            if (m_loader) {
                MFXUnload(m_loader);
                m_loader = (mfxLoader)0;
            }
            m_initialized=false;
            return mfxRes;
        } else {
            return MFX_ERR_NONE;
        }
    }

    virtual mfxStatus QueryIMPL(mfxIMPL *impl) {
        return MFXQueryIMPL(m_session, impl);
    }
    virtual mfxStatus QueryVersion(mfxVersion *version) {
        return MFXQueryVersion(m_session, version);
    }

    virtual mfxStatus JoinSession(mfxSession child_session) {
        return MFXJoinSession(m_session, child_session);
    }
    virtual mfxStatus DisjoinSession() {
        return MFXDisjoinSession(m_session);
    }
    virtual mfxStatus CloneSession(mfxSession *clone) {
        return MFXCloneSession(m_session, clone);
    }
    virtual mfxStatus SetPriority(mfxPriority priority) {
        return MFXSetPriority(m_session, priority);
    }
    virtual mfxStatus GetPriority(mfxPriority *priority) {
        return MFXGetPriority(m_session, priority);
    }

    virtual mfxStatus SetFrameAllocator(mfxFrameAllocator *allocator) {
        return MFXVideoCORE_SetFrameAllocator(m_session, allocator);
    }
    virtual mfxStatus SetHandle(mfxHandleType type, mfxHDL hdl) {
        return MFXVideoCORE_SetHandle(m_session, type, hdl);
    }
    virtual mfxStatus GetHandle(mfxHandleType type, mfxHDL *hdl) {
        return MFXVideoCORE_GetHandle(m_session, type, hdl);
    }
    virtual mfxStatus QueryPlatform(mfxPlatform *platform) {
        return MFXVideoCORE_QueryPlatform(m_session, platform);
    }

    virtual mfxStatus SyncOperation(mfxSyncPoint syncp, mfxU32 wait) {
        return MFXVideoCORE_SyncOperation(m_session, syncp, wait);
    }

    virtual mfxStatus GetSurfaceForEncode(mfxFrameSurface1** output_surf) {
        return MFXMemory_GetSurfaceForEncode(m_session, output_surf);
    }
    virtual mfxStatus GetSurfaceForDecode(mfxFrameSurface1** output_surf) {
        return MFXMemory_GetSurfaceForDecode(m_session, output_surf);
    }
    virtual mfxStatus GetSurfaceForVPP   (mfxFrameSurface1** output_surf) {
        return MFXMemory_GetSurfaceForVPP   (m_session, output_surf);
    }
    virtual mfxStatus GetSurfaceForVPPOut(mfxFrameSurface1** output_surf) {
        return MFXMemory_GetSurfaceForVPPOut(m_session, output_surf);
    }

    virtual operator mfxSession(void) {
        return m_session;
    }

protected:
    mfxSession m_session; // (mfxSession) handle to the owning session
    mfxLoader m_loader;
    bool m_initialized;
    std::vector<mfxConfig> m_Configs;

    mfxStatus CreateConfig(mfxU32 data, const char* propertyName) {
        mfxConfig cfg = MFXCreateConfig(m_loader);
        mfxVariant variant;
        variant.Type     = MFX_VARIANT_TYPE_U32;
        variant.Data.U32 = data;
        mfxStatus sts    = MFXSetConfigFilterProperty(cfg, (mfxU8*)propertyName, variant);

        if (MFX_ERR_NONE == sts) m_Configs.push_back(cfg);

        return sts;
    }


    mfxStatus InitSession(mfxInitParam par) {
        if (false == m_initialized) {
            m_loader = MFXLoad();
            if (!m_loader) return MFX_ERR_NOT_FOUND;

            mfxStatus sts = MFX_ERR_NONE;

            mfxU32 implBasetype=MFX_IMPL_BASETYPE(par.Implementation);
            switch(implBasetype) {

                case MFX_IMPL_AUTO:
                    // For legacy dispatcher compatibility only.  The new dispatcher does not use it.
                case MFX_IMPL_AUTO_ANY:
                    // Selects highest priority implementation providing API version specified
                    // By dispatcher implementation sorting rules, hardware implementations (if available)
                    // will be chosen before software.  If a hardware implementation is not found
                    // a software implementation will be automatically selected.
                    break; 

                case MFX_IMPL_SOFTWARE:
                    // Select a pure software implementation, even if hardware is available
                    sts = CreateConfig(MFX_IMPL_TYPE_SOFTWARE, "mfxImplDescription.Impl");
                    break;
                
                case MFX_IMPL_HARDWARE:
                case MFX_IMPL_HARDWARE_ANY:
                case MFX_IMPL_HARDWARE2:
                case MFX_IMPL_HARDWARE3:
                case MFX_IMPL_HARDWARE4:
                    // Select a hardware accelerated implementation only
                    sts = CreateConfig(MFX_IMPL_TYPE_HARDWARE, "mfxImplDescription.Impl");
                    break;

                default:
                    sts = MFX_ERR_INVALID_VIDEO_PARAM;
                    break;
            }
            if(MFX_ERR_NONE != sts) return MFX_ERR_INVALID_VIDEO_PARAM;


            // Select a Windows adapter if a HARDWAREn implementation is used
            // Note: in Linux, adapters are selected by /dev/dri/renderDn 
            if (MFX_IMPL_HARDWARE2==implBasetype) {
                sts = CreateConfig(1, "mfxImplDescription.VendorImplID");
            } else if (MFX_IMPL_HARDWARE3 == implBasetype) {
                sts = CreateConfig(2, "mfxImplDescription.VendorImplID");
            } else if (MFX_IMPL_HARDWARE4 == implBasetype) {
                sts = CreateConfig(3, "mfxImplDescription.VendorImplID");
            }
            if(MFX_ERR_NONE != sts) return MFX_ERR_INVALID_VIDEO_PARAM;


            // Add acceleration mode if specified
            mfxU32 accelerationMode = MFX_IMPL_ACCELMODE(par.Implementation);
            if (accelerationMode>=MFX_ACCEL_MODE_VIA_D3D9) {
                //VIA_D3D9      0x0200 = MFX_ACCEL_MODE_VIA_D3D9
                //VIA_D3D11     0x0300 = MFX_ACCEL_MODE_VIA_D3D11
                //VIA_VAAPI     0x0400 = MFX_ACCEL_MODE_VIA_VAAPI
                //VIA_HDDLUNITE 0x0500 = MFX_ACCEL_MODE_VIA_HDDLUNITE
                sts = CreateConfig(accelerationMode, "mfxImplDescription.AccelerationMode");
                if(MFX_ERR_NONE != sts) return MFX_ERR_INVALID_VIDEO_PARAM;
            }


            // Filter out implementations not providing specified API level
            sts = CreateConfig(par.Version.Version, "mfxImplDescription.ApiVersion.Version");
            if(MFX_ERR_NONE != sts) return MFX_ERR_INVALID_VIDEO_PARAM;


            // Create session with highest priority implementation remaining after filters
            sts = MFXCreateSession(m_loader, 0, &m_session);
            if(MFX_ERR_NONE != sts) return sts;

            // set initialized flag so that init steps can be skipped if init is called again
            m_initialized = true;
        }
        //already initialized
        return MFX_ERR_NONE;
    }
private:
    MFXVideoSession(const MFXVideoSession &);
    void operator=(MFXVideoSession &);
};

class MFXVideoENCODE {
public:
    explicit MFXVideoENCODE(mfxSession session) {
        m_session = session;
    }
    virtual ~MFXVideoENCODE(void) {
        Close();
    }

    virtual mfxStatus Query(mfxVideoParam *in, mfxVideoParam *out) {
        return MFXVideoENCODE_Query(m_session, in, out);
    }
    virtual mfxStatus QueryIOSurf(mfxVideoParam *par, mfxFrameAllocRequest *request) {
        return MFXVideoENCODE_QueryIOSurf(m_session, par, request);
    }
    virtual mfxStatus Init(mfxVideoParam *par) {
        return MFXVideoENCODE_Init(m_session, par);
    }
    virtual mfxStatus Reset(mfxVideoParam *par) {
        return MFXVideoENCODE_Reset(m_session, par);
    }
    virtual mfxStatus Close(void) {
        return MFXVideoENCODE_Close(m_session);
    }

    virtual mfxStatus GetVideoParam(mfxVideoParam *par) {
        return MFXVideoENCODE_GetVideoParam(m_session, par);
    }
    virtual mfxStatus GetEncodeStat(mfxEncodeStat *stat) {
        return MFXVideoENCODE_GetEncodeStat(m_session, stat);
    }

    virtual mfxStatus EncodeFrameAsync(mfxEncodeCtrl *ctrl,
                                       mfxFrameSurface1 *surface,
                                       mfxBitstream *bs,
                                       mfxSyncPoint *syncp) {
        return MFXVideoENCODE_EncodeFrameAsync(m_session, ctrl, surface, bs, syncp);
    }

    virtual mfxStatus GetSurface(mfxFrameSurface1** output_surf) {
        return MFXMemory_GetSurfaceForEncode(m_session, output_surf);
    }

protected:
    mfxSession m_session; // (mfxSession) handle to the owning session
};

class MFXVideoDECODE {
public:
    explicit MFXVideoDECODE(mfxSession session) {
        m_session = session;
    }
    virtual ~MFXVideoDECODE(void) {
        Close();
    }

    virtual mfxStatus Query(mfxVideoParam *in, mfxVideoParam *out) {
        return MFXVideoDECODE_Query(m_session, in, out);
    }
    virtual mfxStatus DecodeHeader(mfxBitstream *bs, mfxVideoParam *par) {
        return MFXVideoDECODE_DecodeHeader(m_session, bs, par);
    }
    virtual mfxStatus QueryIOSurf(mfxVideoParam *par, mfxFrameAllocRequest *request) {
        return MFXVideoDECODE_QueryIOSurf(m_session, par, request);
    }
    virtual mfxStatus Init(mfxVideoParam *par) {
        return MFXVideoDECODE_Init(m_session, par);
    }
    virtual mfxStatus Reset(mfxVideoParam *par) {
        return MFXVideoDECODE_Reset(m_session, par);
    }
    virtual mfxStatus Close(void) {
        return MFXVideoDECODE_Close(m_session);
    }

    virtual mfxStatus GetVideoParam(mfxVideoParam *par) {
        return MFXVideoDECODE_GetVideoParam(m_session, par);
    }

    virtual mfxStatus GetDecodeStat(mfxDecodeStat *stat) {
        return MFXVideoDECODE_GetDecodeStat(m_session, stat);
    }
    virtual mfxStatus GetPayload(mfxU64 *ts, mfxPayload *payload) {
        return MFXVideoDECODE_GetPayload(m_session, ts, payload);
    }
    virtual mfxStatus SetSkipMode(mfxSkipMode mode) {
        return MFXVideoDECODE_SetSkipMode(m_session, mode);
    }
    virtual mfxStatus DecodeFrameAsync(mfxBitstream *bs,
                                       mfxFrameSurface1 *surface_work,
                                       mfxFrameSurface1 **surface_out,
                                       mfxSyncPoint *syncp) {
        return MFXVideoDECODE_DecodeFrameAsync(m_session, bs, surface_work, surface_out, syncp);
    }

    virtual mfxStatus GetSurface(mfxFrameSurface1** output_surf) {
        return MFXMemory_GetSurfaceForDecode(m_session, output_surf);
    }

protected:
    mfxSession m_session; // (mfxSession) handle to the owning session
};

class MFXVideoVPP {
public:
    explicit MFXVideoVPP(mfxSession session) {
        m_session = session;
    }
    virtual ~MFXVideoVPP(void) {
        Close();
    }

    virtual mfxStatus Query(mfxVideoParam *in, mfxVideoParam *out) {
        return MFXVideoVPP_Query(m_session, in, out);
    }
    virtual mfxStatus QueryIOSurf(mfxVideoParam *par, mfxFrameAllocRequest request[2]) {
        return MFXVideoVPP_QueryIOSurf(m_session, par, request);
    }
    virtual mfxStatus Init(mfxVideoParam *par) {
        return MFXVideoVPP_Init(m_session, par);
    }
    virtual mfxStatus Reset(mfxVideoParam *par) {
        return MFXVideoVPP_Reset(m_session, par);
    }
    virtual mfxStatus Close(void) {
        return MFXVideoVPP_Close(m_session);
    }

    virtual mfxStatus GetVideoParam(mfxVideoParam *par) {
        return MFXVideoVPP_GetVideoParam(m_session, par);
    }
    virtual mfxStatus GetVPPStat(mfxVPPStat *stat) {
        return MFXVideoVPP_GetVPPStat(m_session, stat);
    }
    virtual mfxStatus RunFrameVPPAsync(mfxFrameSurface1 *in,
                                       mfxFrameSurface1 *out,
                                       mfxExtVppAuxData *aux,
                                       mfxSyncPoint *syncp) {
        return MFXVideoVPP_RunFrameVPPAsync(m_session, in, out, aux, syncp);
    }

    virtual mfxStatus GetSurfaceIn(mfxFrameSurface1** output_surf) {
        return MFXMemory_GetSurfaceForVPP(m_session, output_surf);
    }
    virtual mfxStatus GetSurfaceOut(mfxFrameSurface1** output_surf) {
        return MFXMemory_GetSurfaceForVPPOut(m_session, output_surf);
    }

    virtual mfxStatus ProcessFrameAsync(mfxFrameSurface1 *in, mfxFrameSurface1 **out) {
        return MFXVideoVPP_ProcessFrameAsync(m_session, in, out);
    }

protected:
    mfxSession m_session; // (mfxSession) handle to the owning session
};

class MFXVideoDECODE_VPP
{
public:
    explicit MFXVideoDECODE_VPP(mfxSession session) {
        m_session = session;
    }
    virtual ~MFXVideoDECODE_VPP(void) {
        Close();
    }

    virtual mfxStatus Init(mfxVideoParam* decode_par, mfxVideoChannelParam** vpp_par_array, mfxU32 num_channel_par) {
        return MFXVideoDECODE_VPP_Init(m_session, decode_par, vpp_par_array, num_channel_par);
    }
    virtual mfxStatus Reset(mfxVideoParam* decode_par, mfxVideoChannelParam** vpp_par_array, mfxU32 num_channel_par) {
        return MFXVideoDECODE_VPP_Reset(m_session, decode_par, vpp_par_array, num_channel_par);
    }
    virtual mfxStatus GetChannelParam(mfxVideoChannelParam *par, mfxU32 channel_id) {
        return MFXVideoDECODE_VPP_GetChannelParam(m_session, par, channel_id);
    }
    virtual mfxStatus DecodeFrameAsync(mfxBitstream *bs, mfxU32* skip_channels, mfxU32 num_skip_channels, mfxSurfaceArray **surf_array_out) {
        return MFXVideoDECODE_VPP_DecodeFrameAsync(m_session, bs, skip_channels, num_skip_channels, surf_array_out);
    }

    virtual mfxStatus DecodeHeader(mfxBitstream *bs, mfxVideoParam *par) {
        return MFXVideoDECODE_VPP_DecodeHeader(m_session, bs, par);
    }
    virtual mfxStatus Close(void) {
        return MFXVideoDECODE_VPP_Close(m_session);
    }
    virtual mfxStatus GetVideoParam(mfxVideoParam *par) {
        return MFXVideoDECODE_VPP_GetVideoParam(m_session, par);
    }
    virtual mfxStatus GetDecodeStat(mfxDecodeStat *stat) {
        return MFXVideoDECODE_VPP_GetDecodeStat(m_session, stat);
    }
    virtual mfxStatus GetPayload(mfxU64 *ts, mfxPayload *payload) {
        return MFXVideoDECODE_VPP_GetPayload(m_session, ts, payload);
    }
    virtual mfxStatus SetSkipMode(mfxSkipMode mode) {
        return MFXVideoDECODE_VPP_SetSkipMode(m_session, mode);
    }

protected:
    mfxSession m_session; // (mfxSession) handle to the owning session
};

#endif //__MFXVIDEOPLUSPLUS_H
