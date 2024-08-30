.. SPDX-FileCopyrightText: 2021 Intel Corporation
..
.. SPDX-License-Identifier: CC-BY-4.0

==========
Change Log
==========

This section describes the API evolution from version to version.

.. contents::
   :local:
   :depth: 1

------------
Version 2.14
------------

New in this release:

* Added decode HEVC level8.5 (general_level_idc=255) support:

    * :cpp:enumerator:`MFX_LEVEL_HEVC_85`

* Experimental API: Introduced best quality and best speed mode for AI based video frame interpolation:

    * :cpp:enumerator:`MFX_AI_FRAME_INTERPOLATION_MODE_BEST_SPEED`
    * :cpp:enumerator:`MFX_AI_FRAME_INTERPOLATION_MODE_BEST_QUALITY`

* Updated description for decode FrameRate in :cpp:struct:`mfxFrameInfo`

* Experimental API: Introduced sharpen, artifact removal mode and algorithm setting for AI based super resolution:

    * :cpp:enumerator:`MFX_AI_SUPER_RESOLUTION_MODE_SHARPEN`
    * :cpp:enumerator:`MFX_AI_SUPER_RESOLUTION_MODE_ARTIFACTREMOVAL`
    * :cpp:enum:`mfxAISuperResolutionAlgorithm`

* Made mfxvideo++.h session wrappers explicitly non-copyable:

    * MFXVideoENCODE
    * MFXVideoDECODE
    * MFXVideoVPP
    * MFXVideoDECODE_VPP

------------
Version 2.13
------------

New in this release:

* Moved to production API:

    * :cpp:struct:`mfxExtQualityInfoMode`
    * :cpp:struct:`mfxExtQualityInfoOutput`
    * :cpp:enum:`mfxQualityInfoMode`
    * :cpp:struct:`mfxExtAV1ScreenContentTools`
    * :cpp:struct:`mfxExtAlphaChannelEncCtrl`
    * :cpp:struct:`mfxExtAlphaChannelSurface`
    * :cpp:struct:`mfxExtVPPAISuperResolution`
    * :cpp:enum:`mfxAISuperResolutionMode`
    * :cpp:struct:`mfxExtVPPAIFrameInterpolation`
    * :cpp:enum:`mfxAIFrameInterpolationMode`

* Added code name for the platform: Battlemage:

    * :cpp:enum:`MFX_DEPRECATED_ENUM_FIELD_INSIDE(MFX_PLATFORM_BATTLEMAGE)`

------------
Version 2.12
------------

New in this release:

* Experimental API: Added new structures and enumerated types for importing and exporting surfaces:

    * :cpp:struct:`mfxSurfaceVulkanImg2D`
    * :cpp:struct:`mfxExtSurfaceVulkanImg2DExportDescription`

* Experimental API: introduced :cpp:struct:`mfxExtVPPAIFrameInterpolation` and :cpp:enum:`mfxAIFrameInterpolationMode` for AI powered frame interpolation.

* Added code names for the platforms: Meteor Lake, Arrow Lake and Lunar Lake:

    * :cpp:enum:`MFX_DEPRECATED_ENUM_FIELD_INSIDE(MFX_PLATFORM_METEORLAKE)`
    * :cpp:enum:`MFX_DEPRECATED_ENUM_FIELD_INSIDE(MFX_PLATFORM_LUNARLAKE)`
    * :cpp:enum:`MFX_DEPRECATED_ENUM_FIELD_INSIDE(MFX_PLATFORM_ARROWLAKE)`

* Added code name for future platforms:

    * :cpp:enum:`MFX_DEPRECATED_ENUM_FIELD_INSIDE(MFX_PLATFORM_MAXIMUM)`

* Updated description for the function :cpp:func:`MFXVideoCORE_QueryPlatform`.

------------
Version 2.11
------------

New in this release:

* Experimental API: introduced :cpp:struct:`mfxExtAV1ScreenContentTools` to support screen content tools setting for encoder.
* Experimental API: introduced :cpp:struct:`mfxExtAlphaChannelEncCtrl` and :cpp:struct:`mfxExtAlphaChannelSurface` for alpha channel encoding.
* Experimental API: introduced :cpp:struct:`mfxExtQualityInfoMode`, :cpp:struct:`mfxExtQualityInfoOutput` and :cpp:enum:`mfxQualityInfoMode` to report quality information about encoded picture.
* Experimental API: introduced :cpp:struct:`mfxExtVPPAISuperResolution` and :cpp:enum:`mfxAISuperResolutionMode` for AI enhanced video processing filter super resolution.
* Experimental API: introduced :cpp:enum:`mfx3DLutInterpolationMethod` to support 3DLUT interpolation method.
* Experimental API: Added :cpp:enumerator:`MFX_GPUCOPY_FAST`.
* Experimental API: Added new structures and enumerated types for importing and exporting surfaces:

    * :cpp:struct:`mfxSurfaceD3D12Tex2D`
    * :cpp:struct:`mfxExtSurfaceD3D12Tex2DExportDescription`

