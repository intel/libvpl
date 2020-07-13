/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "./cpu_workstream.h"

#ifdef ENABLE_VPP

vplStatus CpuWorkstream::InitVPP(vplm_mem *vm) {
    vplm_image_info in;
    vplm_get_image_info(vm, &in);

    // configure VplInfo for input frames
    m_vppFrameInfoIn.Width  = (uint16_t)in.width;
    m_vppFrameInfoIn.Height = (uint16_t)in.height;

    ConvertFourCC(in.format, m_vppFrameInfoIn.FourCC);
    m_vppFrameInfoIn.NumPlanes =
        (uint32_t)CpuWorkstream::GetImagePlanesNum(m_vppFrameInfoIn.FourCC);

    // default: output = input
    // then update any parameters based on VPL config properties
    m_vppFrameInfoOut = m_vppFrameInfoIn;

    // rescaling
    vplm_variant outRes = properties_[VPL_PROP_OUTPUT_RESOLUTION];
    if ((outRes.size.w > 0) && (outRes.size.h > 0)) {
        m_vppFrameInfoOut.Width  = (uint16_t)outRes.size.w;
        m_vppFrameInfoOut.Height = (uint16_t)outRes.size.h;
    }
    else if ((outRes.size.w != 0) || (outRes.size.h != 0)) {
        return VPL_ERR_INVALID_SIZE;
    }

    // CSC
    VplFourCC outFourCC = (VplFourCC)properties_[VPL_PROP_DST_RAW_FORMAT].int32;
    if (outFourCC)
        m_vppFrameInfoOut.FourCC = outFourCC;

    m_vppFrameInfoOut.NumPlanes =
        (uint32_t)CpuWorkstream::GetImagePlanesNum(m_vppFrameInfoOut.FourCC);

    // check if we need to enable VPP
    m_vppBypass = true;
    if (m_vppFrameInfoOut.Width != m_vppFrameInfoIn.Width ||
        m_vppFrameInfoOut.Height != m_vppFrameInfoIn.Height ||
        m_vppFrameInfoOut.FourCC != m_vppFrameInfoIn.FourCC) {
        m_vppBypass = false;
    }

    if (m_vppBypass == false) {
        int iret = 0;
        AVPixelFormat avInFourCC, avOutFourCC;

        ConvertFourCC(m_vppFrameInfoIn.FourCC, avInFourCC);
        ConvertFourCC(m_vppFrameInfoOut.FourCC, avOutFourCC);

        // init SW rescalar + CSC
        m_avVppContext = sws_getContext(in.width,
                                        in.height,
                                        avInFourCC,
                                        m_vppFrameInfoOut.Width,
                                        m_vppFrameInfoOut.Height,
                                        avOutFourCC,
                                        SWS_BILINEAR,
                                        NULL,
                                        NULL,
                                        NULL);

        if (!m_avVppContext)
            return VPL_ERR_INTERNAL_ERROR;

        // skip when m_avDecFrameOut is created from InitDecode(), in case of decodevpp_fused
        if (m_avDecFrameOut) {
            m_avVppFrameIn =
                m_avDecFrameOut; // this will go away if we remove Fused ops
        }
        else {
            m_avVppFrameIn = av_frame_alloc();
            if (!m_avVppFrameIn)
                return VPL_ERR_OUT_OF_RESOURCES;

            m_avVppFrameIn->width  = m_vppFrameInfoIn.Width;
            m_avVppFrameIn->height = m_vppFrameInfoIn.Height;
            m_avVppFrameIn->format = avInFourCC;

            iret = av_image_alloc(m_avVppFrameIn->data,
                                  m_avVppFrameIn->linesize,
                                  m_avVppFrameIn->width,
                                  m_avVppFrameIn->height,
                                  avInFourCC,
                                  16);

            if (iret < 0)
                return VPL_ERR_INTERNAL_ERROR;
        }

        m_avVppFrameOut = av_frame_alloc();
        if (!m_avVppFrameOut)
            return VPL_ERR_OUT_OF_RESOURCES;

        iret = av_image_alloc(m_avVppFrameOut->data,
                              m_avVppFrameOut->linesize,
                              m_vppFrameInfoOut.Width,
                              m_vppFrameInfoOut.Height,
                              avOutFourCC,
                              16);
        if (iret < 0)
            return VPL_ERR_INTERNAL_ERROR;
    }
    return VPL_OK;
}

