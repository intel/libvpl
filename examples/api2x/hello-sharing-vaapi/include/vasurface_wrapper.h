//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

#include <list>
#include <memory>
#include <vector>
#include "device-vaapi.h"
#include "util.hpp"

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

    // vaSurfaces destroyed in the destructor
    ~ImportedVASurfaceWrapper() {
        if (m_imported_mfx_surface && m_imported_mfx_surface->FrameInterface &&
            m_imported_mfx_surface->FrameInterface->Release) {
            std::ignore = m_imported_mfx_surface->FrameInterface->Release(m_imported_mfx_surface);
        }
        if (m_vaSurfaceID != VA_INVALID_SURFACE) {
            std::ignore = vaDestroySurfaces(m_vaDisplay, &m_vaSurfaceID, 1);
        }
    }

    // Pay attention, that VADisplay should stay alive until owned surface destruction
    VADisplay m_vaDisplay;
};

// List of imported VASurfaces to handle deletion
std::list<ImportedVASurfaceWrapper> m_importedVAsurfaces;

void ReleaseFreeSurfaces() {
    for (std::list<ImportedVASurfaceWrapper>::iterator surface = m_importedVAsurfaces.begin();
         surface != m_importedVAsurfaces.end();) {
        if (surface->m_imported_mfx_surface->Data.Locked == 0)
            surface = m_importedVAsurfaces.erase(surface);
        else
            ++surface;
    }
}