//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

#include "./util.h"

#if defined(_WIN32) || defined(_WIN64)
#if defined _DEBUG
#include <dxgidebug.h>
#endif
#endif

// global to run dump functions only one time
static mfxU32 g_dbgOnceMask = 0;

void DebugDumpNativeSurfaceDesc(mfxFrameSurface1 *pSrf, bool bRunOnce) {
    if (!pSrf)
        return;

    if (bRunOnce && (g_dbgOnceMask & DBG_MASK_NATIVE_SURFACE_DESC))
        return;

    g_dbgOnceMask |= DBG_MASK_NATIVE_SURFACE_DESC;

#if defined(_WIN32) || defined(_WIN64)
    mfxHDL frameHandle      = nullptr;
    mfxResourceType resType = {};

    mfxStatus sts = pSrf->FrameInterface->GetNativeHandle(pSrf, &frameHandle, &resType);
    if (sts != MFX_ERR_NONE || !frameHandle || resType != MFX_RESOURCE_DX11_TEXTURE) {
        printf("Error - DebugDumpNativeSurfaceDesc\n");
        return;
    }

    ID3D11Texture2D *pTex2D   = reinterpret_cast<ID3D11Texture2D *>(frameHandle);
    D3D11_TEXTURE2D_DESC desc = {};
    pTex2D->GetDesc(&desc);

    printf("\nDebug option: DBG_MASK_NATIVE_SURFACE_DESC\n");
    printf("  DBG_MASK_NATIVE_SURFACE_DESC desc\n");
    printf("  -------------------------\n");
    printf("  Width              = %d\n", desc.Width);
    printf("  Height             = %d\n", desc.Height);
    printf("  MipLevels          = %d\n", desc.MipLevels);
    printf("  ArraySize          = %d\n", desc.ArraySize);
    printf("  Format             = %d\n", desc.Format);
    printf("  SampleDesc.Count   = %d\n", desc.SampleDesc.Count);
    printf("  SampleDesc.Quality = %d\n", desc.SampleDesc.Quality);
    printf("  Usage              = 0x%08x\n", desc.Usage);
    printf("  BindFlags          = 0x%08x\n", desc.BindFlags);
    printf("  CPUAccessFlags     = 0x%08x\n", desc.CPUAccessFlags);
    printf("  MiscFlags          = 0x%08x\n", desc.MiscFlags);
    printf("\n");
#endif

    return;
}

void DebugDumpHardwareInterface() {
#if defined(_WIN32) || defined(_WIN64)
#if defined _DEBUG
    const char *dbgLibraryName  = "dxgidebug.dll";
    const char *dbgFunctionName = "DXGIGetDebugInterface";

    // load the DXGI debug library, if installed
    HMODULE dbgLibrary = LoadLibrary(dbgLibraryName);
    if (!dbgLibrary) {
        printf("WARNING: Unable to load debug library %s. Skipping detailed DXGIDebug report.\n", dbgLibraryName);
        return;
    }

    // get address of DXGIGetDebugInterface()
    // see function definition in Windows docs
    typedef HRESULT(WINAPI * DXGIGetDebugInterface)(const IID &, void **);
    DXGIGetDebugInterface fnDbgInterface;
    fnDbgInterface = reinterpret_cast<DXGIGetDebugInterface>(reinterpret_cast<void *>(GetProcAddress(dbgLibrary, dbgFunctionName)));
    if (!fnDbgInterface) {
        printf("WARNING: Unable to load debug function %s. Skipping detailed DXGIDebug report.\n", dbgFunctionName);
        FreeLibrary(dbgLibrary);
        return;
    }

    // call ReportLiveObjects() to print detailed debug report
    IDXGIDebug *dbgInterface = nullptr;
    HRESULT res              = fnDbgInterface(__uuidof(IDXGIDebug), (void **)&dbgInterface);

    if (SUCCEEDED(res))
        dbgInterface->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
    else
        printf("WARNING: Calling %s returned %d. Skipping detailed DXGIDebug report.\n", dbgFunctionName, res);

    // teardown
    dbgInterface->Release();
    FreeLibrary(dbgLibrary);
#endif
#endif

    return;
}

#define STRING_OPTION(x) \
    case x:              \
        return #x

const char *DebugGetStringSurfaceFlags(mfxU32 surfaceFlags) {
    switch (surfaceFlags) {
        STRING_OPTION(MFX_SURFACE_FLAG_IMPORT_SHARED);
        STRING_OPTION(MFX_SURFACE_FLAG_IMPORT_COPY);
        STRING_OPTION(MFX_SURFACE_FLAG_EXPORT_SHARED);
        STRING_OPTION(MFX_SURFACE_FLAG_EXPORT_COPY);
    }

    return "<unknown surface flag type>";
}
