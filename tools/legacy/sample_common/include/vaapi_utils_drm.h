/*############################################################################
  # Copyright (C) 2005 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef __VAAPI_UTILS_DRM_H__
#define __VAAPI_UTILS_DRM_H__

/* CTA‐861‐G:
 * EOTF        EOTF of stream
 * 0           Traditional gamma - SDR Luminance Range
 * 1           Traditional gamma - HDR Luminance Range
 * 2           SMPTE ST 2084 (PQ)
 * 3           Hybrid Log-Gamma (HLG) based on ITU-R BT.2100-0 (HLG)
 */
enum drm_metadata_eotf {
    DRM_METADATA_EOTF_TRADITIONAL_SDR = 0,
    DRM_METADATA_EOTF_TRADITIONAL_HDR,
    DRM_METADATA_EOTF_SMPTE_2084,
    DRM_METADATA_EOTF_HLG_BT2100,
    DRM_METADATA_EOTF_MAX
};

/* CTA-861-G:
 * Static_Metadata_Descriptor_ID   Metadata Descriptor
 * 0                               Static Metadata Type 1
 */
enum drm_static_metadata_id { DRM_STATIC_METADATA_TYPE1 = 0 };

/* Colorspace values as per CEA spec */
#define DRM_MODE_COLORIMETRY_DEFAULT 0

/* CEA 861 Normal Colorimetry options */
#define DRM_MODE_COLORIMETRY_NO_DATA        0
#define DRM_MODE_COLORIMETRY_SMPTE_170M_YCC 1
#define DRM_MODE_COLORIMETRY_BT709_YCC      2

/* CEA 861 Extended Colorimetry Options */
#define DRM_MODE_COLORIMETRY_XVYCC_601   3
#define DRM_MODE_COLORIMETRY_XVYCC_709   4
#define DRM_MODE_COLORIMETRY_SYCC_601    5
#define DRM_MODE_COLORIMETRY_OPYCC_601   6
#define DRM_MODE_COLORIMETRY_OPRGB       7
#define DRM_MODE_COLORIMETRY_BT2020_CYCC 8
#define DRM_MODE_COLORIMETRY_BT2020_RGB  9
#define DRM_MODE_COLORIMETRY_BT2020_YCC  10
/* Additional Colorimetry extension added as part of CTA 861.G */
#define DRM_MODE_COLORIMETRY_DCI_P3_RGB_D65     11
#define DRM_MODE_COLORIMETRY_DCI_P3_RGB_THEATER 12

#define EDID_BLOCK_LENGTH     128
#define EDID_CEA_EXT_ID       0x02
#define EDID_CEA_TAG_EXTENDED 0x7

/* CEA-861-G new EDID blocks for HDR */
#define EDID_CEA_TAG_COLORIMETRY          0x5
#define EDID_CEA_EXT_TAG_STATIC_METADATA  0x6
#define EDID_CEA_EXT_TAG_DYNAMIC_METADATA 0x7

#if defined(LIBVA_DRM_SUPPORT)

    #include <va/va_drm.h>
    #include <va/va_drmcommon.h>
    #include "vaapi_allocator.h"
    #include "vaapi_utils.h"

class drmRenderer;

class DRMLibVA : public CLibVA {
public:
    DRMLibVA(const std::string& devicePath = "", int type = MFX_LIBVA_DRM);
    virtual ~DRMLibVA(void);

    inline int getFD() {
        return m_fd;
    }

protected:
    int m_fd;
    MfxLoader::VA_DRMProxy m_vadrmlib;

private:
    DISALLOW_COPY_AND_ASSIGN(DRMLibVA);
};

    #if defined(DRM_LINUX_HDR_SUPPORT)
struct drmHdrMetaData {
    struct hdr_output_metadata data;
    uint32_t hdrBlobId;
};
    #endif

class drmRenderer : public vaapiAllocatorParams::Exporter {
public:
    drmRenderer(int fd, mfxI32 monitorType);
    virtual ~drmRenderer();

    virtual mfxStatus render(mfxFrameSurface1* pSurface);

    // vaapiAllocatorParams::Exporter methods
    virtual void* acquire(mfxMemId mid);
    virtual void release(mfxMemId mid, void* mem);

    static uint32_t getConnectorType(mfxI32 monitor_type);
    static const char* getConnectorName(uint32_t connector_type);

private:
    drmModeObjectPropertiesPtr getProperties(int fd, int objectId, int32_t objectTypeId);
    bool getConnector(drmModeRes* resource, uint32_t connector_type);
    bool setupConnection(drmModeRes* resource, drmModeConnector* connector);
    bool getPlane();
    bool getAllFormatsAndModifiers();

    bool getConnectorProperties(int fd, int connectorId);

    bool setMaster();
    void dropMaster();
    bool restore();

    bool drmDisplayHasHdr(const uint8_t* edid);
    const uint8_t* edidFindExtendedDataBlock(const uint8_t* edid,
                                             uint8_t* dataLen,
                                             uint32_t blockTag);
    const uint8_t* edidFindCeaExtensionBlock(const uint8_t* edid);
    uint32_t getConnectorPropertyId(const char* propNameToFind);
    uint32_t getConnectorPropertyValue(const char* propNameToFind);
    int drmSetColorSpace(bool enableBT2020);
    int drmSendHdrMetaData(mfxExtMasteringDisplayColourVolume* displayColor,
                           mfxExtContentLightLevelInfo* contentLight,
                           bool enableHDR);

    const MfxLoader::DRM_Proxy m_drmlib;
    const MfxLoader::DrmIntel_Proxy m_drmintellib;

    int m_fd;
    uint32_t m_connector_type;
    uint32_t m_connectorID;
    uint32_t m_encoderID;
    uint32_t m_crtcID;
    uint32_t m_crtcIndex;
    uint32_t m_planeID;
    drmModeModeInfo m_mode;
    drmModeCrtcPtr m_crtc;
    drmModeObjectPropertiesPtr m_connectorProperties;
    drmModeObjectPropertiesPtr m_crtcProperties;
    drm_intel_bufmgr* m_bufmgr;
    bool m_overlay_wrn;
    bool m_bSentHDR;
    bool m_bHdrSupport;
    #if defined(DRM_LINUX_HDR_SUPPORT)
    struct drmHdrMetaData m_hdrMetaData;
    #endif
    bool m_bRequiredTiled4;
    mfxFrameSurface1* m_pCurrentRenderTargetSurface;

private:
    DISALLOW_COPY_AND_ASSIGN(drmRenderer);
};

#endif // #if defined(LIBVA_DRM_SUPPORT)

#endif // #ifndef __VAAPI_UTILS_DRM_H__
