# Intel® Video Processing Library (Intel® VPL) Surface Sharing - Overview of new APIs

Intel® VPL API 2.10 introduces new interfaces for sharing surfaces between Intel® VPL runtime and other applications, frameworks, and graphics APIs. This functionality is only supported when using the internal memory management model.

**Importing** a surface enables Intel® VPL to access raw video data as input to encode or VPP operations without first mapping the data to system memory and then copying it to a surface allocated by Intel® VPL runtime using ``mfxFrameSurfaceInterface::Map``.

**Exporting** a surface similarly enables the application to access raw video data which is the output of decode or VPP operations and which was allocated by Intel® VPL runtime, without first mapping to system memory using ``mfxFrameSurfaceInterface::Map``.

The following provides an overview of the new APIs. Additional information and runnable sample code may be found in the examples directory (examples/api2x/hello-sharing-*).

- Note: The following APIs are defined under the ONEVPL_EXPERIMENTAL macro. This macro must be defined at build time for these APIs to be accessible. Behavior of these APIs may change in future releases. Backwards compatibility and future presence of experiemental APIs is not guaranteed. 

## Importing and exporting surfaces between Intel® VPL runtime and external frameworks

### New header file mfxmemory.h

The following function declarations are moved from mfxvideo.h into mfxmemory.h. For backward compatibility, mfxvideo.h #includes mfxmemory.h
```
    MFXMemory_GetSurfaceForDecode
    MFXMemory_GetSurfaceForEncode
    MFXMemory_GetSurfaceForVPP (with alias: MFXMemory_GetSurfaceForVPPIn)
    MFXMemory_GetSurfaceForVPPOut
``` 
### New function - surface IMPORT

A new interface structure ```mfxMemoryInterface``` is defined, which contains function pointers filled in by the Intel® VPL runtime. New functions are then called by the application (i.e. a callback interface). 

The application requests the interface using ```MFXVideoCORE_GetHandle()```. Inputs to GetHandle() are the mfxSession handle and an enum of type ```mfxHandleType```. The output is a generic handle (```mfxHDL``` = ```void*```) which the application then casts to the appropriate interface structure.

Detailed description:

- Defines a new value for ```mfxHandleType``` named ```MFX_HANDLE_MEMORY_INTERFACE```, which is used to request an interface of type ```mfxMemoryInterface```.

```
MFX_HANDLE_MEMORY_INTERFACE                 = 1001,  /*!< Pointer to interface of type mfxMemoryInterface. */
```

- Defines a new callback interface mfxMemoryInterface in mfxmemory.h
Initially this will include fields for ```Context```, ```Version```, and one new function ```ImportFrameSurface```

```
/* Specifies memory interface. */
typedef struct mfxMemoryInterface {
    mfxHDL              Context; /*!< The context of the memory interface. User should not touch (change, set, null) this pointer. */
    mfxStructVersion    Version; /*!< The version of the structure. */

    /*!
       @brief
        Imports an application-provided surface into mfxFrameSurface1 which may be used as input for encoding or video processing.

       @param[in]      memory_interface  Valid memory interface.
       @param[in]      surf_component    Surface component type. Required for allocating new surfaces from the appropriate pool.
       @param[in,out]  external_surface  Pointer to the mfxSurfaceXXX object describing the surface to be imported. All fields in
                                         mfxSurfaceHeader must be set by the application. mfxSurfaceHeader::SurfaceType is
                                         read by Intel® VPL runtime to determine which particular mfxSurfaceXXX structure is supplied.
                                         For example, if mfxSurfaceXXX::SurfaceType == MFX_SURFACE_TYPE_D3D11_TEX2D, then the handle
                                         will be interpreted as an object of type mfxSurfaceD3D11Tex2D. The application should
                                         set or clear other fields as specified in the corresponding structure description.
                                         After successful import, the value of mfxSurfaceHeader::SurfaceFlags will be replaced with the actual
                                         import type. It can be used to determine which import type (with or without copy) took place in the case
                                         of initial default setting, or if multiple import flags were OR'ed.
                                         All external sync operations on the ext_surface must be completed before calling this function.
       @param[out]     imported_surface  Pointer to a valid mfxFrameSurface1 object containing the imported frame.
                                         imported_surface may be passed as an input to Encode or VPP processing operations.

       @return
       MFX_ERR_NONE The function completed successfully.\n
       MFX_ERR_NULL_PTR If ext_surface or imported_surface are NULL.\n
       MFX_ERR_INVALID_HANDLE If the corresponding session was not initialized.\n
       MFX_ERR_UNSUPPORTED If surf_component is not one of [MFX_SURFACE_COMPONENT_ENCODE, MFX_SURFACE_COMPONENT_VPP_INPUT], or if
                           mfxSurfaceHeader::SurfaceType is not supported by Intel® VPL runtime for this operation.\n

       @since This function is available since API version 2.10.
    */

    /* For reference with Export flow please search for mfxFrameSurfaceInterface::Export. */
    mfxStatus (MFX_CDECL *ImportFrameSurface)(struct mfxMemoryInterface* memory_interface, mfxSurfaceComponent surf_component, mfxSurfaceHeader* external_surface, mfxFrameSurface1** imported_surface);

    mfxHDL     reserved[16];
} mfxMemoryInterface;
```

