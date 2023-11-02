/*############################################################################
  # Copyright (C) 2005 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#if defined(LIBVA_DRM_SUPPORT) || defined(LIBVA_WAYLAND_SUPPORT)

    #include "vaapi_utils_drm.h"
    #include <fcntl.h>
    #include <sys/ioctl.h>
    #include "vaapi_allocator.h"

    #include <stdexcept>

    #include <drm_fourcc.h>
    #include "i915_drm.h"
    #include "vaapi_utils_drm.h"

constexpr mfxU32 MFX_DRI_MAX_NODES_NUM      = 16;
constexpr mfxU32 MFX_DRI_RENDER_START_INDEX = 128;
constexpr mfxU32 MFX_DRI_CARD_START_INDEX   = 0;
constexpr mfxU32 MFX_DRM_DRIVER_NAME_LEN    = 4;
const char* MFX_DRM_INTEL_DRIVER_NAME       = "i915";
const char* MFX_DRM_INTEL_DRIVER_XE_NAME    = "xe";
const char* MFX_DRI_PATH                    = "/dev/dri/";
const char* MFX_DRI_NODE_RENDER             = "renderD";
const char* MFX_DRI_NODE_CARD               = "card";

int get_drm_driver_name(int fd, char* name, int name_size) {
    drm_version_t version = {};
    version.name_len      = name_size;
    version.name          = name;
    return ioctl(fd, DRM_IOWR(0, drm_version), &version);
}

int open_first_intel_adapter(int type) {
    std::string adapterPath                      = MFX_DRI_PATH;
    char driverName[MFX_DRM_DRIVER_NAME_LEN + 1] = {};
    mfxU32 nodeIndex;

    switch (type) {
        case MFX_LIBVA_DRM:
        case MFX_LIBVA_AUTO:
            adapterPath += MFX_DRI_NODE_RENDER;
            nodeIndex = MFX_DRI_RENDER_START_INDEX;
            break;
        case MFX_LIBVA_DRM_MODESET:
            adapterPath += MFX_DRI_NODE_CARD;
            nodeIndex = MFX_DRI_CARD_START_INDEX;
            break;
        default:
            throw std::invalid_argument("Wrong libVA backend type");
    }

    for (mfxU32 i = 0; i < MFX_DRI_MAX_NODES_NUM; ++i) {
        std::string curAdapterPath = adapterPath + std::to_string(nodeIndex + i);

        int fd = open(curAdapterPath.c_str(), O_RDWR);
        if (fd < 0)
            continue;

        if (!get_drm_driver_name(fd, driverName, MFX_DRM_DRIVER_NAME_LEN) &&
            (msdk_match(driverName, MFX_DRM_INTEL_DRIVER_NAME) ||
             msdk_match(driverName, MFX_DRM_INTEL_DRIVER_XE_NAME))) {
            return fd;
        }
        close(fd);
    }

    return -1;
}

int open_intel_adapter(const std::string& devicePath, int type) {
    int fd = -1;

    if (devicePath.empty())
        return open_first_intel_adapter(type);

    switch (type) {
        case MFX_LIBVA_DRM:
        case MFX_LIBVA_AUTO:
            fd = open(devicePath.c_str(), O_RDWR);
            break;
        case MFX_LIBVA_DRM_MODESET:
            // convert corresponsing render node to card node
            if (devicePath.find(MFX_DRI_NODE_RENDER) != std::string::npos) {
                std::string newDevicePath, renderNum;
                newDevicePath = renderNum = devicePath;

                newDevicePath.replace(
                    devicePath.find(MFX_DRI_NODE_RENDER),
                    devicePath.length(),
                    "card" + std::to_string(
                                 std::stoi(renderNum.erase(0,
                                                           devicePath.find(MFX_DRI_NODE_RENDER) +
                                                               sizeof(MFX_DRI_NODE_RENDER) - 1)) -
                                 128));

                fd = open(newDevicePath.c_str(), O_RDWR);
            }
            else if (devicePath.find(MFX_DRI_NODE_CARD) != std::string::npos) {
                fd = open(devicePath.c_str(), O_RDWR);
            }
            break;
        default:
            throw std::invalid_argument("Wrong libVA backend type");
    }

    if (fd < 0) {
        printf("Failed to open specified device\n");
        return -1;
    }

    char driverName[MFX_DRM_DRIVER_NAME_LEN + 1] = {};
    if (!get_drm_driver_name(fd, driverName, MFX_DRM_DRIVER_NAME_LEN) &&
        (msdk_match(driverName, MFX_DRM_INTEL_DRIVER_NAME) ||
         msdk_match(driverName, MFX_DRM_INTEL_DRIVER_XE_NAME))) {
        return fd;
    }
    else {
        close(fd);
        printf("Specified device is not Intel one\n");
        return -1;
    }
}

DRMLibVA::DRMLibVA(const std::string& devicePath, int type) : CLibVA(type), m_fd(-1) {
    mfxStatus sts = MFX_ERR_NONE;

    m_fd = open_intel_adapter(devicePath, type);
    if (m_fd < 0)
        throw std::range_error("Intel GPU was not found");

    m_va_dpy = m_vadrmlib.vaGetDisplayDRM(m_fd);
    if (m_va_dpy) {
        int major_version = 0, minor_version = 0;
        VAStatus va_res = m_libva.vaInitialize(m_va_dpy, &major_version, &minor_version);
        sts             = va_to_mfx_status(va_res);
    }
    else {
        sts = MFX_ERR_NULL_PTR;
    }

    if (MFX_ERR_NONE != sts) {
        if (m_va_dpy)
            m_libva.vaTerminate(m_va_dpy);
        close(m_fd);
        throw std::runtime_error("Loading of VA display was failed");
    }
}

DRMLibVA::~DRMLibVA(void) {
    if (m_va_dpy) {
        m_libva.vaTerminate(m_va_dpy);
    }
    if (m_fd >= 0) {
        close(m_fd);
    }
}

struct drmMonitorsTable {
    mfxI32 mfx_type;
    uint32_t drm_type;
    const char* type_name;
};

drmMonitorsTable g_drmMonitorsTable[] = {
    #define __DECLARE(type) \
        { MFX_MONITOR_##type, DRM_MODE_CONNECTOR_##type, #type }
    __DECLARE(Unknown),   __DECLARE(VGA),       __DECLARE(DVII),        __DECLARE(DVID),
    __DECLARE(DVIA),      __DECLARE(Composite), __DECLARE(SVIDEO),      __DECLARE(LVDS),
    __DECLARE(Component), __DECLARE(9PinDIN),   __DECLARE(HDMIA),       __DECLARE(HDMIB),
    __DECLARE(eDP),       __DECLARE(TV),        __DECLARE(DisplayPort),
    #if defined(DRM_MODE_CONNECTOR_VIRTUAL) // from libdrm 2.4.59
    __DECLARE(VIRTUAL),
    #endif
    #if defined(DRM_MODE_CONNECTOR_DSI) // from libdrm 2.4.59
    __DECLARE(DSI)
    #endif
    #undef __DECLARE
};

uint32_t drmRenderer::getConnectorType(mfxI32 monitor_type) {
    for (size_t i = 0; i < sizeof(g_drmMonitorsTable) / sizeof(g_drmMonitorsTable[0]); ++i) {
        if (g_drmMonitorsTable[i].mfx_type == monitor_type) {
            return g_drmMonitorsTable[i].drm_type;
        }
    }
    return DRM_MODE_CONNECTOR_Unknown;
}

const char* drmRenderer::getConnectorName(uint32_t connector_type) {
    for (size_t i = 0; i < sizeof(g_drmMonitorsTable) / sizeof(g_drmMonitorsTable[0]); ++i) {
        if (g_drmMonitorsTable[i].drm_type == connector_type) {
            return g_drmMonitorsTable[i].type_name;
        }
    }
    return "Unknown";
}

drmRenderer::drmRenderer(int fd, mfxI32 monitorType)
        : m_fd(fd),
          m_connector_type(),
          m_connectorID(),
          m_encoderID(),
          m_crtcID(),
          m_crtcIndex(),
          m_planeID(),
          m_mode(),
          m_crtc(),
          m_connectorProperties(),
          m_crtcProperties(),
          m_bufmgr(NULL),
          m_overlay_wrn(true),
          m_bSentHDR(false),
          m_bHdrSupport(false),
    #if defined(DRM_LINUX_HDR_SUPPORT)
          m_hdrMetaData({}),
    #endif
          m_bRequiredTiled4(false),
          m_pCurrentRenderTargetSurface(NULL) {
    bool res = false;

    if (m_drmlib.drmSetClientCap(m_fd, DRM_CLIENT_CAP_ATOMIC, 1) != 0)
        throw std::invalid_argument("Failed to set atomic");

    uint32_t connectorType = getConnectorType(monitorType);

    if (monitorType == MFX_MONITOR_AUTO) {
        connectorType = DRM_MODE_CONNECTOR_Unknown;
    }
    else if (connectorType == DRM_MODE_CONNECTOR_Unknown) {
        throw std::invalid_argument("Unsupported monitor type");
    }
    drmModeRes* resource = m_drmlib.drmModeGetResources(m_fd);
    if (resource) {
        if (getConnector(resource, connectorType) && getPlane()) {
            res = true;
        }
        m_drmlib.drmModeFreeResources(resource);
    }
    if (!res) {
        throw std::invalid_argument("Failed to allocate renderer");
    }
    printf("drmrender: connected via %s to %dx%d@%d capable display\n",
           getConnectorName(m_connector_type),
           m_mode.hdisplay,
           m_mode.vdisplay,
           m_mode.vrefresh);

    drmSetColorSpace(false);
    drmSendHdrMetaData(NULL, NULL, false);
}

drmRenderer::~drmRenderer() {
    m_drmlib.drmModeFreeCrtc(m_crtc);
    m_drmlib.drmModeFreeObjectProperties(m_connectorProperties);
    m_drmlib.drmModeFreeObjectProperties(m_crtcProperties);

    if (m_bufmgr) {
        m_drmintellib.drm_intel_bufmgr_destroy(m_bufmgr);
        m_bufmgr = NULL;
    }
}

drmModeObjectPropertiesPtr drmRenderer::getProperties(int fd, int objectId, int32_t objectTypeId) {
    return m_drmlib.drmModeObjectGetProperties(fd, objectId, objectTypeId);
}

bool drmRenderer::getConnectorProperties(int fd, int connectorId) {
    if (m_connectorProperties == NULL)
        m_connectorProperties = getProperties(fd, connectorId, DRM_MODE_OBJECT_CONNECTOR);
    return m_connectorProperties != NULL;
}

bool drmRenderer::getConnector(drmModeRes* resource, uint32_t connector_type) {
    bool found                    = false;
    drmModeConnectorPtr connector = NULL;

    for (int i = 0; i < resource->count_connectors; ++i) {
        connector = m_drmlib.drmModeGetConnector(m_fd, resource->connectors[i]);
        if (connector) {
            if ((connector->connector_type == connector_type) ||
                (connector_type == DRM_MODE_CONNECTOR_Unknown)) {
                if (connector->connection == DRM_MODE_CONNECTED) {
                    printf("drmrender: trying connection: %s\n",
                           getConnectorName(connector->connector_type));
                    m_connector_type = connector->connector_type;
                    m_connectorID    = connector->connector_id;
                    m_connectorProperties =
                        getProperties(m_fd, m_connectorID, DRM_MODE_OBJECT_CONNECTOR);
                    found = setupConnection(resource, connector);
                    if (found)
                        printf("drmrender: succeeded...\n");
                    else
                        printf("drmrender: failed...\n");
                }
                else if ((connector_type != DRM_MODE_CONNECTOR_Unknown)) {
                    printf("drmrender: error: requested monitor not connected\n");
                }
            }
            m_drmlib.drmModeFreeConnector(connector);
            if (found)
                return true;
        }
    }
    printf("drmrender: error: requested monitor not available\n");
    return found;
}

bool drmRenderer::drmDisplayHasHdr(const uint8_t* edid) {
    uint8_t dataLen = 0;
    const uint8_t* hdrDb;

    if (!edid) {
        printf("drmrender: invalid EDID\n");
        return false;
    }

    hdrDb = edidFindExtendedDataBlock(edid, &dataLen, EDID_CEA_EXT_TAG_STATIC_METADATA);
    if (hdrDb && dataLen >= 2) {
        return true;
    }

    return false;
}

const uint8_t* drmRenderer::edidFindExtendedDataBlock(const uint8_t* edid,
                                                      uint8_t* dataLen,
                                                      uint32_t blockTag) {
    uint8_t d;
    uint8_t tag;
    uint8_t extendedTag;
    uint8_t dbLen;
    const uint8_t* dbPtr;
    const uint8_t* ceaDbStart;
    const uint8_t* ceaDbEnd;
    const uint8_t* ceaExtBlk;

    if (!edid) {
        printf("drmrender: no EDID in blob\n");
        return NULL;
    }

    ceaExtBlk = edidFindCeaExtensionBlock(edid);
    if (!ceaExtBlk) {
        printf("drmrender: no CEA extension block available\n");
        return NULL;
    }

    /* CEA DB starts at blk[4] and ends at blk[d] */
    d          = ceaExtBlk[2];
    ceaDbStart = ceaExtBlk + 4;
    ceaDbEnd   = ceaExtBlk + d - 1;

    for (dbPtr = ceaDbStart; dbPtr < ceaDbEnd; dbPtr += (dbLen + 1)) {
        /* First data byte contains db length and tag */
        dbLen = dbPtr[0] & 0x1F;
        tag   = dbPtr[0] >> 5;

        /* Metadata bock is extended tag block */
        if (tag != EDID_CEA_TAG_EXTENDED)
            continue;

        /* Extended block uses one extra byte for extended tag */
        extendedTag = dbPtr[1];
        if (extendedTag != blockTag)
            continue;

        *dataLen = dbLen - 1;
        return dbPtr + 2;
    }

    return NULL;
}

