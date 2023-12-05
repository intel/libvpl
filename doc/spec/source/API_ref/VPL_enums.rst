.. SPDX-FileCopyrightText: 2019-2021 Intel Corporation
..
.. SPDX-License-Identifier: CC-BY-4.0
..
  Intel(r) Video Processing Library (Intel(r) VPL)

====================
Enumerator Reference
====================

-----
Angle
-----
The Angle enumerator itemizes valid rotation angles.

.. doxygenenumvalue:: MFX_ANGLE_0
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_ANGLE_90
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_ANGLE_180
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_ANGLE_270
   :project: Intel&reg; VPL

-----------------
BitstreamDataFlag
-----------------

The BitstreamDataFlag enumerator uses bit-ORed values to itemize additional
information about the bitstream buffer.

.. doxygenenumvalue:: MFX_BITSTREAM_NO_FLAG
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_BITSTREAM_COMPLETE_FRAME
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_BITSTREAM_EOS
   :project: Intel&reg; VPL

------------
BPSEIControl
------------

The BPSEIControl enumerator is used to control insertion of buffering period SEI
in the encoded bitstream.

.. doxygenenumvalue:: MFX_BPSEI_DEFAULT
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_BPSEI_IFRAME
   :project: Intel&reg; VPL

---------
BRCStatus
---------

The BRCStatus enumerator itemizes instructions to the encoder by
:cpp:member:`mfxExtBrc::Update`.

.. doxygenenumvalue:: MFX_BRC_OK
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_BRC_BIG_FRAME
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_BRC_SMALL_FRAME
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_BRC_PANIC_BIG_FRAME
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_BRC_PANIC_SMALL_FRAME
   :project: Intel&reg; VPL

-----------
BRefControl
-----------

The BRefControl enumerator is used to control usage of B frames as reference in
AVC encoder.

.. doxygenenumvalue:: MFX_B_REF_UNKNOWN
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_B_REF_OFF
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_B_REF_PYRAMID
   :project: Intel&reg; VPL

----------------
ChromaFormateIdc
----------------

The ChromaFormatIdc enumerator itemizes color-sampling formats.

.. doxygenenumvalue:: MFX_CHROMAFORMAT_MONOCHROME
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_CHROMAFORMAT_YUV420
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_CHROMAFORMAT_YUV422
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_CHROMAFORMAT_YUV444
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_CHROMAFORMAT_YUV400
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_CHROMAFORMAT_YUV411
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_CHROMAFORMAT_YUV422H
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_CHROMAFORMAT_YUV422V
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_CHROMAFORMAT_RESERVED1
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_CHROMAFORMAT_JPEG_SAMPLING
   :project: Intel&reg; VPL

------------
ChromaSiting
------------

The ChromaSiting enumerator defines chroma location. Use bit-OR’ed values to
specify the desired location.

.. doxygenenumvalue:: MFX_CHROMA_SITING_UNKNOWN
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_CHROMA_SITING_VERTICAL_TOP
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_CHROMA_SITING_VERTICAL_CENTER
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_CHROMA_SITING_VERTICAL_BOTTOM
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_CHROMA_SITING_HORIZONTAL_LEFT
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_CHROMA_SITING_HORIZONTAL_CENTER
   :project: Intel&reg; VPL

.. _codec-format-fourcc:

-----------------
CodecFormatFourCC
-----------------

The CodecFormatFourCC enumerator itemizes codecs in the FourCC format.

.. doxygenenumvalue:: MFX_CODEC_AVC
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_CODEC_HEVC
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_CODEC_MPEG2
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_CODEC_VC1
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_CODEC_VP9
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_CODEC_AV1
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_CODEC_JPEG
   :project: Intel&reg; VPL

----------
CodecLevel
----------

The CodecLevel enumerator itemizes codec levels for all codecs.

.. doxygenenumvalue:: MFX_LEVEL_UNKNOWN
   :project: Intel&reg; VPL

H.264 Level 1-1.3
-----------------

.. doxygenenumvalue:: MFX_LEVEL_AVC_1
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_LEVEL_AVC_1b
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_LEVEL_AVC_11
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_LEVEL_AVC_12
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_LEVEL_AVC_13
   :project: Intel&reg; VPL

H.264 Level 2-2.2
-----------------

.. doxygenenumvalue:: MFX_LEVEL_AVC_2
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_LEVEL_AVC_21
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_LEVEL_AVC_22
   :project: Intel&reg; VPL

H.264 Level 3-3.2
-----------------

.. doxygenenumvalue:: MFX_LEVEL_AVC_3
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_LEVEL_AVC_31
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_LEVEL_AVC_32
   :project: Intel&reg; VPL

H.264 Level 4-4.2
-----------------

.. doxygenenumvalue:: MFX_LEVEL_AVC_4
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_LEVEL_AVC_41
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_LEVEL_AVC_42
   :project: Intel&reg; VPL

H.264 Level 5-5.2
-----------------

.. doxygenenumvalue:: MFX_LEVEL_AVC_5
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_LEVEL_AVC_51
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_LEVEL_AVC_52
   :project: Intel&reg; VPL

H.264 Level 6-6.2
-----------------

.. doxygenenumvalue:: MFX_LEVEL_AVC_6
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_LEVEL_AVC_61
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_LEVEL_AVC_62
   :project: Intel&reg; VPL

MPEG2 Levels
------------

.. doxygenenumvalue:: MFX_LEVEL_MPEG2_LOW
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_LEVEL_MPEG2_MAIN
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_LEVEL_MPEG2_HIGH
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_LEVEL_MPEG2_HIGH1440
   :project: Intel&reg; VPL

VC-1 Level Low (Simple and Main Profiles)
-----------------------------------------

.. doxygenenumvalue:: MFX_LEVEL_VC1_LOW
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_LEVEL_VC1_MEDIAN
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_LEVEL_VC1_HIGH
   :project: Intel&reg; VPL

VC-1 Advanced Profile Levels
----------------------------