- Defines a new macro ```MFXGetMemoryInterface``` which is a shortcut to calling ```MFXVideoCORE_GetHandle()``` with handle type ```MFX_HANDLE_MEMORY_INTERFACE```.

```
/*! Alias for returning interface of type mfxMemoryInterface. */
#define MFXGetMemoryInterface(session, piface)  MFXVideoCORE_GetHandle((session), MFX_HANDLE_MEMORY_INTERFACE, (mfxHDL *)(piface))
```

### New function - surface EXPORT
A new function ```Export()``` is added to the ```mfxFrameSurfaceInterface``` callback API.

```
typedef struct mfxFrameSurfaceInterface {
    ...

    /*! @brief
     If successful returns an exported surface, which is a refcounted object allocated by runtime. It could be exported with or without copy, depending
     on export flags and the possibility of such export. Exported surface is valid throughout the session, as long as the original mfxFrameSurface1 
     object is not closed and the refcount of exported surface is not zero.

     @param[in]  surface              Valid surface.
     @param[in]  export_header        Description of export: caller should fill in SurfaceType (type to export to) and SurfaceFlags (allowed export modes).
     @param[out] exported_surface     Exported surface, allocated by runtime, user needs to decrement refcount after usage for object release.
                                      After successful export, the value of mfxSurfaceHeader::SurfaceFlags will contain the actual export mode.


     @return
      MFX_ERR_NONE               If no error. \n
      MFX_ERR_NULL_PTR           If export surface or surface is NULL. \n
      MFX_ERR_UNSUPPORTED        If requested export is not supported. \n
      MFX_ERR_NOT_IMPLEMENTED    If requested export is not implemented. \n
      MFX_ERR_UNKNOWN            Any internal error.
     */

     /* For reference with Import flow please search for mfxMemoryInterface::ImportFrameSurface. */
    mfxStatus           (MFX_CDECL *Export)(mfxFrameSurface1* surface, mfxSurfaceHeader export_header, mfxSurfaceHeader** exported_surface);

    ...
} mfxFrameSurfaceInterface;
```

### New enumerated types

- Defines a new enum ```mfxSurfaceComponent``` to distinguish which internal surface pool frames are imported/exported to/from. This is required in order for Intel® VPL runtime to allocate and assign surfaces from the proper internal frame pool.

```
/*! The mfxSurfaceComponent enumerator specifies the internal surface pool to use when importing surfaces. */
typedef enum {
    MFX_SURFACE_COMPONENT_UNKNOWN     = 0,      /*!< Unknown surface component. */

    MFX_SURFACE_COMPONENT_ENCODE      = 1,      /*!< Shared surface for encoding. */
    MFX_SURFACE_COMPONENT_DECODE      = 2,      /*!< Shared surface for decoding. */
    MFX_SURFACE_COMPONENT_VPP_INPUT   = 3,      /*!< Shared surface for VPP input. */
    MFX_SURFACE_COMPONENT_VPP_OUTPUT  = 4,      /*!< Shared surface for VPP output. */
} mfxSurfaceComponent;
```

- Defines a new enum ```mfxSurfaceType``` which will be part of the common description (header) that describes external surfaces. New definitions will be added to mfxSurfaceType as new external APIs are supported.

```
/*! The mfxSurfaceType enumerator specifies the surface type described by mfxSurfaceHeader. */
typedef enum {
    MFX_SURFACE_TYPE_UNKNOWN               = 0,      /*!< Unknown surface type. */

    MFX_SURFACE_TYPE_D3D11_TEX2D           = 2,      /*!< D3D11 surface of type ID3D11Texture2D. */
    MFX_SURFACE_TYPE_VAAPI                 = 3,      /*!< VA-API surface. */
    MFX_SURFACE_TYPE_OPENCL_IMG2D          = 4,      /*!< OpenCL 2D image (cl_mem). */
} mfxSurfaceType;
```

 - Defines a new list of enums prefixed with ```MFX_SURFACE_FLAG_``` which will be used as both inputs and outputs to Intel® VPL import/export operations. To avoid need for explicit cast when OR'ing flags together, it is not a typedef.
```
/*! This enumerator specifies the sharing modes which are allowed for importing or exporting shared surfaces. */
enum {
    MFX_SURFACE_FLAG_DEFAULT          = 0x0000,      /*!< Default is SHARED import or export. */

    MFX_SURFACE_FLAG_IMPORT_SHARED    = 0x0010,      /*!< Import frames directly by mapping a shared native handle from an application-provided surface to an internally-allocated surface. */
    MFX_SURFACE_FLAG_IMPORT_COPY      = 0x0020,      /*!< Import frames by copying data from an application-provided surface to an internally-allocated surface. */

    MFX_SURFACE_FLAG_EXPORT_SHARED    = 0x0100,      /*!< Export frames directly by mapping a shared native handle from an internally-allocated surface to an application-provided surface. */
    MFX_SURFACE_FLAG_EXPORT_COPY      = 0x0200,      /*!< Export frames by copying data from an internally-allocated surface to an application-provided surface. */
};
```
- As inputs, these flags indicate whether the runtime should attempt to import/export surfaces with zero-copy ("shared") and/or by creating a copy ("copy").
- As outputs, these flages indicate what operation Intel® VPL runtime performed (shared mapping or copy).
- Flags may be OR'd together to indicate, e.g. "attempt zero-copy sharing if possible, otherwise doing a copy is acceptable".

