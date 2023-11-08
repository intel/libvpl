/*############################################################################
  # Copyright (C) 2005 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef __SAMPLE_UTILS_H__
#define __SAMPLE_UTILS_H__

#include <stdio.h>
#include <algorithm>
#include <fstream>
#include <map>
#include <memory>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <atomic>
#include <condition_variable>
#include <iostream>
#include <thread>

#include "mfxdeprecated.h"
#include "mfxplugin.h"
#include "vpl/mfxbrc.h"
#include "vpl/mfxjpeg.h"
#include "vpl/mfxmvc.h"
#include "vpl/mfxstructures.h"
#include "vpl/mfxvideo++.h"
#include "vpl/mfxvideo.h"

#include "vm/atomic_defs.h"
#include "vm/file_defs.h"
#include "vm/strings_defs.h"
#include "vm/thread_defs.h"
#include "vm/time_defs.h"

#include "abstract_splitter.h"
#include "avc_bitstream.h"
#include "avc_headers.h"
#include "avc_nal_spl.h"
#include "avc_spl.h"
#include "vpl_implementation_loader.h"

#include "vpl/mfxsurfacepool.h"

// A macro to disallow the copy constructor and operator= functions
// This should be used in the private: declarations for a class
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
    TypeName(const TypeName&);             \
    void operator=(const TypeName&)

//! Base class for types that should not be assigned.
class no_assign {
    // Deny assignment
    void operator=(const no_assign&);

public:
#if __GNUC__
    //! Explicitly define default construction, because otherwise gcc issues gratuitous warning.
    no_assign() {}
#endif /* __GNUC__ */
};

enum {
    CODEC_VP8 = MFX_MAKEFOURCC('V', 'P', '8', ' '),
    CODEC_MVC = MFX_MAKEFOURCC('M', 'V', 'C', ' '),
};

#define MFX_CODEC_DUMP MFX_MAKEFOURCC('D', 'U', 'M', 'P')
#define MFX_CODEC_RGB4 MFX_FOURCC_RGB4
#define MFX_CODEC_NV12 MFX_FOURCC_NV12
#define MFX_CODEC_I420 MFX_FOURCC_I420
#define MFX_CODEC_I422 MFX_FOURCC_I422
#define MFX_CODEC_P010 MFX_FOURCC_P010
#define MFX_CODEC_YUY2 MFX_FOURCC_YUY2
#define MFX_CODEC_Y210 MFX_FOURCC_Y210

enum {
    MFX_FOURCC_IMC3    = MFX_MAKEFOURCC('I', 'M', 'C', '3'),
    MFX_FOURCC_YUV400  = MFX_MAKEFOURCC('4', '0', '0', 'P'),
    MFX_FOURCC_YUV411  = MFX_MAKEFOURCC('4', '1', '1', 'P'),
    MFX_FOURCC_YUV422H = MFX_MAKEFOURCC('4', '2', '2', 'H'),
    MFX_FOURCC_YUV422V = MFX_MAKEFOURCC('4', '2', '2', 'V'),
    MFX_FOURCC_YUV444  = MFX_MAKEFOURCC('4', '4', '4', 'P'),
    MFX_FOURCC_RGBP24  = MFX_MAKEFOURCC('R', 'G', 'B', 'P'),
};

enum ExtBRCType { EXTBRC_DEFAULT, EXTBRC_OFF, EXTBRC_ON, EXTBRC_IMPLICIT };

namespace QPFile {

enum ReaderStatus {
    READER_ERR_NONE,
    READER_ERR_NOT_INITIALIZED,
    READER_ERR_CODEC_UNSUPPORTED,
    READER_ERR_FILE_NOT_OPEN,
    READER_ERR_INCORRECT_FILE
};

struct FrameInfo {
    mfxU32 displayOrder;
    mfxU16 QP;
    mfxU16 frameType;
};

// QPFile::Reader reads QP and frame type per frame in encoding order
// from external text file (for encoding in qpfile mode)
class Reader {
public:
    mfxStatus Read(const std::string& strFileName, mfxU32 codecid);
    void ResetState();

    mfxU32 GetCurrentEncodedOrder() const;
    mfxU32 GetCurrentDisplayOrder() const;
    mfxU16 GetCurrentQP() const;
    mfxU16 GetCurrentFrameType() const;
    mfxU32 GetFramesNum() const;
    void NextFrame();
    std::string GetErrorMessage() const;

private:
    void ResetState(ReaderStatus set_sts);

    ReaderStatus m_ReaderSts = READER_ERR_NOT_INITIALIZED;
    mfxU32 m_nFrames         = std::numeric_limits<mfxU32>::max();
    mfxU32 m_CurFrameNum     = std::numeric_limits<mfxU32>::max();
    std::vector<FrameInfo> m_FrameVals{};
};

inline bool get_line(std::ifstream& ifs, std::string& line) {
    std::getline(ifs, line, '\n');
    if (!line.empty() && line.back() == '\r')
        line.pop_back();
    return !ifs.fail();
}
inline size_t find_nth(const std::string& str, size_t pos, const std::string& needle, mfxU32 nth) {
    size_t found_pos = str.find(needle, pos);
    for (; nth != 0 && std::string::npos != found_pos; --nth)
        found_pos = str.find(needle, found_pos + 1);
    return found_pos;
}
inline mfxU16 StringToFrameType(std::string str) {
    if ("IDR_REF" == str)
        return MFX_FRAMETYPE_I | MFX_FRAMETYPE_IDR | MFX_FRAMETYPE_REF;
    else if ("I_REF" == str)
        return MFX_FRAMETYPE_I | MFX_FRAMETYPE_REF;
    else if ("P_REF" == str)
        return MFX_FRAMETYPE_P | MFX_FRAMETYPE_REF;
    else if ("P" == str)
        return MFX_FRAMETYPE_P;
    else if ("B_REF" == str)
        return MFX_FRAMETYPE_B | MFX_FRAMETYPE_REF;
    else if ("B" == str)
        return MFX_FRAMETYPE_B;
    else
        return MFX_FRAMETYPE_UNKNOWN;
}
inline std::string ReaderStatusToString(ReaderStatus sts) {
    switch (sts) {
        case READER_ERR_NOT_INITIALIZED:
            return std::string("reader not initialized (qpfile has not yet read the file)\n");
        case READER_ERR_FILE_NOT_OPEN:
            return std::string(
                "failed to open file contains frame parameters (check provided path in -qpfile <path>)\n");
        case READER_ERR_INCORRECT_FILE:
            return std::string("incorrect file with frame parameters\n");
        case READER_ERR_CODEC_UNSUPPORTED:
            return std::string("codecs, except h264 and h265, are not supported\n");
        default:
            return std::string();
    }
}
inline mfxU32 ReadDisplayOrder(const std::string& line) {
    return std::stoi(line.substr(0, find_nth(line, 0, ",", 0)));
}
inline mfxU16 ReadQP(const std::string& line) {
    size_t pos = find_nth(line, 0, ",", 0) + 1;
    return static_cast<mfxU16>(std::stoi(line.substr(pos, find_nth(line, 0, ",", 1) - pos)));
}
inline mfxU16 ReadFrameType(const std::string& line) {
    size_t pos = find_nth(line, 0, ",", 1) + 1;
    return StringToFrameType(line.substr(pos, line.length() - pos));
}
} // namespace QPFile

