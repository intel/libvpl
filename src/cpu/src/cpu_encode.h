/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef SRC_CPU_SRC_CPU_ENCODE_H_
#define SRC_CPU_SRC_CPU_ENCODE_H_

#include <memory>
#include <string>
#include <utility>
#include "src/cpu_common.h"
#include "src/cpu_frame_pool.h"
#include "src/frame_lock.h"

class CpuWorkstream;

class CpuEncode {
public:
    explicit CpuEncode(CpuWorkstream* session);
    ~CpuEncode();

    static mfxStatus EncodeQuery(mfxVideoParam* in, mfxVideoParam* out);
    static mfxStatus EncodeQueryIOSurf(mfxVideoParam* par,
                                       mfxFrameAllocRequest* request);

    mfxStatus InitEncode(mfxVideoParam* par);
    mfxStatus EncodeFrame(mfxFrameSurface1* surface, mfxBitstream* bs);
    mfxStatus GetVideoParam(mfxVideoParam* par);
    mfxStatus GetEncodeSurface(mfxFrameSurface1** surface);

private:
    static mfxStatus ValidateEncodeParams(mfxVideoParam* par);
    int convertTargetUsageVal(int val,
                              int minIn,
                              int maxIn,
                              int minOut,
                              int maxOut);
    mfxStatus InitHEVCParams(mfxVideoParam* par);
    mfxStatus GetHEVCParams(mfxVideoParam* par);
    mfxStatus InitAV1Params(mfxVideoParam* par);
    mfxStatus GetAV1Params(mfxVideoParam* par);
    mfxStatus InitAVCParams(mfxVideoParam* par);
    mfxStatus GetAVCParams(mfxVideoParam* par);
    mfxStatus InitJPEGParams(mfxVideoParam* par);
    mfxStatus GetJPEGParams(mfxVideoParam* par);

    AVFrame* CreateAVFrame(mfxFrameSurface1* surface);

    const AVCodec* m_avEncCodec;
    AVCodecContext* m_avEncContext;
    AVPacket* m_avEncPacket;
    FrameLock m_input_locker;

    mfxVideoParam m_param;

    CpuWorkstream* m_session;

    std::unique_ptr<CpuFramePool> m_encSurfaces;

    /* copy not allowed */
    CpuEncode(const CpuEncode&);
    CpuEncode& operator=(const CpuEncode&);
};

#endif // SRC_CPU_SRC_CPU_ENCODE_H_