void CpuWorkstream::FreeVPP() {
    if (m_avVppContext)
        sws_freeContext(m_avVppContext);
}

mfxStatus CpuWorkstream::ProcessFrame(vplm_mem *mem) {
    if (!mem)
        ERR_EXIT(VPL_WORKSTREAM_VIDEOPROC);

    if (!m_vppInit) {
        vplStatus stat = InitVPP(mem);
        if (stat != VPL_OK)
            ERR_EXIT(VPL_WORKSTREAM_VIDEOPROC);

        m_vppInit = true;
    }

    // map input vplm_mem object to a vplm_cpu_image
    vplm_cpu_image vppCpuImageIn  = { 0 };
    vplm_cpu_image vppCpuImageOut = { 0 };

    vplm_status status =
        vplm_map_image(mem, VPLM_ACCESS_MODE_READWRITE, &vppCpuImageIn);

    if (status != VPLM_SUCCESS)
        ERR_EXIT(VPL_WORKSTREAM_VIDEOPROC);

    // set vplm cpu image for vpl memory creation
    vppCpuImageOut.info.width  = m_vppFrameInfoOut.Width;
    vppCpuImageOut.info.height = m_vppFrameInfoOut.Height;
    vppCpuImageOut.num_planes  = m_vppFrameInfoOut.NumPlanes;
    ConvertFourCC(m_vppFrameInfoOut.FourCC, vppCpuImageOut.info.format);

    if (m_vppBypass == true) {
        // no processing required - set output = input
        for (size_t i = 0; i < vppCpuImageOut.num_planes; i++) {
            vppCpuImageOut.planes[i].data   = vppCpuImageIn.planes[i].data;
            vppCpuImageOut.planes[i].stride = vppCpuImageIn.planes[i].stride;
        }
    }
    else {
        // map pointers from input VPLM frame to AVFrame
        for (size_t i = 0; i < vppCpuImageIn.num_planes; i++) {
            m_avVppFrameIn->data[i]     = vppCpuImageIn.planes[i].data;
            m_avVppFrameIn->linesize[i] = vppCpuImageIn.planes[i].stride;
        }

        // rescale AVFrame
        sws_scale(m_avVppContext,
                  m_avVppFrameIn->data,
                  m_avVppFrameIn->linesize,
                  0,
                  m_avVppFrameIn->height,
                  m_avVppFrameOut->data,
                  m_avVppFrameOut->linesize);

        // map pointers from AVFrame to output VPLM frame
        for (size_t i = 0; i < vppCpuImageOut.num_planes; i++) {
            vppCpuImageOut.planes[i].data   = m_avVppFrameOut->data[i];
            vppCpuImageOut.planes[i].stride = m_avVppFrameOut->linesize[i];
        }
    }

    // unmap input image
    status = vplm_unmap_image(&vppCpuImageIn);
    if (status != VPLM_SUCCESS)
        ERR_EXIT(VPL_WORKSTREAM_VIDEOPROC);

    return MFX_ERR_NONE;
}

#else // ENABLE_VPP

mfxStatus CpuWorkstream::InitVPP(void) {
    return MFX_ERR_UNSUPPORTED;
}

void CpuWorkstream::FreeVPP(void) {
    return;
}

mfxStatus CpuWorkstream::ProcessFrame(void) {
    return MFX_ERR_UNSUPPORTED;
}

#endif // ENABLE_VPP
