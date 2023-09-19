.. SPDX-FileCopyrightText: 2019-2021 Intel Corporation
..
.. SPDX-License-Identifier: CC-BY-4.0

====================
Enumerator Reference
====================

-----
Angle
-----
The Angle enumerator itemizes valid rotation angles.

.. doxygenenumvalue:: MFX_ANGLE_0
   :project: oneVPL

.. doxygenenumvalue:: MFX_ANGLE_90
   :project: oneVPL

.. doxygenenumvalue:: MFX_ANGLE_180
   :project: oneVPL

.. doxygenenumvalue:: MFX_ANGLE_270
   :project: oneVPL

-----------------
BitstreamDataFlag
-----------------

The BitstreamDataFlag enumerator uses bit-ORed values to itemize additional
information about the bitstream buffer.

.. doxygenenumvalue:: MFX_BITSTREAM_NO_FLAG
   :project: oneVPL

.. doxygenenumvalue:: MFX_BITSTREAM_COMPLETE_FRAME
   :project: oneVPL

.. doxygenenumvalue:: MFX_BITSTREAM_EOS
   :project: oneVPL

------------
BPSEIControl
------------

The BPSEIControl enumerator is used to control insertion of buffering period SEI
in the encoded bitstream.

.. doxygenenumvalue:: MFX_BPSEI_DEFAULT
   :project: oneVPL

.. doxygenenumvalue:: MFX_BPSEI_IFRAME
   :project: oneVPL

---------
BRCStatus
---------

The BRCStatus enumerator itemizes instructions to the encoder by
:cpp:member:`mfxExtBrc::Update`.

.. doxygenenumvalue:: MFX_BRC_OK
   :project: oneVPL

.. doxygenenumvalue:: MFX_BRC_BIG_FRAME
   :project: oneVPL

.. doxygenenumvalue:: MFX_BRC_SMALL_FRAME
   :project: oneVPL

.. doxygenenumvalue:: MFX_BRC_PANIC_BIG_FRAME
   :project: oneVPL

.. doxygenenumvalue:: MFX_BRC_PANIC_SMALL_FRAME
   :project: oneVPL

-----------
BRefControl
-----------

The BRefControl enumerator is used to control usage of B frames as reference in
AVC encoder.

.. doxygenenumvalue:: MFX_B_REF_UNKNOWN
   :project: oneVPL

.. doxygenenumvalue:: MFX_B_REF_OFF
   :project: oneVPL

.. doxygenenumvalue:: MFX_B_REF_PYRAMID
   :project: oneVPL

----------------
ChromaFormateIdc
----------------

The ChromaFormatIdc enumerator itemizes color-sampling formats.

.. doxygenenumvalue:: MFX_CHROMAFORMAT_MONOCHROME
   :project: oneVPL

.. doxygenenumvalue:: MFX_CHROMAFORMAT_YUV420
   :project: oneVPL

.. doxygenenumvalue:: MFX_CHROMAFORMAT_YUV422
   :project: oneVPL

.. doxygenenumvalue:: MFX_CHROMAFORMAT_YUV444
   :project: oneVPL

.. doxygenenumvalue:: MFX_CHROMAFORMAT_YUV400
   :project: oneVPL

.. doxygenenumvalue:: MFX_CHROMAFORMAT_YUV411
   :project: oneVPL

.. doxygenenumvalue:: MFX_CHROMAFORMAT_YUV422H
   :project: oneVPL

.. doxygenenumvalue:: MFX_CHROMAFORMAT_YUV422V
   :project: oneVPL

.. doxygenenumvalue:: MFX_CHROMAFORMAT_RESERVED1
   :project: oneVPL

.. doxygenenumvalue:: MFX_CHROMAFORMAT_JPEG_SAMPLING
   :project: oneVPL

------------
ChromaSiting
------------

The ChromaSiting enumerator defines chroma location. Use bit-OR’ed values to
specify the desired location.

.. doxygenenumvalue:: MFX_CHROMA_SITING_UNKNOWN
   :project: oneVPL

.. doxygenenumvalue:: MFX_CHROMA_SITING_VERTICAL_TOP
   :project: oneVPL

.. doxygenenumvalue:: MFX_CHROMA_SITING_VERTICAL_CENTER
   :project: oneVPL

.. doxygenenumvalue:: MFX_CHROMA_SITING_VERTICAL_BOTTOM
   :project: oneVPL

.. doxygenenumvalue:: MFX_CHROMA_SITING_HORIZONTAL_LEFT
   :project: oneVPL

.. doxygenenumvalue:: MFX_CHROMA_SITING_HORIZONTAL_CENTER
   :project: oneVPL

.. _codec-format-fourcc:

-----------------
CodecFormatFourCC
-----------------

The CodecFormatFourCC enumerator itemizes codecs in the FourCC format.

.. doxygenenumvalue:: MFX_CODEC_AVC
   :project: oneVPL

.. doxygenenumvalue:: MFX_CODEC_HEVC
   :project: oneVPL

.. doxygenenumvalue:: MFX_CODEC_MPEG2
   :project: oneVPL

.. doxygenenumvalue:: MFX_CODEC_VC1
   :project: oneVPL

.. doxygenenumvalue:: MFX_CODEC_VP9
   :project: oneVPL

.. doxygenenumvalue:: MFX_CODEC_AV1
   :project: oneVPL

.. doxygenenumvalue:: MFX_CODEC_JPEG
   :project: oneVPL

----------
CodecLevel
----------

The CodecLevel enumerator itemizes codec levels for all codecs.

.. doxygenenumvalue:: MFX_LEVEL_UNKNOWN
   :project: oneVPL

H.264 Level 1-1.3
-----------------

.. doxygenenumvalue:: MFX_LEVEL_AVC_1
   :project: oneVPL

.. doxygenenumvalue:: MFX_LEVEL_AVC_1b
   :project: oneVPL

.. doxygenenumvalue:: MFX_LEVEL_AVC_11
   :project: oneVPL

.. doxygenenumvalue:: MFX_LEVEL_AVC_12
   :project: oneVPL

.. doxygenenumvalue:: MFX_LEVEL_AVC_13
   :project: oneVPL

H.264 Level 2-2.2
-----------------

.. doxygenenumvalue:: MFX_LEVEL_AVC_2
   :project: oneVPL

.. doxygenenumvalue:: MFX_LEVEL_AVC_21
   :project: oneVPL

.. doxygenenumvalue:: MFX_LEVEL_AVC_22
   :project: oneVPL

H.264 Level 3-3.2
-----------------

.. doxygenenumvalue:: MFX_LEVEL_AVC_3
   :project: oneVPL

.. doxygenenumvalue:: MFX_LEVEL_AVC_31
   :project: oneVPL

.. doxygenenumvalue:: MFX_LEVEL_AVC_32
   :project: oneVPL

H.264 Level 4-4.2
-----------------

.. doxygenenumvalue:: MFX_LEVEL_AVC_4
   :project: oneVPL

.. doxygenenumvalue:: MFX_LEVEL_AVC_41
   :project: oneVPL

.. doxygenenumvalue:: MFX_LEVEL_AVC_42
   :project: oneVPL

H.264 Level 5-5.2
-----------------

.. doxygenenumvalue:: MFX_LEVEL_AVC_5
   :project: oneVPL

.. doxygenenumvalue:: MFX_LEVEL_AVC_51
   :project: oneVPL

.. doxygenenumvalue:: MFX_LEVEL_AVC_52
   :project: oneVPL

H.264 Level 6-6.2
-----------------

.. doxygenenumvalue:: MFX_LEVEL_AVC_6
   :project: oneVPL

.. doxygenenumvalue:: MFX_LEVEL_AVC_61
   :project: oneVPL

.. doxygenenumvalue:: MFX_LEVEL_AVC_62
   :project: oneVPL

MPEG2 Levels
------------

.. doxygenenumvalue:: MFX_LEVEL_MPEG2_LOW
   :project: oneVPL

.. doxygenenumvalue:: MFX_LEVEL_MPEG2_MAIN
   :project: oneVPL

.. doxygenenumvalue:: MFX_LEVEL_MPEG2_HIGH
   :project: oneVPL

.. doxygenenumvalue:: MFX_LEVEL_MPEG2_HIGH1440
   :project: oneVPL

VC-1 Level Low (Simple and Main Profiles)
-----------------------------------------

.. doxygenenumvalue:: MFX_LEVEL_VC1_LOW
   :project: oneVPL

.. doxygenenumvalue:: MFX_LEVEL_VC1_MEDIAN
   :project: oneVPL

.. doxygenenumvalue:: MFX_LEVEL_VC1_HIGH
   :project: oneVPL

VC-1 Advanced Profile Levels
----------------------------

.. doxygenenumvalue:: MFX_LEVEL_VC1_0
   :project: oneVPL

.. doxygenenumvalue:: MFX_LEVEL_VC1_1
   :project: oneVPL