* Supported the extraction of AV1 HDR metadata in the decoder.
* Updated description of :cpp:struct:`mfxExtAVCRefListCtrl` to clarify process for querying encoder support.
* Clarified the :cpp:member:`mfxExtCodingOption2::MaxQPI`, :cpp:member:`mfxExtCodingOption2::MaxQPP`, :cpp:member:`mfxExtCodingOption2::MaxQPB`, :cpp:member:`mfxExtCodingOption2::MinQPI`, :cpp:member:`mfxExtCodingOption2::MinQPP`, :cpp:member:`mfxExtCodingOption2::MinQPB` for the video encoding usage.
* Added VVC decode API.
* Update notes on :cpp:member:`mfxInfoMFX::QPI`, :cpp:member:`mfxInfoMFX::QPP`, :cpp:member:`mfxInfoMFX::QPB` to clarify to valid range.
* Moved to production API:

    * :cpp:struct:`mfxConfigInterface`
    * :cpp:enumerator:`mfxHandleType::MFX_HANDLE_CONFIG_INTERFACE`
    * :cpp:enum:`mfxStructureType`
    * :cpp:enumerator:`mfxStatus::MFX_ERR_MORE_EXTBUFFER`

* Experimental API: Removed ``mfxAutoSelectImplDeviceHandle`` and ``mfxAutoSelectImplType``
* Experimental API: Removed ``mfxExtSyncSubmission``

------------
Version 2.10
------------

New in this release:

* Experimental API: introduced :cpp:enumerator:`MFX_CORRUPTION_HW_RESET` to support media reset info report.
* Changed :cpp:enumerator:`MFX_ENCODE_TUNE_DEFAULT` to :cpp:enumerator:`MFX_ENCODE_TUNE_OFF`.
* Experimental API: Removed CPUEncToolsProcessing hint. No need to have explicit parameter. The decision to enable encoding tools will be made according to encoding parameters.
* Extended behavior of fused decode plus VPP operation to disable implicit scaling.
* Added alias :cpp:type:`mfxExtEncodedFrameInfo` as codec-independent version of :cpp:struct:`mfxExtAVCEncodedFrameInfo`.
* Updated description of :cpp:func:`MFXSetConfigFilterProperty` to permit multiple properties per config object.
* Fixed 3DLUT buffer size(system memory) in programming guide.
* Clarified Region of Interest Parameters Setting for dynamic change.
* Removed current working directory from the implementation search path.
* Updated argument names and description of MFX_UUID_COMPUTE_DEVICE_ID macro.
* Added new header file mfxmemory.h, which is automatically included by mfxvideo.h. Moved the following function declarations from mfxvideo.h to mfxmemory.h

    * :cpp:func:`MFXMemory_GetSurfaceForEncode`
    * :cpp:func:`MFXMemory_GetSurfaceForDecode`
    * :cpp:func:`MFXMemory_GetSurfaceForVPP`
    * :cpp:func:`MFXMemory_GetSurfaceForVPPOut`

* Experimental API: Introduced new interface for importing and exporting surfaces. Added new function :cpp:member:`mfxFrameSurfaceInterface::Export`. Added new structures and enumerated types:

    * :cpp:struct:`mfxMemoryInterface`
    * :cpp:enumerator:`mfxHandleType::MFX_HANDLE_MEMORY_INTERFACE`
    * :cpp:enum:`mfxSurfaceComponent`
    * :cpp:enum:`mfxSurfaceType`
    * :cpp:struct:`mfxSurfaceHeader`
    * :cpp:struct:`mfxSurfaceInterface`
    * :cpp:struct:`mfxSurfaceD3D11Tex2D`
    * :cpp:struct:`mfxSurfaceVAAPI`
    * :cpp:struct:`mfxSurfaceOpenCLImg2D`
    * :cpp:struct:`mfxExtSurfaceOpenCLImg2DExportDescription`

* Experimental API: Introduced capabilities query for supported surface import and export operations. Added new structures and enumerated types:

    * :cpp:enumerator:`mfxImplCapsDeliveryFormat::MFX_IMPLCAPS_SURFACE_TYPES`
    * :cpp:struct:`mfxSurfaceTypesSupported`