### New data structures - Common

Each supported external API will require a new data structure which contains the necessary info to permit sharing/copying between Intel® VPL and the external framework. The contents will vary for each external API, but same basic approach is followed as with Intel® VPL extension buffers: define a common "header" structure which is always the first element in the API-specific data structure, followed by additional elements which are particular to each API (frame handle, device context, etc.)

Defines a new data structure ```mfxSurfaceHeader``` which will be included as the first element in each API-specific surface description structure (below).

```
typedef struct {
    mfxSurfaceType SurfaceType;     /*!< Set to the MFX_SURFACE_TYPE enum corresponding to the specific structure. */
    mfxU32         SurfaceFlags;    /*!< Set to the MFX_SURFACE_FLAG enum (or combination) corresponding to the allowed import / export mode(s). Multiple flags may be combined with OR. 
                                         Upon a successful Import or Export operation, this field will indicate the actual mode used.*/

    mfxU32         StructSize;      /*!< Size in bytes of the complete mfxSurfaceXXX structure. */

    mfxU16         NumExtParam;     /*!< The number of extra configuration structures attached to the structure. */
    mfxExtBuffer** ExtParam;        /*!< Points to an array of pointers to the extra configuration structures; see the ExtendedBufferID enumerator for a list of extended configurations. */

    mfxU32 reserved[6];
} mfxSurfaceHeader;
```

Defines a new data structure ```mfxSurfaceInterface``` which includes a set of callback functions to control refcounting and synchronization of imported/exported surfaces. The runtime fills in this structure with hooks to the appropriate function calls, which are implemented in the runtime.
```
typedef struct mfxSurfaceInterface {
    mfxSurfaceHeader Header;  /*!< Exported surface header. Contains description of current surface. */

    mfxStructVersion Version; /*!< The version of the structure. */

    mfxHDL           Context; /*!< The context of the exported surface interface. User should not touch (change, set, null) this pointer. */

    /*! @brief
    Increments the internal reference counter of the surface. The surface is not destroyed until the surface is released using the mfxSurfaceInterface::Release function.
    mfxSurfaceInterface::AddRef should be used each time a new link to the surface is created (for example, copy structure) for proper surface management.

    @param[in]  surface  Valid surface.

    @return
     MFX_ERR_NONE              If no error. \n
     MFX_ERR_NULL_PTR          If surface is NULL. \n
     MFX_ERR_INVALID_HANDLE    If mfxSurfaceInterface->Context is invalid (for example NULL). \n
     MFX_ERR_UNKNOWN           Any internal error.

    */
    mfxStatus           (MFX_CDECL *AddRef)(struct mfxSurfaceInterface* surface);

    /*! @brief
    Decrements the internal reference counter of the surface. mfxSurfaceInterface::Release should be called after using the
    mfxSurfaceInterface::AddRef function to add a surface or when allocation logic requires it. For example, call
    mfxSurfaceInterface::Release to release a surface obtained with the mfxFrameSurfaceInterface::Export function.

    @param[in]  surface  Valid surface.

    @return
     MFX_ERR_NONE               If no error. \n
     MFX_ERR_NULL_PTR           If surface is NULL. \n
     MFX_ERR_INVALID_HANDLE     If mfxSurfaceInterface->Context is invalid (for example NULL). \n
     MFX_ERR_UNDEFINED_BEHAVIOR If Reference Counter of surface is zero before call. \n
     MFX_ERR_UNKNOWN            Any internal error.
    */
    mfxStatus           (MFX_CDECL *Release)(struct mfxSurfaceInterface* surface);

    /*! @brief
    Returns current reference counter of exported surface.

    @param[in]   surface  Valid surface.
    @param[out]  counter  Sets counter to the current reference counter value.

    @return
     MFX_ERR_NONE               If no error. \n
     MFX_ERR_NULL_PTR           If surface or counter is NULL. \n
     MFX_ERR_INVALID_HANDLE     If mfxSurfaceInterface->Context is invalid (for example NULL). \n
     MFX_ERR_UNKNOWN            Any internal error.
    */
    mfxStatus           (MFX_CDECL *GetRefCounter)(struct mfxSurfaceInterface* surface, mfxU32* counter);

    /*! @brief
    This function is only valuable for surfaces which were exported in sharing mode (without a copy).
    Guarantees readiness of both the data (pixels) and any original mfxFrameSurface1 frame's meta information (for example corruption flags) after a function completes.

    Instead of MFXVideoCORE_SyncOperation, users may directly call the mfxSurfaceInterface::Synchronize function after the corresponding
    Decode or VPP function calls (MFXVideoDECODE_DecodeFrameAsync or MFXVideoVPP_RunFrameVPPAsync).
    The prerequisites to call the functions are:

    @li The main processing functions return MFX_ERR_NONE.
    @li A valid surface object.

    @param[in]   surface  Valid surface.
    @param[out]  wait  Wait time in milliseconds.

    @return
     MFX_ERR_NONE               If no error. \n
     MFX_ERR_NULL_PTR           If surface is NULL. \n
     MFX_ERR_INVALID_HANDLE     If any of surface is not valid object . \n
     MFX_WRN_IN_EXECUTION       If the given timeout is expired and the surface is not ready. \n
     MFX_ERR_ABORTED            If the specified asynchronous function aborted due to data dependency on a previous asynchronous function that did not complete. \n
     MFX_ERR_UNKNOWN            Any internal error.
    */
    mfxStatus           (MFX_CDECL *Synchronize)(struct mfxSurfaceInterface* surface, mfxU32 wait);

    mfxHDL reserved[11];
} mfxSurfaceInterface;
```