namespace TCBRCTestFile {

enum ReaderStatus {
    READER_ERR_NONE,
    READER_ERR_NOT_INITIALIZED,
    READER_ERR_CODEC_UNSUPPORTED,
    READER_ERR_FILE_NOT_OPEN,
    READER_ERR_INCORRECT_FILE
};

struct FrameInfo {
    mfxU32 displayOrder;
    mfxU32 targetFrameSize;
};

// TCBRCTestFile reads target frame size in display order
// from external text file (for encoding in Low delay BRC mode)
class Reader {
public:
    mfxStatus Read(const std::string& strFileName, mfxU32 codecid);
    void ResetState();

    mfxU32 GetTargetFrameSize(mfxU32 frameOrder) const;
    mfxU32 GetFramesNum() const;
    void NextFrame();
    std::string GetErrorMessage() const;

private:
    void ResetState(ReaderStatus set_sts);

    ReaderStatus m_ReaderSts = READER_ERR_NOT_INITIALIZED;
    mfxU32 m_CurFrameNum     = std::numeric_limits<mfxU32>::max();
    std::vector<FrameInfo> m_FrameVals{};
};

inline bool get_line(std::ifstream& ifs, std::string& line) {
    std::getline(ifs, line, '\n');
    if (!line.empty() && line.back() == '\r')
        line.pop_back();
    return !ifs.fail();
}
inline size_t find_nth(const std::string& str, size_t pos, const std::string& needle, mfxU32 nth) {
    size_t found_pos = str.find(needle, pos);
    for (; nth != 0 && std::string::npos != found_pos; --nth)
        found_pos = str.find(needle, found_pos + 1);
    return found_pos;
}
inline std::string ReaderStatusToString(ReaderStatus sts) {
    switch (sts) {
        case READER_ERR_NOT_INITIALIZED:
            return std::string(
                "reader not initialized (TCBRCTestfile has not yet read the file)\n");
        case READER_ERR_FILE_NOT_OPEN:
            return std::string(
                "failed to open file  with TargetFrameSize parameters (check provided path in -tcbrcfile <path>)\n");
        case READER_ERR_INCORRECT_FILE:
            return std::string("incorrect file with frame parameters\n");
        case READER_ERR_CODEC_UNSUPPORTED:
            return std::string("h264 and h265 are supported now\n");
        default:
            return std::string();
    }
}
inline mfxU32 ReadDisplayOrder(const std::string& line) {
    size_t pos = find_nth(line, 0, ":", 0);
    if (pos != std::string::npos)
        return std::stoi(line.substr(0, pos));
    else
        return 0;
}
inline mfxU16 ReadTargetFrameSize(const std::string& line) {
    size_t pos = find_nth(line, 0, ":", 0);
    pos        = (pos != std::string::npos) ? pos + 1 : 0;
    return static_cast<mfxU16>(std::stoi(line.substr(pos, line.size() - pos)));
}
} // namespace TCBRCTestFile

mfxStatus GetFrameLength(mfxU16 width, mfxU16 height, mfxU32 ColorFormat, mfxU32& length);

bool IsDecodeCodecSupported(mfxU32 codecFormat);
bool IsEncodeCodecSupported(mfxU32 codecFormat);
bool IsPluginCodecSupported(mfxU32 codecFormat);

// class is used as custom exception
class mfxError : public std::runtime_error {
public:
    mfxError(mfxStatus status = MFX_ERR_UNKNOWN, std::string msg = "")
            : runtime_error(msg),
              m_Status(status) {}

    mfxStatus GetStatus() const {
        return m_Status;
    }

private:
    mfxStatus m_Status;
};

//declare used extension buffers
template <class T>
struct mfx_ext_buffer_id {};

