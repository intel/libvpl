.. SPDX-FileCopyrightText: 2019-2020 Intel Corporation
..
.. SPDX-License-Identifier: CC-BY-4.0

===========================================
|vpl_short_name| for |msdk_full_name| Users
===========================================

|vpl_short_name| is source compatible with |msdk_full_name|. Applications can use
|msdk_full_name| to target older hardware and |vpl_short_name| to target everything else.
Some obsolete features of |msdk_full_name| have been omitted from |vpl_short_name|.
Hereinafter the term "Legacy" will be used to describe a behavior when |vpl_short_name| 
is called by |msdk_full_name| applications.

-----------------------------------------
|vpl_short_name| Ease of Use Enhancements
-----------------------------------------

|vpl_short_name| provides improved ease of use compared to |msdk_full_name|. Ease of use
enhancements include the following:

- Smart dispatcher with discovery of implementation capabilities. See
  :ref:`sdk-session` for more details.
- Simplified decoder initialization. See :ref:`decoding-proc` for more
  details.
- New memory management and components (session) interoperability. See
  :ref:`internal-mem-manage` and :ref:`decoding-proc` for more details.

.. _new-api:

----------------------------
New APIs in |vpl_short_name|
----------------------------

|vpl_short_name| introduces new functions that are not available in |msdk_full_name|.

.. _dispatcher-api:

New |vpl_short_name| dispatcher functions:

- :cpp:func:`MFXLoad`
- :cpp:func:`MFXUnload`
- :cpp:func:`MFXCreateConfig`
- :cpp:func:`MFXSetConfigFilterProperty`
- :cpp:func:`MFXEnumImplementations`
- :cpp:func:`MFXCreateSession`
- :cpp:func:`MFXDispReleaseImplDescription`

New |vpl_short_name| memory management functions:

- :cpp:func:`MFXMemory_GetSurfaceForVPP`
- :cpp:func:`MFXMemory_GetSurfaceForVPPOut`
- :cpp:func:`MFXMemory_GetSurfaceForEncode`
- :cpp:func:`MFXMemory_GetSurfaceForDecode`

New |vpl_short_name| implementation capabilities retrieval functions:

- :cpp:func:`MFXQueryImplsDescription`
- :cpp:func:`MFXReleaseImplDescription`

New |vpl_short_name| session initialization:

- :cpp:func:`MFXInitialize`

---------------------------------
|msdk_full_name| Feature Removals
---------------------------------

The following |msdk_full_name| features are considered obsolete and are not
included in |vpl_short_name|:

- **Audio support.** |vpl_short_name| is intended for video processing. Audio APIs that
  duplicate functionality from other audio libraries such as
  `Sound Open Firmware <https://github.com/thesofproject>`__ have been removed.

- **ENC and PAK interfaces.** Part of the Flexible Encode Infrastructure (FEI)
  and plugin interfaces which provide additional control over the encoding
  process for AVC and HEVC encoders. This feature was removed because it is not
  widely used by customers.

- **User plugins architecture.** |vpl_short_name| enables robust video acceleration through
  API implementations of many different video processing frameworks. Support of
  a |msdk_full_name| user plugin framework is obsolete. |msdk_full_name| RAW acceleration (Camera API)
  which is implemented as plugin is also obsolete, |vpl_short_name| enables RAW acceleration (Camera
  API) through |vpl_short_name| runtime such as |vpl_short_name| GPU runtime.

- **External buffer memory management.** A set of callback functions to replace
  internal memory allocation is obsolete.

- **Video Processing extended runtime functionality.** Video processing function
  MFXVideoVPP_RunFrameVPPAsyncEx is used for plugins only and is obsolete.

- **External threading.** The new threading model makes the MFXDoWork function
  obsolete.

- **Multi-frame encode.** A set of external buffers to combine several frames
  into one encoding call. This feature was removed because it is device specific
  and not commonly used.

- **Surface Type Neutral Transcoding.** Opaque memory support is removed and 
  replaced with internal memory allocation concept.   