const uint8_t* drmRenderer::edidFindCeaExtensionBlock(const uint8_t* edid) {
    uint8_t ext_blks;
    int blk;
    const uint8_t* ext = NULL;

    if (!edid) {
        printf("drmrender: no EDID\n");
        return NULL;
    }

    ext_blks = edid[126];
    if (!ext_blks) {
        printf("drmrender: EDID doesn't have any extension block\n");
        return NULL;
    }

    for (blk = 0; blk < ext_blks; blk++) {
        ext = edid + EDID_BLOCK_LENGTH * (blk + 1);
        if (ext[0] == EDID_CEA_EXT_ID)
            break;
    }

    if (blk == ext_blks)
        return NULL;

    return ext;
}

bool drmRenderer::setupConnection(drmModeRes* resource, drmModeConnector* connector) {
    bool ret = false;
    drmModeEncoderPtr encoder;
    uint64_t edidBlobId;
    drmModePropertyBlobRes* edidBlob;

    if (!connector->count_modes) {
        printf("drmrender: error: no valid modes for %s connector\n",
               getConnectorName(connector->connector_type));
        return false;
    }

    // we will use the first available mode - that's always mode with the highest resolution
    m_mode = connector->modes[0];

    edidBlobId = getConnectorPropertyValue("EDID");
    edidBlob   = m_drmlib.drmModeGetPropertyBlob(m_fd, edidBlobId);

    if (drmDisplayHasHdr(static_cast<uint8_t const*>(edidBlob->data)))
        m_bHdrSupport = true;

    m_drmlib.drmModeFreePropertyBlob(edidBlob);

    // trying encoder+crtc which are currently attached to connector
    m_encoderID = connector->encoder_id;
    encoder     = m_drmlib.drmModeGetEncoder(m_fd, m_encoderID);
    if (encoder) {
        m_crtcID         = encoder->crtc_id;
        m_crtcProperties = getProperties(m_fd, m_crtcID, DRM_MODE_OBJECT_CRTC);
        for (int j = 0; j < resource->count_crtcs; ++j) {
            if (m_crtcID == resource->crtcs[j]) {
                m_crtcIndex = j;
                break;
            }
        }
        ret = true;
        printf("drmrender: selected crtc already attached to connector\n");
        m_drmlib.drmModeFreeEncoder(encoder);
    }

    // if previous attempt to get crtc failed, let performs global search
    // searching matching encoder+crtc globally
    if (!ret) {
        for (int i = 0; i < connector->count_encoders; ++i) {
            encoder = m_drmlib.drmModeGetEncoder(m_fd, connector->encoders[i]);
            if (encoder) {
                for (int j = 0; j < resource->count_crtcs; ++j) {
                    // check whether this CRTC works with the encoder
                    if (!((encoder->possible_crtcs & (1 << j)) &&
                          (encoder->crtc_id == resource->crtcs[j])))
                        continue;

                    m_encoderID = connector->encoders[i];
                    m_crtcIndex = j;
                    m_crtcID    = resource->crtcs[j];
                    ret         = true;
                    printf("drmrender: found crtc with global search\n");
                    break;
                }
                m_drmlib.drmModeFreeEncoder(encoder);
                if (ret)
                    break;
            }
        }
    }
    if (ret) {
        m_crtc = m_drmlib.drmModeGetCrtc(m_fd, m_crtcID);
        if (!m_crtc)
            ret = false;
    }
    else {
        printf("drmrender: failed to select crtc\n");
    }
    return ret;
}