template <>
struct mfx_ext_buffer_id<mfxExtCodingOption> {
    enum { id = MFX_EXTBUFF_CODING_OPTION };
};
template <>
struct mfx_ext_buffer_id<mfxExtCodingOption2> {
    enum { id = MFX_EXTBUFF_CODING_OPTION2 };
};
template <>
struct mfx_ext_buffer_id<mfxExtCodingOption3> {
    enum { id = MFX_EXTBUFF_CODING_OPTION3 };
};
template <>
struct mfx_ext_buffer_id<mfxExtAvcTemporalLayers> {
    enum { id = MFX_EXTBUFF_AVC_TEMPORAL_LAYERS };
};
template <>
struct mfx_ext_buffer_id<mfxExtTemporalLayers> {
    enum { id = MFX_EXTBUFF_UNIVERSAL_TEMPORAL_LAYERS };
};
template <>
struct mfx_ext_buffer_id<mfxExtAVCRefListCtrl> {
    enum { id = MFX_EXTBUFF_AVC_REFLIST_CTRL };
};
template <>
struct mfx_ext_buffer_id<mfxExtThreadsParam> {
    enum { id = MFX_EXTBUFF_THREADS_PARAM };
};
template <>
struct mfx_ext_buffer_id<mfxExtHEVCRefLists> {
    enum { id = MFX_EXTBUFF_HEVC_REFLISTS };
};
template <>
struct mfx_ext_buffer_id<mfxExtBRC> {
    enum { id = MFX_EXTBUFF_BRC };
};
template <>
struct mfx_ext_buffer_id<mfxExtHEVCParam> {
    enum { id = MFX_EXTBUFF_HEVC_PARAM };
};
template <>
struct mfx_ext_buffer_id<mfxExtDecVideoProcessing> {
    enum { id = MFX_EXTBUFF_DEC_VIDEO_PROCESSING };
};
template <>
struct mfx_ext_buffer_id<mfxExtDecodeErrorReport> {
    enum { id = MFX_EXTBUFF_DECODE_ERROR_REPORT };
};
template <>
struct mfx_ext_buffer_id<mfxExtMVCSeqDesc> {
    enum { id = MFX_EXTBUFF_MVC_SEQ_DESC };
};
template <>
struct mfx_ext_buffer_id<mfxExtVPPDoNotUse> {
    enum { id = MFX_EXTBUFF_VPP_DONOTUSE };
};
template <>
struct mfx_ext_buffer_id<mfxExtVPPDoUse> {
    enum { id = MFX_EXTBUFF_VPP_DOUSE };
};
template <>
struct mfx_ext_buffer_id<mfxExtVPPDeinterlacing> {
    enum { id = MFX_EXTBUFF_VPP_DEINTERLACING };
};
template <>
struct mfx_ext_buffer_id<mfxExtCodingOptionSPSPPS> {
    enum { id = MFX_EXTBUFF_CODING_OPTION_SPSPPS };
};
template <>
struct mfx_ext_buffer_id<mfxExtVppMctf> {
    enum { id = MFX_EXTBUFF_VPP_MCTF };
};
template <>
struct mfx_ext_buffer_id<mfxExtVPPComposite> {
    enum { id = MFX_EXTBUFF_VPP_COMPOSITE };
};
template <>
struct mfx_ext_buffer_id<mfxExtVPPFieldProcessing> {
    enum { id = MFX_EXTBUFF_VPP_FIELD_PROCESSING };
};
template <>
struct mfx_ext_buffer_id<mfxExtVPPDetail> {
    enum { id = MFX_EXTBUFF_VPP_DETAIL };
};
template <>
struct mfx_ext_buffer_id<mfxExtVPPFrameRateConversion> {
    enum { id = MFX_EXTBUFF_VPP_FRAME_RATE_CONVERSION };
};
template <>
struct mfx_ext_buffer_id<mfxExtHEVCTiles> {
    enum { id = MFX_EXTBUFF_HEVC_TILES };
};
template <>
struct mfx_ext_buffer_id<mfxExtVP9Param> {
    enum { id = MFX_EXTBUFF_VP9_PARAM };
};
template <>
struct mfx_ext_buffer_id<mfxExtAV1BitstreamParam> {
    enum { id = MFX_EXTBUFF_AV1_BITSTREAM_PARAM };
};
template <>
struct mfx_ext_buffer_id<mfxExtAV1ResolutionParam> {
    enum { id = MFX_EXTBUFF_AV1_RESOLUTION_PARAM };
};
template <>
struct mfx_ext_buffer_id<mfxExtAV1TileParam> {
    enum { id = MFX_EXTBUFF_AV1_TILE_PARAM };
};
template <>
struct mfx_ext_buffer_id<mfxExtVideoSignalInfo> {
    enum { id = MFX_EXTBUFF_VIDEO_SIGNAL_INFO };
};
template <>
struct mfx_ext_buffer_id<mfxExtHEVCRegion> {
    enum { id = MFX_EXTBUFF_HEVC_REGION };
};
template <>
struct mfx_ext_buffer_id<mfxExtAVCRoundingOffset> {
    enum { id = MFX_EXTBUFF_AVC_ROUNDING_OFFSET };
};
template <>
struct mfx_ext_buffer_id<mfxExtPartialBitstreamParam> {
    enum { id = MFX_EXTBUFF_PARTIAL_BITSTREAM_PARAM };
};
template <>
struct mfx_ext_buffer_id<mfxExtVPPDenoise> {
    enum { id = MFX_EXTBUFF_VPP_DENOISE };
};
template <>
struct mfx_ext_buffer_id<mfxExtVPPDenoise2> {
    enum { id = MFX_EXTBUFF_VPP_DENOISE2 };
};
template <>
struct mfx_ext_buffer_id<mfxExtVPPProcAmp> {
    enum { id = MFX_EXTBUFF_VPP_PROCAMP };
};
template <>
struct mfx_ext_buffer_id<mfxExtVPPImageStab> {
    enum { id = MFX_EXTBUFF_VPP_IMAGE_STABILIZATION };
};
template <>
struct mfx_ext_buffer_id<mfxExtVPPVideoSignalInfo> {
    enum { id = MFX_EXTBUFF_VPP_VIDEO_SIGNAL_INFO };
};
template <>
struct mfx_ext_buffer_id<mfxExtVPPMirroring> {
    enum { id = MFX_EXTBUFF_VPP_MIRRORING };
};
template <>
struct mfx_ext_buffer_id<mfxExtVPPColorFill> {
    enum { id = MFX_EXTBUFF_VPP_COLORFILL };
};
template <>
struct mfx_ext_buffer_id<mfxExtVPPRotation> {
    enum { id = MFX_EXTBUFF_VPP_ROTATION };
};
template <>
struct mfx_ext_buffer_id<mfxExtVPPScaling> {
    enum { id = MFX_EXTBUFF_VPP_SCALING };
};
template <>
struct mfx_ext_buffer_id<mfxExtColorConversion> {
    enum { id = MFX_EXTBUFF_VPP_COLOR_CONVERSION };
};
template <>
struct mfx_ext_buffer_id<mfxExtPredWeightTable> {
    enum { id = MFX_EXTBUFF_PRED_WEIGHT_TABLE };
};
template <>
struct mfx_ext_buffer_id<mfxExtHyperModeParam> {
    enum { id = MFX_EXTBUFF_HYPER_MODE_PARAM };
};
template <>
struct mfx_ext_buffer_id<mfxExtAllocationHints> {
    enum { id = MFX_EXTBUFF_ALLOCATION_HINTS };
};
template <>
struct mfx_ext_buffer_id<mfxExtVPP3DLut> {
    enum { id = MFX_EXTBUFF_VPP_3DLUT };
};

template <>
struct mfx_ext_buffer_id<mfxExtMasteringDisplayColourVolume> {
    enum { id = MFX_EXTBUFF_MASTERING_DISPLAY_COLOUR_VOLUME };
};
template <>
struct mfx_ext_buffer_id<mfxExtContentLightLevelInfo> {
    enum { id = MFX_EXTBUFF_CONTENT_LIGHT_LEVEL_INFO };
};

#ifdef ONEVPL_EXPERIMENTAL
template <>
struct mfx_ext_buffer_id<mfxExtVPPPercEncPrefilter> {
    enum { id = MFX_EXTBUFF_VPP_PERC_ENC_PREFILTER };
};
template <>
struct mfx_ext_buffer_id<mfxExtTuneEncodeQuality> {
    enum { id = MFX_EXTBUFF_TUNE_ENCODE_QUALITY };
};
#endif

constexpr uint16_t max_num_ext_buffers =
    63 * 2; // '*2' is for max estimation if all extBuffer were 'paired'

//helper function to initialize mfx ext buffer structure
template <class T>
void init_ext_buffer(T& ext_buffer) {
    memset(&ext_buffer, 0, sizeof(ext_buffer));
    reinterpret_cast<mfxExtBuffer*>(&ext_buffer)->BufferId = mfx_ext_buffer_id<T>::id;
    reinterpret_cast<mfxExtBuffer*>(&ext_buffer)->BufferSz = sizeof(ext_buffer);
}

template <typename T>
struct IsPairedMfxExtBuffer : std::false_type {};
template <>
struct IsPairedMfxExtBuffer<mfxExtAVCRefListCtrl> : std::true_type {};
template <>
struct IsPairedMfxExtBuffer<mfxExtAVCRoundingOffset> : std::true_type {};
template <>
struct IsPairedMfxExtBuffer<mfxExtPredWeightTable> : std::true_type {};
template <typename R>
struct ExtParamAccessor {
private:
    using mfxExtBufferDoublePtr = mfxExtBuffer**;

public:
    mfxU16& NumExtParam;
    mfxExtBufferDoublePtr& ExtParam;
    ExtParamAccessor(const R& r)
            : NumExtParam(const_cast<mfxU16&>(r.NumExtParam)),
              ExtParam(const_cast<mfxExtBufferDoublePtr&>(r.ExtParam)) {}
};

template <>
struct ExtParamAccessor<mfxFrameSurface1> {
private:
    using mfxExtBufferDoublePtr = mfxExtBuffer**;

public:
    mfxU16& NumExtParam;
    mfxExtBufferDoublePtr& ExtParam;
    ExtParamAccessor(const mfxFrameSurface1& r)
            : NumExtParam(const_cast<mfxU16&>(r.Data.NumExtParam)),
              ExtParam(const_cast<mfxExtBufferDoublePtr&>(r.Data.ExtParam)) {}
};