### New data structures - framework-specific

Defines a set of new data structures of the form ```mfxSurfaceXXX```, where each XXX corresponds to an external framework such as DirectX11 (``mfxSurfaceD3D11Tex2D``), OpenCL (``mfxSurfaceOpenCLImg2D``), etc. Note that each struct begins with mfxSurfaceInterface, so any component may parse this structure by reading the first value (```SurfaceInterface.Header.SurfaceType```) and then casting the object to the corresponding ```mfxSurfaceXXX``` definition.

```
typedef struct {
    mfxSurfaceInterface SurfaceInterface;

    mfxHDL texture2D;                           /*!< Pointer to texture, type ID3D11Texture2D */
    mfxHDL reserved[7];
} mfxSurfaceD3D11Tex2D;
```

```
typedef struct {
    mfxSurfaceInterface SurfaceInterface;

    mfxHDL vaDisplay;                           /*!< Object of type VADisplay. */
    mfxU32 vaSurfaceID;                         /*!< Object of type VASurfaceID. */
    mfxU32 reserved1;

    mfxHDL reserved[6];
} mfxSurfaceVAAPI;
```

```
typedef struct {
    mfxSurfaceInterface SurfaceInterface;

    mfxHDL ocl_context;                         /*!< Pointer to OpenCL context, type cl_context */
    mfxHDL ocl_command_queue;                   /*!< Pointer to OpenCL command queue, type cl_command_queue */

    mfxHDL ocl_image[4];                        /*!< Pointer to OpenCL 2D images, type cl_mem */
    mfxU32 ocl_image_num;                       /*!< Number of valid images (planes), depends on color format */

    mfxHDL reserved[8];
} mfxSurfaceOpenCLImg2D;
```

```
/*!
   Optional extension buffer, which can be attached to mfxSurfaceHeader::ExtParam
   (second parameter of mfxFrameSurfaceInterface::Export) in order to pass OCL parameters
   during mfxFrameSurface1 exporting to OCL surface.
   If buffer is not provided all resources will be created by Intel® VPL RT internally.
*/
typedef struct {
    mfxExtBuffer    Header;                     /*!< Extension buffer header. Header.BufferId must be equal to MFX_EXTBUFF_EXPORT_SHARING_DESC_OCL. */

    mfxHDL ocl_context;                         /*!< Pointer to OpenCL context, type cl_context */
    mfxHDL ocl_command_queue;                   /*!< Pointer to OpenCL command queue, type cl_command_queue */

    mfxHDL reserved[8];
} mfxExtSurfaceOpenCLImg2DExportDescription;
```

## Querying for supported frameworks

Prior to session creation, applications may query the surface sharing capabilities of each implementation. This is possible by defining a new ```mfxImplCapsDeliveryFormat``` value:

```
MFX_IMPLCAPS_SURFACE_TYPES     = 5, /*!< Deliver capabilities as mfxSurfaceTypesSupported structure. */
```

To access surface sharing capabilities, Intel® VPL dispatcher will call MFXQueryImplsDescription(format = MFX_IMPLCAPS_SURFACE_TYPES), and each runtime which supports the surface sharing APIs will return an array of mfxSurfaceTypesSupported structures (one per implementation) which descripbe the supported surface types, along with the associated components and sharing modes that each supports. The description structure is defined as follows:

```
typedef struct {
    mfxStructVersion Version;                       /*!< Version of the structure. */

    mfxU16 NumSurfaceTypes;                         /*!< Number of supported surface types. */
    struct surftype {
        mfxSurfaceType SurfaceType;                 /*!< Supported surface type. */
        mfxU32         reserved[6];                 /*!< Reserved for future use. */
        mfxU16         NumSurfaceComponents;        /*!< Number of supported surface components. */
        struct surfcomp {
            mfxSurfaceComponent SurfaceComponent;   /*!< Supported surface component. */
            mfxU32              SurfaceFlags;       /*!< Supported surface flags for this component (may be OR'd). */
            mfxU32              reserved[7];        /*!< Reserved for future use. */
        } *SurfaceComponents;
    } *SurfaceTypes;

    mfxU32   reserved[4];                           /*!< Reserved for future use. */
} mfxSurfaceTypesSupported;
```

As with other Intel® VPL implementation capabilities reports, an application may use this information in two ways:
1) It can call MFXEnumImplementations(format = MFX_IMPLCAPS_SURFACE_TYPES) either before or after session creation, and then use the contents of the returned mfxSurfaceTypesSupported structures to inform the construction of media processing pipelines.
2) It can call MFXSetConfigFilterProperty() prior to session creation, which causes the dispatcher to filter out (remove) any implementations which do not support the requested surface sharing operations. See an example in the code snippets below.