bool drmRenderer::getPlane() {
    drmModePlaneResPtr planes = m_drmlib.drmModeGetPlaneResources(m_fd);
    if (!planes) {
        return false;
    }
    for (uint32_t i = 0; i < planes->count_planes; ++i) {
        drmModePlanePtr plane = m_drmlib.drmModeGetPlane(m_fd, planes->planes[i]);
        if (plane) {
            if (plane->possible_crtcs & (1 << m_crtcIndex)) {
                for (uint32_t j = 0; j < plane->count_formats; ++j) {
                    if ((plane->formats[j] == DRM_FORMAT_XRGB8888) ||
    #if defined(DRM_LINUX_P010_SUPPORT)
                        (plane->formats[j] == DRM_FORMAT_P010) ||
    #endif
                        (plane->formats[j] == DRM_FORMAT_NV12)) {
                        m_planeID = plane->plane_id;

                        if (!getAllFormatsAndModifiers())
                            printf("drmrender: failed to obtain plane properties\n");

                        m_drmlib.drmModeFreePlane(plane);
                        m_drmlib.drmModeFreePlaneResources(planes);
                        return true;
                    }
                }
            }
            m_drmlib.drmModeFreePlane(plane);
        }
    }
    m_drmlib.drmModeFreePlaneResources(planes);
    return false;
}