.. doxygenenumvalue:: MFX_LEVEL_VC1_2
   :project: oneVPL

.. doxygenenumvalue:: MFX_LEVEL_VC1_3
   :project: oneVPL

.. doxygenenumvalue:: MFX_LEVEL_VC1_4
   :project: oneVPL

HEVC Levels
-----------

.. doxygenenumvalue:: MFX_LEVEL_HEVC_1
   :project: oneVPL

.. doxygenenumvalue:: MFX_LEVEL_HEVC_2
   :project: oneVPL

.. doxygenenumvalue:: MFX_LEVEL_HEVC_21
   :project: oneVPL

.. doxygenenumvalue:: MFX_LEVEL_HEVC_3
   :project: oneVPL

.. doxygenenumvalue:: MFX_LEVEL_HEVC_31
   :project: oneVPL

.. doxygenenumvalue:: MFX_LEVEL_HEVC_4
   :project: oneVPL

.. doxygenenumvalue:: MFX_LEVEL_HEVC_41
   :project: oneVPL

.. doxygenenumvalue:: MFX_LEVEL_HEVC_5
   :project: oneVPL

.. doxygenenumvalue:: MFX_LEVEL_HEVC_51
   :project: oneVPL

.. doxygenenumvalue:: MFX_LEVEL_HEVC_52
   :project: oneVPL

.. doxygenenumvalue:: MFX_LEVEL_HEVC_6
   :project: oneVPL

.. doxygenenumvalue:: MFX_LEVEL_HEVC_61
   :project: oneVPL

.. doxygenenumvalue:: MFX_LEVEL_HEVC_62
   :project: oneVPL

.. _codec_profile:

AV1 Levels
-----------

.. doxygenenumvalue:: MFX_LEVEL_AV1_2
   :project: oneVPL

.. doxygenenumvalue:: MFX_LEVEL_AV1_21
   :project: oneVPL

.. doxygenenumvalue:: MFX_LEVEL_AV1_22
   :project: oneVPL

.. doxygenenumvalue:: MFX_LEVEL_AV1_23
   :project: oneVPL

.. doxygenenumvalue:: MFX_LEVEL_AV1_3
   :project: oneVPL

.. doxygenenumvalue:: MFX_LEVEL_AV1_31
   :project: oneVPL

.. doxygenenumvalue:: MFX_LEVEL_AV1_32
   :project: oneVPL

.. doxygenenumvalue:: MFX_LEVEL_AV1_33
   :project: oneVPL

.. doxygenenumvalue:: MFX_LEVEL_AV1_4
   :project: oneVPL

.. doxygenenumvalue:: MFX_LEVEL_AV1_41
   :project: oneVPL

.. doxygenenumvalue:: MFX_LEVEL_AV1_42
   :project: oneVPL

.. doxygenenumvalue:: MFX_LEVEL_AV1_43
   :project: oneVPL

.. doxygenenumvalue:: MFX_LEVEL_AV1_5
   :project: oneVPL

.. doxygenenumvalue:: MFX_LEVEL_AV1_51
   :project: oneVPL

.. doxygenenumvalue:: MFX_LEVEL_AV1_52
   :project: oneVPL

.. doxygenenumvalue:: MFX_LEVEL_AV1_53
   :project: oneVPL

.. doxygenenumvalue:: MFX_LEVEL_AV1_6
   :project: oneVPL

.. doxygenenumvalue:: MFX_LEVEL_AV1_61
   :project: oneVPL

.. doxygenenumvalue:: MFX_LEVEL_AV1_62
   :project: oneVPL

.. doxygenenumvalue:: MFX_LEVEL_AV1_63
   :project: oneVPL

.. doxygenenumvalue:: MFX_LEVEL_AV1_7
   :project: oneVPL

.. doxygenenumvalue:: MFX_LEVEL_AV1_71
   :project: oneVPL

.. doxygenenumvalue:: MFX_LEVEL_AV1_72
   :project: oneVPL

.. doxygenenumvalue:: MFX_LEVEL_AV1_73
   :project: oneVPL
   
   
------------
CodecProfile
------------

The CodecProfile enumerator itemizes codec profiles for all codecs.

.. doxygenenumvalue:: MFX_PROFILE_UNKNOWN
   :project: oneVPL

H.264 Profiles
--------------

.. doxygenenumvalue:: MFX_PROFILE_AVC_BASELINE
   :project: oneVPL

.. doxygenenumvalue:: MFX_PROFILE_AVC_MAIN
   :project: oneVPL

.. doxygenenumvalue:: MFX_PROFILE_AVC_EXTENDED
   :project: oneVPL

.. doxygenenumvalue:: MFX_PROFILE_AVC_HIGH
   :project: oneVPL

.. doxygenenumvalue:: MFX_PROFILE_AVC_HIGH10
   :project: oneVPL

.. doxygenenumvalue:: MFX_PROFILE_AVC_HIGH_422
   :project: oneVPL

.. doxygenenumvalue:: MFX_PROFILE_AVC_CONSTRAINED_BASELINE
   :project: oneVPL

.. doxygenenumvalue:: MFX_PROFILE_AVC_CONSTRAINED_HIGH
   :project: oneVPL

AV1 Profiles
------------

.. doxygenenumvalue:: MFX_PROFILE_AV1_MAIN
   :project: oneVPL

.. doxygenenumvalue:: MFX_PROFILE_AV1_HIGH
   :project: oneVPL

.. doxygenenumvalue:: MFX_PROFILE_AV1_PRO
   :project: oneVPL

VC-1 Profiles
-------------

.. doxygenenumvalue:: MFX_PROFILE_VC1_SIMPLE
   :project: oneVPL

.. doxygenenumvalue:: MFX_PROFILE_VC1_MAIN
   :project: oneVPL

.. doxygenenumvalue:: MFX_PROFILE_VC1_ADVANCED
   :project: oneVPL

VP8 Profiles
------------

.. doxygenenumvalue:: MFX_PROFILE_VP8_0
   :project: oneVPL

.. doxygenenumvalue:: MFX_PROFILE_VP8_1
   :project: oneVPL

.. doxygenenumvalue:: MFX_PROFILE_VP8_2
   :project: oneVPL

.. doxygenenumvalue:: MFX_PROFILE_VP8_3
   :project: oneVPL

VP9 Profiles
------------

.. doxygenenumvalue:: MFX_PROFILE_VP9_0
   :project: oneVPL

.. doxygenenumvalue:: MFX_PROFILE_VP9_1
   :project: oneVPL

.. doxygenenumvalue:: MFX_PROFILE_VP9_2
   :project: oneVPL

.. doxygenenumvalue:: MFX_PROFILE_VP9_3
   :project: oneVPL

H.264 Constraints
-----------------

Combined with H.264 profile, these flags impose additional constraints. See the
H.264 specification for the list of constraints.

.. doxygenenumvalue:: MFX_PROFILE_AVC_CONSTRAINT_SET0
   :project: oneVPL

.. doxygenenumvalue:: MFX_PROFILE_AVC_CONSTRAINT_SET1
   :project: oneVPL

.. doxygenenumvalue:: MFX_PROFILE_AVC_CONSTRAINT_SET2
   :project: oneVPL

.. doxygenenumvalue:: MFX_PROFILE_AVC_CONSTRAINT_SET3
   :project: oneVPL

.. doxygenenumvalue:: MFX_PROFILE_AVC_CONSTRAINT_SET4
   :project: oneVPL

.. doxygenenumvalue:: MFX_PROFILE_AVC_CONSTRAINT_SET5
   :project: oneVPL

JPEG Profiles
-------------

.. doxygenenumvalue:: MFX_PROFILE_JPEG_BASELINE
   :project: oneVPL

-----------------
CodingOptionValue
-----------------

The CodingOptionValue enumerator defines a three-state coding option setting.

.. doxygenenumvalue:: MFX_CODINGOPTION_UNKNOWN
   :project: oneVPL

.. doxygenenumvalue:: MFX_CODINGOPTION_ON
   :project: oneVPL

.. doxygenenumvalue:: MFX_CODINGOPTION_OFF
   :project: oneVPL

.. doxygenenumvalue:: MFX_CODINGOPTION_ADAPTIVE
   :project: oneVPL

-----------
ColorFourCC
-----------

The ColorFourCC enumerator itemizes color formats.

.. doxygenenumvalue:: MFX_FOURCC_NV12
   :project: oneVPL

.. doxygenenumvalue:: MFX_FOURCC_NV21
   :project: oneVPL

.. doxygenenumvalue:: MFX_FOURCC_YV12
   :project: oneVPL

.. doxygenenumvalue:: MFX_FOURCC_IYUV
   :project: oneVPL

.. doxygenenumvalue:: MFX_FOURCC_I420
   :project: oneVPL

.. doxygenenumvalue:: MFX_FOURCC_I422
   :project: oneVPL

.. doxygenenumvalue:: MFX_FOURCC_NV16
   :project: oneVPL