## Code Snippets

### Using MFXSetConfigFilterProperty() to require surface sharing capabilities
```
// this example requires that the implementation provide D3D11 sharing for decode with EXPORT_SHARED mode (zero-copy)
// note that all three properties should be applied to the same mfxConfig object to avoid any ambiguity in 
//   describing the implementation capabilities

mfxConfig config = MFXCreateConfig(loader);
mfxVariant var = {};
var.Version.Version = (mfxU16)MFX_VARIANT_VERSION;

var.Type     = MFX_VARIANT_TYPE_U32;
var.Data.U32 = MFX_SURFACE_TYPE_D3D11_TEX2D;
MFXSetConfigFilterProperty(config, (mfxU8 *)"mfxSurfaceTypesSupported.surftype.SurfaceType", var);

var.Type     = MFX_VARIANT_TYPE_U32;
var.Data.U32 = MFX_SURFACE_COMPONENT_DECODE;
MFXSetConfigFilterProperty(config, (mfxU8 *)"mfxSurfaceTypesSupported.surftype.surfcomp.SurfaceComponent", var);

var.Type     = MFX_VARIANT_TYPE_U32;
var.Data.U32 = MFX_SURFACE_FLAG_EXPORT_SHARED;
MFXSetConfigFilterProperty(config, (mfxU8 *)"mfxSurfaceTypesSupported.surftype.surfcomp.SurfaceFlags", var);

MFXCreateSession(loader, 0, &session);
```

### Exporting a frame decoded in Intel® VPL to a D3D11 texture
```
// decode frame
sts = MFXVideoDECODE_DecodeFrameAsync(session, (isDrainingDec ? NULL : &fileInfo->bitstream), NULL, &pmfxDecOutSurface, &syncpD);

// run VPP on frame
sts = MFXVideoVPP_ProcessFrameAsync(session, pmfxDecOutSurface, &pmfxVPPOutSurface);

// release decoded frame (decrease reference count) after passing to VPP
pmfxDecOutSurface->FrameInterface->Release(pmfxDecOutSurface);

// wait for frame processing to complete
do {
   sts = pmfxVPPOutSurface->FrameInterface->Synchronize(pmfxVPPOutSurface, 100);
} while (sts == MFX_WRN_IN_EXECUTION);

// set header, all other fields should be empty
mfxSurfaceHeader export_header = {};
export_header.SurfaceType      = MFX_SURFACE_TYPE_D3D11_TEX2D;
export_header.SurfaceFlags     = MFX_SURFACE_FLAG_EXPORT_SHARED;

// export mfxFrameSurface1 to a D3D11 texture, zero-copy enabled
mfxSurfaceD3D11Tex2D *exported_surface = nullptr;
sts = pmfxVPPOutSurface->FrameInterface->Export(pmfxVPPOutSurface, export_header, (mfxSurfaceHeader **)(&exported_surface));

// render the original D3D11 texture (output of VPP)
// RenderFrame() is a function which renders the exported surface to the display
ID3D11Texture2D *pTex2D = exported_surface->texture2D;
sts = RenderFrame(pTex2D);

// release the exported surface
sts = exported_surface->SurfaceInterface.Release(&(exported_surface->SurfaceInterface));

// release VPP output frame
pmfxVPPOutSurface->FrameInterface->Release(pmfxVPPOutSurface);
```

### Importing a D3D11 texture as a Intel® VPL frame for encoding
```
// get interface with import function (only call once per session)
mfxMemoryInterface *memoryInterface = nullptr;
sts = MFXGetMemoryInterface(session, &memoryInterface);

// CaptureFrame is a function which allocates and returns a D3D11 texture to encode
//   (for example: desktop duplication)
ID3D11Texture2D *pTex2D;
sts = CaptureFrame(&pTex2D);

// import D3D11 surface into mfxFrameSurface1
// set header.surfaceType, device, and texture2D, all other fields should be empty
mfxSurfaceD3D11Tex2D extSurfD3D11                 = {};
extSurfD3D11.SurfaceInterface.Header.SurfaceType  = MFX_SURFACE_TYPE_D3D11_TEX2D;
extSurfD3D11.SurfaceInterface.Header.SurfaceFlags = (MFX_SURFACE_FLAG_IMPORT_SHARED | MFX_SURFACE_FLAG_IMPORT_COPY);
extSurfD3D11.SurfaceInterface.Header.StructSize   = sizeof(mfxSurfaceD3D11Tex2D);

// pointer to the texture to import
extSurfD3D11.texture2D = pTex2D;

// import D3D11 texture, zero-copy preferred, copy okay
mfxFrameSurface1 *pmfxEncSurface = nullptr;
sts = memoryInterface->ImportFrameSurface(memoryInterface, MFX_SURFACE_COMPONENT_ENCODE, (mfxSurfaceHeader *)extSurfD3D11, &pmfxEncSurface);

// encode
sts = MFXVideoENCODE_EncodeFrameAsync(session, NULL, (isDrainingEnc ? NULL : pmfxEncSurface), &fileInfo->bitstream, &syncp);

// release imported surface
pmfxEncSurface->FrameInterface->Release(pmfxEncSurface);
```