/** ExtBufHolder is an utility class which
 *  provide interface for mfxExtBuffer objects management in any mfx structure (e.g. mfxVideoParam)
 */
template <typename T>
class ExtBufHolder : public T {
public:
    ExtBufHolder() : T() {
        m_ext_buf.reserve(max_num_ext_buffers);
    }

    ~ExtBufHolder() // only buffers allocated by wrapper can be released
    {
        for (auto it = m_ext_buf.begin(); it != m_ext_buf.end(); it++) {
            delete[](mfxU8*)(*it);
        }
    }

    ExtBufHolder(const ExtBufHolder& ref) {
        m_ext_buf.reserve(max_num_ext_buffers);
        *this = ref; // call to operator=
    }

    ExtBufHolder& operator=(const ExtBufHolder& ref) {
        const T* src_base = &ref;
        this->operator=(*src_base);
        return *this;
    }

    ExtBufHolder(const T& ref) {
        *this = ref; // call to operator=
    }

    ExtBufHolder& operator=(const T& ref) {
        // copy content of main structure type T
        T* dst_base       = this;
        const T* src_base = &ref;
        *dst_base         = *src_base;

        //remove all existing extension buffers
        ClearBuffers();

        const auto ref_ = ExtParamAccessor<T>(ref);

        //reproduce list of extension buffers and copy its content
        for (size_t i = 0; i < ref_.NumExtParam; ++i) {
            const auto src_buf = ref_.ExtParam[i];
            if (!src_buf)
                throw mfxError(MFX_ERR_NULL_PTR, "Null pointer attached to source ExtParam");
            if (!IsCopyAllowed(src_buf->BufferId)) {
                auto msg =
                    "Deep copy of '" + Fourcc2Str(src_buf->BufferId) + "' extBuffer is not allowed";
                throw mfxError(MFX_ERR_UNDEFINED_BEHAVIOR, msg);
            }

            // 'false' below is because here we just copy extBuffer's one by one
            auto dst_buf = AddExtBuffer(src_buf->BufferId, src_buf->BufferSz, false);
            // copy buffer content w/o restoring its type
            memcpy((void*)dst_buf, (void*)src_buf, src_buf->BufferSz);
        }

        return *this;
    }

    ExtBufHolder(ExtBufHolder&&)            = default;
    ExtBufHolder& operator=(ExtBufHolder&&) = default;

    mfxExtBuffer* AddExtBuffer(mfxU32 id, mfxU32 size) {
        return AddExtBuffer(id, size, false);
    }

    // Always returns a valid pointer or throws an exception
    template <typename TB>
    TB* AddExtBuffer() {
        mfxExtBuffer* b =
            AddExtBuffer(mfx_ext_buffer_id<TB>::id, sizeof(TB), IsPairedMfxExtBuffer<TB>::value);
        return (TB*)b;
    }

    template <typename TB>
    void RemoveExtBuffer() {
        auto it = std::find_if(m_ext_buf.begin(),
                               m_ext_buf.end(),
                               CmpExtBufById(mfx_ext_buffer_id<TB>::id));
        if (it != m_ext_buf.end()) {
            delete[](mfxU8*)(*it);
            it = m_ext_buf.erase(it);

            if (IsPairedMfxExtBuffer<TB>::value) {
                if (it == m_ext_buf.end() || (*it)->BufferId != mfx_ext_buffer_id<TB>::id)
                    throw mfxError(MFX_ERR_NULL_PTR,
                                   "RemoveExtBuffer: ExtBuffer's parity has been broken");

                delete[](mfxU8*)(*it);
                m_ext_buf.erase(it);
            }

            RefreshBuffers();
        }
    }

    template <typename TB>
    TB* GetExtBuffer(uint32_t fieldId = 0) const {
        return (TB*)FindExtBuffer(mfx_ext_buffer_id<TB>::id, fieldId);
    }

    template <typename TB>
    operator TB*() {
        return (TB*)FindExtBuffer(mfx_ext_buffer_id<TB>::id, 0);
    }

    template <typename TB>
    operator TB*() const {
        return (TB*)FindExtBuffer(mfx_ext_buffer_id<TB>::id, 0);
    }

private:
    mfxExtBuffer* AddExtBuffer(mfxU32 id, mfxU32 size, bool isPairedExtBuffer) {
        if (!size || !id)
            throw mfxError(MFX_ERR_NULL_PTR, "AddExtBuffer: wrong size or id!");

        auto it = std::find_if(m_ext_buf.begin(), m_ext_buf.end(), CmpExtBufById(id));
        if (it == m_ext_buf.end()) {
            auto buf = (mfxExtBuffer*)new mfxU8[size];
            memset(buf, 0, size);
            m_ext_buf.push_back(buf);

            buf->BufferId = id;
            buf->BufferSz = size;

            if (isPairedExtBuffer) {
                // Allocate the other mfxExtBuffer _right_after_ the first one ...
                buf = (mfxExtBuffer*)new mfxU8[size];
                memset(buf, 0, size);
                m_ext_buf.push_back(buf);

                buf->BufferId = id;
                buf->BufferSz = size;

                RefreshBuffers();
                return m_ext_buf[m_ext_buf.size() - 2]; // ... and return a pointer to the first one
            }

            RefreshBuffers();
            return m_ext_buf.back();
        }

        return *it;
    }

    mfxExtBuffer* FindExtBuffer(mfxU32 id, uint32_t fieldId) const {
        auto it = std::find_if(m_ext_buf.begin(), m_ext_buf.end(), CmpExtBufById(id));
        if (fieldId && it != m_ext_buf.end()) {
            ++it;
            return it != m_ext_buf.end() ? *it : nullptr;
        }
        return it != m_ext_buf.end() ? *it : nullptr;
    }

    void RefreshBuffers() {
        auto this_        = ExtParamAccessor<T>(*this);
        this_.NumExtParam = static_cast<mfxU16>(m_ext_buf.size());
        this_.ExtParam    = this_.NumExtParam ? m_ext_buf.data() : nullptr;
    }

    void ClearBuffers() {
        if (m_ext_buf.size()) {
            for (auto it = m_ext_buf.begin(); it != m_ext_buf.end(); it++) {
                delete[](mfxU8*)(*it);
            }
            m_ext_buf.clear();
        }
        RefreshBuffers();
    }

    bool IsCopyAllowed(mfxU32 id) {
        static const mfxU32 allowed[] = {
            MFX_EXTBUFF_CODING_OPTION,       MFX_EXTBUFF_CODING_OPTION2,
            MFX_EXTBUFF_CODING_OPTION3,      MFX_EXTBUFF_BRC,
            MFX_EXTBUFF_HEVC_PARAM,          MFX_EXTBUFF_VP9_PARAM,
            MFX_EXTBUFF_AV1_BITSTREAM_PARAM, MFX_EXTBUFF_AV1_RESOLUTION_PARAM,
            MFX_EXTBUFF_AV1_TILE_PARAM,      MFX_EXTBUFF_DEC_VIDEO_PROCESSING,
            MFX_EXTBUFF_ALLOCATION_HINTS
        };

        auto it =
            std::find_if(std::begin(allowed), std::end(allowed), [&id](const mfxU32 allowed_id) {
                return allowed_id == id;
            });
        return it != std::end(allowed);
    }