.. doxygenenumvalue:: MFX_LEVEL_VC1_0
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_LEVEL_VC1_1
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_LEVEL_VC1_2
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_LEVEL_VC1_3
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_LEVEL_VC1_4
   :project: Intel&reg; VPL

HEVC Levels
-----------

.. doxygenenumvalue:: MFX_LEVEL_HEVC_1
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_LEVEL_HEVC_2
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_LEVEL_HEVC_21
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_LEVEL_HEVC_3
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_LEVEL_HEVC_31
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_LEVEL_HEVC_4
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_LEVEL_HEVC_41
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_LEVEL_HEVC_5
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_LEVEL_HEVC_51
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_LEVEL_HEVC_52
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_LEVEL_HEVC_6
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_LEVEL_HEVC_61
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_LEVEL_HEVC_62
   :project: Intel&reg; VPL

.. _codec_profile:

AV1 Levels
-----------

.. doxygenenumvalue:: MFX_LEVEL_AV1_2
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_LEVEL_AV1_21
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_LEVEL_AV1_22
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_LEVEL_AV1_23
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_LEVEL_AV1_3
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_LEVEL_AV1_31
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_LEVEL_AV1_32
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_LEVEL_AV1_33
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_LEVEL_AV1_4
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_LEVEL_AV1_41
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_LEVEL_AV1_42
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_LEVEL_AV1_43
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_LEVEL_AV1_5
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_LEVEL_AV1_51
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_LEVEL_AV1_52
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_LEVEL_AV1_53
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_LEVEL_AV1_6
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_LEVEL_AV1_61
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_LEVEL_AV1_62
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_LEVEL_AV1_63
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_LEVEL_AV1_7
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_LEVEL_AV1_71
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_LEVEL_AV1_72
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_LEVEL_AV1_73
   :project: Intel&reg; VPL


------------
CodecProfile
------------

The CodecProfile enumerator itemizes codec profiles for all codecs.

.. doxygenenumvalue:: MFX_PROFILE_UNKNOWN
   :project: Intel&reg; VPL

H.264 Profiles
--------------

.. doxygenenumvalue:: MFX_PROFILE_AVC_BASELINE
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_PROFILE_AVC_MAIN
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_PROFILE_AVC_EXTENDED
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_PROFILE_AVC_HIGH
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_PROFILE_AVC_HIGH10
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_PROFILE_AVC_HIGH_422
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_PROFILE_AVC_CONSTRAINED_BASELINE
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_PROFILE_AVC_CONSTRAINED_HIGH
   :project: Intel&reg; VPL

AV1 Profiles
------------

.. doxygenenumvalue:: MFX_PROFILE_AV1_MAIN
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_PROFILE_AV1_HIGH
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_PROFILE_AV1_PRO
   :project: Intel&reg; VPL

VC-1 Profiles
-------------

.. doxygenenumvalue:: MFX_PROFILE_VC1_SIMPLE
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_PROFILE_VC1_MAIN
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_PROFILE_VC1_ADVANCED
   :project: Intel&reg; VPL

VP8 Profiles
------------

.. doxygenenumvalue:: MFX_PROFILE_VP8_0
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_PROFILE_VP8_1
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_PROFILE_VP8_2
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_PROFILE_VP8_3
   :project: Intel&reg; VPL

VP9 Profiles
------------

.. doxygenenumvalue:: MFX_PROFILE_VP9_0
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_PROFILE_VP9_1
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_PROFILE_VP9_2
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_PROFILE_VP9_3
   :project: Intel&reg; VPL

H.264 Constraints
-----------------

Combined with H.264 profile, these flags impose additional constraints. See the
H.264 specification for the list of constraints.

.. doxygenenumvalue:: MFX_PROFILE_AVC_CONSTRAINT_SET0
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_PROFILE_AVC_CONSTRAINT_SET1
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_PROFILE_AVC_CONSTRAINT_SET2
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_PROFILE_AVC_CONSTRAINT_SET3
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_PROFILE_AVC_CONSTRAINT_SET4
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_PROFILE_AVC_CONSTRAINT_SET5
   :project: Intel&reg; VPL

JPEG Profiles
-------------

.. doxygenenumvalue:: MFX_PROFILE_JPEG_BASELINE
   :project: Intel&reg; VPL

-----------------
CodingOptionValue
-----------------

The CodingOptionValue enumerator defines a three-state coding option setting.

.. doxygenenumvalue:: MFX_CODINGOPTION_UNKNOWN
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_CODINGOPTION_ON
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_CODINGOPTION_OFF
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_CODINGOPTION_ADAPTIVE
   :project: Intel&reg; VPL

-----------
ColorFourCC
-----------

The ColorFourCC enumerator itemizes color formats.

.. doxygenenumvalue:: MFX_FOURCC_NV12
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_FOURCC_NV21
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_FOURCC_YV12
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_FOURCC_IYUV
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_FOURCC_I420
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_FOURCC_I422
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_FOURCC_NV16
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_FOURCC_YUY2
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_FOURCC_RGB565
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_FOURCC_RGBP
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_FOURCC_RGB4
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_FOURCC_BGRA
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_FOURCC_P8
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_FOURCC_P8_TEXTURE
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_FOURCC_P010
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_FOURCC_I010
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_FOURCC_I210
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_FOURCC_P016
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_FOURCC_P210
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_FOURCC_BGR4
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_FOURCC_A2RGB10
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_FOURCC_ARGB16
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_FOURCC_ABGR16
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_FOURCC_R16
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_FOURCC_AYUV
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_FOURCC_AYUV_RGB4
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_FOURCC_UYVY
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_FOURCC_Y210
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_FOURCC_Y410
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_FOURCC_Y216
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_FOURCC_Y416
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_FOURCC_BGRP
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_FOURCC_XYUV
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_FOURCC_ABGR16F
   :project: Intel&reg; VPL

-----------
ContentInfo
-----------

The ContentInfo enumerator itemizes content types for the encoding session.