bool drmRenderer::getAllFormatsAndModifiers() {
    drmModeObjectProperties* planeProps = NULL;
    drmModePropertyRes** planePropsInfo = NULL;
    drmModePropertyBlobPtr blob;
    #if defined(DRM_LINUX_FORMAT_MODIFIER_BLOB_SUPPORT)
    drmModeFormatModifierIterator iter = { 0 };
    #endif
    uint32_t i;

    planeProps = m_drmlib.drmModeObjectGetProperties(m_fd, m_planeID, DRM_MODE_OBJECT_PLANE);
    if (!planeProps)
        return false;

    planePropsInfo =
        (drmModePropertyRes**)malloc(planeProps->count_props * sizeof(drmModePropertyRes*));

    for (i = 0; i < planeProps->count_props; i++)
        planePropsInfo[i] = m_drmlib.drmModeGetProperty(m_fd, planeProps->props[i]);

    for (i = 0; i < planeProps->count_props; i++) {
        if (strcmp(planePropsInfo[i]->name, "IN_FORMATS"))
            continue;

        blob = m_drmlib.drmModeGetPropertyBlob(m_fd, planeProps->prop_values[i]);
        if (!blob)
            continue;

    #if defined(DRM_LINUX_FORMAT_MODIFIER_BLOB_SUPPORT)
        while (m_drmlib.drmModeFormatModifierBlobIterNext(blob, &iter)) {
            if (iter.mod == DRM_FORMAT_MOD_INVALID)
                break;

        #if defined(DRM_LINUX_MODIFIER_TILED4_SUPPORT)
            if ((iter.fmt == DRM_FORMAT_NV12 || iter.fmt == DRM_FORMAT_P010) &&
                (iter.mod == I915_FORMAT_MOD_4_TILED)) {
                m_bRequiredTiled4 = true;
                break;
            }
        #endif
        }
    #endif
        m_drmlib.drmModeFreePropertyBlob(blob);
    }

    if (planePropsInfo) {
        for (i = 0; i < planeProps->count_props; i++) {
            if (planePropsInfo[i])
                m_drmlib.drmModeFreeProperty(planePropsInfo[i]);
        }
        free(planePropsInfo);
    }

    m_drmlib.drmModeFreeObjectProperties(planeProps);
    return true;
}

