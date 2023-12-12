/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

// Example using Intel® Video Processing Library (Intel® VPL)
#include <stdlib.h>

#include "mfxdefs.h"
#include "mfxstructures.h"
#include "mfxsession.h"
#include "mfxvideo.h"

/* These macro required for code compilation. */
#define UNUSED_PARAM(x) (void)(x)

static mfxSession session;
static mfxSyncPoint syncp;
static mfxBitstream bitstream;
typedef void *ID3D11Texture2D;

static int CaptureFrame(ID3D11Texture2D **p)
{
    UNUSED_PARAM(p);
    return 1;
}

static int RenderFrame(ID3D11Texture2D *p)
{
    UNUSED_PARAM(p);
    return 1;
}

static void prg_surface_sharing1 () {
/*beg1*/

// get interface with import function
mfxMemoryInterface *memoryInterface = nullptr;
MFXGetMemoryInterface(session, &memoryInterface);

// capture desktop as a D3D11 texture using an OS-specific capture API
ID3D11Texture2D *pTexture2D;
CaptureFrame(&pTexture2D);

// import D3D11 texture into Intel® VPL, zero-copy (shared) is preferred, copy is permitted if zero-copy is not supported
mfxSurfaceD3D11Tex2D d3d11_surface                 = {};
d3d11_surface.SurfaceInterface.Header.SurfaceType  = MFX_SURFACE_TYPE_D3D11_TEX2D;
d3d11_surface.SurfaceInterface.Header.SurfaceFlags = (MFX_SURFACE_FLAG_IMPORT_SHARED | MFX_SURFACE_FLAG_IMPORT_COPY);
d3d11_surface.SurfaceInterface.Header.StructSize   = sizeof(mfxSurfaceD3D11Tex2D);

// pass the pointer to the shared D3D11 texture
d3d11_surface.texture2D = pTexture2D;

// external_surface is a pointer to mfxSurfaceHeader but points to a complete structure of type mfxSurfaceD3D11Tex2D
mfxSurfaceHeader *external_surface = reinterpret_cast<mfxSurfaceHeader *>(&d3d11_surface);

// ImportFrameSurface() will return an Intel® VPL surface which may then be used as input for encode or VPP
mfxFrameSurface1 *imported_surface = nullptr;
memoryInterface->ImportFrameSurface(memoryInterface, MFX_SURFACE_COMPONENT_ENCODE, external_surface, &imported_surface);

// encode the surface
MFXVideoENCODE_EncodeFrameAsync(session, nullptr, imported_surface, &bitstream, &syncp);

// release imported surface
imported_surface->FrameInterface->Release(imported_surface);
/*end1*/
}

static void prg_surface_sharing2 () {
/*beg2*/

// decode frame
mfxFrameSurface1 *decoded_surface = nullptr;
MFXVideoDECODE_DecodeFrameAsync(session, &bitstream, nullptr, &decoded_surface, &syncp);

// run VPP on frame
mfxFrameSurface1 *vpp_out_surface = nullptr;
MFXVideoVPP_ProcessFrameAsync(session, decoded_surface, &vpp_out_surface);

// release decoded frame (decrease reference count) after passing to VPP
decoded_surface->FrameInterface->Release(decoded_surface);

// export mfxFrameSurface1 from Intel® VPL to a shared D3D11 texture, zero-copy (shared) is enabled
mfxSurfaceHeader export_header = {};
export_header.SurfaceType  = MFX_SURFACE_TYPE_D3D11_TEX2D;
export_header.SurfaceFlags = MFX_SURFACE_FLAG_EXPORT_SHARED;

// exported_surface is a pointer to mfxSurfaceHeader but will point to a complete structure of type mfxSurfaceD3D11Tex2D
mfxSurfaceHeader *exported_surface = nullptr;
vpp_out_surface->FrameInterface->Export(vpp_out_surface, export_header, &exported_surface);

// get pointer to the shared D3D11 texture
mfxSurfaceD3D11Tex2D *d3d11_surface = reinterpret_cast<mfxSurfaceD3D11Tex2D *>(exported_surface);
ID3D11Texture2D *pTexture2D = reinterpret_cast<ID3D11Texture2D *>(d3d11_surface->texture2D);

// render the D3D11 texture to screen
RenderFrame(pTexture2D);

// release exported surface
mfxSurfaceInterface *exported_surface_interface = reinterpret_cast<mfxSurfaceInterface *>(exported_surface);
exported_surface_interface->Release(exported_surface_interface);

// release VPP output frame
vpp_out_surface->FrameInterface->Release(vpp_out_surface);

/*end2*/
}