.. doxygenenumvalue:: MFX_CONTENT_UNKNOWN
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_CONTENT_FULL_SCREEN_VIDEO
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_CONTENT_NON_VIDEO_SCREEN
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_CONTENT_NOISY_VIDEO
   :project: Intel&reg; VPL

----------
Corruption
----------

The Corruption enumerator itemizes the decoding corruption types. It is a
bit-OR’ed value of the following.

.. doxygenenumvalue:: MFX_CORRUPTION_NO
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_CORRUPTION_MINOR
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_CORRUPTION_MAJOR
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_CORRUPTION_ABSENT_TOP_FIELD
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_CORRUPTION_ABSENT_BOTTOM_FIELD
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_CORRUPTION_REFERENCE_FRAME
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_CORRUPTION_REFERENCE_LIST
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_CORRUPTION_HW_RESET
   :project: Intel&reg; VPL

.. note:: Flag MFX_CORRUPTION_ABSENT_TOP_FIELD/MFX_CORRUPTION_ABSENT_BOTTOM_FIELD
         is set by the AVC decoder when it detects that one of fields is not
         present in the bitstream. Which field is absent depends on value of
         bottom_field_flag (ITU-T\* H.264 7.4.3).

-----------------
DeinterlacingMode
-----------------

The DeinterlacingMode enumerator itemizes VPP deinterlacing modes.

.. doxygenenumvalue:: MFX_DEINTERLACING_BOB
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_DEINTERLACING_ADVANCED
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_DEINTERLACING_AUTO_DOUBLE
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_DEINTERLACING_AUTO_SINGLE
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_DEINTERLACING_FULL_FR_OUT
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_DEINTERLACING_HALF_FR_OUT
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_DEINTERLACING_24FPS_OUT
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_DEINTERLACING_FIXED_TELECINE_PATTERN
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_DEINTERLACING_30FPS_OUT
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_DEINTERLACING_DETECT_INTERLACE
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_DEINTERLACING_ADVANCED_NOREF
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_DEINTERLACING_ADVANCED_SCD
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_DEINTERLACING_FIELD_WEAVING
   :project: Intel&reg; VPL

----------
ErrorTypes
----------

The ErrorTypes enumerator uses bit-ORed values to itemize bitstream error types.

.. doxygenenumvalue:: MFX_ERROR_NO
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_ERROR_PPS
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_ERROR_SPS
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_ERROR_SLICEHEADER
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_ERROR_SLICEDATA
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_ERROR_FRAME_GAP
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_ERROR_JPEG_APP0_MARKER
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_ERROR_JPEG_APP1_MARKER
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_ERROR_JPEG_APP2_MARKER
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_ERROR_JPEG_APP3_MARKER
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_ERROR_JPEG_APP4_MARKER
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_ERROR_JPEG_APP5_MARKER
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_ERROR_JPEG_APP6_MARKER
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_ERROR_JPEG_APP7_MARKER
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_ERROR_JPEG_APP8_MARKER
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_ERROR_JPEG_APP9_MARKER
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_ERROR_JPEG_APP10_MARKER
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_ERROR_JPEG_APP11_MARKER
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_ERROR_JPEG_APP12_MARKER
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_ERROR_JPEG_APP13_MARKER
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_ERROR_JPEG_APP14_MARKER
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_ERROR_JPEG_DQT_MARKER
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_ERROR_JPEG_SOF0_MARKER
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_ERROR_JPEG_DHT_MARKER
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_ERROR_JPEG_DRI_MARKER
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_ERROR_JPEG_SOS_MARKER
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_ERROR_JPEG_UNKNOWN_MARKER
   :project: Intel&reg; VPL

.. _extendedbufferid:

----------------
ExtendedBufferID
----------------

The ExtendedBufferID enumerator itemizes and defines identifiers (BufferId) for
extended buffers or video processing algorithm identifiers.

.. doxygenenumvalue:: MFX_EXTBUFF_THREADS_PARAM
  :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_CODING_OPTION
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_CODING_OPTION_SPSPPS
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_DONOTUSE
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_AUXDATA
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_DENOISE2
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_3DLUT
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_SCENE_ANALYSIS
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_PROCAMP
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_DETAIL
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_VIDEO_SIGNAL_INFO
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_VIDEO_SIGNAL_INFO_IN
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_VIDEO_SIGNAL_INFO_OUT
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_DOUSE
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_AVC_REFLIST_CTRL
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_FRAME_RATE_CONVERSION
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_PICTURE_TIMING_SEI
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_AVC_TEMPORAL_LAYERS
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_CODING_OPTION2
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_IMAGE_STABILIZATION
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_ENCODER_CAPABILITY
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_ENCODER_RESET_OPTION
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_ENCODED_FRAME_INFO
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_COMPOSITE
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_VIDEO_SIGNAL_INFO
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_ENCODER_ROI
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_DEINTERLACING
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_AVC_REFLISTS
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_DEC_VIDEO_PROCESSING
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_FIELD_PROCESSING
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_CODING_OPTION3
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_CHROMA_LOC_INFO
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_MBQP
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_MB_FORCE_INTRA
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_HEVC_TILES
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_MB_DISABLE_SKIP_MAP
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_HEVC_PARAM
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_DECODED_FRAME_INFO
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_TIME_CODE
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_HEVC_REGION
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_PRED_WEIGHT_TABLE
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_DIRTY_RECTANGLES
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_MOVING_RECTANGLES
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_CODING_OPTION_VPS
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_ROTATION
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_ENCODED_SLICES_INFO
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_SCALING
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_HEVC_REFLIST_CTRL
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_HEVC_REFLISTS
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_HEVC_TEMPORAL_LAYERS
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_MIRRORING
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_MV_OVER_PIC_BOUNDARIES
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_COLORFILL
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_DECODE_ERROR_REPORT
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_COLOR_CONVERSION
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_CONTENT_LIGHT_LEVEL_INFO
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_MASTERING_DISPLAY_COLOUR_VOLUME
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_MASTERING_DISPLAY_COLOUR_VOLUME_IN
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_MASTERING_DISPLAY_COLOUR_VOLUME_OUT
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_ENCODED_UNITS_INFO
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_MCTF
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_VP9_SEGMENTATION
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_VP9_TEMPORAL_LAYERS
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_VP9_PARAM
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_AVC_ROUNDING_OFFSET
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_PARTIAL_BITSTREAM_PARAM
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_BRC
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_VP8_CODING_OPTION
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_JPEG_QT
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_JPEG_HUFFMAN
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_ENCODER_IPCM_AREA
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_INSERT_HEADERS
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_MVC_SEQ_DESC
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_MVC_TARGET_VIEWS
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_CENC_PARAM
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_DEVICE_AFFINITY_MASK
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_CROPS
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_AV1_FILM_GRAIN_PARAM
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_AV1_SEGMENTATION
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_ALLOCATION_HINTS
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_UNIVERSAL_TEMPORAL_LAYERS
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_UNIVERSAL_REFLIST_CTRL
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_ENCODESTATS
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_SYNCSUBMISSION
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_TUNE_ENCODE_QUALITY
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_PERC_ENC_PREFILTER
   :project: Intel&reg; VPL