* Experimental API: Introduced new interface for configuring initialization parameters. Added new structures and enumerated types:

    * :cpp:struct:`mfxConfigInterface`
    * :cpp:enumerator:`mfxHandleType::MFX_HANDLE_CONFIG_INTERFACE`
    * :cpp:enum:`mfxStructureType`
    * :cpp:enumerator:`mfxStatus::MFX_ERR_MORE_EXTBUFFER`

* Experimental API: previously released experimental features were moved to production . See
  :ref:`experimental-api-table` for more details.

* Not supported in the encoding of VDEnc or LowPower ON:

    * :cpp:enumerator:`CodecProfile::MFX_PROFILE_AVC_MULTIVIEW_HIGH`
    * :cpp:enumerator:`CodecProfile::MFX_PROFILE_AVC_STEREO_HIGH`

-----------
Version 2.9
-----------

New in this release:

* Deprecated :cpp:member:`mfxExtCodingOption2::BitrateLimit`.
* Added note that applications must call MFXVideoENCODE_Query() to check for support of :cpp:struct:`mfxExtChromaLocInfo` and :cpp:struct:`mfxExtHEVCRegion` extension buffers.
* Added AV1 HDR metadata description and further clarified :cpp:struct:`mfxExtMasteringDisplayColourVolume` and :cpp:struct:`mfxExtContentLightLevelInfo`.
* Added deprecation messages to the functions :cpp:func:`MFXQueryAdapters`, :cpp:func:`MFXQueryAdaptersDecode`, and :cpp:func:`MFXQueryAdaptersNumber`.
  Applications should use the process described in :ref:`vpl-dispatcher` to enumerate and select adapters.
* Fixed multiple spelling errors.
* Added extension buffer ``mfxExtSyncSubmission`` to return submission synchronization sync point.
* Added extension buffer :cpp:struct:`mfxExtVPPPercEncPrefilter` to control perceptual encoding prefilter.
* Deprecated :cpp:member:`mfxPlatform::CodeName` and corresponding enum values.
* Added :cpp:member:`mfxExtendedDeviceId::RevisionID` and :cpp:struct:`extDeviceUUID` to be aligned across multiple domains including compute and specify device UUID accordingly.
* Added extension buffer :cpp:struct:`mfxExtTuneEncodeQuality` and correspondent enumeration to specify encoding tuning option.
* Updated description of :cpp:func:`MFXEnumImplementations` to clarify that the input :cpp:type:`mfxImplCapsDeliveryFormat` determines the type of structure returned.
* Updated mfxvideo++.h to use MFXLoad API.
* Added ``mfxAutoSelectImplDeviceHandle`` and ``mfxAutoSelectImplType`` for automatically selecting a suitable implementation based on application-provided device handle.


-----------
Version 2.8
-----------

New in this release:

* Introduced :cpp:enumerator:`MFX_FOURCC_ABGR16F` FourCC for 16-bit float point (per channel) 4:4:4 ABGR format.
* Clarified the :cpp:member:`mfxExtMasteringDisplayColourVolume::DisplayPrimariesX`, :cpp:member:`mfxExtMasteringDisplayColourVolume::DisplayPrimariesY` for the video processing usage.
* Added :cpp:enumerator:`MFX_CONTENT_NOISY_VIDEO` in ContentInfo definition.
* Added Camera Processing API for Camera RAW data.
* Introduced hint to disable external video frames caching for GPU copy.
* Clarified usage of :cpp:member:`mfxExtMasteringDisplayColourVolume::InsertPayloadToggle` and
  :cpp:member:`mfxExtContentLightLevelInfo::InsertPayloadToggle` during decode operations.
* Fixed multiple spelling errors.
* Experimental API: introduced :cpp:member:`mfxExtMBQP::Pitch` value for QP map defined in :cpp:struct:`mfxExtMBQP`.
* Clarified when :cpp:func:`MFXEnumImplementations` may be called for implementation capabilities query.
* Added table with filenames included in the dispatcher's search process.

Bug Fixes:

* Fixed :ref:`Experimental API table <experimental-api-table>` to note that :cpp:type:`mfxExtRefListCtrl` and :cpp:enumerator:`MFX_EXTBUFF_UNIVERSAL_REFLIST_CTRL` were moved to production in version 2.8.

-----------
Version 2.7
-----------

New in this release:

* :cpp:member:`mfxExtVppAuxData::RepeatedFrame` flag is actual again and returned back from deprecation state.
* Clarified GPUCopy control behavior.
* Introduced MFX_FOURCC_XYUV FourCC for non-alpha packed 4:4:4 format.
* Notice added to the :cpp:member:`mfxFrameSurfaceInterface::OnComplete` to clarify when
  library can call this callback.
* New product names for platforms:

    * Code name Alder Lake N.

