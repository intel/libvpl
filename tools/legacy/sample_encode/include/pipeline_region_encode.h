/*############################################################################
  # Copyright (C) 2005 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef __PIPELINE_REGION_ENCODE_H__
#define __PIPELINE_REGION_ENCODE_H__

#include "pipeline_encode.h"

#ifndef MFX_VERSION
    #error MFX_VERSION not defined
#endif

class CMSDKResource {
public:
    CMSDKResource() : Session(), pEncoder(NULL), TaskPool() {}

    MainVideoSession Session;
    MFXVideoENCODE* pEncoder;
    CEncTaskPool TaskPool;
};

class CResourcesPool {
public:
    CResourcesPool() {}

    ~CResourcesPool() {
        delete[] m_resources;
    }

    CMSDKResource& operator[](int index) {
        return m_resources[index];
    }

    int GetSize() {
        return m_size;
    }

    mfxStatus Init(int size, mfxIMPL impl, mfxVersion* pVer);
    mfxStatus Init(int size, VPLImplementationLoader* Loader, mfxU32 nSyncOpTimeout);
    mfxStatus InitTaskPools(CSmplBitstreamWriter* pWriter,
                            mfxU32 nPoolSize,
                            mfxU32 nBufferSize,
                            mfxU32 CodecId,
                            void* pOtherWriter = NULL,
                            bool bUseHWLib     = false);
    mfxStatus CreateEncoders();
    mfxStatus CreatePlugins(mfxPluginUID pluginGUID, mfxChar* pluginPath);

    mfxStatus GetFreeTask(int resourceNum, sTask** ppTask);
    void CloseAndDeleteEverything();

protected:
    CMSDKResource* m_resources = nullptr;
    int m_size                 = 0;
    mfxU32 m_nSyncOpTimeout    = 0; // SyncOperation timeout in msec

private:
    CResourcesPool(const CResourcesPool& src) {
        (void)src;
    }
    CResourcesPool& operator=(const CResourcesPool& src) {
        (void)src;
        return *this;
    }
};

/* This class implements a pipeline with 2 mfx components: vpp (video preprocessing) and encode */
class CRegionEncodingPipeline : public CEncodingPipeline {
public:
    CRegionEncodingPipeline();
    virtual ~CRegionEncodingPipeline();

    virtual mfxStatus Init(sInputParams* pParams);
    virtual mfxStatus Run();
    virtual void Close();
    virtual mfxStatus ResetMFXComponents(sInputParams* pParams);

    void SetMultiView();
    void SetNumView(mfxU32 numViews) {
        m_nNumView = numViews;
    }

protected:
    mfxI64 m_timeAll;
    CResourcesPool m_resources;

    virtual mfxStatus InitMfxEncParams(sInputParams* pParams);

    virtual mfxStatus CreateAllocator();

    virtual MFXVideoSession& GetFirstSession() {
        return m_resources[0].Session;
    }
    virtual MFXVideoENCODE* GetFirstEncoder() {
        return m_resources[0].pEncoder;
    }
};

#endif // __PIPELINE_REGION_ENCODE_H__