----------------
ExtMemBufferType
----------------

.. doxygenenumvalue:: MFX_MEMTYPE_PERSISTENT_MEMORY
   :project: Intel&reg; VPL

.. _extmemframetype:

---------------
ExtMemFrameType
---------------

The ExtMemFrameType enumerator specifies the memory type of frame. It is a
bit-ORed value of one of the following. For information on working with video
memory surfaces, see the :ref:`Working with Hardware Acceleration section<hw-acceleration>`.

.. doxygenenumvalue:: MFX_MEMTYPE_DXVA2_DECODER_TARGET
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_MEMTYPE_DXVA2_PROCESSOR_TARGET
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_MEMTYPE_VIDEO_MEMORY_DECODER_TARGET
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_MEMTYPE_VIDEO_MEMORY_PROCESSOR_TARGET
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_MEMTYPE_SYSTEM_MEMORY
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_MEMTYPE_RESERVED1
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_MEMTYPE_FROM_ENCODE
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_MEMTYPE_FROM_DECODE
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_MEMTYPE_FROM_VPPIN
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_MEMTYPE_FROM_VPPOUT
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_MEMTYPE_FROM_ENC
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_MEMTYPE_INTERNAL_FRAME
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_MEMTYPE_EXTERNAL_FRAME
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_MEMTYPE_EXPORT_FRAME
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_MEMTYPE_SHARED_RESOURCE
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_MEMTYPE_VIDEO_MEMORY_ENCODER_TARGET
   :project: Intel&reg; VPL

----------------
Frame Data Flags
----------------

.. doxygenenumvalue:: MFX_TIMESTAMP_UNKNOWN
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_FRAMEORDER_UNKNOWN
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_FRAMEDATA_TIMESTAMP_UNKNOWN
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_FRAMEDATA_ORIGINAL_TIMESTAMP
   :project: Intel&reg; VPL

---------
FrameType
---------

The FrameType enumerator itemizes frame types. Use bit-ORed values to specify
all that apply.

.. doxygenenumvalue:: MFX_FRAMETYPE_UNKNOWN
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_FRAMETYPE_I
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_FRAMETYPE_P
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_FRAMETYPE_B
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_FRAMETYPE_S
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_FRAMETYPE_REF
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_FRAMETYPE_IDR
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_FRAMETYPE_xI
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_FRAMETYPE_xP
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_FRAMETYPE_xB
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_FRAMETYPE_xS
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_FRAMETYPE_xREF
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_FRAMETYPE_xIDR
   :project: Intel&reg; VPL

-------
FrcAlgm
-------

The FrcAlgm enumerator itemizes frame rate conversion algorithms. See description
of mfxExtVPPFrameRateConversion structure for more details.

.. doxygenenumvalue:: MFX_FRCALGM_PRESERVE_TIMESTAMP
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_FRCALGM_DISTRIBUTED_TIMESTAMP
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_FRCALGM_FRAME_INTERPOLATION
   :project: Intel&reg; VPL

----------------------
GeneralConstraintFlags
----------------------

The GeneralConstraintFlags enumerator uses bit-ORed values to itemize HEVC bitstream
indications for specific profiles. Each value indicates for format range extensions profiles.
To specify HEVC Main 10 Still Picture profile applications have to set
mfxInfoMFX::CodecProfile == MFX_PROFILE_HEVC_MAIN10 and
mfxExtHEVCParam::GeneralConstraintFlags == MFX_HEVC_CONSTR_REXT_ONE_PICTURE_ONLY.

.. doxygenenumvalue:: MFX_HEVC_CONSTR_REXT_MAX_12BIT
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_HEVC_CONSTR_REXT_MAX_10BIT
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_HEVC_CONSTR_REXT_MAX_8BIT
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_HEVC_CONSTR_REXT_MAX_422CHROMA
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_HEVC_CONSTR_REXT_MAX_420CHROMA
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_HEVC_CONSTR_REXT_MAX_MONOCHROME
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_HEVC_CONSTR_REXT_INTRA
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_HEVC_CONSTR_REXT_ONE_PICTURE_ONLY
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_HEVC_CONSTR_REXT_LOWER_BIT_RATE
   :project: Intel&reg; VPL

----------
GopOptFlag
----------

The GopOptFlag enumerator itemizes special properties in the GOP (Group of
Pictures) sequence.

.. doxygenenumvalue:: MFX_GOP_CLOSED
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_GOP_STRICT
   :project: Intel&reg; VPL

.. _gpu_copy:

-------
GPUCopy
-------

.. doxygenenumvalue:: MFX_GPUCOPY_DEFAULT
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_GPUCOPY_ON
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_GPUCOPY_OFF
   :project: Intel&reg; VPL

-------------
HEVC Profiles
-------------