bool drmRenderer::setMaster() {
    int wait_count = 0;
    do {
        if (!m_drmlib.drmSetMaster(m_fd))
            return true;
        usleep(100);
        ++wait_count;
    } while (wait_count < 30000);
    printf("drmrender: error: failed to get drm mastership during 3 seconds - aborting\n");
    return false;
}

void drmRenderer::dropMaster() {
    m_drmlib.drmDropMaster(m_fd);
}

bool drmRenderer::restore() {
    if (!setMaster())
        return false;

    int ret = m_drmlib.drmModeSetCrtc(m_fd,
                                      m_crtcID,
                                      m_crtc->buffer_id,
                                      m_crtc->x,
                                      m_crtc->y,
                                      &m_connectorID,
                                      1,
                                      &m_mode);
    if (ret) {
        printf("drmrender: failed to restore original mode\n");
        return false;
    }
    dropMaster();
    return true;
}

uint32_t drmRenderer::getConnectorPropertyId(const char* propNameToFind) {
    uint32_t id = 0;

    if (!getConnectorProperties(m_fd, m_connectorID)) {
        return id;
    }

    drmModePropertyPtr property;
    uint32_t i;

    for (i = 0; i < m_connectorProperties->count_props; i++) {
        property = m_drmlib.drmModeGetProperty(m_fd, m_connectorProperties->props[i]);
        if (!property)
            continue;
        if (msdk_match(property->name, propNameToFind))
            id = property->prop_id;

        m_drmlib.drmModeFreeProperty(property);
        property = NULL;

        if (id)
            break;
    }

    return id;
}