.. doxygenenumvalue:: MFX_FOURCC_YUY2
   :project: oneVPL

.. doxygenenumvalue:: MFX_FOURCC_RGB565
   :project: oneVPL

.. doxygenenumvalue:: MFX_FOURCC_RGBP
   :project: oneVPL

.. doxygenenumvalue:: MFX_FOURCC_RGB4
   :project: oneVPL

.. doxygenenumvalue:: MFX_FOURCC_BGRA
   :project: oneVPL

.. doxygenenumvalue:: MFX_FOURCC_P8
   :project: oneVPL

.. doxygenenumvalue:: MFX_FOURCC_P8_TEXTURE
   :project: oneVPL

.. doxygenenumvalue:: MFX_FOURCC_P010
   :project: oneVPL

.. doxygenenumvalue:: MFX_FOURCC_I010
   :project: oneVPL

.. doxygenenumvalue:: MFX_FOURCC_I210
   :project: oneVPL

.. doxygenenumvalue:: MFX_FOURCC_P016
   :project: oneVPL

.. doxygenenumvalue:: MFX_FOURCC_P210
   :project: oneVPL

.. doxygenenumvalue:: MFX_FOURCC_BGR4
   :project: oneVPL

.. doxygenenumvalue:: MFX_FOURCC_A2RGB10
   :project: oneVPL

.. doxygenenumvalue:: MFX_FOURCC_ARGB16
   :project: oneVPL

.. doxygenenumvalue:: MFX_FOURCC_ABGR16
   :project: oneVPL

.. doxygenenumvalue:: MFX_FOURCC_R16
   :project: oneVPL

.. doxygenenumvalue:: MFX_FOURCC_AYUV
   :project: oneVPL

.. doxygenenumvalue:: MFX_FOURCC_AYUV_RGB4
   :project: oneVPL

.. doxygenenumvalue:: MFX_FOURCC_UYVY
   :project: oneVPL

.. doxygenenumvalue:: MFX_FOURCC_Y210
   :project: oneVPL

.. doxygenenumvalue:: MFX_FOURCC_Y410
   :project: oneVPL

.. doxygenenumvalue:: MFX_FOURCC_Y216
   :project: oneVPL

.. doxygenenumvalue:: MFX_FOURCC_Y416
   :project: oneVPL

.. doxygenenumvalue:: MFX_FOURCC_BGRP
   :project: oneVPL

.. doxygenenumvalue:: MFX_FOURCC_XYUV
   :project: oneVPL

.. doxygenenumvalue:: MFX_FOURCC_ABGR16F
   :project: oneVPL

-----------
ContentInfo
-----------

The ContentInfo enumerator itemizes content types for the encoding session.

.. doxygenenumvalue:: MFX_CONTENT_UNKNOWN
   :project: oneVPL

.. doxygenenumvalue:: MFX_CONTENT_FULL_SCREEN_VIDEO
   :project: oneVPL

.. doxygenenumvalue:: MFX_CONTENT_NON_VIDEO_SCREEN
   :project: oneVPL

.. doxygenenumvalue:: MFX_CONTENT_NOISY_VIDEO
   :project: oneVPL

----------
Corruption
----------

The Corruption enumerator itemizes the decoding corruption types. It is a
bit-OR’ed value of the following.

.. doxygenenumvalue:: MFX_CORRUPTION_NO
   :project: oneVPL

.. doxygenenumvalue:: MFX_CORRUPTION_MINOR
   :project: oneVPL

.. doxygenenumvalue:: MFX_CORRUPTION_MAJOR
   :project: oneVPL

.. doxygenenumvalue:: MFX_CORRUPTION_ABSENT_TOP_FIELD
   :project: oneVPL

.. doxygenenumvalue:: MFX_CORRUPTION_ABSENT_BOTTOM_FIELD
   :project: oneVPL

.. doxygenenumvalue:: MFX_CORRUPTION_REFERENCE_FRAME
   :project: oneVPL

.. doxygenenumvalue:: MFX_CORRUPTION_REFERENCE_LIST
   :project: oneVPL

.. doxygenenumvalue:: MFX_CORRUPTION_HW_RESET
   :project: oneVPL

.. note:: Flag MFX_CORRUPTION_ABSENT_TOP_FIELD/MFX_CORRUPTION_ABSENT_BOTTOM_FIELD
         is set by the AVC decoder when it detects that one of fields is not
         present in the bitstream. Which field is absent depends on value of
         bottom_field_flag (ITU-T\* H.264 7.4.3).

-----------------
DeinterlacingMode
-----------------

The DeinterlacingMode enumerator itemizes VPP deinterlacing modes.

.. doxygenenumvalue:: MFX_DEINTERLACING_BOB
   :project: oneVPL

.. doxygenenumvalue:: MFX_DEINTERLACING_ADVANCED
   :project: oneVPL

.. doxygenenumvalue:: MFX_DEINTERLACING_AUTO_DOUBLE
   :project: oneVPL

.. doxygenenumvalue:: MFX_DEINTERLACING_AUTO_SINGLE
   :project: oneVPL

.. doxygenenumvalue:: MFX_DEINTERLACING_FULL_FR_OUT
   :project: oneVPL

.. doxygenenumvalue:: MFX_DEINTERLACING_HALF_FR_OUT
   :project: oneVPL

.. doxygenenumvalue:: MFX_DEINTERLACING_24FPS_OUT
   :project: oneVPL

.. doxygenenumvalue:: MFX_DEINTERLACING_FIXED_TELECINE_PATTERN
   :project: oneVPL

.. doxygenenumvalue:: MFX_DEINTERLACING_30FPS_OUT
   :project: oneVPL

.. doxygenenumvalue:: MFX_DEINTERLACING_DETECT_INTERLACE
   :project: oneVPL

.. doxygenenumvalue:: MFX_DEINTERLACING_ADVANCED_NOREF
   :project: oneVPL

.. doxygenenumvalue:: MFX_DEINTERLACING_ADVANCED_SCD
   :project: oneVPL

.. doxygenenumvalue:: MFX_DEINTERLACING_FIELD_WEAVING
   :project: oneVPL

----------
ErrorTypes
----------

The ErrorTypes enumerator uses bit-ORed values to itemize bitstream error types.

.. doxygenenumvalue:: MFX_ERROR_NO
   :project: oneVPL

.. doxygenenumvalue:: MFX_ERROR_PPS
   :project: oneVPL

.. doxygenenumvalue:: MFX_ERROR_SPS
   :project: oneVPL

.. doxygenenumvalue:: MFX_ERROR_SLICEHEADER
   :project: oneVPL

.. doxygenenumvalue:: MFX_ERROR_SLICEDATA
   :project: oneVPL

.. doxygenenumvalue:: MFX_ERROR_FRAME_GAP
   :project: oneVPL

.. doxygenenumvalue:: MFX_ERROR_JPEG_APP0_MARKER
   :project: oneVPL

.. doxygenenumvalue:: MFX_ERROR_JPEG_APP1_MARKER
   :project: oneVPL

.. doxygenenumvalue:: MFX_ERROR_JPEG_APP2_MARKER
   :project: oneVPL

.. doxygenenumvalue:: MFX_ERROR_JPEG_APP3_MARKER
   :project: oneVPL

.. doxygenenumvalue:: MFX_ERROR_JPEG_APP4_MARKER
   :project: oneVPL

.. doxygenenumvalue:: MFX_ERROR_JPEG_APP5_MARKER
   :project: oneVPL

.. doxygenenumvalue:: MFX_ERROR_JPEG_APP6_MARKER
   :project: oneVPL

.. doxygenenumvalue:: MFX_ERROR_JPEG_APP7_MARKER
   :project: oneVPL

.. doxygenenumvalue:: MFX_ERROR_JPEG_APP8_MARKER
   :project: oneVPL

.. doxygenenumvalue:: MFX_ERROR_JPEG_APP9_MARKER
   :project: oneVPL

.. doxygenenumvalue:: MFX_ERROR_JPEG_APP10_MARKER
   :project: oneVPL

.. doxygenenumvalue:: MFX_ERROR_JPEG_APP11_MARKER
   :project: oneVPL

.. doxygenenumvalue:: MFX_ERROR_JPEG_APP12_MARKER
   :project: oneVPL

.. doxygenenumvalue:: MFX_ERROR_JPEG_APP13_MARKER
   :project: oneVPL

.. doxygenenumvalue:: MFX_ERROR_JPEG_APP14_MARKER
   :project: oneVPL

.. doxygenenumvalue:: MFX_ERROR_JPEG_DQT_MARKER
   :project: oneVPL

.. doxygenenumvalue:: MFX_ERROR_JPEG_SOF0_MARKER
   :project: oneVPL

.. doxygenenumvalue:: MFX_ERROR_JPEG_DHT_MARKER
   :project: oneVPL