.. doxygenenumvalue:: MFX_PROFILE_HEVC_MAIN
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_PROFILE_HEVC_MAIN10
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_PROFILE_HEVC_MAINSP
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_PROFILE_HEVC_REXT
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_PROFILE_HEVC_SCC
   :project: Intel&reg; VPL

----------
HEVC Tiers
----------

.. doxygenenumvalue:: MFX_TIER_HEVC_MAIN
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_TIER_HEVC_HIGH
   :project: Intel&reg; VPL

------------------
HEVCRegionEncoding
------------------
The HEVCRegionEncoding enumerator itemizes HEVC region's encoding.

.. doxygenenumvalue:: MFX_HEVC_REGION_ENCODING_ON
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_HEVC_REGION_ENCODING_OFF
   :project: Intel&reg; VPL

--------------
HEVCRegionType
--------------

The HEVCRegionType enumerator itemizes type of HEVC region.

.. doxygenenumvalue:: MFX_HEVC_REGION_SLICE
   :project: Intel&reg; VPL

-------------
ImageStabMode
-------------

The ImageStabMode enumerator itemizes image stabilization modes. See description
of mfxExtVPPImageStab structure for more details.

.. doxygenenumvalue:: MFX_IMAGESTAB_MODE_UPSCALE
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_IMAGESTAB_MODE_BOXING
   :project: Intel&reg; VPL

----------------
InsertHDRPayload
----------------

The InsertHDRPayload enumerator itemizes HDR payloads insertion rules.

.. doxygenenumvalue:: MFX_PAYLOAD_OFF
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_PAYLOAD_IDR
   :project: Intel&reg; VPL

-----------------
InterpolationMode
-----------------

The InterpolationMode enumerator specifies type of interpolation method used by
VPP scaling filter.

.. doxygenenumvalue:: MFX_INTERPOLATION_DEFAULT
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_INTERPOLATION_NEAREST_NEIGHBOR
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_INTERPOLATION_BILINEAR
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_INTERPOLATION_ADVANCED
   :project: Intel&reg; VPL

--------
DataType
--------

.. doxygenenum:: mfxDataType
   :project: Intel&reg; VPL

-------------------
3DLutChannelMapping
-------------------

.. doxygenenum:: mfx3DLutChannelMapping
   :project: Intel&reg; VPL

-----------------
3DLutMemoryLayout
-----------------

.. doxygenenum:: mfx3DLutMemoryLayout
   :project: Intel&reg; VPL

-------------------------------------
IntraPredBlockSize/InterPredBlockSize
-------------------------------------

IntraPredBlockSize/InterPredBlockSize specifies minimum block size of inter-prediction.

.. doxygenenumvalue:: MFX_BLOCKSIZE_UNKNOWN
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_BLOCKSIZE_MIN_16X16
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_BLOCKSIZE_MIN_8X8
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_BLOCKSIZE_MIN_4X4
   :project: Intel&reg; VPL

-----------------
IntraRefreshTypes
-----------------

The IntraRefreshTypes enumerator itemizes types of intra refresh.

.. doxygenenumvalue:: MFX_REFRESH_NO
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_REFRESH_VERTICAL
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_REFRESH_HORIZONTAL
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_REFRESH_SLICE
   :project: Intel&reg; VPL

---------
IOPattern
---------

The IOPattern enumerator itemizes memory access patterns for API functions.
Use bit-ORed values to specify input and output access patterns.

.. doxygenenumvalue:: MFX_IOPATTERN_IN_VIDEO_MEMORY
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_IOPATTERN_IN_SYSTEM_MEMORY
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_IOPATTERN_OUT_VIDEO_MEMORY
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_IOPATTERN_OUT_SYSTEM_MEMORY
   :project: Intel&reg; VPL

---------------
JPEGColorFormat
---------------

The JPEGColorFormat enumerator itemizes the JPEG color format options.

.. doxygenenumvalue:: MFX_JPEG_COLORFORMAT_UNKNOWN
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_JPEG_COLORFORMAT_YCbCr
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_JPEG_COLORFORMAT_RGB
   :project: Intel&reg; VPL

------------
JPEGScanType
------------

The JPEGScanType enumerator itemizes the JPEG scan types.

.. doxygenenumvalue:: MFX_SCANTYPE_UNKNOWN
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_SCANTYPE_INTERLEAVED
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_SCANTYPE_NONINTERLEAVED
   :project: Intel&reg; VPL

-----------
LongTermIdx
-----------

The LongTermIdx specifies long term index of picture control

.. doxygenenumvalue:: MFX_LONGTERM_IDX_NO_IDX
   :project: Intel&reg; VPL

---------------------
LookAheadDownSampling
---------------------

The LookAheadDownSampling enumerator is used to control down sampling in look
ahead bitrate control mode in AVC encoder.

.. doxygenenumvalue:: MFX_LOOKAHEAD_DS_UNKNOWN
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_LOOKAHEAD_DS_OFF
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_LOOKAHEAD_DS_2x
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_LOOKAHEAD_DS_4x
   :project: Intel&reg; VPL

--------
MBQPMode
--------

The MBQPMode enumerator itemizes QP update modes.

.. doxygenenumvalue:: MFX_MBQP_MODE_QP_VALUE
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_MBQP_MODE_QP_DELTA
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_MBQP_MODE_QP_ADAPTIVE
   :project: Intel&reg; VPL

----------------
mfxComponentType
----------------

.. doxygenenum:: mfxComponentType
   :project: Intel&reg; VPL

-------------
mfxHandleType
-------------

.. doxygenenum:: mfxHandleType
   :project: Intel&reg; VPL

-------
mfxIMPL
-------

.. doxygentypedef:: mfxIMPL
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_IMPL_AUTO
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_IMPL_SOFTWARE
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_IMPL_HARDWARE
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_IMPL_AUTO_ANY
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_IMPL_HARDWARE_ANY
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_IMPL_HARDWARE2
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_IMPL_HARDWARE3
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_IMPL_HARDWARE4
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_IMPL_RUNTIME
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_IMPL_VIA_ANY
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_IMPL_VIA_D3D9
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_IMPL_VIA_D3D11
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_IMPL_VIA_VAAPI
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_IMPL_VIA_HDDLUNITE
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_IMPL_UNSUPPORTED
   :project: Intel&reg; VPL

