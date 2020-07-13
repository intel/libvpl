/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef SRC_CPU_SRC_CPU_WORKSTREAM_H_
#define SRC_CPU_SRC_CPU_WORKSTREAM_H_

#include <chrono>
#include <future>
#include <string>

#include "vpl/mfxstructures.h"

#include "vpl/mfxjpeg.h"

#define ENABLE_LIBAV_AUTO_THREADS

#if !defined(WIN32) && !defined(memcpy_s)
    #define memcpy_s(dest, destsz, src, count) memcpy(dest, src, count)
#endif

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavutil/imgutils.h"
#include "libavutil/opt.h"
#include "libswscale/swscale.h"
}

#define ERR_EXIT(ws)                    \
    { /* optional logging, etc. here */ \
        return MFX_ERR_UNKNOWN;         \
    }

class CpuWorkstream {
public:
    CpuWorkstream();
    ~CpuWorkstream();

    // decode
    mfxStatus InitDecode(mfxU32 FourCC);
    mfxStatus DecodeHeader(mfxBitstream *bs, mfxVideoParam *par);
    mfxStatus DecodeFrame(mfxBitstream *bs,
                          mfxFrameSurface1 *surface_work,
                          mfxFrameSurface1 **surface_out);
    void FreeDecode(void);

    // VPP
    mfxStatus InitVPP(void);
    mfxStatus ProcessFrame(void);
    void FreeVPP(void);

    // encode
    mfxStatus InitEncode(mfxVideoParam *par);
    mfxStatus EncodeFrame(mfxFrameSurface1 *surface, mfxBitstream *bs);
    void FreeEncode(void);

    mfxStatus Sync(mfxU32 wait);

    bool getDecInit() {
        return m_decInit;
    }
    bool getVppInit() {
        return m_vppInit;
    }
    bool getEncInit() {
        return m_encInit;
    }

private:
    bool m_decInit;
    bool m_vppInit;
    bool m_vppBypass;
    bool m_encInit;

    CpuWorkstream(const CpuWorkstream &) { /* copy not allowed */
    }
    CpuWorkstream &operator=(const CpuWorkstream &) {
        return *this; /* copy not allowed */
    }

    void AVFrame2mfxFrameSurface(mfxFrameSurface1 *surface_work);

    mfxStatus InitHEVCParams(mfxVideoParam *par);
    mfxStatus InitAV1Params(mfxVideoParam *par);

    // libav objects - Decode
    const AVCodec *m_avDecCodec;
    AVCodecContext *m_avDecContext;
    AVCodecParserContext *m_avDecParser;
    AVPacket *m_avDecPacket;

    // bitstream buffer - Decode
    uint8_t *m_bsDecData;
    uint32_t m_bsDecValidBytes;
    uint32_t m_bsDecMaxBytes;

    // libav objects - VPP
    struct SwsContext *m_avVppContext;

    // libav objects - Encode
    const AVCodec *m_avEncCodec;
    AVCodecContext *m_avEncContext;
    AVPacket *m_avEncPacket;

    // libav frames
    AVFrame *m_avDecFrameOut;
    AVFrame *m_avVppFrameIn;
    AVFrame *m_avVppFrameOut;
    AVFrame *m_avEncFrameIn;

    // other internal state
    mfxU32 m_encCodecId;
};

#endif // SRC_CPU_SRC_CPU_WORKSTREAM_H_