    struct CmpExtBufById {
        mfxU32 id;

        CmpExtBufById(mfxU32 _id) : id(_id){};

        bool operator()(mfxExtBuffer* b) {
            return (b && b->BufferId == id);
        };
    };

    static std::string Fourcc2Str(mfxU32 fourcc) {
        std::string s;
        for (size_t i = 0; i < 4; i++) {
            s.push_back(*(i + (char*)&fourcc));
        }
        return s;
    }

    std::vector<mfxExtBuffer*> m_ext_buf;
};

using MfxVideoParamsWrapper = ExtBufHolder<mfxVideoParam>;
using mfxEncodeCtrlWrap     = ExtBufHolder<mfxEncodeCtrl>;
using mfxInitParamlWrap     = ExtBufHolder<mfxInitParam>;
using mfxFrameSurfaceWrap   = ExtBufHolder<mfxFrameSurface1>;

class mfxBitstreamWrapper : public ExtBufHolder<mfxBitstream> {
    typedef ExtBufHolder<mfxBitstream> base;

public:
    mfxBitstreamWrapper() : base(), m_data() {}

    mfxBitstreamWrapper(mfxU32 n_bytes) : base(), m_data() {
        Extend(n_bytes);
    }

    mfxBitstreamWrapper(const mfxBitstreamWrapper& bs_wrapper)
            : base(bs_wrapper),
              m_data(bs_wrapper.m_data) {
        Data = m_data.data();
    }

    mfxBitstreamWrapper& operator=(mfxBitstreamWrapper const& bs_wrapper) {
        mfxBitstreamWrapper tmp(bs_wrapper);

        *this = std::move(tmp);

        return *this;
    }

    mfxBitstreamWrapper(mfxBitstreamWrapper&& bs_wrapper)            = default;
    mfxBitstreamWrapper& operator=(mfxBitstreamWrapper&& bs_wrapper) = default;
    ~mfxBitstreamWrapper()                                           = default;

    void Extend(mfxU32 n_bytes) {
        if (MaxLength >= n_bytes)
            return;

        m_data.reserve(n_bytes);

        Data      = m_data.data();
        MaxLength = n_bytes;
    }

private:
    std::vector<mfxU8> m_data;
};

class CSmplYUVReader {
public:
    typedef std::list<std::string>::iterator ls_iterator;
    CSmplYUVReader();
    virtual ~CSmplYUVReader();

    virtual void Close();
    virtual mfxStatus Init(std::list<std::string> inputs,
                           mfxU32 ColorFormat,
                           bool shouldShiftP010 = false);
    virtual mfxStatus SkipNframesFromBeginning(mfxU16 w, mfxU16 h, mfxU32 viewId, mfxU32 nframes);
    virtual mfxStatus LoadNextFrame(mfxFrameSurface1* pSurface);
    virtual mfxStatus LoadNextFrame(mfxFrameSurface1* pSurface, int bytes_to_read, mfxU8* buf_read);
    virtual void Reset();
    mfxU32 m_ColorFormat; // color format of input YUV data, YUV420 or NV12

protected:
    std::vector<FILE*> m_files;

    bool shouldShift10BitsHigh;
    bool m_bInited;
};

class CSmplBitstreamWriter {
public:
    CSmplBitstreamWriter();
    virtual ~CSmplBitstreamWriter();

    virtual mfxStatus Init(const char* strFileName);
    virtual void ForceInitStatus(bool status);
    virtual mfxStatus WriteNextFrame(mfxBitstream* pMfxBitstream,
                                     bool isPrint         = true,
                                     bool isCompleteFrame = true);
    virtual mfxStatus WriteNextFrame(mfxBitstream* pMfxBitstream, mfxU32 targetID, mfxU32 frameNum);
    virtual mfxStatus Reset();
    virtual void Close();
    mfxU32 m_nProcessedFramesNum;
    bool m_bSkipWriting;

protected:
    CSmplBitstreamWriter(CSmplBitstreamWriter const&)                  = delete;
    const CSmplBitstreamWriter& operator=(CSmplBitstreamWriter const&) = delete;

    FILE* m_fSource;
    bool m_bInited;
    std::string m_sFile;
};

class CSmplYUVWriter {
public:
    CSmplYUVWriter();
    virtual ~CSmplYUVWriter();

    virtual void Close();
    virtual mfxStatus Init(const char* strFileName, const mfxU32 numViews);
    virtual mfxStatus Reset();
    virtual mfxStatus WriteNextFrame(mfxFrameSurface1* pSurface);
    virtual mfxStatus WriteNextFrameI420(mfxFrameSurface1* pSurface);

    void SetMultiView() {
        m_bIsMultiView = true;
    }

protected:
    CSmplYUVWriter(CSmplYUVWriter const&)                  = delete;
    const CSmplYUVWriter& operator=(CSmplYUVWriter const&) = delete;

    FILE *m_fDest, **m_fDestMVC;
    bool m_bInited, m_bIsMultiView;
    mfxU32 m_numCreatedFiles;
    std::string m_sFile;
    mfxU32 m_nViews;
};

class CSmplBitstreamReader {
public:
    CSmplBitstreamReader();
    virtual ~CSmplBitstreamReader();

    //resets position to file begin
    virtual void Reset();
    virtual void Close();
    virtual mfxStatus Init(const char* strFileName);
    virtual mfxStatus ReadNextFrame(mfxBitstream* pBS);

protected:
    CSmplBitstreamReader(CSmplBitstreamReader const&)                  = delete;
    const CSmplBitstreamReader& operator=(CSmplBitstreamReader const&) = delete;

    FILE* m_fSource;
    bool m_bInited;
};

class CH264FrameReader : public CSmplBitstreamReader {
public:
    CH264FrameReader();
    virtual ~CH264FrameReader();

    /** Free resources.*/
    virtual void Close();
    virtual mfxStatus Init(const char* strFileName);
    virtual mfxStatus ReadNextFrame(mfxBitstream* pBS);

private:
    mfxBitstream* m_processedBS;
    // input bit stream
    mfxBitstreamWrapper m_originalBS;

    mfxStatus PrepareNextFrame(mfxBitstream* in, mfxBitstream** out);

    // is stream ended
    bool m_isEndOfStream;

    std::unique_ptr<AbstractSplitter> m_pNALSplitter;
    FrameSplitterInfo* m_frame;
    mfxU8* m_plainBuffer;
    mfxU32 m_plainBufferSize;
    mfxBitstream m_outBS;
};

//provides output bistream with at least 1 frame, reports about error
class CJPEGFrameReader : public CSmplBitstreamReader {
    enum JPEGMarker { SOI = 0xD8FF, EOI = 0xD9FF };

public:
    virtual mfxStatus ReadNextFrame(mfxBitstream* pBS);

protected:
    mfxU32 FindMarker(mfxBitstream* pBS, mfxU32 startOffset, JPEGMarker marker);
};