.. doxygendefine:: MFX_IMPL_BASETYPE
   :project: Intel&reg; VPL

-------------------------
mfxImplCapsDeliveryFormat
-------------------------

.. doxygenenum:: mfxImplCapsDeliveryFormat
   :project: Intel&reg; VPL

-------------------
mfxMediaAdapterType
-------------------

.. doxygenenum:: mfxMediaAdapterType
   :project: Intel&reg; VPL

--------------
mfxMemoryFlags
--------------

.. doxygenenum:: mfxMemoryFlags
   :project: Intel&reg; VPL

--------------
MfxNalUnitType
--------------

Specifies NAL unit types supported by the HEVC encoder.

.. doxygenenumvalue:: MFX_HEVC_NALU_TYPE_UNKNOWN
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_HEVC_NALU_TYPE_TRAIL_N
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_HEVC_NALU_TYPE_TRAIL_R
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_HEVC_NALU_TYPE_RADL_N
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_HEVC_NALU_TYPE_RADL_R
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_HEVC_NALU_TYPE_RASL_N
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_HEVC_NALU_TYPE_RASL_R
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_HEVC_NALU_TYPE_IDR_W_RADL
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_HEVC_NALU_TYPE_IDR_N_LP
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_HEVC_NALU_TYPE_CRA_NUT
   :project: Intel&reg; VPL

-----------
mfxPriority
-----------

.. doxygenenum:: mfxPriority
   :project: Intel&reg; VPL

---------------
mfxResourceType
---------------

.. doxygenenum:: mfxResourceType
   :project: Intel&reg; VPL

-----------
mfxSkipMode
-----------

.. doxygenenum:: mfxSkipMode
   :project: Intel&reg; VPL

---------
mfxStatus
---------

.. doxygenenum:: mfxStatus
   :project: Intel&reg; VPL

-------------
MirroringType
-------------

The MirroringType enumerator itemizes mirroring types.

.. doxygenenumvalue:: MFX_MIRRORING_DISABLED
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_MIRRORING_HORIZONTAL
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_MIRRORING_VERTICAL
   :project: Intel&reg; VPL

-----------
DenoiseMode
-----------

The mfxDenoiseMode enumerator itemizes denoise modes.

.. doxygenenum:: mfxDenoiseMode
   :project: Intel&reg; VPL

---------------
MPEG-2 Profiles
---------------

.. doxygenenumvalue:: MFX_PROFILE_MPEG2_SIMPLE
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_PROFILE_MPEG2_MAIN
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_PROFILE_MPEG2_HIGH
   :project: Intel&reg; VPL

------------------------------------------
Multi-view Video Coding Extension Profiles
------------------------------------------

.. doxygenenumvalue:: MFX_PROFILE_AVC_MULTIVIEW_HIGH
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_PROFILE_AVC_STEREO_HIGH
   :project: Intel&reg; VPL

-----------
MVPrecision
-----------

The MVPrecision enumerator specifies the motion estimation precision

.. doxygenenumvalue:: MFX_MVPRECISION_UNKNOWN
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_MVPRECISION_INTEGER
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_MVPRECISION_HALFPEL
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_MVPRECISION_QUARTERPEL
   :project: Intel&reg; VPL

------------
NominalRange
------------

The NominalRange enumerator itemizes pixel's value nominal range.

.. doxygenenumvalue:: MFX_NOMINALRANGE_UNKNOWN
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_NOMINALRANGE_0_255
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_NOMINALRANGE_16_235
   :project: Intel&reg; VPL

----------------------
PartialBitstreamOutput
----------------------

The PartialBitstreamOutput enumerator indicates flags of partial bitstream output
type.

.. doxygenenumvalue:: MFX_PARTIAL_BITSTREAM_NONE
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_PARTIAL_BITSTREAM_SLICE
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_PARTIAL_BITSTREAM_BLOCK
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_PARTIAL_BITSTREAM_ANY
   :project: Intel&reg; VPL

----------------
PayloadCtrlFlags
----------------

The PayloadCtrlFlags enumerator itemizes additional payload properties.

.. doxygenenumvalue:: MFX_PAYLOAD_CTRL_SUFFIX
   :project: Intel&reg; VPL

.. _PicStruct:

---------
PicStruct
---------

The PicStruct enumerator itemizes picture structure. Use bit-OR’ed values to
specify the desired picture type.

.. doxygenenumvalue:: MFX_PICSTRUCT_UNKNOWN
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_PICSTRUCT_PROGRESSIVE
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_PICSTRUCT_FIELD_TFF
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_PICSTRUCT_FIELD_BFF
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_PICSTRUCT_FIELD_REPEATED
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_PICSTRUCT_FRAME_DOUBLING
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_PICSTRUCT_FRAME_TRIPLING
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_PICSTRUCT_FIELD_SINGLE
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_PICSTRUCT_FIELD_TOP
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_PICSTRUCT_FIELD_BOTTOM
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_PICSTRUCT_FIELD_PAIRED_PREV
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_PICSTRUCT_FIELD_PAIRED_NEXT
   :project: Intel&reg; VPL

-------
PicType
-------

The PicType enumerator itemizes picture type.

.. doxygenenumvalue:: MFX_PICTYPE_UNKNOWN
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_PICTYPE_FRAME
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_PICTYPE_TOPFIELD
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_PICTYPE_BOTTOMFIELD
   :project: Intel&reg; VPL

--------
PRefType
--------

The PRefType enumerator itemizes models of reference list construction and DPB
management when GopRefDist=1.

.. doxygenenumvalue:: MFX_P_REF_DEFAULT
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_P_REF_SIMPLE
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_P_REF_PYRAMID
   :project: Intel&reg; VPL

