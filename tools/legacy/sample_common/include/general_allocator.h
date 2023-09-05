/*############################################################################
  # Copyright (C) 2005 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef __GENERAL_ALLOCATOR_H__
#define __GENERAL_ALLOCATOR_H__

#include "base_allocator.h"
#include "sample_utils.h"

#include <map>
#include <memory>

class SysMemFrameAllocator;

// Wrapper on standard allocator for concurrent allocation of
// D3D and system surfaces
class GeneralAllocator : public BaseFrameAllocator {
public:
    GeneralAllocator();
    virtual ~GeneralAllocator();

    virtual mfxStatus Init(mfxAllocatorParams* pParams);
    virtual mfxStatus Close();

protected:
    virtual mfxStatus LockFrame(mfxMemId mid, mfxFrameData* ptr);
    virtual mfxStatus UnlockFrame(mfxMemId mid, mfxFrameData* ptr);
    virtual mfxStatus GetFrameHDL(mfxMemId mid, mfxHDL* handle);

    virtual mfxStatus ReleaseResponse(mfxFrameAllocResponse* response);
    virtual mfxStatus AllocImpl(mfxFrameAllocRequest* request, mfxFrameAllocResponse* response);
    virtual mfxStatus ReallocImpl(mfxMemId midIn,
                                  const mfxFrameInfo* info,
                                  mfxU16 memType,
                                  mfxMemId* midOut);
    virtual mfxStatus Create3DLutMemory(mfxMemId memId, const char* lut3d_file_name);
    virtual mfxStatus Release3DLutMemory(mfxMemId memId);

    void StoreFrameMids(bool isD3DFrames, mfxFrameAllocResponse* response);
    bool isD3DMid(mfxHDL mid);

    std::mutex m_MidsGuard;
    std::map<mfxHDL, bool> m_Mids;
    std::unique_ptr<BaseFrameAllocator> m_D3DAllocator;
    std::unique_ptr<SysMemFrameAllocator> m_SYSAllocator;

private:
    DISALLOW_COPY_AND_ASSIGN(GeneralAllocator);
};

#endif //__GENERAL_ALLOCATOR_H__