uint32_t drmRenderer::getConnectorPropertyValue(const char* propNameToFind) {
    uint32_t value = 0;

    if (!getConnectorProperties(m_fd, m_connectorID)) {
        return value;
    }

    drmModePropertyPtr property;
    uint32_t i;

    for (i = 0; i < m_connectorProperties->count_props; i++) {
        property = m_drmlib.drmModeGetProperty(m_fd, m_connectorProperties->props[i]);
        if (!property)
            continue;
        if (msdk_match(property->name, propNameToFind))
            value = m_connectorProperties->prop_values[i];

        m_drmlib.drmModeFreeProperty(property);
        property = NULL;

        if (value)
            break;
    }

    return value;
}

int drmRenderer::drmSetColorSpace(bool enableBT2020) {
    uint32_t property_colorspace_id = getConnectorPropertyId("Colorspace");

    if (!property_colorspace_id)
        return -1;

    int ret = m_drmlib.drmModeObjectSetProperty(
        m_fd,
        m_connectorID,
        DRM_MODE_OBJECT_CONNECTOR,
        property_colorspace_id,
        (enableBT2020) ? DRM_MODE_COLORIMETRY_BT2020_RGB : DRM_MODE_COLORIMETRY_DEFAULT);

    return ret;
}

int drmRenderer::drmSendHdrMetaData(mfxExtMasteringDisplayColourVolume* displayColor,
                                    mfxExtContentLightLevelInfo* contentLight,
                                    bool enableHDR) {
    #if defined(DRM_LINUX_HDR_SUPPORT)
    int ret;
    uint32_t propertyHdrId = getConnectorPropertyId("HDR_OUTPUT_METADATA");
    memset(&m_hdrMetaData.data.hdmi_metadata_type1,
           0,
           sizeof(m_hdrMetaData.data.hdmi_metadata_type1));

    if (!propertyHdrId)
        return -1;

    if (m_hdrMetaData.hdrBlobId) {
        m_drmlib.drmModeDestroyPropertyBlob(m_fd, m_hdrMetaData.hdrBlobId);
        m_hdrMetaData.hdrBlobId = 0;
    }

    drmModeAtomicReqPtr request = m_drmlib.drmModeAtomicAlloc();
    if (!request) {
        return -1;
    }

    if (enableHDR) {
        m_hdrMetaData.data.metadata_type            = DRM_STATIC_METADATA_TYPE1;
        m_hdrMetaData.data.hdmi_metadata_type1.eotf = DRM_METADATA_EOTF_SMPTE_2084;
        if (m_hdrMetaData.data.hdmi_metadata_type1.eotf) {
            if (displayColor->InsertPayloadToggle == MFX_PAYLOAD_IDR) {
                m_hdrMetaData.data.hdmi_metadata_type1.display_primaries[0].x =
                    displayColor->DisplayPrimariesX[0];
                m_hdrMetaData.data.hdmi_metadata_type1.display_primaries[0].y =
                    displayColor->DisplayPrimariesY[0];
                m_hdrMetaData.data.hdmi_metadata_type1.display_primaries[1].x =
                    displayColor->DisplayPrimariesX[1];
                m_hdrMetaData.data.hdmi_metadata_type1.display_primaries[1].y =
                    displayColor->DisplayPrimariesY[1];
                m_hdrMetaData.data.hdmi_metadata_type1.display_primaries[2].x =
                    displayColor->DisplayPrimariesX[2];
                m_hdrMetaData.data.hdmi_metadata_type1.display_primaries[2].y =
                    displayColor->DisplayPrimariesY[2];
                m_hdrMetaData.data.hdmi_metadata_type1.white_point.x = displayColor->WhitePointX;
                m_hdrMetaData.data.hdmi_metadata_type1.white_point.y = displayColor->WhitePointY;
                m_hdrMetaData.data.hdmi_metadata_type1.max_display_mastering_luminance =
                    displayColor->MaxDisplayMasteringLuminance / 10000;
                m_hdrMetaData.data.hdmi_metadata_type1.min_display_mastering_luminance =
                    displayColor->MinDisplayMasteringLuminance;
            }
            if (contentLight->InsertPayloadToggle == MFX_PAYLOAD_IDR) {
                m_hdrMetaData.data.hdmi_metadata_type1.max_cll = contentLight->MaxContentLightLevel;
                m_hdrMetaData.data.hdmi_metadata_type1.max_fall =
                    contentLight->MaxPicAverageLightLevel;
            }
        }
    }

    ret = m_drmlib.drmModeCreatePropertyBlob(m_fd,
                                             &m_hdrMetaData.data,
                                             sizeof(hdr_output_metadata),
                                             &m_hdrMetaData.hdrBlobId);
    if (ret)
        return -1;

    ret = m_drmlib.drmModeAtomicAddProperty(request,
                                            m_connectorID,
                                            propertyHdrId,
                                            m_hdrMetaData.hdrBlobId);
    if (ret < 0)
        return -1;

    ret = m_drmlib.drmModeAtomicCommit(m_fd, request, DRM_MODE_ATOMIC_ALLOW_MODESET, NULL);
    if (ret)
        return -1;

    m_drmlib.drmModeAtomicFree(request);
    #endif
    return 0;
}

