/*############################################################################
  # Copyright (C) 2005 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef __D3D_ALLOCATOR_H__
#define __D3D_ALLOCATOR_H__

#if defined(_WIN32) || defined(_WIN64)

    #include <atlbase.h>
    #include <d3d9.h>
    #include <dxva2api.h>
    #include <vector>
    #include "base_allocator.h"

enum eTypeHandle { DXVA2_PROCESSOR = 0x00, DXVA2_DECODER = 0x01 };

struct D3DAllocatorParams : mfxAllocatorParams {
    IDirect3DDeviceManager9* pManager;
    DWORD surfaceUsage;

    D3DAllocatorParams() : pManager(), surfaceUsage() {}
};

class D3DFrameAllocator : public BaseFrameAllocator {
public:
    D3DFrameAllocator();
    virtual ~D3DFrameAllocator();

    virtual mfxStatus Init(mfxAllocatorParams* pParams);
    virtual mfxStatus Close();

    virtual IDirect3DDeviceManager9* GetDeviceManager() {
        return m_manager;
    };

    virtual mfxStatus LockFrame(mfxMemId mid, mfxFrameData* ptr);
    virtual mfxStatus UnlockFrame(mfxMemId mid, mfxFrameData* ptr);
    virtual mfxStatus GetFrameHDL(mfxMemId mid, mfxHDL* handle);

    virtual mfxStatus Create3DLutMemory(mfxMemId memId, mfxU8* lut3d, mfxU32 lut3d_size) {
        return MFX_ERR_NONE;
    }
    virtual mfxStatus Release3DLutMemory(mfxMemId memId) {
        return MFX_ERR_NONE;
    }

protected:
    virtual mfxStatus CheckRequestType(mfxFrameAllocRequest* request);
    virtual mfxStatus ReleaseResponse(mfxFrameAllocResponse* response);
    virtual mfxStatus AllocImpl(mfxFrameAllocRequest* request, mfxFrameAllocResponse* response);
    virtual mfxStatus ReallocImpl(mfxMemId midIn,
                                  const mfxFrameInfo* info,
                                  mfxU16 memType,
                                  mfxMemId* midOut);

    void DeallocateMids(mfxHDLPair** pairs, int n);

    std::vector<mfxHDLPair**> m_midsAllocated;

    CComPtr<IDirect3DDeviceManager9> m_manager;
    CComPtr<IDirectXVideoDecoderService> m_decoderService;
    CComPtr<IDirectXVideoProcessorService> m_processorService;
    HANDLE m_hDecoder;
    HANDLE m_hProcessor;
    DWORD m_surfaceUsage;
};

#endif // #if defined( _WIN32 ) || defined ( _WIN64 )
#endif // __D3D_ALLOCATOR_H__
