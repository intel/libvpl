/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef SRC_CPU_SRC_CPU_WORKSTREAM_H_
#define SRC_CPU_SRC_CPU_WORKSTREAM_H_

#include <map>
#include <memory>
#include "src/cpu_common.h"
#include "src/cpu_decode.h"
#include "src/cpu_encode.h"
#include "src/cpu_frame.h"
#include "src/cpu_frame_pool.h"
#include "src/cpu_vpp.h"

class CpuWorkstream {
public:
    CpuWorkstream();
    ~CpuWorkstream();

    void SetDecoder(CpuDecode* decode) {
        m_decode.reset(decode);
    }
    void SetEncoder(CpuEncode* encode) {
        m_encode.reset(encode);
    }
    void SetVPP(CpuVPP* vpp) {
        m_vpp.reset(vpp);
    }

    CpuDecode* GetDecoder() {
        return m_decode.get();
    }
    CpuEncode* GetEncoder() {
        return m_encode.get();
    }
    CpuVPP* GetVPP() {
        return m_vpp.get();
    }

    mfxStatus Sync(mfxSyncPoint& syncp, mfxU32 wait);

    mfxStatus SetFrameAllocator(mfxFrameAllocator* allocator) {
        RET_IF_FALSE(allocator, MFX_ERR_NULL_PTR);
        m_allocator = *allocator;
        return MFX_ERR_NONE;
    }

    mfxFrameAllocator* GetFrameAllocator() {
        return m_allocator.pthis ? &m_allocator : nullptr;
    }

    void SetHandle(mfxHandleType ht, mfxHDL hdl) {
        m_handles[ht] = hdl;
    }

    mfxHDL* GetHandle(mfxHandleType ht) {
        if (m_handles.find(ht) == m_handles.end()) {
            return nullptr;
        }
        else {
            return &m_handles[ht];
        }
    }

private:
    std::unique_ptr<CpuDecode> m_decode;
    std::unique_ptr<CpuEncode> m_encode;
    std::unique_ptr<CpuVPP> m_vpp;

    mfxFrameAllocator m_allocator;
    std::map<mfxHandleType, mfxHDL> m_handles;

    /* copy not allowed */
    CpuWorkstream(const CpuWorkstream&);
    CpuWorkstream& operator=(const CpuWorkstream&);
};

#endif // SRC_CPU_SRC_CPU_WORKSTREAM_H_