void* drmRenderer::acquire(mfxMemId mid) {
    vaapiMemId* vmid  = (vaapiMemId*)mid;
    uint32_t fbhandle = 0;

    if (vmid->m_buffer_info.mem_type == VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME) {
        if (!m_bufmgr) {
            m_bufmgr = m_drmintellib.drm_intel_bufmgr_gem_init(m_fd, 4096);
            if (!m_bufmgr)
                return NULL;
        }

        drm_intel_bo* bo =
            m_drmintellib.drm_intel_bo_gem_create_from_prime(m_bufmgr,
                                                             (int)vmid->m_buffer_info.handle,
                                                             vmid->m_buffer_info.mem_size);
        if (!bo)
            return NULL;

        int ret = m_drmlib.drmModeAddFB(m_fd,
                                        vmid->m_image.width,
                                        vmid->m_image.height,
                                        24,
                                        32,
                                        vmid->m_image.pitches[0],
                                        bo->handle,
                                        &fbhandle);
        if (ret) {
            return NULL;
        }
        m_drmintellib.drm_intel_bo_unreference(bo);
    }
    else if (vmid->m_buffer_info.mem_type == VA_SURFACE_ATTRIB_MEM_TYPE_KERNEL_DRM) {
        struct drm_gem_open flink_open;
        struct drm_gem_close flink_close;

        MSDK_ZERO_MEMORY(flink_open);
        flink_open.name = vmid->m_buffer_info.handle;
        int ret         = m_drmlib.drmIoctl(m_fd, DRM_IOCTL_GEM_OPEN, &flink_open);
        if (ret)
            return NULL;

        uint32_t handles[4], pitches[4], offsets[4], pixel_format, flags = 0;
        uint64_t modifiers[4];

        memset(&handles, 0, sizeof(handles));
        memset(&pitches, 0, sizeof(pitches));
        memset(&offsets, 0, sizeof(offsets));
        memset(&modifiers, 0, sizeof(modifiers));

        handles[0] = flink_open.handle;
        pitches[0] = vmid->m_image.pitches[0];
        offsets[0] = vmid->m_image.offsets[0];

        if (VA_FOURCC_NV12 == vmid->m_fourcc
    #if defined(DRM_LINUX_P010_SUPPORT)
            || VA_FOURCC_P010 == vmid->m_fourcc
    #endif
        ) {
            pixel_format = DRM_FORMAT_NV12;
    #if defined(DRM_LINUX_P010_SUPPORT)
            if (VA_FOURCC_P010 == vmid->m_fourcc)
                pixel_format = DRM_FORMAT_P010;
    #endif
            handles[1]   = flink_open.handle;
            pitches[1]   = vmid->m_image.pitches[1];
            offsets[1]   = vmid->m_image.offsets[1];
            modifiers[0] = modifiers[1] = I915_FORMAT_MOD_Y_TILED;
            flags                       = DRM_MODE_FB_MODIFIERS;

            if (m_bRequiredTiled4) {
    #if defined(DRM_LINUX_MODIFIER_TILED4_SUPPORT)
                modifiers[0] = modifiers[1] = I915_FORMAT_MOD_4_TILED;
    #endif
            }
            else {
                modifiers[0] = modifiers[1] = I915_FORMAT_MOD_Y_TILED;

                struct drm_i915_gem_set_tiling set_tiling;
                memset(&set_tiling, 0, sizeof(set_tiling));
                set_tiling.handle      = flink_open.handle;
                set_tiling.tiling_mode = I915_TILING_Y;
                set_tiling.stride      = vmid->m_image.pitches[0];
                ret = m_drmlib.drmIoctl(m_fd, DRM_IOCTL_I915_GEM_SET_TILING, &set_tiling);
                if (ret) {
                    printf("DRM_IOCTL_I915_GEM_SET_TILING Failed ret = %d\n", ret);
                    return NULL;
                }
            }
        }
        else {
            pixel_format = DRM_FORMAT_XRGB8888;
        }

        ret = m_drmlib.drmModeAddFB2WithModifiers(m_fd,
                                                  vmid->m_image.width,
                                                  vmid->m_image.height,
                                                  pixel_format,
                                                  handles,
                                                  pitches,
                                                  offsets,
                                                  modifiers,
                                                  &fbhandle,
                                                  flags);
        if (ret)
            return NULL;

        MSDK_ZERO_MEMORY(flink_close);
        flink_close.handle = flink_open.handle;
        ret                = m_drmlib.drmIoctl(m_fd, DRM_IOCTL_GEM_CLOSE, &flink_close);
        if (ret)
            return NULL;
    }
    else {
        return NULL;
    }
    try {
        uint32_t* hdl = new uint32_t;
        *hdl          = fbhandle;
        return hdl;
    }
    catch (...) {
        return NULL;
    }
}