.. doxygenenumvalue:: MFX_ERROR_JPEG_DRI_MARKER
   :project: oneVPL

.. doxygenenumvalue:: MFX_ERROR_JPEG_SOS_MARKER
   :project: oneVPL

.. doxygenenumvalue:: MFX_ERROR_JPEG_UNKNOWN_MARKER
   :project: oneVPL

.. _extendedbufferid:

----------------
ExtendedBufferID
----------------

The ExtendedBufferID enumerator itemizes and defines identifiers (BufferId) for
extended buffers or video processing algorithm identifiers.

.. doxygenenumvalue:: MFX_EXTBUFF_THREADS_PARAM
  :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_CODING_OPTION
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_CODING_OPTION_SPSPPS
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_DONOTUSE
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_AUXDATA
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_DENOISE2
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_3DLUT
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_SCENE_ANALYSIS
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_PROCAMP
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_DETAIL
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_VIDEO_SIGNAL_INFO
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_VIDEO_SIGNAL_INFO_IN
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_VIDEO_SIGNAL_INFO_OUT
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_DOUSE
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_AVC_REFLIST_CTRL
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_FRAME_RATE_CONVERSION
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_PICTURE_TIMING_SEI
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_AVC_TEMPORAL_LAYERS
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_CODING_OPTION2
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_IMAGE_STABILIZATION
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_ENCODER_CAPABILITY
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_ENCODER_RESET_OPTION
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_ENCODED_FRAME_INFO
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_COMPOSITE
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_VIDEO_SIGNAL_INFO
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_ENCODER_ROI
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_DEINTERLACING
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_AVC_REFLISTS
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_DEC_VIDEO_PROCESSING
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_FIELD_PROCESSING
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_CODING_OPTION3
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_CHROMA_LOC_INFO
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_MBQP
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_MB_FORCE_INTRA
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_HEVC_TILES
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_MB_DISABLE_SKIP_MAP
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_HEVC_PARAM
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_DECODED_FRAME_INFO
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_TIME_CODE
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_HEVC_REGION
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_PRED_WEIGHT_TABLE
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_DIRTY_RECTANGLES
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_MOVING_RECTANGLES
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_CODING_OPTION_VPS
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_ROTATION
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_ENCODED_SLICES_INFO
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_SCALING
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_HEVC_REFLIST_CTRL
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_HEVC_REFLISTS
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_HEVC_TEMPORAL_LAYERS
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_MIRRORING
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_MV_OVER_PIC_BOUNDARIES
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_COLORFILL
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_DECODE_ERROR_REPORT
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_COLOR_CONVERSION
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_CONTENT_LIGHT_LEVEL_INFO
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_MASTERING_DISPLAY_COLOUR_VOLUME
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_MASTERING_DISPLAY_COLOUR_VOLUME_IN
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_MASTERING_DISPLAY_COLOUR_VOLUME_OUT
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_ENCODED_UNITS_INFO
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_MCTF
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_VP9_SEGMENTATION
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_VP9_TEMPORAL_LAYERS
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_VP9_PARAM
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_AVC_ROUNDING_OFFSET
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_PARTIAL_BITSTREAM_PARAM
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_BRC
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_VP8_CODING_OPTION
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_JPEG_QT
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_JPEG_HUFFMAN
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_ENCODER_IPCM_AREA
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_INSERT_HEADERS
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_MVC_SEQ_DESC
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_MVC_TARGET_VIEWS
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_CENC_PARAM
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_DEVICE_AFFINITY_MASK
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_CROPS
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_AV1_FILM_GRAIN_PARAM
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_AV1_SEGMENTATION
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_ALLOCATION_HINTS
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_UNIVERSAL_TEMPORAL_LAYERS
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_UNIVERSAL_REFLIST_CTRL
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_ENCODESTATS
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_SYNCSUBMISSION
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_TUNE_ENCODE_QUALITY
   :project: oneVPL

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_PERC_ENC_PREFILTER
   :project: oneVPL

----------------
ExtMemBufferType
----------------

.. doxygenenumvalue:: MFX_MEMTYPE_PERSISTENT_MEMORY
   :project: oneVPL

.. _extmemframetype:

---------------
ExtMemFrameType
---------------

The ExtMemFrameType enumerator specifies the memory type of frame. It is a
bit-ORed value of one of the following. For information on working with video
memory surfaces, see the :ref:`Working with Hardware Acceleration section<hw-acceleration>`.

.. doxygenenumvalue:: MFX_MEMTYPE_DXVA2_DECODER_TARGET
   :project: oneVPL

.. doxygenenumvalue:: MFX_MEMTYPE_DXVA2_PROCESSOR_TARGET
   :project: oneVPL

.. doxygenenumvalue:: MFX_MEMTYPE_VIDEO_MEMORY_DECODER_TARGET
   :project: oneVPL

.. doxygenenumvalue:: MFX_MEMTYPE_VIDEO_MEMORY_PROCESSOR_TARGET
   :project: oneVPL

.. doxygenenumvalue:: MFX_MEMTYPE_SYSTEM_MEMORY
   :project: oneVPL

.. doxygenenumvalue:: MFX_MEMTYPE_RESERVED1
   :project: oneVPL

.. doxygenenumvalue:: MFX_MEMTYPE_FROM_ENCODE
   :project: oneVPL

.. doxygenenumvalue:: MFX_MEMTYPE_FROM_DECODE
   :project: oneVPL

.. doxygenenumvalue:: MFX_MEMTYPE_FROM_VPPIN
   :project: oneVPL

.. doxygenenumvalue:: MFX_MEMTYPE_FROM_VPPOUT
   :project: oneVPL

.. doxygenenumvalue:: MFX_MEMTYPE_FROM_ENC
   :project: oneVPL

.. doxygenenumvalue:: MFX_MEMTYPE_INTERNAL_FRAME
   :project: oneVPL

.. doxygenenumvalue:: MFX_MEMTYPE_EXTERNAL_FRAME
   :project: oneVPL

.. doxygenenumvalue:: MFX_MEMTYPE_EXPORT_FRAME
   :project: oneVPL

.. doxygenenumvalue:: MFX_MEMTYPE_SHARED_RESOURCE
   :project: oneVPL

.. doxygenenumvalue:: MFX_MEMTYPE_VIDEO_MEMORY_ENCODER_TARGET
   :project: oneVPL

----------------
Frame Data Flags
----------------

.. doxygenenumvalue:: MFX_TIMESTAMP_UNKNOWN
   :project: oneVPL

.. doxygenenumvalue:: MFX_FRAMEORDER_UNKNOWN
   :project: oneVPL

.. doxygenenumvalue:: MFX_FRAMEDATA_TIMESTAMP_UNKNOWN
   :project: oneVPL

.. doxygenenumvalue:: MFX_FRAMEDATA_ORIGINAL_TIMESTAMP
   :project: oneVPL

---------
FrameType
---------

The FrameType enumerator itemizes frame types. Use bit-ORed values to specify
all that apply.

.. doxygenenumvalue:: MFX_FRAMETYPE_UNKNOWN
   :project: oneVPL

.. doxygenenumvalue:: MFX_FRAMETYPE_I
   :project: oneVPL

.. doxygenenumvalue:: MFX_FRAMETYPE_P
   :project: oneVPL

.. doxygenenumvalue:: MFX_FRAMETYPE_B
   :project: oneVPL

.. doxygenenumvalue:: MFX_FRAMETYPE_S
   :project: oneVPL

.. doxygenenumvalue:: MFX_FRAMETYPE_REF
   :project: oneVPL

.. doxygenenumvalue:: MFX_FRAMETYPE_IDR
   :project: oneVPL

.. doxygenenumvalue:: MFX_FRAMETYPE_xI
   :project: oneVPL

.. doxygenenumvalue:: MFX_FRAMETYPE_xP
   :project: oneVPL

.. doxygenenumvalue:: MFX_FRAMETYPE_xB
   :project: oneVPL

.. doxygenenumvalue:: MFX_FRAMETYPE_xS
   :project: oneVPL

.. doxygenenumvalue:: MFX_FRAMETYPE_xREF
   :project: oneVPL

.. doxygenenumvalue:: MFX_FRAMETYPE_xIDR
   :project: oneVPL

-------
FrcAlgm
-------

The FrcAlgm enumerator itemizes frame rate conversion algorithms. See description
of mfxExtVPPFrameRateConversion structure for more details.

.. doxygenenumvalue:: MFX_FRCALGM_PRESERVE_TIMESTAMP
   :project: oneVPL

.. doxygenenumvalue:: MFX_FRCALGM_DISTRIBUTED_TIMESTAMP
   :project: oneVPL

.. doxygenenumvalue:: MFX_FRCALGM_FRAME_INTERPOLATION
   :project: oneVPL

----------------------
GeneralConstraintFlags
----------------------