* Annotated missed aliases  :cpp:type:`mfxExtHEVCRefListCtrl`,  :cpp:type:`mfxExtHEVCRefLists`, :cpp:type:`mfxExtHEVCTemporalLayers`.
* New dispatcher's config properties:

    * Pass through extension buffer to :cpp:struct:`mfxInitializationParam`.
    * Select host or device responsible for the memory copy between host and device.

* Refined description of struct `mfxExtMasteringDisplayColourVolume` and `mfxExtContentLightLevelInfo` for HDR SEI decoder usage.
* Experimental API: introduced interface to get statistics after encode.

Bug Fixes:

* Fixed missprint in the :cpp:struct:`mfxExtDeviceAffinityMask` description.
* MFXVideoENCODE_Query description fixed for query mode 1.

-----------
Version 2.6
-----------

New in this release:

* New development practice to treat some new API features as experimental was introduced.
  All new experimental API is wrapped with ONE_EXPERIMENTAL macro.
* Experimental API: introduced MFX_HANDLE_PXP_CONTEXT to support protected content.
* Experimental API: introduced CPUEncToolsProcessing hint to run adaptive encoding tools on CPU.
* Experimental API: extended device ID reporting to cover multi-adapter cases.
* Experimental API: introduced common alias for mfxExtAVCRefListCtrl
* Experimental API: mfxExtDecodeErrorReport ErrorTypes enum extended with new JPEG/MJPEG decode error report.
* Clarified LowPower flag meaning.
* Described that mfxExtThreadsParam can be attached to mfxInitializationParam during session initialization.
* Refined description of the MFXVideoDECODE_VPP_DecodeFrameAsync function.
* New dispatcher's config filter property: MediaAdapterType.
* Marked all deprecated fields as MFX_DEPRECATED.
* Introduced priority loading option for custom libraries. 
* Clarified AV1 encoder behavior about writing of IVF headers.
* Removed outdated note about loading priority of |msdk_full_name|. For loading details see 
  :ref:`vpl_coexistense`.
* Spelled out mfxVariant type usage for strings.
* New product names for platforms:

    * Code name DG2,
    * Code name ATS-M.

-----------
Version 2.5
-----------

New in this release:

* Added mfxMediaAdapterType to capability reporting.
* Added surface pool interface.
* Helper macro definition to simplify filter properties set up process
  for dispatcher.
* Added mfxExtAV1BitstreamParam, mfxExtAV1ResolutionParam and mfxExtAV1TileParam for AV1e.
* Added MFX_RESOURCE_VA_SURFACE_PTR and MFX_RESOURCE_VA_BUFFER_PTR enumerators.
* Clarified HEVC Main 10 Still Picture Profile configuration.
* External Buffer ID of mfxExtVideoSignalInfo and mfxExtMasteringDisplayColourVolume for video processing.
* New MFX_WRN_ALLOC_TIMEOUT_EXPIRED return status. Indicates that all surfaces are currently
  in use and timeout set by mfxExtAllocationHints for allocation of new surfaces through
  functions GetSurfaceForXXX expired.
* Introduced universal temporal layering structure.
* Added MFX_RESOURCE_VA_SURFACE_PTR and MFX_RESOURCE_VA_BUFFER_PTR enumerators. 
* Introduced segmentation interface for AV1e, including ext-buffers and enums.
* Introduced planar I422 and I210 FourCC codes.

Bug Fixes:

* Dispatcher: Removed /etc/ld.so.cache from |vpl_short_name| search order.
* mfxSurfaceArray: CDECL attribute added to the member-functions.

Deprecated:

* mfxExtVPPDenoise extension buffer.


-----------
Version 2.4
-----------

* Added ability to retrieve path to the shared library with the implementation.
* Added 3DLUT (Three-Dimensional Look Up Table) filter in VPP.
* Added mfxGUID structure to specify Globally Unique Identifiers (GUIDs).
* Added QueryInterface function to mfxFrameSurfaceInterface.
* Added AdaptiveRef and alias for ExtBrcAdaptiveLTR.
* Added MFX_FOURCC_BGRP FourCC for Planar BGR format.
* Environmental variables to control dispatcher's logger.

-----------
Version 2.3
-----------

* Encoding in Hyper mode.
* New product names for platforms:

    * Code name Rocket Lake,
    * Code name Alder Lake S,
    * Code name Alder Lake P,
    * Code name for Arctic Sound P.
    * For spec version 2.3.1 MFX_PLATFORM_XEHP_SDV alias was added

* mfx.h header file is added which includes all header files.
* Added deprecation messages (deprecation macro) to the functions MFXInit and
  MFXInitEx functions definition.