void drmRenderer::release(mfxMemId mid, void* mem) {
    uint32_t* hdl = (uint32_t*)mem;
    if (!hdl)
        return;
    if (!restore()) {
        printf(
            "drmrender: warning: failure to restore original mode may lead to application segfault!\n");
    }
    m_drmlib.drmModeRmFB(m_fd, *hdl);
    delete (hdl);
}

mfxStatus drmRenderer::render(mfxFrameSurface1* pSurface) {
    int ret;
    vaapiMemId* memid;
    uint32_t fbhandle;

    if (!pSurface || !pSurface->Data.MemId)
        return MFX_ERR_INVALID_HANDLE;
    memid = (vaapiMemId*)(pSurface->Data.MemId);
    if (!memid->m_custom)
        return MFX_ERR_INVALID_HANDLE;
    fbhandle = *(uint32_t*)memid->m_custom;

    // rendering on the screen
    if (!setMaster()) {
        return MFX_ERR_UNKNOWN;
    }
    if ((m_mode.hdisplay == memid->m_image.width) && (m_mode.vdisplay == memid->m_image.height)) {
        // surface in the framebuffer exactly matches crtc scanout port, so we
        // can scanout from this framebuffer for the whole crtc
        ret = m_drmlib.drmModeSetCrtc(m_fd, m_crtcID, fbhandle, 0, 0, &m_connectorID, 1, &m_mode);
        if (ret) {
            return MFX_ERR_UNKNOWN;
        }
    }
    else {
        if (m_overlay_wrn) {
            m_overlay_wrn = false;
            printf("drmrender: warning: rendering via OVERLAY plane\n");
        }
    // to support direct panel tonemap for 8K@60 HDR playback via CH7218 connected
    // to 8K HDR panel.
    #if (MFX_VERSION >= 2006)
        mfxExtMasteringDisplayColourVolume* displayColor =
            (mfxExtMasteringDisplayColourVolume*)GetExtBuffer(
                pSurface->Data.ExtParam,
                pSurface->Data.NumExtParam,
                MFX_EXTBUFF_MASTERING_DISPLAY_COLOUR_VOLUME);
        mfxExtContentLightLevelInfo* contentLight =
            (mfxExtContentLightLevelInfo*)GetExtBuffer(pSurface->Data.ExtParam,
                                                       pSurface->Data.NumExtParam,
                                                       MFX_EXTBUFF_CONTENT_LIGHT_LEVEL_INFO);

        if (!m_bSentHDR && m_bHdrSupport && (displayColor && contentLight)) {
            if (displayColor->InsertPayloadToggle == MFX_PAYLOAD_IDR ||
                contentLight->InsertPayloadToggle == MFX_PAYLOAD_IDR) {
                // both panel and bitstream have HDR support
                if (drmSetColorSpace(true) || drmSendHdrMetaData(displayColor, contentLight, true))
                    return MFX_ERR_UNKNOWN;
            }
            m_bSentHDR = true;
        }
    #endif
        // surface in the framebuffer exactly does NOT match crtc scanout port,
        // and we can only use overlay technique with possible resize (depending on the driver))
        ret = m_drmlib.drmModeSetPlane(m_fd,
                                       m_planeID,
                                       m_crtcID,
                                       fbhandle,
                                       0,
                                       0,
                                       0,
                                       m_crtc->width,
                                       m_crtc->height,
                                       pSurface->Info.CropX << 16,
                                       pSurface->Info.CropY << 16,
                                       pSurface->Info.CropW << 16,
                                       pSurface->Info.CropH << 16);
        if (ret) {
            return MFX_ERR_UNKNOWN;
        }
    }
    dropMaster();

    /* Unlock previous Render Target Surface (if exists) */
    if (NULL != m_pCurrentRenderTargetSurface)
        msdk_atomic_dec16((volatile mfxU16*)&m_pCurrentRenderTargetSurface->Data.Locked);

    /* new Render target */
    m_pCurrentRenderTargetSurface = pSurface;
    /* And lock it */
    msdk_atomic_inc16((volatile mfxU16*)&m_pCurrentRenderTargetSurface->Data.Locked);
    return MFX_ERR_NONE;
}

#endif // #if defined(LIBVA_DRM_SUPPORT)