-----------
TuneQuality
-----------

The TuneQuality enumerator specifies tuning option for encode. Multiple tuning options
can be combined using bit mask.

.. doxygenenumvalue:: MFX_ENCODE_TUNE_OFF
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_ENCODE_TUNE_PSNR
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_ENCODE_TUNE_SSIM
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_ENCODE_TUNE_MS_SSIM
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_ENCODE_TUNE_VMAF
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_ENCODE_TUNE_PERCEPTUAL
   :project: Intel&reg; VPL

---------
Protected
---------

The Protected enumerator describes the protection schemes.

.. doxygenenumvalue:: MFX_PROTECTION_CENC_WV_CLASSIC
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_PROTECTION_CENC_WV_GOOGLE_DASH
   :project: Intel&reg; VPL

.. _RateControlMethod:

-----------------
RateControlMethod
-----------------

The RateControlMethod enumerator itemizes bitrate control methods.

.. doxygenenumvalue:: MFX_RATECONTROL_CBR
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_RATECONTROL_VBR
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_RATECONTROL_CQP
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_RATECONTROL_AVBR
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_RATECONTROL_LA
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_RATECONTROL_ICQ
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_RATECONTROL_VCM
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_RATECONTROL_LA_ICQ
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_RATECONTROL_LA_HRD
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_RATECONTROL_QVBR
   :project: Intel&reg; VPL

-------
ROImode
-------

The ROImode enumerator itemizes QP adjustment mode for ROIs.

.. doxygenenumvalue:: MFX_ROI_MODE_PRIORITY
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_ROI_MODE_QP_DELTA
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_ROI_MODE_QP_VALUE
   :project: Intel&reg; VPL

--------
Rotation
--------

The Rotation enumerator itemizes the JPEG rotation options.

.. doxygenenumvalue:: MFX_ROTATION_0
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_ROTATION_90
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_ROTATION_180
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_ROTATION_270
   :project: Intel&reg; VPL

--------------------
SampleAdaptiveOffset
--------------------

The SampleAdaptiveOffset enumerator uses bit-ORed values to itemize corresponding
HEVC encoding feature.

.. doxygenenumvalue:: MFX_SAO_UNKNOWN
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_SAO_DISABLE
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_SAO_ENABLE_LUMA
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_SAO_ENABLE_CHROMA
   :project: Intel&reg; VPL

-----------
ScalingMode
-----------

The ScalingMode enumerator itemizes variants of scaling filter implementation.

.. doxygenenumvalue:: MFX_SCALING_MODE_DEFAULT
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_SCALING_MODE_LOWPOWER
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_SCALING_MODE_QUALITY
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_SCALING_MODE_VENDOR
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_SCALING_MODE_INTEL_GEN_COMPUTE
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_SCALING_MODE_INTEL_GEN_VDBOX
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_SCALING_MODE_INTEL_GEN_VEBOX
   :project: Intel&reg; VPL

------------
ScenarioInfo
------------

The ScenarioInfo enumerator itemizes scenarios for the encoding session.

.. doxygenenumvalue:: MFX_SCENARIO_UNKNOWN
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_SCENARIO_DISPLAY_REMOTING
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_SCENARIO_VIDEO_CONFERENCE
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_SCENARIO_ARCHIVE
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_SCENARIO_LIVE_STREAMING
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_SCENARIO_CAMERA_CAPTURE
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_SCENARIO_VIDEO_SURVEILLANCE
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_SCENARIO_GAME_STREAMING
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_SCENARIO_REMOTE_GAMING
   :project: Intel&reg; VPL

--------------
SegmentFeature
--------------

The SegmentFeature enumerator indicates features enabled for the segment.
These values are used with the mfxVP9SegmentParam::FeatureEnabled parameter.

.. doxygenenumvalue:: MFX_VP9_SEGMENT_FEATURE_QINDEX
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_VP9_SEGMENT_FEATURE_LOOP_FILTER
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_VP9_SEGMENT_FEATURE_REFERENCE
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_VP9_SEGMENT_FEATURE_SKIP
   :project: Intel&reg; VPL

------------------
SegmentIdBlockSize
------------------

The SegmentIdBlockSize enumerator indicates the block size represented by each
segment_id in segmentation map. These values are used with the
mfxExtVP9Segmentation::SegmentIdBlockSize parameter.

.. doxygenenumvalue:: MFX_VP9_SEGMENT_ID_BLOCK_SIZE_UNKNOWN
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_VP9_SEGMENT_ID_BLOCK_SIZE_8x8
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_VP9_SEGMENT_ID_BLOCK_SIZE_16x16
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_VP9_SEGMENT_ID_BLOCK_SIZE_32x32
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_VP9_SEGMENT_ID_BLOCK_SIZE_64x64
   :project: Intel&reg; VPL

---------
SkipFrame
---------

The SkipFrame enumerator is used to define usage of mfxEncodeCtrl::SkipFrame
parameter.

.. doxygenenumvalue:: MFX_SKIPFRAME_NO_SKIP
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_SKIPFRAME_INSERT_DUMMY
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_SKIPFRAME_INSERT_NOTHING
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_SKIPFRAME_BRC_ONLY
   :project: Intel&reg; VPL

-----------
TargetUsage
-----------

The TargetUsage enumerator itemizes a range of numbers from MFX_TARGETUSAGE_1,
best quality, to MFX_TARGETUSAGE_7, best speed. It indicates trade-offs between
quality and speed. The application can use any number in the range. The actual
number of supported target usages depends on implementation. If the specified
target usage is not supported, the encoder will use the closest supported value.

.. doxygenenumvalue:: MFX_TARGETUSAGE_1
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_TARGETUSAGE_2
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_TARGETUSAGE_3
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_TARGETUSAGE_4
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_TARGETUSAGE_5
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_TARGETUSAGE_6
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_TARGETUSAGE_7
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_TARGETUSAGE_UNKNOWN
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_TARGETUSAGE_BEST_QUALITY
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_TARGETUSAGE_BALANCED
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_TARGETUSAGE_BEST_SPEED
   :project: Intel&reg; VPL