The GeneralConstraintFlags enumerator uses bit-ORed values to itemize HEVC bitstream 
indications for specific profiles. Each value indicates for format range extensions profiles. 
To specify HEVC Main 10 Still Picture profile applications have to set 
mfxInfoMFX::CodecProfile == MFX_PROFILE_HEVC_MAIN10 and 
mfxExtHEVCParam::GeneralConstraintFlags == MFX_HEVC_CONSTR_REXT_ONE_PICTURE_ONLY.

.. doxygenenumvalue:: MFX_HEVC_CONSTR_REXT_MAX_12BIT
   :project: oneVPL

.. doxygenenumvalue:: MFX_HEVC_CONSTR_REXT_MAX_10BIT
   :project: oneVPL

.. doxygenenumvalue:: MFX_HEVC_CONSTR_REXT_MAX_8BIT
   :project: oneVPL

.. doxygenenumvalue:: MFX_HEVC_CONSTR_REXT_MAX_422CHROMA
   :project: oneVPL

.. doxygenenumvalue:: MFX_HEVC_CONSTR_REXT_MAX_420CHROMA
   :project: oneVPL

.. doxygenenumvalue:: MFX_HEVC_CONSTR_REXT_MAX_MONOCHROME
   :project: oneVPL

.. doxygenenumvalue:: MFX_HEVC_CONSTR_REXT_INTRA
   :project: oneVPL

.. doxygenenumvalue:: MFX_HEVC_CONSTR_REXT_ONE_PICTURE_ONLY
   :project: oneVPL

.. doxygenenumvalue:: MFX_HEVC_CONSTR_REXT_LOWER_BIT_RATE
   :project: oneVPL

----------
GopOptFlag
----------

The GopOptFlag enumerator itemizes special properties in the GOP (Group of
Pictures) sequence.

.. doxygenenumvalue:: MFX_GOP_CLOSED
   :project: oneVPL

.. doxygenenumvalue:: MFX_GOP_STRICT
   :project: oneVPL

.. _gpu_copy:

-------
GPUCopy
-------

.. doxygenenumvalue:: MFX_GPUCOPY_DEFAULT
   :project: oneVPL

.. doxygenenumvalue:: MFX_GPUCOPY_ON
   :project: oneVPL

.. doxygenenumvalue:: MFX_GPUCOPY_OFF
   :project: oneVPL

-------------
HEVC Profiles
-------------

.. doxygenenumvalue:: MFX_PROFILE_HEVC_MAIN
   :project: oneVPL

.. doxygenenumvalue:: MFX_PROFILE_HEVC_MAIN10
   :project: oneVPL

.. doxygenenumvalue:: MFX_PROFILE_HEVC_MAINSP
   :project: oneVPL

.. doxygenenumvalue:: MFX_PROFILE_HEVC_REXT
   :project: oneVPL

.. doxygenenumvalue:: MFX_PROFILE_HEVC_SCC
   :project: oneVPL

----------
HEVC Tiers
----------

.. doxygenenumvalue:: MFX_TIER_HEVC_MAIN
   :project: oneVPL

.. doxygenenumvalue:: MFX_TIER_HEVC_HIGH
   :project: oneVPL

------------------
HEVCRegionEncoding
------------------
The HEVCRegionEncoding enumerator itemizes HEVC region's encoding.

.. doxygenenumvalue:: MFX_HEVC_REGION_ENCODING_ON
   :project: oneVPL

.. doxygenenumvalue:: MFX_HEVC_REGION_ENCODING_OFF
   :project: oneVPL

--------------
HEVCRegionType
--------------

The HEVCRegionType enumerator itemizes type of HEVC region.

.. doxygenenumvalue:: MFX_HEVC_REGION_SLICE
   :project: oneVPL

-------------
ImageStabMode
-------------

The ImageStabMode enumerator itemizes image stabilization modes. See description
of mfxExtVPPImageStab structure for more details.

.. doxygenenumvalue:: MFX_IMAGESTAB_MODE_UPSCALE
   :project: oneVPL

.. doxygenenumvalue:: MFX_IMAGESTAB_MODE_BOXING
   :project: oneVPL

----------------
InsertHDRPayload
----------------

The InsertHDRPayload enumerator itemizes HDR payloads insertion rules.

.. doxygenenumvalue:: MFX_PAYLOAD_OFF
   :project: oneVPL

.. doxygenenumvalue:: MFX_PAYLOAD_IDR
   :project: oneVPL

-----------------
InterpolationMode
-----------------

The InterpolationMode enumerator specifies type of interpolation method used by
VPP scaling filter.

.. doxygenenumvalue:: MFX_INTERPOLATION_DEFAULT
   :project: oneVPL

.. doxygenenumvalue:: MFX_INTERPOLATION_NEAREST_NEIGHBOR
   :project: oneVPL

.. doxygenenumvalue:: MFX_INTERPOLATION_BILINEAR
   :project: oneVPL

.. doxygenenumvalue:: MFX_INTERPOLATION_ADVANCED
   :project: oneVPL

--------
DataType
--------

.. doxygenenum:: mfxDataType
   :project: oneVPL

-------------------
3DLutChannelMapping
-------------------

.. doxygenenum:: mfx3DLutChannelMapping
   :project: oneVPL

-----------------
3DLutMemoryLayout
-----------------

.. doxygenenum:: mfx3DLutMemoryLayout
   :project: oneVPL

-------------------------------------
IntraPredBlockSize/InterPredBlockSize
-------------------------------------

IntraPredBlockSize/InterPredBlockSize specifies minimum block size of inter-prediction.

.. doxygenenumvalue:: MFX_BLOCKSIZE_UNKNOWN
   :project: oneVPL

.. doxygenenumvalue:: MFX_BLOCKSIZE_MIN_16X16
   :project: oneVPL

.. doxygenenumvalue:: MFX_BLOCKSIZE_MIN_8X8
   :project: oneVPL

.. doxygenenumvalue:: MFX_BLOCKSIZE_MIN_4X4
   :project: oneVPL

-----------------
IntraRefreshTypes
-----------------

The IntraRefreshTypes enumerator itemizes types of intra refresh.

.. doxygenenumvalue:: MFX_REFRESH_NO
   :project: oneVPL

.. doxygenenumvalue:: MFX_REFRESH_VERTICAL
   :project: oneVPL

.. doxygenenumvalue:: MFX_REFRESH_HORIZONTAL
   :project: oneVPL

.. doxygenenumvalue:: MFX_REFRESH_SLICE
   :project: oneVPL

---------
IOPattern
---------

The IOPattern enumerator itemizes memory access patterns for API functions.
Use bit-ORed values to specify input and output access patterns.

.. doxygenenumvalue:: MFX_IOPATTERN_IN_VIDEO_MEMORY
   :project: oneVPL

.. doxygenenumvalue:: MFX_IOPATTERN_IN_SYSTEM_MEMORY
   :project: oneVPL

.. doxygenenumvalue:: MFX_IOPATTERN_OUT_VIDEO_MEMORY
   :project: oneVPL

.. doxygenenumvalue:: MFX_IOPATTERN_OUT_SYSTEM_MEMORY
   :project: oneVPL

---------------
JPEGColorFormat
---------------

The JPEGColorFormat enumerator itemizes the JPEG color format options.

.. doxygenenumvalue:: MFX_JPEG_COLORFORMAT_UNKNOWN
   :project: oneVPL

.. doxygenenumvalue:: MFX_JPEG_COLORFORMAT_YCbCr
   :project: oneVPL

.. doxygenenumvalue:: MFX_JPEG_COLORFORMAT_RGB
   :project: oneVPL

------------
JPEGScanType
------------

The JPEGScanType enumerator itemizes the JPEG scan types.

.. doxygenenumvalue:: MFX_SCANTYPE_UNKNOWN
   :project: oneVPL

.. doxygenenumvalue:: MFX_SCANTYPE_INTERLEAVED
   :project: oneVPL

.. doxygenenumvalue:: MFX_SCANTYPE_NONINTERLEAVED
   :project: oneVPL

-----------
LongTermIdx
-----------

The LongTermIdx specifies long term index of picture control

.. doxygenenumvalue:: MFX_LONGTERM_IDX_NO_IDX
   :project: oneVPL

---------------------
LookAheadDownSampling
---------------------

The LookAheadDownSampling enumerator is used to control down sampling in look
ahead bitrate control mode in AVC encoder.

.. doxygenenumvalue:: MFX_LOOKAHEAD_DS_UNKNOWN
   :project: oneVPL

.. doxygenenumvalue:: MFX_LOOKAHEAD_DS_OFF
   :project: oneVPL

.. doxygenenumvalue:: MFX_LOOKAHEAD_DS_2x
   :project: oneVPL

.. doxygenenumvalue:: MFX_LOOKAHEAD_DS_4x
   :project: oneVPL

--------
MBQPMode
--------

The MBQPMode enumerator itemizes QP update modes.

.. doxygenenumvalue:: MFX_MBQP_MODE_QP_VALUE
   :project: oneVPL