//appends output bistream with exactly 1 frame, reports about error
class CIVFFrameReader : public CSmplBitstreamReader {
public:
    CIVFFrameReader();
    virtual void Reset();
    virtual mfxStatus Init(const char* strFileName);
    virtual mfxStatus ReadNextFrame(mfxBitstream* pBS);

protected:
    /*bytes 0-3    signature: 'DKIF'
    bytes 4-5    version (should be 0)
    bytes 6-7    length of header in bytes
    bytes 8-11   codec FourCC (e.g., 'VP80')
    bytes 12-13  width in pixels
    bytes 14-15  height in pixels
    bytes 16-19  frame rate
    bytes 20-23  time scale
    bytes 24-27  number of frames in file
    bytes 28-31  unused*/

    struct DKIFHrd {
        mfxU32 dkif;
        mfxU16 version;
        mfxU16 header_len;
        mfxU32 codec_FourCC;
        mfxU16 width;
        mfxU16 height;
        mfxU32 frame_rate;
        mfxU32 time_scale;
        mfxU32 num_frames;
        mfxU32 unused;
    } m_hdr;
    mfxStatus ReadHeader();
};

// writes bitstream to duplicate-file & supports joining
// (for ViewOutput encoder mode)
class CSmplBitstreamDuplicateWriter : public CSmplBitstreamWriter {
public:
    CSmplBitstreamDuplicateWriter();

    virtual mfxStatus InitDuplicate(const char* strFileName);
    virtual mfxStatus JoinDuplicate(CSmplBitstreamDuplicateWriter* pJoinee);
    virtual mfxStatus WriteNextFrame(mfxBitstream* pMfxBitstream, bool isPrint = true);
    virtual void Close();

protected:
    FILE* m_fSourceDuplicate;
    bool m_bJoined;

private:
    using CSmplBitstreamWriter::WriteNextFrame;
};

//timeinterval calculation helper

template <int tag = 0>
class CTimeInterval {
    static double g_Freq;
    double& m_start;
    double m_own; //reference to this if external counter not required
    //since QPC functions are quite slow it makes sense to optionally enable them
    bool m_bEnable;
    msdk_tick m_StartTick;

    CTimeInterval(const CTimeInterval&) {}
    CTimeInterval& operator=(const CTimeInterval&) {
        return *this;
    }

public:
    CTimeInterval(double& dRef, bool bEnable = true)
            : m_start(dRef),
              m_bEnable(bEnable),
              m_StartTick(0) {
        if (!m_bEnable)
            return;
        Initialize();
    }
    CTimeInterval(bool bEnable = true)
            : m_start(m_own),
              m_own(),
              m_bEnable(bEnable),
              m_StartTick(0) {
        if (!m_bEnable)
            return;
        Initialize();
    }

    //updates external value with current time
    double Commit() {
        if (!m_bEnable)
            return 0.0;

        if (0.0 != g_Freq) {
            m_start = MSDK_GET_TIME(msdk_time_get_tick(), m_StartTick, g_Freq);
        }
        return m_start;
    }
    //last comitted value
    double Last() {
        return m_start;
    }
    ~CTimeInterval() {
        Commit();
    }

private:
    void Initialize() {
        if (0.0 == g_Freq) {
            g_Freq = (double)msdk_time_get_frequency();
        }
        m_StartTick = msdk_time_get_tick();
    }
};

template <int tag>
double CTimeInterval<tag>::g_Freq = 0.0f;

/** Helper class to measure execution time of some code. Use this class
 * if you need manual measurements.
 *
 * Usage example:
 * {
 *   CTimer timer;
 *   msdk_tick summary_tick;
 *
 *   timer.Start()
 *   function_to_measure();
 *   summary_tick = timer.GetDelta();
 *   printf("Elapsed time 1: %f\n", timer.GetTime());
 *   ...
 *   if (condition) timer.Start();
     function_to_measure();
 *   if (condition) {
 *     summary_tick += timer.GetDelta();
 *     printf("Elapsed time 2: %f\n", timer.GetTime();
 *   }
 *   printf("Overall time: %f\n", CTimer::ConvertToSeconds(summary_tick);
 * }
 */
class CTimer {
public:
    CTimer() : start(0) {}
    static msdk_tick GetFrequency() {
        if (!frequency)
            frequency = msdk_time_get_frequency();
        return frequency;
    }
    static mfxF64 ConvertToSeconds(msdk_tick elapsed) {
        return MSDK_GET_TIME(elapsed, 0, GetFrequency());
    }

    inline void Start() {
        start = msdk_time_get_tick();
    }
    inline msdk_tick GetDelta() {
        return msdk_time_get_tick() - start;
    }
    inline mfxF64 GetTime() {
        return MSDK_GET_TIME(msdk_time_get_tick(), start, GetFrequency());
    }

protected:
    static msdk_tick frequency;
    msdk_tick start;

private:
    CTimer(const CTimer&);
    void operator=(const CTimer&);
};

/** Helper class to measure overall execution time of some code. Use this
 * class if you want to measure execution time of the repeatedly executed
 * code.
 *
 * Usage example 1:
 *
 * msdk_tick summary_tick = 0;
 *
 * void function() {
 *
 * {
 *   CAutoTimer timer(&summary_tick);
 *   ...
 * }
 *     ...
 * int main() {
 *   for (;condition;) {
 *     function();
 *   }
 *   printf("Elapsed time: %f\n", CTimer::ConvertToSeconds(summary_tick);
 *   return 0;
 * }
 *
 * Usage example 2:
 * {
 *   msdk_tick summary_tick = 0;
 *
 *   {
 *     CAutoTimer timer(&summary_tick);
 *
 *     for (;condition;) {
 *       ...
 *       {
 *         function_to_measure();
 *         timer.Sync();
 *         printf("Progress: %f\n", CTimer::ConvertToSeconds(summary_tick);
 *       }
 *       ...
 *     }
 *   }
 *   printf("Elapsed time: %f\n", CTimer::ConvertToSeconds(summary_tick);
 * }
 *
 */
class CAutoTimer {
public:
    CAutoTimer(msdk_tick& _elapsed) : elapsed(_elapsed), start(0) {
        elapsed = _elapsed;
        start   = msdk_time_get_tick();
    }
    ~CAutoTimer() {
        elapsed += msdk_time_get_tick() - start;
    }
    msdk_tick Sync() {
        msdk_tick cur = msdk_time_get_tick();
        elapsed += cur - start;
        start = cur;
        return elapsed;
    }

protected:
    msdk_tick& elapsed;
    msdk_tick start;

private:
    CAutoTimer(const CAutoTimer&);
    void operator=(const CAutoTimer&);
};

mfxStatus ConvertFrameRate(mfxF64 dFrameRate, mfxU32* pnFrameRateExtN, mfxU32* pnFrameRateExtD);
mfxF64 CalculateFrameRate(mfxU32 nFrameRateExtN, mfxU32 nFrameRateExtD);

template <class T>
mfxU16 GetFreeSurfaceIndex(T* pSurfacesPool, mfxU16 nPoolSize) {
    constexpr mfxU16 MSDK_INVALID_SURF_IDX = 0xffff;

    if (pSurfacesPool) {
        for (mfxU16 i = 0; i < nPoolSize; i++) {
            if (0 == pSurfacesPool[i].Data.Locked) {
                return i;
            }
        }
    }

    return MSDK_INVALID_SURF_IDX;
}