---------------
TelecinePattern
---------------

The TelecinePattern enumerator itemizes telecine patterns.

.. doxygenenumvalue:: MFX_TELECINE_PATTERN_32
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_TELECINE_PATTERN_2332
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_TELECINE_PATTERN_FRAME_REPEAT
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_TELECINE_PATTERN_41
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_TELECINE_POSITION_PROVIDED
   :project: Intel&reg; VPL

-------------
TimeStampCalc
-------------

The TimeStampCalc enumerator itemizes time-stamp calculation methods.

.. doxygenenumvalue:: MFX_TIMESTAMPCALC_UNKNOWN
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_TIMESTAMPCALC_TELECINE
   :project: Intel&reg; VPL

--------------
TransferMatrix
--------------

The TransferMatrix enumerator itemizes color transfer matrices.

.. doxygenenumvalue:: MFX_TRANSFERMATRIX_UNKNOWN
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_TRANSFERMATRIX_BT709
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_TRANSFERMATRIX_BT601
   :project: Intel&reg; VPL

--------------
TrellisControl
--------------

The TrellisControl enumerator is used to control trellis quantization in AVC
encoder. The application can turn it on or off for any combination of I, P, and
B frames by combining different enumerator values. For example,
MFX_TRELLIS_I | MFX_TRELLIS_B turns it on for I and B frames.

.. doxygenenumvalue:: MFX_TRELLIS_UNKNOWN
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_TRELLIS_OFF
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_TRELLIS_I
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_TRELLIS_P
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_TRELLIS_B
   :project: Intel&reg; VPL

-----------------
VP9ReferenceFrame
-----------------

The VP9ReferenceFrame enumerator itemizes reference frame type by
the mfxVP9SegmentParam::ReferenceFrame parameter.

.. doxygenenumvalue:: MFX_VP9_REF_INTRA
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_VP9_REF_LAST
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_VP9_REF_GOLDEN
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_VP9_REF_ALTREF
   :project: Intel&reg; VPL

----------------------
VPPFieldProcessingMode
----------------------

The VPPFieldProcessingMode enumerator is used to control VPP field processing
algorithm.

.. doxygenenumvalue:: MFX_VPP_COPY_FRAME
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_VPP_COPY_FIELD
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_VPP_SWAP_FIELDS
   :project: Intel&reg; VPL

------------
WeightedPred
------------

The WeightedPred enumerator itemizes weighted prediction modes.

.. doxygenenumvalue:: MFX_WEIGHTED_PRED_UNKNOWN
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_WEIGHTED_PRED_DEFAULT
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_WEIGHTED_PRED_EXPLICIT
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_WEIGHTED_PRED_IMPLICIT
   :project: Intel&reg; VPL

--------------
FilmGrainFlags
--------------

The FilmGrainFlags enumerator itemizes flags in AV1 film grain parameters.

.. doxygenenumvalue:: MFX_FILM_GRAIN_NO
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_FILM_GRAIN_APPLY
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_FILM_GRAIN_UPDATE
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_FILM_GRAIN_CHROMA_SCALING_FROM_LUMA
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_FILM_GRAIN_OVERLAP
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_FILM_GRAIN_CLIP_TO_RESTRICTED_RANGE
   :project: Intel&reg; VPL

------------
mfxHyperMode
------------

.. doxygenenum:: mfxHyperMode
   :project: Intel&reg; VPL

-----------------------
mfxPoolAllocationPolicy
-----------------------

.. doxygenenum:: mfxPoolAllocationPolicy
   :project: Intel&reg; VPL

--------------
mfxVPPPoolType
--------------

.. doxygenenum:: mfxVPPPoolType
   :project: Intel&reg; VPL

------------------------
mfxAV1SegmentIdBlockSize
------------------------

The mfxAV1SegmentIdBlockSize enumerator indicates the block size represented by each segment_id in segmentation map.

.. doxygenenum:: mfxAV1SegmentIdBlockSize
   :project: Intel&reg; VPL

-----------------
AV1SegmentFeature
-----------------

The AV1SegmentFeature enumerator indicates features enabled for the segment.

.. doxygenenumvalue:: MFX_AV1_SEGMENT_FEATURE_ALT_QINDEX
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_AV1_SEGMENT_FEATURE_ALT_LF_Y_VERT
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_AV1_SEGMENT_FEATURE_ALT_LF_Y_HORZ
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_AV1_SEGMENT_FEATURE_ALT_LF_U
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_AV1_SEGMENT_FEATURE_ALT_LF_V
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_AV1_SEGMENT_FEATURE_REFERENCE
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_AV1_SEGMENT_FEATURE_SKIP
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_AV1_SEGMENT_FEATURE_GLOBALMV
   :project: Intel&reg; VPL

--------------------------
mfxEncodeBlkStatsMemLayout
--------------------------

.. doxygenenum:: mfxEncodeBlkStatsMemLayout
   :project: Intel&reg; VPL

------------------
mfxEncodeStatsMode
------------------

.. doxygenenum:: mfxEncodeStatsMode
   :project: Intel&reg; VPL

----------------
EncodeStatsLevel
----------------

Flags to specify what statistics will be reported by the implementation.

.. doxygenenumvalue:: MFX_ENCODESTATS_LEVEL_BLK
   :project: Intel&reg; VPL

.. doxygenenumvalue:: MFX_ENCODESTATS_LEVEL_FRAME
   :project: Intel&reg; VPL


-------------------
mfxSurfaceComponent
-------------------

.. doxygenenum:: mfxSurfaceComponent
   :project: Intel&reg; VPL


--------------
mfxSurfaceType
--------------

.. doxygenenum:: mfxSurfaceType
   :project: Intel&reg; VPL

----------------
mfxStructureType
----------------

.. doxygenenum:: mfxStructureType
   :project: Intel&reg; VPL