.. doxygenenumvalue:: MFX_MBQP_MODE_QP_DELTA
   :project: oneVPL

.. doxygenenumvalue:: MFX_MBQP_MODE_QP_ADAPTIVE
   :project: oneVPL

----------------
mfxComponentType
----------------

.. doxygenenum:: mfxComponentType
   :project: oneVPL

-------------
mfxHandleType
-------------

.. doxygenenum:: mfxHandleType
   :project: oneVPL

-------
mfxIMPL
-------

.. doxygentypedef:: mfxIMPL
   :project: oneVPL

.. doxygenenumvalue:: MFX_IMPL_AUTO
   :project: oneVPL

.. doxygenenumvalue:: MFX_IMPL_SOFTWARE
   :project: oneVPL

.. doxygenenumvalue:: MFX_IMPL_HARDWARE
   :project: oneVPL

.. doxygenenumvalue:: MFX_IMPL_AUTO_ANY
   :project: oneVPL

.. doxygenenumvalue:: MFX_IMPL_HARDWARE_ANY
   :project: oneVPL

.. doxygenenumvalue:: MFX_IMPL_HARDWARE2
   :project: oneVPL

.. doxygenenumvalue:: MFX_IMPL_HARDWARE3
   :project: oneVPL

.. doxygenenumvalue:: MFX_IMPL_HARDWARE4
   :project: oneVPL

.. doxygenenumvalue:: MFX_IMPL_RUNTIME
   :project: oneVPL

.. doxygenenumvalue:: MFX_IMPL_VIA_ANY
   :project: oneVPL

.. doxygenenumvalue:: MFX_IMPL_VIA_D3D9
   :project: oneVPL

.. doxygenenumvalue:: MFX_IMPL_VIA_D3D11
   :project: oneVPL

.. doxygenenumvalue:: MFX_IMPL_VIA_VAAPI
   :project: oneVPL
   
.. doxygenenumvalue:: MFX_IMPL_VIA_HDDLUNITE
   :project: oneVPL

.. doxygenenumvalue:: MFX_IMPL_UNSUPPORTED
   :project: oneVPL

.. doxygendefine:: MFX_IMPL_BASETYPE
   :project: oneVPL

-------------------------
mfxImplCapsDeliveryFormat
-------------------------

.. doxygenenum:: mfxImplCapsDeliveryFormat
   :project: oneVPL

-------------------
mfxMediaAdapterType
-------------------

.. doxygenenum:: mfxMediaAdapterType
   :project: oneVPL

--------------
mfxMemoryFlags
--------------

.. doxygenenum:: mfxMemoryFlags
   :project: oneVPL

--------------
MfxNalUnitType
--------------

Specifies NAL unit types supported by the HEVC encoder.

.. doxygenenumvalue:: MFX_HEVC_NALU_TYPE_UNKNOWN
   :project: oneVPL

.. doxygenenumvalue:: MFX_HEVC_NALU_TYPE_TRAIL_N
   :project: oneVPL

.. doxygenenumvalue:: MFX_HEVC_NALU_TYPE_TRAIL_R
   :project: oneVPL

.. doxygenenumvalue:: MFX_HEVC_NALU_TYPE_RADL_N
   :project: oneVPL

.. doxygenenumvalue:: MFX_HEVC_NALU_TYPE_RADL_R
   :project: oneVPL

.. doxygenenumvalue:: MFX_HEVC_NALU_TYPE_RASL_N
   :project: oneVPL

.. doxygenenumvalue:: MFX_HEVC_NALU_TYPE_RASL_R
   :project: oneVPL

.. doxygenenumvalue:: MFX_HEVC_NALU_TYPE_IDR_W_RADL
   :project: oneVPL

.. doxygenenumvalue:: MFX_HEVC_NALU_TYPE_IDR_N_LP
   :project: oneVPL

.. doxygenenumvalue:: MFX_HEVC_NALU_TYPE_CRA_NUT
   :project: oneVPL

-----------
mfxPriority
-----------

.. doxygenenum:: mfxPriority
   :project: oneVPL

---------------
mfxResourceType
---------------

.. doxygenenum:: mfxResourceType
   :project: oneVPL

-----------
mfxSkipMode
-----------

.. doxygenenum:: mfxSkipMode
   :project: oneVPL

---------
mfxStatus
---------

.. doxygenenum:: mfxStatus
   :project: oneVPL

-------------
MirroringType
-------------

The MirroringType enumerator itemizes mirroring types.

.. doxygenenumvalue:: MFX_MIRRORING_DISABLED
   :project: oneVPL

.. doxygenenumvalue:: MFX_MIRRORING_HORIZONTAL
   :project: oneVPL

.. doxygenenumvalue:: MFX_MIRRORING_VERTICAL
   :project: oneVPL

-----------
DenoiseMode
-----------

The mfxDenoiseMode enumerator itemizes denoise modes.

.. doxygenenum:: mfxDenoiseMode
   :project: oneVPL

---------------
MPEG-2 Profiles
---------------

.. doxygenenumvalue:: MFX_PROFILE_MPEG2_SIMPLE
   :project: oneVPL

.. doxygenenumvalue:: MFX_PROFILE_MPEG2_MAIN
   :project: oneVPL

.. doxygenenumvalue:: MFX_PROFILE_MPEG2_HIGH
   :project: oneVPL

------------------------------------------
Multi-view Video Coding Extension Profiles
------------------------------------------

.. doxygenenumvalue:: MFX_PROFILE_AVC_MULTIVIEW_HIGH
   :project: oneVPL

.. doxygenenumvalue:: MFX_PROFILE_AVC_STEREO_HIGH
   :project: oneVPL

-----------
MVPrecision
-----------

The MVPrecision enumerator specifies the motion estimation precision

.. doxygenenumvalue:: MFX_MVPRECISION_UNKNOWN
   :project: oneVPL

.. doxygenenumvalue:: MFX_MVPRECISION_INTEGER
   :project: oneVPL

.. doxygenenumvalue:: MFX_MVPRECISION_HALFPEL
   :project: oneVPL

.. doxygenenumvalue:: MFX_MVPRECISION_QUARTERPEL
   :project: oneVPL

------------
NominalRange
------------

The NominalRange enumerator itemizes pixel's value nominal range.

.. doxygenenumvalue:: MFX_NOMINALRANGE_UNKNOWN
   :project: oneVPL

.. doxygenenumvalue:: MFX_NOMINALRANGE_0_255
   :project: oneVPL

.. doxygenenumvalue:: MFX_NOMINALRANGE_16_235
   :project: oneVPL

----------------------
PartialBitstreamOutput
----------------------

The PartialBitstreamOutput enumerator indicates flags of partial bitstream output
type.

.. doxygenenumvalue:: MFX_PARTIAL_BITSTREAM_NONE
   :project: oneVPL

.. doxygenenumvalue:: MFX_PARTIAL_BITSTREAM_SLICE
   :project: oneVPL

.. doxygenenumvalue:: MFX_PARTIAL_BITSTREAM_BLOCK
   :project: oneVPL

.. doxygenenumvalue:: MFX_PARTIAL_BITSTREAM_ANY
   :project: oneVPL

----------------
PayloadCtrlFlags
----------------

The PayloadCtrlFlags enumerator itemizes additional payload properties.

.. doxygenenumvalue:: MFX_PAYLOAD_CTRL_SUFFIX
   :project: oneVPL

.. _PicStruct:

---------
PicStruct
---------

The PicStruct enumerator itemizes picture structure. Use bit-OR’ed values to
specify the desired picture type.

.. doxygenenumvalue:: MFX_PICSTRUCT_UNKNOWN
   :project: oneVPL

.. doxygenenumvalue:: MFX_PICSTRUCT_PROGRESSIVE
   :project: oneVPL

.. doxygenenumvalue:: MFX_PICSTRUCT_FIELD_TFF
   :project: oneVPL

.. doxygenenumvalue:: MFX_PICSTRUCT_FIELD_BFF
   :project: oneVPL

.. doxygenenumvalue:: MFX_PICSTRUCT_FIELD_REPEATED
   :project: oneVPL

.. doxygenenumvalue:: MFX_PICSTRUCT_FRAME_DOUBLING
   :project: oneVPL

.. doxygenenumvalue:: MFX_PICSTRUCT_FRAME_TRIPLING
   :project: oneVPL

.. doxygenenumvalue:: MFX_PICSTRUCT_FIELD_SINGLE
   :project: oneVPL

.. doxygenenumvalue:: MFX_PICSTRUCT_FIELD_TOP
   :project: oneVPL

.. doxygenenumvalue:: MFX_PICSTRUCT_FIELD_BOTTOM
   :project: oneVPL

.. doxygenenumvalue:: MFX_PICSTRUCT_FIELD_PAIRED_PREV
   :project: oneVPL

.. doxygenenumvalue:: MFX_PICSTRUCT_FIELD_PAIRED_NEXT
   :project: oneVPL