mfxU16 GetFreeSurface(mfxFrameSurface1* pSurfacesPool, mfxU16 nPoolSize);

void FreeSurfacePool(mfxFrameSurface1* pSurfacesPool, mfxU16 nPoolSize);

mfxU16 CalculateDefaultBitrate(mfxU32 nCodecId,
                               mfxU32 nTargetUsage,
                               mfxU32 nWidth,
                               mfxU32 nHeight,
                               mfxF64 dFrameRate);

//serialization fnc set
std::string CodecIdToStr(mfxU32 nFourCC);
mfxU16 StrToTargetUsage(std::string strInput);
const char* TargetUsageToStr(mfxU16 tu);
const char* ColorFormatToStr(mfxU32 format);

// sets bitstream->PicStruct parsing first APP0 marker in bitstream
mfxStatus MJPEG_AVI_ParsePicStruct(mfxBitstream* bitstream);

// For MVC encoding/decoding purposes
std::string FormMVCFileName(const char* strFileName, const mfxU32 numView);

//piecewise linear function for bitrate approximation
class PartiallyLinearFNC {
    mfxF64* m_pX;
    mfxF64* m_pY;
    mfxU32 m_nPoints;
    mfxU32 m_nAllocated;

public:
    PartiallyLinearFNC();
    ~PartiallyLinearFNC();

    void AddPair(mfxF64 x, mfxF64 y);
    mfxF64 at(mfxF64);

private:
    DISALLOW_COPY_AND_ASSIGN(PartiallyLinearFNC);
};

// function for getting a pointer to a specific external buffer from the array
mfxExtBuffer* GetExtBuffer(mfxExtBuffer** ebuffers, mfxU32 nbuffers, mfxU32 BufferId);

// returns false if buf length is insufficient, otherwise
// skips step bytes in buf with specified length and returns true
template <typename Buf_t, typename Length_t>
bool skip(const Buf_t*& buf, Length_t& length, Length_t step) {
    if (length < step)
        return false;

    buf += step;
    length -= step;

    return true;
}

//do not link MediaSDK dispatched if class not used
struct MSDKAdapter {
    // returns the number of adapter associated with MSDK session, 0 for SW session
    static mfxU32 GetNumber(VPLImplementationLoader* loader) {
        mfxU32 adapterNum = 0; // default

        // get from lib, which was found by loader and will be used for creating session
        if (loader) {
            adapterNum = loader->GetDeviceIDAndAdapter().second;
        }

        return adapterNum;
    }

    // returns the number of adapter associated with MSDK session, 0 for SW session
    static mfxU32 GetNumber(mfxSession session, mfxIMPL implVia = 0) {
        mfxU32 adapterNum = 0; // default
        mfxIMPL impl      = MFX_IMPL_SOFTWARE; // default in case no HW IMPL is found

        // we don't care for error codes in further code; if something goes wrong we fall back to the default adapter
        if (session) {
            MFXQueryIMPL(session, &impl);
        }
        else {
            // an auxiliary session, internal for this function
            mfxSession auxSession;
            memset(&auxSession, 0, sizeof(auxSession));

            mfxVersion ver = { { 1, 1 } }; // minimum API version which supports multiple devices
            MFXInit(MFX_IMPL_HARDWARE_ANY | implVia, &ver, &auxSession);
            MFXQueryIMPL(auxSession, &impl);
            MFXClose(auxSession);
        }

        // extract the base implementation type
        mfxIMPL baseImpl = MFX_IMPL_BASETYPE(impl);

        const struct {
            // actual implementation
            mfxIMPL impl;
            // adapter's number
            mfxU32 adapterID;

        } implTypes[] = { { MFX_IMPL_HARDWARE, 0 },
                          { MFX_IMPL_SOFTWARE, 0 },
                          { MFX_IMPL_HARDWARE2, 1 },
                          { MFX_IMPL_HARDWARE3, 2 },
                          { MFX_IMPL_HARDWARE4, 3 } };

        // get corresponding adapter number
        for (mfxU8 i = 0; i < sizeof(implTypes) / sizeof(*implTypes); i++) {
            if (implTypes[i].impl == baseImpl) {
                adapterNum = implTypes[i].adapterID;
                break;
            }
        }

        return adapterNum;
    }
};

struct APIChangeFeatures {
    bool JpegDecode;
    bool JpegEncode;
    bool MVCDecode;
    bool MVCEncode;
    bool IntraRefresh;
    bool LowLatency;
    bool ViewOutput;
    bool LookAheadBRC;
    bool AudioDecode;
    bool SupportCodecPluginAPI;
};

inline mfxU32 MakeVersion(const mfxU16 major, const mfxU16 minor) {
    return major * 1000 + minor;
}

inline mfxU32 MakeVersion(const mfxVersion version) {
    return MakeVersion(version.Major, version.Minor);
}

inline bool operator<(const mfxVersion& l, const mfxVersion& r) {
    return MakeVersion(l) < MakeVersion(r);
}

mfxVersion getMinimalRequiredVersion(const APIChangeFeatures& features);

enum msdkAPIFeature {
    MSDK_FEATURE_NONE,
    MSDK_FEATURE_MVC,
    MSDK_FEATURE_JPEG_DECODE,
    MSDK_FEATURE_LOW_LATENCY,
    MSDK_FEATURE_MVC_VIEWOUTPUT,
    MSDK_FEATURE_JPEG_ENCODE,
    MSDK_FEATURE_LOOK_AHEAD,
    MSDK_FEATURE_PLUGIN_API
};

/* Returns true if feature is supported in the given API version */
bool CheckVersion(mfxVersion* version, msdkAPIFeature feature);

void ConfigureAspectRatioConversion(mfxInfoVPP* pVppInfo);

void SEICalcSizeType(std::vector<mfxU8>& data, mfxU16 type, mfxU32 size);

mfxU8 Char2Hex(char ch);

enum MsdkTraceLevel {
    MSDK_TRACE_LEVEL_SILENT   = -1,
    MSDK_TRACE_LEVEL_CRITICAL = 0,
    MSDK_TRACE_LEVEL_ERROR    = 1,
    MSDK_TRACE_LEVEL_WARNING  = 2,
    MSDK_TRACE_LEVEL_INFO     = 3,
    MSDK_TRACE_LEVEL_DEBUG    = 4,
};

std::string NoFullPath(const std::string&);
int msdk_trace_get_level();
void msdk_trace_set_level(int);
bool msdk_trace_is_printable(int);

std::ostream& operator<<(std::ostream& os, MsdkTraceLevel tt);

template <typename T>
mfxStatus msdk_opt_read(const char* string, T& value);

template <size_t S>
mfxStatus msdk_opt_read(const char* string, char (&value)[S]) {
    value[0] = 0;
#if defined(_WIN32) || defined(_WIN64)
    value[S - 1] = 0;
    return (0 == strncpy_s(value, string, S - 1)) ? MFX_ERR_NONE : MFX_ERR_UNKNOWN;
#else
    if (strlen(string) < S) {
        strncpy(value, string, S - 1);
        value[S - 1] = 0;
        return MFX_ERR_NONE;
    }
    return MFX_ERR_UNKNOWN;
#endif
}