### Exporting a frame decoded in Intel® VPL to a VAAPI surface
```
// decode frame
sts = MFXVideoDECODE_DecodeFrameAsync(session, (isDrainingDec ? NULL : &fileInfo->bitstream), NULL, &pmfxDecOutSurface, &syncpD);

// run VPP on frame
sts = MFXVideoVPP_ProcessFrameAsync(session, pmfxDecOutSurface, &pmfxVPPOutSurface);

// release decoded frame (decrease reference count) after passing to VPP
pmfxDecOutSurface->FrameInterface->Release(pmfxDecOutSurface);

// wait for frame processing to complete
do {
   sts = pmfxVPPOutSurface->FrameInterface->Synchronize(pmfxVPPOutSurface, 100);
} while (sts == MFX_WRN_IN_EXECUTION);

// set header, all other fields should be empty
mfxSurfaceHeader export_header = {};
export_header.SurfaceType      = MFX_SURFACE_TYPE_VAAPI;
export_header.SurfaceFlags     = MFX_SURFACE_FLAG_EXPORT_SHARED;

// export mfxFrameSurface1 from Intel® VPL to a VAAPI surface, zero-copy enabled
mfxSurfaceVAAPI *exported_surface = nullptr;
sts = pmfxVPPOutSurface->FrameInterface->Export(pmfxVPPOutSurface, export_header, (mfxSurfaceHeader **)(&exported_surface));

// save the original VAAPI texture (output of VPP)
// SaveFrame() is a function which saves the exported surface to a file
VASurfaceID surfID = exported_surface->vaSurfaceID;
sts = SaveFrame(surfID);

// release the exported surface
sts = exported_surface->SurfaceInterface.Release(&(exported_surface->SurfaceInterface));

// release VPP output frame
pmfxVPPOutSurface->FrameInterface->Release(pmfxVPPOutSurface);
```

### Importing a VAAPI surface as a Intel® VPL frame for encoding

```
// get interface with import function (only call once per session)
mfxMemoryInterface *memoryInterface = nullptr;
sts = MFXGetMemoryInterface(session, &memoryInterface);

// CaptureFrame is a function which allocates and returns a VAAPI texture to encode
//   (for example: desktop duplication)
VASurfaceID vaSurfaceID;
sts = CaptureFrame(&vaSurfaceID);

// import VAAPI surface into mfxFrameSurface1
// set header.surfaceType, device, and texture2D, all other fields should be empty
mfxSurfaceVAAPI extSurfVAAPI                      = {};
extSurfVAAPI.SurfaceInterface.Header.SurfaceType  = MFX_SURFACE_TYPE_VAAPI;
extSurfVAAPI.SurfaceInterface.Header.SurfaceFlags = (MFX_SURFACE_FLAG_IMPORT_SHARED | MFX_SURFACE_FLAG_IMPORT_COPY);
extSurfVAAPI.SurfaceInterface.Header.StructSize   = sizeof(mfxSurfaceVAAPI);

// pointer to the surface to import and the display context of type VADisplay
extSurfVAAPI.vaDisplay   = GetVADisplay();
extSurfVAAPI.vaSurfaceID = vaSurfaceID;

// import VAAPI surface, zero-copy preferred, copy okay
mfxFrameSurface1 *pmfxEncSurface = nullptr;
sts = memoryInterface->ImportFrameSurface(memoryInterface, MFX_SURFACE_COMPONENT_ENCODE, (mfxSurfaceHeader *)extSurfVAAPI, &pmfxEncSurface);

// encode
sts = MFXVideoENCODE_EncodeFrameAsync(session, NULL, (isDrainingEnc ? NULL : pmfxEncSurface), &fileInfo->bitstream, &syncp);

// release imported surface
pmfxEncSurface->FrameInterface->Release(pmfxEncSurface);
```

### Initializing OpenCL extension functions
```
// to import/export with OpenCL, the application must check for the presence of and 
//  retreive the addresses of the following functions from the OpenCL runtime
// this only happens one time, during initialization of the OpenCL context
// these function pointers will be used in the code snippets which follow

clCreateFromD3D11Texture2DKHR_fn   pfn_clCreateFromD3D11Texture2DKHR;
clEnqueueAcquireD3D11ObjectsKHR_fn pfn_clEnqueueAcquireD3D11ObjectsKHR;
clEnqueueReleaseD3D11ObjectsKHR_fn pfn_clEnqueueReleaseD3D11ObjectsKHR;

cl_platform_id platform_id = GetPlatformID(); // platform ID for the device used to create the OpenCL context

pfn_clCreateFromD3D11Texture2DKHR   = clGetExtensionFunctionAddressForPlatform(platform_id, (const char *)"clCreateFromD3D11Texture2DKHR");
pfn_clEnqueueAcquireD3D11ObjectsKHR = clGetExtensionFunctionAddressForPlatform(platform_id, (const char *)"clEnqueueAcquireD3D11ObjectsKHR");
pfn_clEnqueueReleaseD3D11ObjectsKHR = clGetExtensionFunctionAddressForPlatform(platform_id, (const char *)"clEnqueueReleaseD3D11ObjectsKHR");
```

