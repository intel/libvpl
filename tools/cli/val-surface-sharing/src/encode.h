//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

#ifndef TOOLS_CLI_VAL_SURFACE_SHARING_SRC_ENCODE_H_
#define TOOLS_CLI_VAL_SURFACE_SHARING_SRC_ENCODE_H_

#include <list>
#include <memory>
#include <vector>
#include "./hw-device.h"
#include "./util.h"

#ifdef TOOLS_ENABLE_OPENCL
    #include "./process-frames-ocl.h"
#endif

class CEncodeTest : public CTest {
public:
    CEncodeTest();
    virtual ~CEncodeTest();

    mfxStatus Init(int tIndex,
                   Options *opts,
                   std::vector<mfxU32> *adapterNumbers = nullptr) override;
    mfxStatus Run() override;

private:
    int m_tIndex;
    mfxLoader m_loader;
    mfxSession m_session;
    mfxU32 m_adapterNum;

    DevCtx *m_pDevCtx;
#ifdef TOOLS_ENABLE_OPENCL
    OpenCLCtx *m_pOclCtx;
#endif
    mfxVideoParam m_encParams;
    Options *m_pOpts;
    FrameInfo m_frameInfo;
    FileInfo m_fileInfo;
    mfxMemoryInterface *m_memoryInterface;

    mfxStatus CreateVPLSession();
    mfxStatus ProcessStreamEncode();

#ifdef __linux__

    struct ImportedVASurfaceWrapper {
        mfxFrameSurface1 *m_imported_mfx_surface;
        VASurfaceID m_vaSurfaceID;

        ImportedVASurfaceWrapper(mfxFrameSurface1 *surf, VASurfaceID vaid, VADisplay vadisplay)
                : m_imported_mfx_surface(surf),
                  m_vaSurfaceID(vaid),
                  m_vaDisplay(vadisplay) {}

        ImportedVASurfaceWrapper(const ImportedVASurfaceWrapper &)            = delete;
        ImportedVASurfaceWrapper &operator=(const ImportedVASurfaceWrapper &) = delete;

        ImportedVASurfaceWrapper(ImportedVASurfaceWrapper &&)            = delete;
        ImportedVASurfaceWrapper &operator=(ImportedVASurfaceWrapper &&) = delete;

        ~ImportedVASurfaceWrapper() {
            if (m_imported_mfx_surface && m_imported_mfx_surface->FrameInterface &&
                m_imported_mfx_surface->FrameInterface->Release) {
                std::ignore =
                    m_imported_mfx_surface->FrameInterface->Release(m_imported_mfx_surface);
            }
            if (m_vaSurfaceID != VA_INVALID_SURFACE) {
                std::ignore = vaDestroySurfaces(m_vaDisplay, &m_vaSurfaceID, 1);
            }
        }

        // Pay attention, that VADisplay should stay alive until owned surface destruction
        VADisplay m_vaDisplay;
    };

    void ReleaseFreeSurfaces();

    std::list<ImportedVASurfaceWrapper> m_importedVAsurfaces;
#endif
};

#endif // TOOLS_CLI_VAL_SURFACE_SHARING_SRC_ENCODE_H_