-------
PicType
-------

The PicType enumerator itemizes picture type.

.. doxygenenumvalue:: MFX_PICTYPE_UNKNOWN
   :project: oneVPL

.. doxygenenumvalue:: MFX_PICTYPE_FRAME
   :project: oneVPL

.. doxygenenumvalue:: MFX_PICTYPE_TOPFIELD
   :project: oneVPL

.. doxygenenumvalue:: MFX_PICTYPE_BOTTOMFIELD
   :project: oneVPL

--------
PRefType
--------

The PRefType enumerator itemizes models of reference list construction and DPB
management when GopRefDist=1.

.. doxygenenumvalue:: MFX_P_REF_DEFAULT
   :project: oneVPL

.. doxygenenumvalue:: MFX_P_REF_SIMPLE
   :project: oneVPL

.. doxygenenumvalue:: MFX_P_REF_PYRAMID
   :project: oneVPL

-----------
TuneQuality
-----------

The TuneQuality enumerator specifies tuning option for encode. Multiple tuning options 
can be combined using bit mask.

.. doxygenenumvalue:: MFX_ENCODE_TUNE_OFF
   :project: oneVPL

.. doxygenenumvalue:: MFX_ENCODE_TUNE_PSNR
   :project: oneVPL

.. doxygenenumvalue:: MFX_ENCODE_TUNE_SSIM
   :project: oneVPL

.. doxygenenumvalue:: MFX_ENCODE_TUNE_MS_SSIM
   :project: oneVPL

.. doxygenenumvalue:: MFX_ENCODE_TUNE_VMAF
   :project: oneVPL

.. doxygenenumvalue:: MFX_ENCODE_TUNE_PERCEPTUAL
   :project: oneVPL

---------
Protected
---------

The Protected enumerator describes the protection schemes.

.. doxygenenumvalue:: MFX_PROTECTION_CENC_WV_CLASSIC
   :project: oneVPL

.. doxygenenumvalue:: MFX_PROTECTION_CENC_WV_GOOGLE_DASH
   :project: oneVPL

.. _RateControlMethod:

-----------------
RateControlMethod
-----------------

The RateControlMethod enumerator itemizes bitrate control methods.

.. doxygenenumvalue:: MFX_RATECONTROL_CBR
   :project: oneVPL

.. doxygenenumvalue:: MFX_RATECONTROL_VBR
   :project: oneVPL

.. doxygenenumvalue:: MFX_RATECONTROL_CQP
   :project: oneVPL

.. doxygenenumvalue:: MFX_RATECONTROL_AVBR
   :project: oneVPL

.. doxygenenumvalue:: MFX_RATECONTROL_LA
   :project: oneVPL

.. doxygenenumvalue:: MFX_RATECONTROL_ICQ
   :project: oneVPL

.. doxygenenumvalue:: MFX_RATECONTROL_VCM
   :project: oneVPL

.. doxygenenumvalue:: MFX_RATECONTROL_LA_ICQ
   :project: oneVPL

.. doxygenenumvalue:: MFX_RATECONTROL_LA_HRD
   :project: oneVPL

.. doxygenenumvalue:: MFX_RATECONTROL_QVBR
   :project: oneVPL

-------
ROImode
-------

The ROImode enumerator itemizes QP adjustment mode for ROIs.

.. doxygenenumvalue:: MFX_ROI_MODE_PRIORITY
   :project: oneVPL

.. doxygenenumvalue:: MFX_ROI_MODE_QP_DELTA
   :project: oneVPL

.. doxygenenumvalue:: MFX_ROI_MODE_QP_VALUE
   :project: oneVPL

--------
Rotation
--------

The Rotation enumerator itemizes the JPEG rotation options.

.. doxygenenumvalue:: MFX_ROTATION_0
   :project: oneVPL

.. doxygenenumvalue:: MFX_ROTATION_90
   :project: oneVPL

.. doxygenenumvalue:: MFX_ROTATION_180
   :project: oneVPL

.. doxygenenumvalue:: MFX_ROTATION_270
   :project: oneVPL

--------------------
SampleAdaptiveOffset
--------------------

The SampleAdaptiveOffset enumerator uses bit-ORed values to itemize corresponding
HEVC encoding feature.

.. doxygenenumvalue:: MFX_SAO_UNKNOWN
   :project: oneVPL

.. doxygenenumvalue:: MFX_SAO_DISABLE
   :project: oneVPL

.. doxygenenumvalue:: MFX_SAO_ENABLE_LUMA
   :project: oneVPL

.. doxygenenumvalue:: MFX_SAO_ENABLE_CHROMA
   :project: oneVPL

-----------
ScalingMode
-----------

The ScalingMode enumerator itemizes variants of scaling filter implementation.

.. doxygenenumvalue:: MFX_SCALING_MODE_DEFAULT
   :project: oneVPL

.. doxygenenumvalue:: MFX_SCALING_MODE_LOWPOWER
   :project: oneVPL

.. doxygenenumvalue:: MFX_SCALING_MODE_QUALITY
   :project: oneVPL

.. doxygenenumvalue:: MFX_SCALING_MODE_VENDOR
   :project: oneVPL

.. doxygenenumvalue:: MFX_SCALING_MODE_INTEL_GEN_COMPUTE
   :project: oneVPL

.. doxygenenumvalue:: MFX_SCALING_MODE_INTEL_GEN_VDBOX
   :project: oneVPL

.. doxygenenumvalue:: MFX_SCALING_MODE_INTEL_GEN_VEBOX
   :project: oneVPL

------------
ScenarioInfo
------------

The ScenarioInfo enumerator itemizes scenarios for the encoding session.

.. doxygenenumvalue:: MFX_SCENARIO_UNKNOWN
   :project: oneVPL

.. doxygenenumvalue:: MFX_SCENARIO_DISPLAY_REMOTING
   :project: oneVPL

.. doxygenenumvalue:: MFX_SCENARIO_VIDEO_CONFERENCE
   :project: oneVPL

.. doxygenenumvalue:: MFX_SCENARIO_ARCHIVE
   :project: oneVPL

.. doxygenenumvalue:: MFX_SCENARIO_LIVE_STREAMING
   :project: oneVPL

.. doxygenenumvalue:: MFX_SCENARIO_CAMERA_CAPTURE
   :project: oneVPL

.. doxygenenumvalue:: MFX_SCENARIO_VIDEO_SURVEILLANCE
   :project: oneVPL

.. doxygenenumvalue:: MFX_SCENARIO_GAME_STREAMING
   :project: oneVPL

.. doxygenenumvalue:: MFX_SCENARIO_REMOTE_GAMING
   :project: oneVPL

--------------
SegmentFeature
--------------

The SegmentFeature enumerator indicates features enabled for the segment.
These values are used with the mfxVP9SegmentParam::FeatureEnabled parameter.

.. doxygenenumvalue:: MFX_VP9_SEGMENT_FEATURE_QINDEX
   :project: oneVPL

.. doxygenenumvalue:: MFX_VP9_SEGMENT_FEATURE_LOOP_FILTER
   :project: oneVPL

.. doxygenenumvalue:: MFX_VP9_SEGMENT_FEATURE_REFERENCE
   :project: oneVPL

.. doxygenenumvalue:: MFX_VP9_SEGMENT_FEATURE_SKIP
   :project: oneVPL

------------------
SegmentIdBlockSize
------------------

The SegmentIdBlockSize enumerator indicates the block size represented by each
segment_id in segmentation map. These values are used with the
mfxExtVP9Segmentation::SegmentIdBlockSize parameter.

.. doxygenenumvalue:: MFX_VP9_SEGMENT_ID_BLOCK_SIZE_UNKNOWN
   :project: oneVPL

.. doxygenenumvalue:: MFX_VP9_SEGMENT_ID_BLOCK_SIZE_8x8
   :project: oneVPL

.. doxygenenumvalue:: MFX_VP9_SEGMENT_ID_BLOCK_SIZE_16x16
   :project: oneVPL

.. doxygenenumvalue:: MFX_VP9_SEGMENT_ID_BLOCK_SIZE_32x32
   :project: oneVPL

.. doxygenenumvalue:: MFX_VP9_SEGMENT_ID_BLOCK_SIZE_64x64
   :project: oneVPL

---------
SkipFrame
---------

The SkipFrame enumerator is used to define usage of mfxEncodeCtrl::SkipFrame
parameter.

.. doxygenenumvalue:: MFX_SKIPFRAME_NO_SKIP
   :project: oneVPL

.. doxygenenumvalue:: MFX_SKIPFRAME_INSERT_DUMMY
   :project: oneVPL

.. doxygenenumvalue:: MFX_SKIPFRAME_INSERT_NOTHING
   :project: oneVPL

.. doxygenenumvalue:: MFX_SKIPFRAME_BRC_ONLY
   :project: oneVPL

-----------
TargetUsage
-----------