### Exporting a frame decoded in Intel® VPL to an OpenCL image for processing
- Note: Currently supported on Windows only. Exported images will be provided as shared surfaces created with the clCreateFromD3D11Texture2DKHR extension.
```
// decode frame
sts = MFXVideoDECODE_DecodeFrameAsync(session, (isDrainingDec ? NULL : &fileInfo->bitstream), NULL, &pmfxDecOutSurface, &syncpD);

// run VPP on frame
sts = MFXVideoVPP_ProcessFrameAsync(session, pmfxDecOutSurface, &pmfxVPPOutSurface);

// release decoded frame (decrease reference count) after passing to VPP
pmfxDecOutSurface->FrameInterface->Release(pmfxDecOutSurface);

// wait for frame processing to complete
do {
   sts = pmfxVPPOutSurface->FrameInterface->Synchronize(pmfxVPPOutSurface, 100);
} while (sts == MFX_WRN_IN_EXECUTION);

// set header, all other fields should be empty
mfxSurfaceHeader export_header = {};
export_header.SurfaceType      = MFX_SURFACE_TYPE_OPENCL_IMG2D;
export_header.SurfaceFlags     = MFX_SURFACE_FLAG_EXPORT_SHARED;
export_header.NumExtParam      = 1;

// provide extension buffer with the OpenCL context and command queue which
//   were initialized by the application
// the runtime needs this information to confirm that the exported surface is accessible
//   in this context and command queue
mfxExtSurfaceOpenCLImg2DExportDescription export_header_buf = {};
export_header_buf.Header.BufferId    = MFX_EXTBUFF_EXPORT_SHARING_DESC_OCL;
export_header_buf.ocl_context        = GetOpenCLContext();
export_header_buf.ocl_command_queue  = GetOpenCLCommandQueue();
export_header_buf.Header.BufferSz    = sizeof(mfxExtSurfaceOpenCLImg2DExportDescription);

// attach the extension buffer to mfxSurfaceHeader
std::vector<mfxExtBuffer *> extBufs;
extBufs.push_back((mfxExtBuffer *)&export_header_buf);
export_header.ExtParam = extBufs.data();

// export mfxFrameSurface1 from Intel® VPL to an OpenCL image, zero-copy enabled
mfxSurfaceOpenCLImg2D *extSurfOCL = nullptr;
sts = pmfxVPPOutSurface->FrameInterface->Export(pmfxVPPOutSurface, export_header, (mfxSurfaceHeader **)&extSurfOCL);

// access the exported OpenCL image
// in this example the video format = NV12 and consists of two planes (0 = Y and 1 = interleaved UV)
cl_mem clBufInY = (cl_mem)extSurfOCL->ocl_image[0]
cl_mem clBufInUV = (cl_mem)extSurfOCL->ocl_image[1];

// lock the shared surface for OCL exclusive use
cl_mem surfaces[2] = { clBufInY, clBufInUV };
clError = m_pfn_clEnqueueAcquireD3D11ObjectsKHR(m_clqueue, 2, surfaces, 0, NULL, NULL);

// pass the exported images as arguments to OpenCL kernels
clError = clSetKernelArg(m_clkernelY, 0, sizeof(cl_mem), &clBufInY);
clError = clSetKernelArg(m_clkernelUV, 0, sizeof(cl_mem), &clBufInUV);

// enqueue the kernels for processing
cl_event kernelEvents[2] = {};
clError = clEnqueueNDRangeKernel(m_clqueue, m_clkernelY, 2, NULL, m_GlobalWorkSizeY, m_LocalWorkSizeY, 0, NULL, &kernelEvents[0]);
clError = clEnqueueNDRangeKernel(m_clqueue, m_clkernelUV, 2, NULL, m_GlobalWorkSizeUV, m_LocalWorkSizeUV, 0, NULL, &kernelEvents[1]);

// wait for kernels to finish (also flushes command queue)
clError = clWaitForEvents(sizeof(kernelEvents) / sizeof(kernelEvents[0]), kernelEvents);

// unlock the shared exported surface from OCL exclusive use
clError = pfn_clEnqueueReleaseD3D11ObjectsKHR(m_clqueue, 2, surfaces, 0, NULL, NULL);

// release the exported surface
sts = extSurfOCL->SurfaceInterface.Release(&(extSurfOCL->SurfaceInterface));

// release VPP output frame
pmfxVPPOutSurface->FrameInterface->Release(pmfxVPPOutSurface);```
```

### Importing an OpenCL image as a Intel® VPL frame for encoding
- Note: Currently supported on Windows only.
```
// get interface with import function (only call once per session)
mfxMemoryInterface *memoryInterface = nullptr;
sts = MFXGetMemoryInterface(session, &memoryInterface);

// allocate an OCL surface (NV12) the first time it's needed

// Y plane
cl_mem clBufInY = nullptr;
cl_image_format src_format   = {};
cl_image_desc src_image_desc = {};

src_format.image_channel_order     = CL_R;
src_format.image_channel_data_type = CL_UNORM_INT8;
src_image_desc.image_width  = width;
src_image_desc.image_height = height;
src_image_desc.image_type   = CL_MEM_OBJECT_IMAGE2D;
clBufInY = clCreateImage(m_clcontext, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, &src_format, &src_image_desc, 0, &error);