- **Raw Acceleration.** |msdk_full_name| RAW acceleration (Camera API) which is
  implemented as plugin is obsolete, replaced by |vpl_short_name| and |vpl_short_name| runtime implementation.
  |vpl_short_name| reused most of |msdk_full_name| Camera API, but |vpl_short_name| camera API is not backward
  compatible with |msdk_full_name| camera API due to obsolete plugin mechanism in |vpl_short_name| 
  and a few difference between |vpl_short_name| and |msdk_full_name|. The major difference between |vpl_short_name| 
  and |msdk_full_name| are listed:
  1) mfxCamGammaParam and mfxExtCamGammaCorrection are removed in |vpl_short_name|; 2) Added
  reserved bytes in mfxExtCamHotPixelRemoval, mfxCamVignetteCorrectionParam and
  mfxCamVignetteCorrectionElement for future extension; 3) Changed CCM from mfxF64
  to mfxF32 in mfxExtCamColorCorrection3x3 and added more reserved bytes.

.. _deprecated-api:

-----------------------------
|msdk_full_name| API Removals
-----------------------------

The following |msdk_full_name| functions are not included in |vpl_short_name|:

- **Audio related functions**

  - MFXAudioCORE_SyncOperation()
  - MFXAudioDECODE_Close()
  - MFXAudioDECODE_DecodeFrameAsync()
  - MFXAudioDECODE_DecodeHeader()
  - MFXAudioDECODE_GetAudioParam()
  - MFXAudioDECODE_Init()
  - MFXAudioDECODE_Query()
  - MFXAudioDECODE_QueryIOSize()
  - MFXAudioDECODE_Reset()
  - MFXAudioENCODE_Close()
  - MFXAudioENCODE_EncodeFrameAsync()
  - MFXAudioENCODE_GetAudioParam()
  - MFXAudioENCODE_Init()
  - MFXAudioENCODE_Query()
  - MFXAudioENCODE_QueryIOSize()
  - MFXAudioENCODE_Reset()

- **Flexible encode infrastructure functions**

  - MFXVideoENC_Close()
  - MFXVideoENC_GetVideoParam()
  - MFXVideoENC_Init()
  - MFXVideoENC_ProcessFrameAsync()
  - MFXVideoENC_Query()
  - MFXVideoENC_QueryIOSurf()
  - MFXVideoENC_Reset()
  - MFXVideoPAK_Close()
  - MFXVideoPAK_GetVideoParam()
  - MFXVideoPAK_Init()
  - MFXVideoPAK_ProcessFrameAsync()
  - MFXVideoPAK_Query()
  - MFXVideoPAK_QueryIOSurf()
  - MFXVideoPAK_Reset()

- **User plugin functions**

  - MFXAudioUSER_ProcessFrameAsync()
  - MFXAudioUSER_Register()
  - MFXAudioUSER_Unregister()
  - MFXVideoUSER_GetPlugin()
  - MFXVideoUSER_ProcessFrameAsync()
  - MFXVideoUSER_Register()
  - MFXVideoUSER_Unregister()
  - MFXVideoUSER_Load()
  - MFXVideoUSER_LoadByPath()
  - MFXVideoUSER_UnLoad()
  - MFXDoWork()

- **Memory functions**

  - MFXVideoCORE_SetBufferAllocator()

- **Video processing functions**

  - MFXVideoVPP_RunFrameVPPAsyncEx()

- **Memory type and IOPattern enumerations**
  
  - MFX_IOPATTERN_IN_OPAQUE_MEMORY
  - MFX_IOPATTERN_OUT_OPAQUE_MEMORY
  - MFX_MEMTYPE_OPAQUE_FRAME

.. important:: Corresponding extension buffers are also removed.

The following behaviors occur when attempting to use a |msdk_full_name| API that
is not supported by |vpl_short_name|:

- Code compiled with the |vpl_short_name| API headers will generate a compile and/or
  link error when attempting to use a removed API.

- Code previously compiled with |msdk_full_name| and executed using a |vpl_short_name|
  runtime will generate an :cpp:enumerator:`MFX_ERR_NOT_IMPLEMENTED` error when
  calling a removed function.

---------------------------
|msdk_full_name| Legacy API
---------------------------

|vpl_short_name| contains following header files from |msdk_full_name| included for the
simplification of existing applications migration to |vpl_short_name|:

- mfxvideo++.h

.. important:: |msdk_full_name| obsolete API removed from those header files.
   Code compiled with the |vpl_short_name| API headers will generate a compile and/or
   link error when attempting to use a removed API.
