/*############################################################################
  # Copyright (C) 2005 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef __SYSMEM_ALLOCATOR_H__
#define __SYSMEM_ALLOCATOR_H__

#include <stdlib.h>
#include <vector>
#include "base_allocator.h"

struct sBuffer {
    mfxU32 id;
    mfxU32 nbytes;
    mfxU16 type;
};

struct sFrame {
    mfxU32 id;
    mfxFrameInfo info;
};

struct SysMemAllocatorParams : mfxAllocatorParams {
    SysMemAllocatorParams() : mfxAllocatorParams(), pBufferAllocator(NULL) {}
    MFXBufferAllocator* pBufferAllocator;
};

class SysMemFrameAllocator : public BaseFrameAllocator {
public:
    SysMemFrameAllocator();
    virtual ~SysMemFrameAllocator();

    virtual mfxStatus Init(mfxAllocatorParams* pParams);
    virtual mfxStatus Close();
    virtual mfxStatus LockFrame(mfxMemId mid, mfxFrameData* ptr);
    virtual mfxStatus UnlockFrame(mfxMemId mid, mfxFrameData* ptr);
    virtual mfxStatus GetFrameHDL(mfxMemId mid, mfxHDL* handle);

protected:
    virtual mfxStatus CheckRequestType(mfxFrameAllocRequest* request);
    virtual mfxStatus ReleaseResponse(mfxFrameAllocResponse* response);
    virtual mfxStatus AllocImpl(mfxFrameAllocRequest* request, mfxFrameAllocResponse* response);
    virtual mfxStatus ReallocImpl(mfxMemId midIn,
                                  const mfxFrameInfo* info,
                                  mfxU16 memType,
                                  mfxMemId* midOut);

    virtual mfxStatus Create3DLutMemory(mfxMemId memId, const char* lut3d_file_name) {
        return MFX_ERR_NONE;
    }
    virtual mfxStatus Release3DLutMemory(mfxMemId memId) {
        return MFX_ERR_NONE;
    }

    MFXBufferAllocator* m_pBufferAllocator;
    bool m_bOwnBufferAllocator;

    std::vector<mfxFrameAllocResponse*> m_vResp;

    mfxMemId* GetMidHolder(mfxMemId mid);
};

class SysMemBufferAllocator : public MFXBufferAllocator {
public:
    SysMemBufferAllocator();
    virtual ~SysMemBufferAllocator();
    virtual mfxStatus AllocBuffer(mfxU32 nbytes, mfxU16 type, mfxMemId* mid);
    virtual mfxStatus LockBuffer(mfxMemId mid, mfxU8** ptr);
    virtual mfxStatus UnlockBuffer(mfxMemId mid);
    virtual mfxStatus FreeBuffer(mfxMemId mid);
};

#endif // __SYSMEM_ALLOCATOR_H__