template <typename T>
inline mfxStatus msdk_opt_read(const std::string& string, T& value) {
    return msdk_opt_read(string.c_str(), value);
}

mfxStatus StrFormatToCodecFormatFourCC(char* strInput, mfxU32& codecFormat);
const char* StatusToString(mfxStatus sts);
mfxI32 getMonitorType(char* str);

void WaitForDeviceToBecomeFree(MFXVideoSession& session,
                               mfxSyncPoint& syncPoint,
                               mfxStatus& currentStatus);

mfxU16 FourCCToChroma(mfxU32 fourCC);

class FPSLimiter {
public:
    FPSLimiter()  = default;
    ~FPSLimiter() = default;
    void Reset(mfxU32 fps) {
        m_delayTicks = fps ? msdk_time_get_frequency() / fps : 0;
    }
    void Work() {
        msdk_tick current_tick = msdk_time_get_tick();
        while (m_delayTicks && (m_startTick + m_delayTicks > current_tick)) {
            msdk_tick left_tick = m_startTick + m_delayTicks - current_tick;
            uint32_t sleepTime  = (uint32_t)(left_tick * 1000 / msdk_time_get_frequency());
            MSDK_SLEEP(sleepTime);
            current_tick = msdk_time_get_tick();
        };
        m_startTick = msdk_time_get_tick();
    }

protected:
    msdk_tick m_startTick  = 0;
    msdk_tick m_delayTicks = 0;
};

#if defined(_WIN32) || defined(_WIN64)
mfxStatus PrintLoadedModules();
#else
int PrintLibMFXPath(struct dl_phdr_info* info, size_t size, void* data);
#endif

class SurfaceUtilizationSynchronizer {
private:
    std::mutex m_mutexWait;
    std::mutex m_mutexSurface;
    std::condition_variable m_cv;
    std::atomic<bool> m_wait;
    std::vector<mfxFrameSurface1*> m_Surfaces;
    const mfxU16 m_NumFrameForAlloc;

    bool QueryFree() {
        std::lock_guard<std::mutex> lock(m_mutexSurface);
        if (m_Surfaces.size() < m_NumFrameForAlloc) {
            return true;
        }
        else {
            for (std::vector<mfxFrameSurface1*>::iterator it = m_Surfaces.begin();
                 it != m_Surfaces.end();
                 it++) {
                mfxU32 refCount = -1;
                mfxStatus sts   = (*it)->FrameInterface->GetRefCounter((*it), &refCount);
                // 2 means that only library and synchronizer have control over surface
                // and all components don't use it now
                if (sts == MFX_ERR_NONE && refCount <= 2 && (*it)->Data.Locked == 0) {
                    (*it)->FrameInterface->Release((*it));
                    m_Surfaces.erase(it);
                    return true;
                }
            }
        }

        return false;
    }

public:
    SurfaceUtilizationSynchronizer(mfxU16 numFrameForAlloc)
            : m_wait(false),
              m_NumFrameForAlloc(numFrameForAlloc) {}

    mfxStatus PushSurface(mfxFrameSurface1* surface) {
        std::lock_guard<std::mutex> lock(m_mutexSurface);
        mfxStatus sts;
        if (!surface || !surface->FrameInterface || !surface->FrameInterface->AddRef) {
            return MFX_ERR_UNKNOWN;
        }
        sts = surface->FrameInterface->AddRef(surface);
        if (sts == MFX_ERR_NONE) {
            m_Surfaces.push_back(surface);
        }
        return sts;
    }

    void StartWaitingForFree() {
        std::unique_lock<std::mutex> lock(m_mutexWait);

        if (!QueryFree()) {
            m_wait = true;
            m_cv.wait(lock, [&] {
                return !m_wait;
            });
        }
    }

    mfxU16 GetNumFrameForAlloc() const {
        return m_NumFrameForAlloc;
    }

    void NotifyFreeCome() {
        std::lock_guard<std::mutex> lock(m_mutexWait);

        if (!m_wait) {
            return;
        }

        if (QueryFree()) {
            m_wait = false;
            m_cv.notify_all();
        }
    }
};

class CIVFFrameWriter : public CSmplBitstreamWriter {
public:
    CIVFFrameWriter();

    virtual mfxStatus Reset();
    virtual mfxStatus Init(const char* strFileName,
                           const mfxU16 w,
                           const mfxU16 h,
                           const mfxU32 fr_nom,
                           const mfxU32 fr_denom);
    virtual mfxStatus WriteNextFrame(mfxBitstream* pMfxBitstream, bool isPrint = true);
    virtual void Close();
    mfxU64 GetProcessedFrame() {
        return m_frameNum;
    }

protected:
    /* 32 bytes for stream header
    bytes 0-3    signature: 'DKIF'
    bytes 4-5    version (should be 0)
    bytes 6-7    length of header in bytes
    bytes 8-11   codec FourCC (e.g., 'VP80')
    bytes 12-13  width in pixels
    bytes 14-15  height in pixels
    bytes 16-19  frame rate
    bytes 20-23  time scale
    bytes 24-27  number of frames in file
    bytes 28-31  unused
    */
    struct streamHeader {
        mfxU32 dkif;
        mfxU16 version;
        mfxU16 header_len;
        mfxU32 codec_FourCC;
        mfxU16 width;
        mfxU16 height;
        mfxU32 frame_rate;
        mfxU32 time_scale;
        mfxU32 num_frames;
        mfxU32 unused;
    } m_streamHeader;

    /* 12 bytes for frame header
    bytes 0-3   frame size
    bytes 4-11  pts
    */
    struct frameHeader {
        mfxU32 frame_size;
        mfxU32 pts_high;
        mfxU32 pts_low;
    } m_frameHeader;

    mfxU64 m_frameNum;
    mfxStatus WriteStreamHeader();
    mfxStatus WriteFrameHeader();
    void UpdateNumberOfFrames();

private:
    using CSmplBitstreamWriter::Init;
    using CSmplBitstreamWriter::WriteNextFrame;
};

class CBitstreamWriterForParallelEncoding : public CSmplBitstreamWriter {
public:
    virtual mfxStatus WriteNextFrame(mfxBitstream* pMfxBitstream, mfxU32 targetID, mfxU32 frameNum);
    virtual mfxStatus Reset();

    mfxU32 m_GopSize          = 0;
    mfxU32 m_NumberOfEncoders = 0;
    mfxU32 m_BaseEncoderID    = 0;
    bool m_WriteBsToStdout    = false;

private:
    int m_LastWrittenFrameNumber = -1;
    std::mutex m_Mutex{};
    std::map<mfxU32, std::vector<mfxU8>> m_Buffer{}; //key is targetID
    std::map<mfxU32, mfxU32> m_FirstFrameInBuffer{}; //key is target ID
    std::map<mfxU32, mfxU32> m_LastFrameInBuffer{}; //key is target ID
};

mfxStatus SetParameters(mfxSession session, MfxVideoParamsWrapper& par, const std::string& params);

#endif //__SAMPLE_UTILS_H__
