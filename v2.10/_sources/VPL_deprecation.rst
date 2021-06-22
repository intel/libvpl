.. SPDX-FileCopyrightText: 2021 Intel Corporation
..
.. SPDX-License-Identifier: CC-BY-4.0

==============
Deprecated API
==============

The following is a list of deprecated interfaces, starting from API version 2.0.


.. list-table:: Deprecated API
   :header-rows: 1
   :widths: auto

   * - **API**
     - **Deprecated in API Version**
     - **Removed in API Version**
     - **Alternatives**
     - **Behaviour change of deprecated API**
   * - :cpp:func:`MFXQueryAdapters`
     - 2.9
     -
     - :cpp:func:`MFXEnumImplementations` + :cpp:func:`MFXSetConfigFilterProperty`
     - No change
   * - :cpp:func:`MFXQueryAdaptersDecode`
     - 2.9
     -
     - :cpp:func:`MFXEnumImplementations` + :cpp:func:`MFXSetConfigFilterProperty`
     - No change
   * - :cpp:func:`MFXQueryAdaptersNumber`
     - 2.9
     -
     - :cpp:func:`MFXEnumImplementations` + :cpp:func:`MFXSetConfigFilterProperty`
     - No change
   * - :cpp:member:`mfxExtCodingOption2::BitrateLimit`
     - 2.9
     -
     - Flag is ignored on VDEnc
     - No change
   * - :cpp:enumerator:`MFX_PLATFORM_UNKNOWN`
     - 2.9
     -
     - Field :cpp:member:`mfxPlatform::CodeName` is not filled by the library
     - No change
   * - :cpp:enumerator:`MFX_PLATFORM_SANDYBRIDGE`
     - 2.9
     -
     - Field :cpp:member:`mfxPlatform::CodeName` is not filled by the library
     - No change
   * - :cpp:enumerator:`MFX_PLATFORM_IVYBRIDGE`
     - 2.9
     -
     - Field :cpp:member:`mfxPlatform::CodeName` is not filled by the library
     - No change
   * - :cpp:enumerator:`MFX_PLATFORM_HASWELL`
     - 2.9
     -
     - Field :cpp:member:`mfxPlatform::CodeName` is not filled by the library
     - No change
   * - :cpp:enumerator:`MFX_PLATFORM_BAYTRAIL`
     - 2.9
     -
     - Field :cpp:member:`mfxPlatform::CodeName` is not filled by the library
     - No change
   * - :cpp:enumerator:`MFX_PLATFORM_BROADWELL`
     - 2.9
     -
     - Field :cpp:member:`mfxPlatform::CodeName` is not filled by the library
     - No change
   * - :cpp:enumerator:`MFX_PLATFORM_CHERRYTRAIL`
     - 2.9
     -
     - Field :cpp:member:`mfxPlatform::CodeName` is not filled by the library
     - No change
   * - :cpp:enumerator:`MFX_PLATFORM_SKYLAKE`
     - 2.9
     -
     - Field :cpp:member:`mfxPlatform::CodeName` is not filled by the library
     - No change
   * - :cpp:enumerator:`MFX_PLATFORM_APOLLOLAKE`
     - 2.9
     -
     - Field :cpp:member:`mfxPlatform::CodeName` is not filled by the library
     - No change
   * - :cpp:enumerator:`MFX_PLATFORM_KABYLAKE`
     - 2.9
     -
     - Field :cpp:member:`mfxPlatform::CodeName` is not filled by the library
     - No change
   * - :cpp:enumerator:`MFX_PLATFORM_GEMINILAKE`
     - 2.9
     -
     - Field :cpp:member:`mfxPlatform::CodeName` is not filled by the library
     - No change
   * - :cpp:enumerator:`MFX_PLATFORM_COFFEELAKE`
     - 2.9
     -
     - Field :cpp:member:`mfxPlatform::CodeName` is not filled by the library
     - No change
   * - :cpp:enumerator:`MFX_PLATFORM_CANNONLAKE`
     - 2.9
     -
     - Field :cpp:member:`mfxPlatform::CodeName` is not filled by the library
     - No change
   * - :cpp:enumerator:`MFX_PLATFORM_ICELAKE`
     - 2.9
     -
     - Field :cpp:member:`mfxPlatform::CodeName` is not filled by the library
     - No change
   * - :cpp:enumerator:`MFX_PLATFORM_JASPERLAKE`
     - 2.9
     -
     - Field :cpp:member:`mfxPlatform::CodeName` is not filled by the library
     - No change
   * - :cpp:enumerator:`MFX_PLATFORM_ELKHARTLAKE`
     - 2.9
     -
     - Field :cpp:member:`mfxPlatform::CodeName` is not filled by the library
     - No change
   * - :cpp:enumerator:`MFX_PLATFORM_TIGERLAKE`
     - 2.9
     -
     - Field :cpp:member:`mfxPlatform::CodeName` is not filled by the library
     - No change
   * - :cpp:enumerator:`MFX_PLATFORM_ROCKETLAKE`
     - 2.9
     -
     - Field :cpp:member:`mfxPlatform::CodeName` is not filled by the library
     - No change
   * - :cpp:enumerator:`MFX_PLATFORM_ALDERLAKE_S`
     - 2.9
     -
     - Field :cpp:member:`mfxPlatform::CodeName` is not filled by the library
     - No change
   * - :cpp:enumerator:`MFX_PLATFORM_ALDERLAKE_P`
     - 2.9
     -
     - Field :cpp:member:`mfxPlatform::CodeName` is not filled by the library
     - No change
   * - :cpp:enumerator:`MFX_PLATFORM_ARCTICSOUND_P`
     - 2.9
     -
     - Field :cpp:member:`mfxPlatform::CodeName` is not filled by the library
     - No change
   * - :cpp:enumerator:`MFX_PLATFORM_XEHP_SDV`
     - 2.9
     -
     - Field :cpp:member:`mfxPlatform::CodeName` is not filled by the library
     - No change
   * - :cpp:enumerator:`MFX_PLATFORM_DG2`
     - 2.9
     -
     - Field :cpp:member:`mfxPlatform::CodeName` is not filled by the library
     - No change
   * - :cpp:enumerator:`MFX_PLATFORM_ATS_M`
     - 2.9
     -
     - Field :cpp:member:`mfxPlatform::CodeName` is not filled by the library
     - No change
   * - :cpp:enumerator:`MFX_PLATFORM_ALDERLAKE_N`
     - 2.9
     -
     - Field :cpp:member:`mfxPlatform::CodeName` is not filled by the library
     - No change
   * - :cpp:enumerator:`MFX_PLATFORM_KEEMBAY`
     - 2.9
     -
     - Field :cpp:member:`mfxPlatform::CodeName` is not filled by the library
     - No change   
   * - :cpp:member:`mfxPlatform::CodeName`
     - 2.9
     -
     - Field is not filled by the library
     - No change
   * - :cpp:func:`MFXInit`
     - 2.3
     -
     - :cpp:func:`MFXLoad` + :cpp:func:`MFXCreateSession`
     - API may return :cpp:enumerator:`MFX_ERR_NOT_IMPLEMENTED` status
   * - :cpp:func:`MFXInitEx`
     - 2.3
     -
     - :cpp:func:`MFXLoad` + :cpp:func:`MFXCreateSession`
     - API may return :cpp:enumerator:`MFX_ERR_NOT_IMPLEMENTED` status
   * - :cpp:struct:`mfxExtVPPDenoise`
     - 2.5
     -
     - Use :cpp:struct:`mfxExtVPPDenoise2`
     - No change
   * - :cpp:enumerator:`MFX_FOURCC_RGB3`
     - 2.0
     -
     - Use :cpp:enumerator:`MFX_FOURCC_RGB4`
     - No change
   * - :cpp:member:`mfxExtCodingOption::EndOfSequence`
     - 2.0
     -
     - Flag is ignored
     - No change
   * - :cpp:member:`mfxExtCodingOption::EndOfStream`
     - 2.0
     -
     - Flag is ignored
     - No change
   * - :cpp:member:`mfxExtCodingOption3::ExtBrcAdaptiveLTR`
     - 2.4
     -
     - Use :cpp:member:`mfxExtCodingOption3::AdaptiveLTR`
     - No change
   * - :cpp:enumerator:`MFX_EXTBUFF_VPP_SCENE_CHANGE`
     - 2.0
     -
     - Ignored
     - No change
   * - :cpp:member:`mfxExtVppAuxData::SpatialComplexity`
     - 2.0
     -
     - Field is not filled by the library
     - No change
   * - :cpp:member:`mfxExtVppAuxData::TemporalComplexity`
     - 2.0
     -
     - Field is not filled by the library
     - No change
   * - :cpp:member:`mfxExtVppAuxData::SceneChangeRate`
     - 2.0
     -
     - Field is not filled by the library
     - No change