The TargetUsage enumerator itemizes a range of numbers from MFX_TARGETUSAGE_1,
best quality, to MFX_TARGETUSAGE_7, best speed. It indicates trade-offs between
quality and speed. The application can use any number in the range. The actual
number of supported target usages depends on implementation. If the specified
target usage is not supported, the encoder will use the closest supported value.

.. doxygenenumvalue:: MFX_TARGETUSAGE_1
   :project: oneVPL

.. doxygenenumvalue:: MFX_TARGETUSAGE_2
   :project: oneVPL

.. doxygenenumvalue:: MFX_TARGETUSAGE_3
   :project: oneVPL

.. doxygenenumvalue:: MFX_TARGETUSAGE_4
   :project: oneVPL

.. doxygenenumvalue:: MFX_TARGETUSAGE_5
   :project: oneVPL

.. doxygenenumvalue:: MFX_TARGETUSAGE_6
   :project: oneVPL

.. doxygenenumvalue:: MFX_TARGETUSAGE_7
   :project: oneVPL

.. doxygenenumvalue:: MFX_TARGETUSAGE_UNKNOWN
   :project: oneVPL

.. doxygenenumvalue:: MFX_TARGETUSAGE_BEST_QUALITY
   :project: oneVPL

.. doxygenenumvalue:: MFX_TARGETUSAGE_BALANCED
   :project: oneVPL

.. doxygenenumvalue:: MFX_TARGETUSAGE_BEST_SPEED
   :project: oneVPL

---------------
TelecinePattern
---------------

The TelecinePattern enumerator itemizes telecine patterns.

.. doxygenenumvalue:: MFX_TELECINE_PATTERN_32
   :project: oneVPL

.. doxygenenumvalue:: MFX_TELECINE_PATTERN_2332
   :project: oneVPL

.. doxygenenumvalue:: MFX_TELECINE_PATTERN_FRAME_REPEAT
   :project: oneVPL

.. doxygenenumvalue:: MFX_TELECINE_PATTERN_41
   :project: oneVPL

.. doxygenenumvalue:: MFX_TELECINE_POSITION_PROVIDED
   :project: oneVPL

-------------
TimeStampCalc
-------------

The TimeStampCalc enumerator itemizes time-stamp calculation methods.

.. doxygenenumvalue:: MFX_TIMESTAMPCALC_UNKNOWN
   :project: oneVPL

.. doxygenenumvalue:: MFX_TIMESTAMPCALC_TELECINE
   :project: oneVPL

--------------
TransferMatrix
--------------

The TransferMatrix enumerator itemizes color transfer matrices.

.. doxygenenumvalue:: MFX_TRANSFERMATRIX_UNKNOWN
   :project: oneVPL

.. doxygenenumvalue:: MFX_TRANSFERMATRIX_BT709
   :project: oneVPL

.. doxygenenumvalue:: MFX_TRANSFERMATRIX_BT601
   :project: oneVPL

--------------
TrellisControl
--------------

The TrellisControl enumerator is used to control trellis quantization in AVC
encoder. The application can turn it on or off for any combination of I, P, and
B frames by combining different enumerator values. For example,
MFX_TRELLIS_I | MFX_TRELLIS_B turns it on for I and B frames.

.. doxygenenumvalue:: MFX_TRELLIS_UNKNOWN
   :project: oneVPL

.. doxygenenumvalue:: MFX_TRELLIS_OFF
   :project: oneVPL

.. doxygenenumvalue:: MFX_TRELLIS_I
   :project: oneVPL

.. doxygenenumvalue:: MFX_TRELLIS_P
   :project: oneVPL

.. doxygenenumvalue:: MFX_TRELLIS_B
   :project: oneVPL

-----------------
VP9ReferenceFrame
-----------------

The VP9ReferenceFrame enumerator itemizes reference frame type by
the mfxVP9SegmentParam::ReferenceFrame parameter.

.. doxygenenumvalue:: MFX_VP9_REF_INTRA
   :project: oneVPL

.. doxygenenumvalue:: MFX_VP9_REF_LAST
   :project: oneVPL

.. doxygenenumvalue:: MFX_VP9_REF_GOLDEN
   :project: oneVPL

.. doxygenenumvalue:: MFX_VP9_REF_ALTREF
   :project: oneVPL

----------------------
VPPFieldProcessingMode
----------------------

The VPPFieldProcessingMode enumerator is used to control VPP field processing
algorithm.

.. doxygenenumvalue:: MFX_VPP_COPY_FRAME
   :project: oneVPL

.. doxygenenumvalue:: MFX_VPP_COPY_FIELD
   :project: oneVPL

.. doxygenenumvalue:: MFX_VPP_SWAP_FIELDS
   :project: oneVPL

------------
WeightedPred
------------

The WeightedPred enumerator itemizes weighted prediction modes.

.. doxygenenumvalue:: MFX_WEIGHTED_PRED_UNKNOWN
   :project: oneVPL

.. doxygenenumvalue:: MFX_WEIGHTED_PRED_DEFAULT
   :project: oneVPL

.. doxygenenumvalue:: MFX_WEIGHTED_PRED_EXPLICIT
   :project: oneVPL

.. doxygenenumvalue:: MFX_WEIGHTED_PRED_IMPLICIT
   :project: oneVPL

--------------
FilmGrainFlags 
--------------

The FilmGrainFlags enumerator itemizes flags in AV1 film grain parameters. 

.. doxygenenumvalue:: MFX_FILM_GRAIN_NO
   :project: oneVPL

.. doxygenenumvalue:: MFX_FILM_GRAIN_APPLY
   :project: oneVPL

.. doxygenenumvalue:: MFX_FILM_GRAIN_UPDATE
   :project: oneVPL

.. doxygenenumvalue:: MFX_FILM_GRAIN_CHROMA_SCALING_FROM_LUMA
   :project: oneVPL

.. doxygenenumvalue:: MFX_FILM_GRAIN_OVERLAP
   :project: oneVPL
   
.. doxygenenumvalue:: MFX_FILM_GRAIN_CLIP_TO_RESTRICTED_RANGE
   :project: oneVPL

------------
mfxHyperMode
------------

.. doxygenenum:: mfxHyperMode
   :project: oneVPL

-----------------------
mfxPoolAllocationPolicy
-----------------------

.. doxygenenum:: mfxPoolAllocationPolicy
   :project: oneVPL
   
--------------
mfxVPPPoolType
--------------

.. doxygenenum:: mfxVPPPoolType
   :project: oneVPL
 
------------------------
mfxAV1SegmentIdBlockSize
------------------------

The mfxAV1SegmentIdBlockSize enumerator indicates the block size represented by each segment_id in segmentation map.

.. doxygenenum:: mfxAV1SegmentIdBlockSize
   :project: oneVPL

-----------------
AV1SegmentFeature
-----------------

The AV1SegmentFeature enumerator indicates features enabled for the segment.

.. doxygenenumvalue:: MFX_AV1_SEGMENT_FEATURE_ALT_QINDEX
   :project: oneVPL

.. doxygenenumvalue:: MFX_AV1_SEGMENT_FEATURE_ALT_LF_Y_VERT
   :project: oneVPL

.. doxygenenumvalue:: MFX_AV1_SEGMENT_FEATURE_ALT_LF_Y_HORZ
   :project: oneVPL

.. doxygenenumvalue:: MFX_AV1_SEGMENT_FEATURE_ALT_LF_U
   :project: oneVPL

.. doxygenenumvalue:: MFX_AV1_SEGMENT_FEATURE_ALT_LF_V
   :project: oneVPL

.. doxygenenumvalue:: MFX_AV1_SEGMENT_FEATURE_REFERENCE
   :project: oneVPL

.. doxygenenumvalue:: MFX_AV1_SEGMENT_FEATURE_SKIP
   :project: oneVPL

.. doxygenenumvalue:: MFX_AV1_SEGMENT_FEATURE_GLOBALMV
   :project: oneVPL

--------------------------
mfxEncodeBlkStatsMemLayout
--------------------------

.. doxygenenum:: mfxEncodeBlkStatsMemLayout
   :project: oneVPL

------------------
mfxEncodeStatsMode
------------------

.. doxygenenum:: mfxEncodeStatsMode
   :project: oneVPL

----------------
EncodeStatsLevel
----------------

Flags to specify what statistics will be reported by the implementation.

.. doxygenenumvalue:: MFX_ENCODESTATS_LEVEL_BLK
   :project: oneVPL

.. doxygenenumvalue:: MFX_ENCODESTATS_LEVEL_FRAME
   :project: oneVPL


-------------------
mfxSurfaceComponent
-------------------

.. doxygenenum:: mfxSurfaceComponent
   :project: oneVPL


--------------
mfxSurfaceType
--------------

.. doxygenenum:: mfxSurfaceType
   :project: oneVPL

----------------
mfxStructureType
----------------

.. doxygenenum:: mfxStructureType
   :project: oneVPL