// UV plane
cl_mem clBufInUV = nullptr;
src_format.image_channel_order     = CL_RG;
src_format.image_channel_data_type = CL_UNORM_INT8;
src_image_desc.image_width /= 2;
src_image_desc.image_height /= 2;
clBufInUV = clCreateImage(m_clcontext, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, &src_format, &src_image_desc, 0, &error);

// now load data into the OpenCL image and do some processing...
// (see the Export example for main steps in running kernels)

// once OpenCL processing is complete, import the surface

// set header and ocl_context, ocl_flags is optional, all other fields should be empty
mfxSurfaceOpenCLImg2D extSurfOCL               = {};
extSurfOCL.SurfaceInterface.Header.SurfaceType = MFX_SURFACE_TYPE_OPENCL_IMG2D;

// Specify either SHARED or COPY as permitted import modes
// NOTE: we expect that runtime only supports copy mode in current implementation
// After calling ImportFrameSurface, we can check extSurfOCL.SurfaceInterface.Header.SurfaceFlags to see what
//   import mode was actually used by the runtime.
extSurfOCL.SurfaceInterface.Header.SurfaceFlags = (MFX_SURFACE_FLAG_IMPORT_SHARED | MFX_SURFACE_FLAG_IMPORT_COPY);
extSurfOCL.SurfaceInterface.Header.StructSize   = sizeof(mfxSurfaceOpenCLImg2D);

// pass the OpenCL context and command queue which were initialized by the application
extSurfOCL.ocl_context       = GetOpenCLContext();
extSurfOCL.ocl_command_queue = GetOpenCLCommandQueue();

extSurfOCL.ocl_image_num = 2;
extSurfOCL.ocl_image[0]  = clBufInY;
extSurfOCL.ocl_image[1]  = clBufInUV;

// import the OpenCL surface
mfxFrameSurface1 *pmfxEncSurface = nullptr;
sts = memoryInterface->ImportFrameSurface(encCtx->memoryInterface, MFX_SURFACE_COMPONENT_ENCODE, (mfxSurfaceHeader *)(&extSurfOCL), &pmfxEncSurface);

// encode
sts = MFXVideoENCODE_EncodeFrameAsync(session, NULL, (isDrainingEnc ? NULL : pmfxEncSurface), &fileInfo->bitstream, &syncp);

// release OCL surfaces (decrease refcount)
clReleaseMemObject(clBufInY);
clReleaseMemObject(clBufInUV);

// release imported surface
pmfxEncSurface->FrameInterface->Release(pmfxEncSurface);
```

## Reference counting and release of imported surfaces

The following describes details the proper release procedure and sequence for surface in import/shared mode

If resources required for importing were created by user, then user should release it.

* The exact moment of possible deallocation of the original surface is dependent on the type of Import and the type of surface. For some types (i.e. for those who support refcounts - DXVA), it is possible to release right after successful Import.

  For DXVA, GPU RT adds refcount on imported d3d11 texture, so user can release original texture right after import.

  For VAAPI, user can’t delete original VAsurface, because it will obviously invalidate imported surface. So user must wait till destruction of imported `mfxFrameSurface1` first.

For example,
```
surfaceForImport.vaSurfaceID = vaSurfaceID;
...
m_memoryInterface->ImportFrameSurface(m_memoryInterface,  MFX_SURFACE_COMPONENT_ENCODE,  &surfaceForImport.SurfaceInterface.Header,  &pmfxEncSurface);
```

In general, user should destroy `surfaceForImport` after destruction of `pmfxEncSurface` (it is destroyed when refcount reaches zero, refcount decreased by `Release()` calls) then it is 100% safe.

But in some cases, like DXVA textures, user may destroy surfaceForImport earlier.

When Encoder does frame reordering (it happens, when there are B-frames in GOP), then user must keep reordered mfxFrameSurfaces alive, until they are encoded. To understand when it is safe to delete the mfxFrameSurface – you need to check value of `mfxFrameSurface1::Data::Locked`.

When it is `0`, then component doesn’t need this mfxFrameSurface anymore. So it is safe to delete or, much better, reuse it.

It is essential for legacy allocation model (external allocator), when we managing surface pool by hands.
Also it can be used for Shared imported native handles without refcounts (like VAsurfaces), i.e. it is right moment to release `mfxFrameSurface1` object then native handle. Order matters, original handle should be destroyed after `mfxFrameSurface1` object.

For refcounted case management of `mfxFrameSurface1` objects is much simpler. Encoder (Decoder and VPP do the same) will `AddRef()` surface inside, so it is safe to `Release()` `mfxFrameSurface1` object right after submission disregard the returned status. `mfxFrameSurface1` will not be deleted, because Encoder holds it.

Important note: there is no dependency between original handle and `mfxFrameSurface1` object for copy imported surface. They could be managed (i.e. deleted) independently.

For Export case (for both Copy and Shared type) original `mfxFrameSurface1` and Exported `mfxSurfaceXXX` can be managed (i.e. `Release()`) independently, Intel® VPL RT manages dependency and keeps all required resources alive.
