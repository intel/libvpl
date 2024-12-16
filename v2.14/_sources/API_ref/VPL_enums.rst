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
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_ANGLE_90
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_ANGLE_180
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_ANGLE_270
   :project: DEF_BREATHE_PROJECT

-----------------
BitstreamDataFlag
-----------------

The BitstreamDataFlag enumerator uses bit-ORed values to itemize additional
information about the bitstream buffer.

.. doxygenenumvalue:: MFX_BITSTREAM_NO_FLAG
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_BITSTREAM_COMPLETE_FRAME
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_BITSTREAM_EOS
   :project: DEF_BREATHE_PROJECT

------------
BPSEIControl
------------

The BPSEIControl enumerator is used to control insertion of buffering period SEI
in the encoded bitstream.

.. doxygenenumvalue:: MFX_BPSEI_DEFAULT
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_BPSEI_IFRAME
   :project: DEF_BREATHE_PROJECT

---------
BRCStatus
---------

The BRCStatus enumerator itemizes instructions to the encoder by
:cpp:member:`mfxExtBrc::Update`.

.. doxygenenumvalue:: MFX_BRC_OK
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_BRC_BIG_FRAME
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_BRC_SMALL_FRAME
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_BRC_PANIC_BIG_FRAME
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_BRC_PANIC_SMALL_FRAME
   :project: DEF_BREATHE_PROJECT

-----------
BRefControl
-----------

The BRefControl enumerator is used to control usage of B frames as reference in
AVC encoder.

.. doxygenenumvalue:: MFX_B_REF_UNKNOWN
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_B_REF_OFF
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_B_REF_PYRAMID
   :project: DEF_BREATHE_PROJECT

----------------
ChromaFormateIdc
----------------

The ChromaFormatIdc enumerator itemizes color-sampling formats.

.. doxygenenumvalue:: MFX_CHROMAFORMAT_MONOCHROME
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_CHROMAFORMAT_YUV420
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_CHROMAFORMAT_YUV422
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_CHROMAFORMAT_YUV444
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_CHROMAFORMAT_YUV400
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_CHROMAFORMAT_YUV411
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_CHROMAFORMAT_YUV422H
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_CHROMAFORMAT_YUV422V
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_CHROMAFORMAT_RESERVED1
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_CHROMAFORMAT_JPEG_SAMPLING
   :project: DEF_BREATHE_PROJECT

------------
ChromaSiting
------------

The ChromaSiting enumerator defines chroma location. Use bit-OR’ed values to
specify the desired location.

.. doxygenenumvalue:: MFX_CHROMA_SITING_UNKNOWN
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_CHROMA_SITING_VERTICAL_TOP
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_CHROMA_SITING_VERTICAL_CENTER
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_CHROMA_SITING_VERTICAL_BOTTOM
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_CHROMA_SITING_HORIZONTAL_LEFT
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_CHROMA_SITING_HORIZONTAL_CENTER
   :project: DEF_BREATHE_PROJECT

.. _codec-format-fourcc:

-----------------
CodecFormatFourCC
-----------------

The CodecFormatFourCC enumerator itemizes codecs in the FourCC format.

.. doxygenenumvalue:: MFX_CODEC_AVC
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_CODEC_HEVC
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_CODEC_VVC
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_CODEC_MPEG2
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_CODEC_VC1
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_CODEC_VP9
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_CODEC_AV1
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_CODEC_JPEG
   :project: DEF_BREATHE_PROJECT

----------
CodecLevel
----------

The CodecLevel enumerator itemizes codec levels for all codecs.

.. doxygenenumvalue:: MFX_LEVEL_UNKNOWN
   :project: DEF_BREATHE_PROJECT

H.264 Level 1-1.3
-----------------

.. doxygenenumvalue:: MFX_LEVEL_AVC_1
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_AVC_1b
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_AVC_11
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_AVC_12
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_AVC_13
   :project: DEF_BREATHE_PROJECT

H.264 Level 2-2.2
-----------------

.. doxygenenumvalue:: MFX_LEVEL_AVC_2
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_AVC_21
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_AVC_22
   :project: DEF_BREATHE_PROJECT

H.264 Level 3-3.2
-----------------

.. doxygenenumvalue:: MFX_LEVEL_AVC_3
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_AVC_31
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_AVC_32
   :project: DEF_BREATHE_PROJECT

H.264 Level 4-4.2
-----------------

.. doxygenenumvalue:: MFX_LEVEL_AVC_4
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_AVC_41
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_AVC_42
   :project: DEF_BREATHE_PROJECT

H.264 Level 5-5.2
-----------------

.. doxygenenumvalue:: MFX_LEVEL_AVC_5
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_AVC_51
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_AVC_52
   :project: DEF_BREATHE_PROJECT

H.264 Level 6-6.2
-----------------

.. doxygenenumvalue:: MFX_LEVEL_AVC_6
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_AVC_61
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_AVC_62
   :project: DEF_BREATHE_PROJECT

MPEG2 Levels
------------

.. doxygenenumvalue:: MFX_LEVEL_MPEG2_LOW
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_MPEG2_MAIN
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_MPEG2_HIGH
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_MPEG2_HIGH1440
   :project: DEF_BREATHE_PROJECT

VC-1 Level Low (Simple and Main Profiles)
-----------------------------------------

.. doxygenenumvalue:: MFX_LEVEL_VC1_LOW
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_VC1_MEDIAN
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_VC1_HIGH
   :project: DEF_BREATHE_PROJECT

VC-1 Advanced Profile Levels
----------------------------

.. doxygenenumvalue:: MFX_LEVEL_VC1_0
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_VC1_1
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_VC1_2
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_VC1_3
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_VC1_4
   :project: DEF_BREATHE_PROJECT

HEVC Levels
-----------

.. doxygenenumvalue:: MFX_LEVEL_HEVC_1
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_HEVC_2
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_HEVC_21
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_HEVC_3
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_HEVC_31
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_HEVC_4
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_HEVC_41
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_HEVC_5
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_HEVC_51
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_HEVC_52
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_HEVC_6
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_HEVC_61
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_HEVC_62
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_HEVC_85
   :project: DEF_BREATHE_PROJECT

.. _codec_profile:

AV1 Levels
-----------

.. doxygenenumvalue:: MFX_LEVEL_AV1_2
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_AV1_21
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_AV1_22
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_AV1_23
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_AV1_3
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_AV1_31
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_AV1_32
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_AV1_33
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_AV1_4
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_AV1_41
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_AV1_42
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_AV1_43
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_AV1_5
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_AV1_51
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_AV1_52
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_AV1_53
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_AV1_6
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_AV1_61
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_AV1_62
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_AV1_63
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_AV1_7
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_AV1_71
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_AV1_72
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_AV1_73
   :project: DEF_BREATHE_PROJECT

VVC Levels
-----------

.. doxygenenumvalue:: MFX_LEVEL_VVC_1
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_VVC_2
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_VVC_21
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_VVC_3
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_VVC_31
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_VVC_4
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_VVC_41
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_VVC_5
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_VVC_51
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_VVC_52
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_VVC_6
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_VVC_61
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_VVC_62
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LEVEL_VVC_155
   :project: DEF_BREATHE_PROJECT


------------
CodecProfile
------------

The CodecProfile enumerator itemizes codec profiles for all codecs.

.. doxygenenumvalue:: MFX_PROFILE_UNKNOWN
   :project: DEF_BREATHE_PROJECT

H.264 Profiles
--------------

.. doxygenenumvalue:: MFX_PROFILE_AVC_BASELINE
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_PROFILE_AVC_MAIN
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_PROFILE_AVC_EXTENDED
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_PROFILE_AVC_HIGH
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_PROFILE_AVC_HIGH10
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_PROFILE_AVC_HIGH_422
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_PROFILE_AVC_CONSTRAINED_BASELINE
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_PROFILE_AVC_CONSTRAINED_HIGH
   :project: DEF_BREATHE_PROJECT

AV1 Profiles
------------

.. doxygenenumvalue:: MFX_PROFILE_AV1_MAIN
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_PROFILE_AV1_HIGH
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_PROFILE_AV1_PRO
   :project: DEF_BREATHE_PROJECT

VVC Profiles
-------------

.. doxygenenumvalue:: MFX_PROFILE_VVC_MAIN10
   :project: DEF_BREATHE_PROJECT

VVC Tiers
----------

.. doxygenenumvalue:: MFX_TIER_VVC_MAIN
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_TIER_VVC_HIGH
   :project: DEF_BREATHE_PROJECT

VC-1 Profiles
-------------

.. doxygenenumvalue:: MFX_PROFILE_VC1_SIMPLE
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_PROFILE_VC1_MAIN
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_PROFILE_VC1_ADVANCED
   :project: DEF_BREATHE_PROJECT

VP8 Profiles
------------

.. doxygenenumvalue:: MFX_PROFILE_VP8_0
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_PROFILE_VP8_1
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_PROFILE_VP8_2
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_PROFILE_VP8_3
   :project: DEF_BREATHE_PROJECT

VP9 Profiles
------------

.. doxygenenumvalue:: MFX_PROFILE_VP9_0
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_PROFILE_VP9_1
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_PROFILE_VP9_2
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_PROFILE_VP9_3
   :project: DEF_BREATHE_PROJECT

H.264 Constraints
-----------------

Combined with H.264 profile, these flags impose additional constraints. See the
H.264 specification for the list of constraints.

.. doxygenenumvalue:: MFX_PROFILE_AVC_CONSTRAINT_SET0
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_PROFILE_AVC_CONSTRAINT_SET1
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_PROFILE_AVC_CONSTRAINT_SET2
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_PROFILE_AVC_CONSTRAINT_SET3
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_PROFILE_AVC_CONSTRAINT_SET4
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_PROFILE_AVC_CONSTRAINT_SET5
   :project: DEF_BREATHE_PROJECT

JPEG Profiles
-------------

.. doxygenenumvalue:: MFX_PROFILE_JPEG_BASELINE
   :project: DEF_BREATHE_PROJECT

-----------------
CodingOptionValue
-----------------

The CodingOptionValue enumerator defines a three-state coding option setting.

.. doxygenenumvalue:: MFX_CODINGOPTION_UNKNOWN
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_CODINGOPTION_ON
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_CODINGOPTION_OFF
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_CODINGOPTION_ADAPTIVE
   :project: DEF_BREATHE_PROJECT

-----------
ColorFourCC
-----------

The ColorFourCC enumerator itemizes color formats.

.. doxygenenumvalue:: MFX_FOURCC_NV12
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_FOURCC_NV21
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_FOURCC_YV12
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_FOURCC_IYUV
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_FOURCC_I420
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_FOURCC_I422
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_FOURCC_NV16
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_FOURCC_YUY2
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_FOURCC_RGB565
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_FOURCC_RGBP
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_FOURCC_RGB4
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_FOURCC_BGRA
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_FOURCC_P8
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_FOURCC_P8_TEXTURE
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_FOURCC_P010
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_FOURCC_I010
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_FOURCC_I210
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_FOURCC_P016
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_FOURCC_P210
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_FOURCC_BGR4
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_FOURCC_A2RGB10
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_FOURCC_ARGB16
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_FOURCC_ABGR16
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_FOURCC_R16
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_FOURCC_AYUV
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_FOURCC_AYUV_RGB4
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_FOURCC_UYVY
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_FOURCC_Y210
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_FOURCC_Y410
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_FOURCC_Y216
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_FOURCC_Y416
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_FOURCC_BGRP
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_FOURCC_XYUV
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_FOURCC_ABGR16F
   :project: DEF_BREATHE_PROJECT

-----------
ContentInfo
-----------

The ContentInfo enumerator itemizes content types for the encoding session.

.. doxygenenumvalue:: MFX_CONTENT_UNKNOWN
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_CONTENT_FULL_SCREEN_VIDEO
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_CONTENT_NON_VIDEO_SCREEN
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_CONTENT_NOISY_VIDEO
   :project: DEF_BREATHE_PROJECT

----------
Corruption
----------

The Corruption enumerator itemizes the decoding corruption types. It is a
bit-OR’ed value of the following.

.. doxygenenumvalue:: MFX_CORRUPTION_NO
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_CORRUPTION_MINOR
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_CORRUPTION_MAJOR
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_CORRUPTION_ABSENT_TOP_FIELD
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_CORRUPTION_ABSENT_BOTTOM_FIELD
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_CORRUPTION_REFERENCE_FRAME
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_CORRUPTION_REFERENCE_LIST
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_CORRUPTION_HW_RESET
   :project: DEF_BREATHE_PROJECT

.. note:: Flag MFX_CORRUPTION_ABSENT_TOP_FIELD/MFX_CORRUPTION_ABSENT_BOTTOM_FIELD
         is set by the AVC decoder when it detects that one of fields is not
         present in the bitstream. Which field is absent depends on value of
         bottom_field_flag (ITU-T\* H.264 7.4.3).

-----------------
DeinterlacingMode
-----------------

The DeinterlacingMode enumerator itemizes VPP deinterlacing modes.

.. doxygenenumvalue:: MFX_DEINTERLACING_BOB
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_DEINTERLACING_ADVANCED
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_DEINTERLACING_AUTO_DOUBLE
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_DEINTERLACING_AUTO_SINGLE
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_DEINTERLACING_FULL_FR_OUT
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_DEINTERLACING_HALF_FR_OUT
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_DEINTERLACING_24FPS_OUT
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_DEINTERLACING_FIXED_TELECINE_PATTERN
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_DEINTERLACING_30FPS_OUT
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_DEINTERLACING_DETECT_INTERLACE
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_DEINTERLACING_ADVANCED_NOREF
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_DEINTERLACING_ADVANCED_SCD
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_DEINTERLACING_FIELD_WEAVING
   :project: DEF_BREATHE_PROJECT

----------
ErrorTypes
----------

The ErrorTypes enumerator uses bit-ORed values to itemize bitstream error types.

.. doxygenenumvalue:: MFX_ERROR_NO
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_ERROR_PPS
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_ERROR_SPS
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_ERROR_SLICEHEADER
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_ERROR_SLICEDATA
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_ERROR_FRAME_GAP
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_ERROR_JPEG_APP0_MARKER
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_ERROR_JPEG_APP1_MARKER
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_ERROR_JPEG_APP2_MARKER
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_ERROR_JPEG_APP3_MARKER
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_ERROR_JPEG_APP4_MARKER
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_ERROR_JPEG_APP5_MARKER
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_ERROR_JPEG_APP6_MARKER
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_ERROR_JPEG_APP7_MARKER
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_ERROR_JPEG_APP8_MARKER
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_ERROR_JPEG_APP9_MARKER
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_ERROR_JPEG_APP10_MARKER
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_ERROR_JPEG_APP11_MARKER
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_ERROR_JPEG_APP12_MARKER
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_ERROR_JPEG_APP13_MARKER
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_ERROR_JPEG_APP14_MARKER
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_ERROR_JPEG_DQT_MARKER
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_ERROR_JPEG_SOF0_MARKER
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_ERROR_JPEG_DHT_MARKER
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_ERROR_JPEG_DRI_MARKER
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_ERROR_JPEG_SOS_MARKER
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_ERROR_JPEG_UNKNOWN_MARKER
   :project: DEF_BREATHE_PROJECT

.. _extendedbufferid:

----------------
ExtendedBufferID
----------------

The ExtendedBufferID enumerator itemizes and defines identifiers (BufferId) for
extended buffers or video processing algorithm identifiers.

.. doxygenenumvalue:: MFX_EXTBUFF_THREADS_PARAM
  :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_CODING_OPTION
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_CODING_OPTION_SPSPPS
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_DONOTUSE
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_AUXDATA
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_DENOISE2
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_3DLUT
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_SCENE_ANALYSIS
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_PROCAMP
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_DETAIL
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_VIDEO_SIGNAL_INFO
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_VIDEO_SIGNAL_INFO_IN
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_VIDEO_SIGNAL_INFO_OUT
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_DOUSE
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_AVC_REFLIST_CTRL
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_FRAME_RATE_CONVERSION
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_PICTURE_TIMING_SEI
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_AVC_TEMPORAL_LAYERS
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_CODING_OPTION2
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_IMAGE_STABILIZATION
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_ENCODER_CAPABILITY
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_ENCODER_RESET_OPTION
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_ENCODED_FRAME_INFO
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_COMPOSITE
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_VIDEO_SIGNAL_INFO
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_ENCODER_ROI
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_DEINTERLACING
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_AVC_REFLISTS
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_DEC_VIDEO_PROCESSING
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_FIELD_PROCESSING
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_CODING_OPTION3
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_CHROMA_LOC_INFO
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_MBQP
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_MB_FORCE_INTRA
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_HEVC_TILES
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_MB_DISABLE_SKIP_MAP
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_HEVC_PARAM
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_DECODED_FRAME_INFO
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_TIME_CODE
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_HEVC_REGION
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_PRED_WEIGHT_TABLE
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_DIRTY_RECTANGLES
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_MOVING_RECTANGLES
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_CODING_OPTION_VPS
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_ROTATION
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_ENCODED_SLICES_INFO
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_SCALING
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_HEVC_REFLIST_CTRL
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_HEVC_REFLISTS
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_HEVC_TEMPORAL_LAYERS
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_MIRRORING
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_MV_OVER_PIC_BOUNDARIES
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_COLORFILL
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_DECODE_ERROR_REPORT
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_COLOR_CONVERSION
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_CONTENT_LIGHT_LEVEL_INFO
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_MASTERING_DISPLAY_COLOUR_VOLUME
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_MASTERING_DISPLAY_COLOUR_VOLUME_IN
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_MASTERING_DISPLAY_COLOUR_VOLUME_OUT
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_ENCODED_UNITS_INFO
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_MCTF
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_VP9_SEGMENTATION
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_VP9_TEMPORAL_LAYERS
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_VP9_PARAM
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_AVC_ROUNDING_OFFSET
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_PARTIAL_BITSTREAM_PARAM
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_BRC
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_VP8_CODING_OPTION
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_JPEG_QT
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_JPEG_HUFFMAN
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_ENCODER_IPCM_AREA
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_INSERT_HEADERS
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_MVC_SEQ_DESC
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_MVC_TARGET_VIEWS
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_CENC_PARAM
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_DEVICE_AFFINITY_MASK
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_CROPS
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_AV1_FILM_GRAIN_PARAM
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_AV1_SEGMENTATION
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_ALLOCATION_HINTS
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_UNIVERSAL_TEMPORAL_LAYERS
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_UNIVERSAL_REFLIST_CTRL
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_ENCODESTATS
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_TUNE_ENCODE_QUALITY
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_EXTBUFF_VPP_PERC_ENC_PREFILTER
   :project: DEF_BREATHE_PROJECT

----------------
ExtMemBufferType
----------------

.. doxygenenumvalue:: MFX_MEMTYPE_PERSISTENT_MEMORY
   :project: DEF_BREATHE_PROJECT

.. _extmemframetype:

---------------
ExtMemFrameType
---------------

The ExtMemFrameType enumerator specifies the memory type of frame. It is a
bit-ORed value of one of the following. For information on working with video
memory surfaces, see the :ref:`Working with Hardware Acceleration section<hw-acceleration>`.

.. doxygenenumvalue:: MFX_MEMTYPE_DXVA2_DECODER_TARGET
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_MEMTYPE_DXVA2_PROCESSOR_TARGET
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_MEMTYPE_VIDEO_MEMORY_DECODER_TARGET
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_MEMTYPE_VIDEO_MEMORY_PROCESSOR_TARGET
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_MEMTYPE_SYSTEM_MEMORY
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_MEMTYPE_RESERVED1
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_MEMTYPE_FROM_ENCODE
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_MEMTYPE_FROM_DECODE
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_MEMTYPE_FROM_VPPIN
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_MEMTYPE_FROM_VPPOUT
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_MEMTYPE_FROM_ENC
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_MEMTYPE_INTERNAL_FRAME
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_MEMTYPE_EXTERNAL_FRAME
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_MEMTYPE_EXPORT_FRAME
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_MEMTYPE_SHARED_RESOURCE
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_MEMTYPE_VIDEO_MEMORY_ENCODER_TARGET
   :project: DEF_BREATHE_PROJECT

----------------
Frame Data Flags
----------------

.. doxygenenumvalue:: MFX_TIMESTAMP_UNKNOWN
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_FRAMEORDER_UNKNOWN
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_FRAMEDATA_TIMESTAMP_UNKNOWN
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_FRAMEDATA_ORIGINAL_TIMESTAMP
   :project: DEF_BREATHE_PROJECT

---------
FrameType
---------

The FrameType enumerator itemizes frame types. Use bit-ORed values to specify
all that apply.

.. doxygenenumvalue:: MFX_FRAMETYPE_UNKNOWN
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_FRAMETYPE_I
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_FRAMETYPE_P
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_FRAMETYPE_B
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_FRAMETYPE_S
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_FRAMETYPE_REF
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_FRAMETYPE_IDR
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_FRAMETYPE_xI
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_FRAMETYPE_xP
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_FRAMETYPE_xB
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_FRAMETYPE_xS
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_FRAMETYPE_xREF
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_FRAMETYPE_xIDR
   :project: DEF_BREATHE_PROJECT

-------
FrcAlgm
-------

The FrcAlgm enumerator itemizes frame rate conversion algorithms. See description
of mfxExtVPPFrameRateConversion structure for more details.

.. doxygenenumvalue:: MFX_FRCALGM_PRESERVE_TIMESTAMP
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_FRCALGM_DISTRIBUTED_TIMESTAMP
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_FRCALGM_FRAME_INTERPOLATION
   :project: DEF_BREATHE_PROJECT

----------------------
GeneralConstraintFlags
----------------------

The GeneralConstraintFlags enumerator uses bit-ORed values to itemize HEVC bitstream
indications for specific profiles. Each value indicates for format range extensions profiles.
To specify HEVC Main 10 Still Picture profile applications have to set
mfxInfoMFX::CodecProfile == MFX_PROFILE_HEVC_MAIN10 and
mfxExtHEVCParam::GeneralConstraintFlags == MFX_HEVC_CONSTR_REXT_ONE_PICTURE_ONLY.

.. doxygenenumvalue:: MFX_HEVC_CONSTR_REXT_MAX_12BIT
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_HEVC_CONSTR_REXT_MAX_10BIT
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_HEVC_CONSTR_REXT_MAX_8BIT
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_HEVC_CONSTR_REXT_MAX_422CHROMA
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_HEVC_CONSTR_REXT_MAX_420CHROMA
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_HEVC_CONSTR_REXT_MAX_MONOCHROME
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_HEVC_CONSTR_REXT_INTRA
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_HEVC_CONSTR_REXT_ONE_PICTURE_ONLY
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_HEVC_CONSTR_REXT_LOWER_BIT_RATE
   :project: DEF_BREATHE_PROJECT

----------
GopOptFlag
----------

The GopOptFlag enumerator itemizes special properties in the GOP (Group of
Pictures) sequence.

.. doxygenenumvalue:: MFX_GOP_CLOSED
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_GOP_STRICT
   :project: DEF_BREATHE_PROJECT

.. _gpu_copy:

-------
GPUCopy
-------

.. doxygenenumvalue:: MFX_GPUCOPY_DEFAULT
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_GPUCOPY_ON
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_GPUCOPY_OFF
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_GPUCOPY_SAFE
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_GPUCOPY_FAST
   :project: DEF_BREATHE_PROJECT

-------------
HEVC Profiles
-------------

.. doxygenenumvalue:: MFX_PROFILE_HEVC_MAIN
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_PROFILE_HEVC_MAIN10
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_PROFILE_HEVC_MAINSP
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_PROFILE_HEVC_REXT
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_PROFILE_HEVC_SCC
   :project: DEF_BREATHE_PROJECT

----------
HEVC Tiers
----------

.. doxygenenumvalue:: MFX_TIER_HEVC_MAIN
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_TIER_HEVC_HIGH
   :project: DEF_BREATHE_PROJECT

------------------
HEVCRegionEncoding
------------------
The HEVCRegionEncoding enumerator itemizes HEVC region's encoding.

.. doxygenenumvalue:: MFX_HEVC_REGION_ENCODING_ON
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_HEVC_REGION_ENCODING_OFF
   :project: DEF_BREATHE_PROJECT

--------------
HEVCRegionType
--------------

The HEVCRegionType enumerator itemizes type of HEVC region.

.. doxygenenumvalue:: MFX_HEVC_REGION_SLICE
   :project: DEF_BREATHE_PROJECT

-------------
ImageStabMode
-------------

The ImageStabMode enumerator itemizes image stabilization modes. See description
of mfxExtVPPImageStab structure for more details.

.. doxygenenumvalue:: MFX_IMAGESTAB_MODE_UPSCALE
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_IMAGESTAB_MODE_BOXING
   :project: DEF_BREATHE_PROJECT

----------------
InsertHDRPayload
----------------

The InsertHDRPayload enumerator itemizes HDR payloads insertion rules.

.. doxygenenumvalue:: MFX_PAYLOAD_OFF
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_PAYLOAD_IDR
   :project: DEF_BREATHE_PROJECT

-----------------
InterpolationMode
-----------------

The InterpolationMode enumerator specifies type of interpolation method used by
VPP scaling filter.

.. doxygenenumvalue:: MFX_INTERPOLATION_DEFAULT
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_INTERPOLATION_NEAREST_NEIGHBOR
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_INTERPOLATION_BILINEAR
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_INTERPOLATION_ADVANCED
   :project: DEF_BREATHE_PROJECT

--------
DataType
--------

.. doxygenenum:: mfxDataType
   :project: DEF_BREATHE_PROJECT

-------------------
3DLutChannelMapping
-------------------

.. doxygenenum:: mfx3DLutChannelMapping
   :project: DEF_BREATHE_PROJECT

-----------------
3DLutMemoryLayout
-----------------

.. doxygenenum:: mfx3DLutMemoryLayout
   :project: DEF_BREATHE_PROJECT

------------------------
3DLutInterpolationMethod
------------------------

.. doxygenenum:: mfx3DLutInterpolationMethod
   :project: DEF_BREATHE_PROJECT

-------------------------------------
IntraPredBlockSize/InterPredBlockSize
-------------------------------------

IntraPredBlockSize/InterPredBlockSize specifies minimum block size of inter-prediction.

.. doxygenenumvalue:: MFX_BLOCKSIZE_UNKNOWN
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_BLOCKSIZE_MIN_16X16
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_BLOCKSIZE_MIN_8X8
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_BLOCKSIZE_MIN_4X4
   :project: DEF_BREATHE_PROJECT

-----------------
IntraRefreshTypes
-----------------

The IntraRefreshTypes enumerator itemizes types of intra refresh.

.. doxygenenumvalue:: MFX_REFRESH_NO
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_REFRESH_VERTICAL
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_REFRESH_HORIZONTAL
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_REFRESH_SLICE
   :project: DEF_BREATHE_PROJECT

---------
IOPattern
---------

The IOPattern enumerator itemizes memory access patterns for API functions.
Use bit-ORed values to specify input and output access patterns.

.. doxygenenumvalue:: MFX_IOPATTERN_IN_VIDEO_MEMORY
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_IOPATTERN_IN_SYSTEM_MEMORY
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_IOPATTERN_OUT_VIDEO_MEMORY
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_IOPATTERN_OUT_SYSTEM_MEMORY
   :project: DEF_BREATHE_PROJECT

---------------
JPEGColorFormat
---------------

The JPEGColorFormat enumerator itemizes the JPEG color format options.

.. doxygenenumvalue:: MFX_JPEG_COLORFORMAT_UNKNOWN
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_JPEG_COLORFORMAT_YCbCr
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_JPEG_COLORFORMAT_RGB
   :project: DEF_BREATHE_PROJECT

------------
JPEGScanType
------------

The JPEGScanType enumerator itemizes the JPEG scan types.

.. doxygenenumvalue:: MFX_SCANTYPE_UNKNOWN
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_SCANTYPE_INTERLEAVED
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_SCANTYPE_NONINTERLEAVED
   :project: DEF_BREATHE_PROJECT

-----------
LongTermIdx
-----------

The LongTermIdx specifies long term index of picture control

.. doxygenenumvalue:: MFX_LONGTERM_IDX_NO_IDX
   :project: DEF_BREATHE_PROJECT

---------------------
LookAheadDownSampling
---------------------

The LookAheadDownSampling enumerator is used to control down sampling in look
ahead bitrate control mode in AVC encoder.

.. doxygenenumvalue:: MFX_LOOKAHEAD_DS_UNKNOWN
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LOOKAHEAD_DS_OFF
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LOOKAHEAD_DS_2x
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_LOOKAHEAD_DS_4x
   :project: DEF_BREATHE_PROJECT

--------
MBQPMode
--------

The MBQPMode enumerator itemizes QP update modes.

.. doxygenenumvalue:: MFX_MBQP_MODE_QP_VALUE
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_MBQP_MODE_QP_DELTA
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_MBQP_MODE_QP_ADAPTIVE
   :project: DEF_BREATHE_PROJECT

----------------
mfxComponentType
----------------

.. doxygenenum:: mfxComponentType
   :project: DEF_BREATHE_PROJECT

-------------
mfxHandleType
-------------

.. doxygenenum:: mfxHandleType
   :project: DEF_BREATHE_PROJECT

-------
mfxIMPL
-------

.. doxygentypedef:: mfxIMPL
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_IMPL_AUTO
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_IMPL_SOFTWARE
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_IMPL_HARDWARE
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_IMPL_AUTO_ANY
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_IMPL_HARDWARE_ANY
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_IMPL_HARDWARE2
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_IMPL_HARDWARE3
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_IMPL_HARDWARE4
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_IMPL_RUNTIME
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_IMPL_VIA_ANY
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_IMPL_VIA_D3D9
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_IMPL_VIA_D3D11
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_IMPL_VIA_VAAPI
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_IMPL_VIA_HDDLUNITE
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_IMPL_UNSUPPORTED
   :project: DEF_BREATHE_PROJECT

.. doxygendefine:: MFX_IMPL_BASETYPE
   :project: DEF_BREATHE_PROJECT

-------------------------
mfxImplCapsDeliveryFormat
-------------------------

.. doxygenenum:: mfxImplCapsDeliveryFormat
   :project: DEF_BREATHE_PROJECT

-------------------
mfxMediaAdapterType
-------------------

.. doxygenenum:: mfxMediaAdapterType
   :project: DEF_BREATHE_PROJECT

--------------
mfxMemoryFlags
--------------

.. doxygenenum:: mfxMemoryFlags
   :project: DEF_BREATHE_PROJECT

--------------
MfxNalUnitType
--------------

Specifies NAL unit types supported by the HEVC encoder.

.. doxygenenumvalue:: MFX_HEVC_NALU_TYPE_UNKNOWN
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_HEVC_NALU_TYPE_TRAIL_N
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_HEVC_NALU_TYPE_TRAIL_R
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_HEVC_NALU_TYPE_RADL_N
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_HEVC_NALU_TYPE_RADL_R
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_HEVC_NALU_TYPE_RASL_N
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_HEVC_NALU_TYPE_RASL_R
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_HEVC_NALU_TYPE_IDR_W_RADL
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_HEVC_NALU_TYPE_IDR_N_LP
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_HEVC_NALU_TYPE_CRA_NUT
   :project: DEF_BREATHE_PROJECT

-----------
mfxPriority
-----------

.. doxygenenum:: mfxPriority
   :project: DEF_BREATHE_PROJECT

---------------
mfxResourceType
---------------

.. doxygenenum:: mfxResourceType
   :project: DEF_BREATHE_PROJECT

-----------
mfxSkipMode
-----------

.. doxygenenum:: mfxSkipMode
   :project: DEF_BREATHE_PROJECT

---------
mfxStatus
---------

.. doxygenenum:: mfxStatus
   :project: DEF_BREATHE_PROJECT

-------------
MirroringType
-------------

The MirroringType enumerator itemizes mirroring types.

.. doxygenenumvalue:: MFX_MIRRORING_DISABLED
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_MIRRORING_HORIZONTAL
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_MIRRORING_VERTICAL
   :project: DEF_BREATHE_PROJECT

-----------
DenoiseMode
-----------

The mfxDenoiseMode enumerator itemizes denoise modes.

.. doxygenenum:: mfxDenoiseMode
   :project: DEF_BREATHE_PROJECT

---------------
MPEG-2 Profiles
---------------

.. doxygenenumvalue:: MFX_PROFILE_MPEG2_SIMPLE
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_PROFILE_MPEG2_MAIN
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_PROFILE_MPEG2_HIGH
   :project: DEF_BREATHE_PROJECT

------------------------------------------
Multi-view Video Coding Extension Profiles
------------------------------------------

.. doxygenenumvalue:: MFX_PROFILE_AVC_MULTIVIEW_HIGH
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_PROFILE_AVC_STEREO_HIGH
   :project: DEF_BREATHE_PROJECT

-----------
MVPrecision
-----------

The MVPrecision enumerator specifies the motion estimation precision

.. doxygenenumvalue:: MFX_MVPRECISION_UNKNOWN
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_MVPRECISION_INTEGER
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_MVPRECISION_HALFPEL
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_MVPRECISION_QUARTERPEL
   :project: DEF_BREATHE_PROJECT

------------
NominalRange
------------

The NominalRange enumerator itemizes pixel's value nominal range.

.. doxygenenumvalue:: MFX_NOMINALRANGE_UNKNOWN
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_NOMINALRANGE_0_255
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_NOMINALRANGE_16_235
   :project: DEF_BREATHE_PROJECT

----------------------
PartialBitstreamOutput
----------------------

The PartialBitstreamOutput enumerator indicates flags of partial bitstream output
type.

.. doxygenenumvalue:: MFX_PARTIAL_BITSTREAM_NONE
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_PARTIAL_BITSTREAM_SLICE
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_PARTIAL_BITSTREAM_BLOCK
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_PARTIAL_BITSTREAM_ANY
   :project: DEF_BREATHE_PROJECT

----------------
PayloadCtrlFlags
----------------

The PayloadCtrlFlags enumerator itemizes additional payload properties.

.. doxygenenumvalue:: MFX_PAYLOAD_CTRL_SUFFIX
   :project: DEF_BREATHE_PROJECT

.. _PicStruct:

---------
PicStruct
---------

The PicStruct enumerator itemizes picture structure. Use bit-OR’ed values to
specify the desired picture type.

.. doxygenenumvalue:: MFX_PICSTRUCT_UNKNOWN
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_PICSTRUCT_PROGRESSIVE
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_PICSTRUCT_FIELD_TFF
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_PICSTRUCT_FIELD_BFF
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_PICSTRUCT_FIELD_REPEATED
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_PICSTRUCT_FRAME_DOUBLING
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_PICSTRUCT_FRAME_TRIPLING
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_PICSTRUCT_FIELD_SINGLE
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_PICSTRUCT_FIELD_TOP
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_PICSTRUCT_FIELD_BOTTOM
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_PICSTRUCT_FIELD_PAIRED_PREV
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_PICSTRUCT_FIELD_PAIRED_NEXT
   :project: DEF_BREATHE_PROJECT

-------
PicType
-------

The PicType enumerator itemizes picture type.

.. doxygenenumvalue:: MFX_PICTYPE_UNKNOWN
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_PICTYPE_FRAME
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_PICTYPE_TOPFIELD
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_PICTYPE_BOTTOMFIELD
   :project: DEF_BREATHE_PROJECT

--------
PRefType
--------

The PRefType enumerator itemizes models of reference list construction and DPB
management when GopRefDist=1.

.. doxygenenumvalue:: MFX_P_REF_DEFAULT
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_P_REF_SIMPLE
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_P_REF_PYRAMID
   :project: DEF_BREATHE_PROJECT

-----------
TuneQuality
-----------

The TuneQuality enumerator specifies tuning option for encode. Multiple tuning options
can be combined using bit mask.

.. doxygenenumvalue:: MFX_ENCODE_TUNE_OFF
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_ENCODE_TUNE_PSNR
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_ENCODE_TUNE_SSIM
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_ENCODE_TUNE_MS_SSIM
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_ENCODE_TUNE_VMAF
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_ENCODE_TUNE_PERCEPTUAL
   :project: DEF_BREATHE_PROJECT

---------
Protected
---------

The Protected enumerator describes the protection schemes.

.. doxygenenumvalue:: MFX_PROTECTION_CENC_WV_CLASSIC
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_PROTECTION_CENC_WV_GOOGLE_DASH
   :project: DEF_BREATHE_PROJECT

.. _RateControlMethod:

-----------------
RateControlMethod
-----------------

The RateControlMethod enumerator itemizes bitrate control methods.

.. doxygenenumvalue:: MFX_RATECONTROL_CBR
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_RATECONTROL_VBR
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_RATECONTROL_CQP
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_RATECONTROL_AVBR
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_RATECONTROL_LA
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_RATECONTROL_ICQ
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_RATECONTROL_VCM
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_RATECONTROL_LA_ICQ
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_RATECONTROL_LA_HRD
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_RATECONTROL_QVBR
   :project: DEF_BREATHE_PROJECT

-------
ROImode
-------

The ROImode enumerator itemizes QP adjustment mode for ROIs.

.. doxygenenumvalue:: MFX_ROI_MODE_PRIORITY
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_ROI_MODE_QP_DELTA
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_ROI_MODE_QP_VALUE
   :project: DEF_BREATHE_PROJECT

--------
Rotation
--------

The Rotation enumerator itemizes the JPEG rotation options.

.. doxygenenumvalue:: MFX_ROTATION_0
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_ROTATION_90
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_ROTATION_180
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_ROTATION_270
   :project: DEF_BREATHE_PROJECT

--------------------
SampleAdaptiveOffset
--------------------

The SampleAdaptiveOffset enumerator uses bit-ORed values to itemize corresponding
HEVC encoding feature.

.. doxygenenumvalue:: MFX_SAO_UNKNOWN
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_SAO_DISABLE
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_SAO_ENABLE_LUMA
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_SAO_ENABLE_CHROMA
   :project: DEF_BREATHE_PROJECT

-----------
ScalingMode
-----------

The ScalingMode enumerator itemizes variants of scaling filter implementation.

.. doxygenenumvalue:: MFX_SCALING_MODE_DEFAULT
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_SCALING_MODE_LOWPOWER
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_SCALING_MODE_QUALITY
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_SCALING_MODE_VENDOR
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_SCALING_MODE_INTEL_GEN_COMPUTE
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_SCALING_MODE_INTEL_GEN_VDBOX
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_SCALING_MODE_INTEL_GEN_VEBOX
   :project: DEF_BREATHE_PROJECT

------------
ScenarioInfo
------------

The ScenarioInfo enumerator itemizes scenarios for the encoding session.

.. doxygenenumvalue:: MFX_SCENARIO_UNKNOWN
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_SCENARIO_DISPLAY_REMOTING
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_SCENARIO_VIDEO_CONFERENCE
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_SCENARIO_ARCHIVE
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_SCENARIO_LIVE_STREAMING
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_SCENARIO_CAMERA_CAPTURE
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_SCENARIO_VIDEO_SURVEILLANCE
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_SCENARIO_GAME_STREAMING
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_SCENARIO_REMOTE_GAMING
   :project: DEF_BREATHE_PROJECT

--------------
SegmentFeature
--------------

The SegmentFeature enumerator indicates features enabled for the segment.
These values are used with the mfxVP9SegmentParam::FeatureEnabled parameter.

.. doxygenenumvalue:: MFX_VP9_SEGMENT_FEATURE_QINDEX
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_VP9_SEGMENT_FEATURE_LOOP_FILTER
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_VP9_SEGMENT_FEATURE_REFERENCE
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_VP9_SEGMENT_FEATURE_SKIP
   :project: DEF_BREATHE_PROJECT

------------------
SegmentIdBlockSize
------------------

The SegmentIdBlockSize enumerator indicates the block size represented by each
segment_id in segmentation map. These values are used with the
mfxExtVP9Segmentation::SegmentIdBlockSize parameter.

.. doxygenenumvalue:: MFX_VP9_SEGMENT_ID_BLOCK_SIZE_UNKNOWN
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_VP9_SEGMENT_ID_BLOCK_SIZE_8x8
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_VP9_SEGMENT_ID_BLOCK_SIZE_16x16
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_VP9_SEGMENT_ID_BLOCK_SIZE_32x32
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_VP9_SEGMENT_ID_BLOCK_SIZE_64x64
   :project: DEF_BREATHE_PROJECT

---------
SkipFrame
---------

The SkipFrame enumerator is used to define usage of mfxEncodeCtrl::SkipFrame
parameter.

.. doxygenenumvalue:: MFX_SKIPFRAME_NO_SKIP
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_SKIPFRAME_INSERT_DUMMY
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_SKIPFRAME_INSERT_NOTHING
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_SKIPFRAME_BRC_ONLY
   :project: DEF_BREATHE_PROJECT

-----------
TargetUsage
-----------

The TargetUsage enumerator itemizes a range of numbers from MFX_TARGETUSAGE_1,
best quality, to MFX_TARGETUSAGE_7, best speed. It indicates trade-offs between
quality and speed. The application can use any number in the range. The actual
number of supported target usages depends on implementation. If the specified
target usage is not supported, the encoder will use the closest supported value.

.. doxygenenumvalue:: MFX_TARGETUSAGE_1
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_TARGETUSAGE_2
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_TARGETUSAGE_3
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_TARGETUSAGE_4
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_TARGETUSAGE_5
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_TARGETUSAGE_6
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_TARGETUSAGE_7
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_TARGETUSAGE_UNKNOWN
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_TARGETUSAGE_BEST_QUALITY
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_TARGETUSAGE_BALANCED
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_TARGETUSAGE_BEST_SPEED
   :project: DEF_BREATHE_PROJECT

---------------
TelecinePattern
---------------

The TelecinePattern enumerator itemizes telecine patterns.

.. doxygenenumvalue:: MFX_TELECINE_PATTERN_32
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_TELECINE_PATTERN_2332
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_TELECINE_PATTERN_FRAME_REPEAT
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_TELECINE_PATTERN_41
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_TELECINE_POSITION_PROVIDED
   :project: DEF_BREATHE_PROJECT

-------------
TimeStampCalc
-------------

The TimeStampCalc enumerator itemizes time-stamp calculation methods.

.. doxygenenumvalue:: MFX_TIMESTAMPCALC_UNKNOWN
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_TIMESTAMPCALC_TELECINE
   :project: DEF_BREATHE_PROJECT

--------------
TransferMatrix
--------------

The TransferMatrix enumerator itemizes color transfer matrices.

.. doxygenenumvalue:: MFX_TRANSFERMATRIX_UNKNOWN
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_TRANSFERMATRIX_BT709
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_TRANSFERMATRIX_BT601
   :project: DEF_BREATHE_PROJECT

--------------
TrellisControl
--------------

The TrellisControl enumerator is used to control trellis quantization in AVC
encoder. The application can turn it on or off for any combination of I, P, and
B frames by combining different enumerator values. For example,
MFX_TRELLIS_I | MFX_TRELLIS_B turns it on for I and B frames.

.. doxygenenumvalue:: MFX_TRELLIS_UNKNOWN
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_TRELLIS_OFF
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_TRELLIS_I
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_TRELLIS_P
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_TRELLIS_B
   :project: DEF_BREATHE_PROJECT

-----------------
VP9ReferenceFrame
-----------------

The VP9ReferenceFrame enumerator itemizes reference frame type by
the mfxVP9SegmentParam::ReferenceFrame parameter.

.. doxygenenumvalue:: MFX_VP9_REF_INTRA
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_VP9_REF_LAST
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_VP9_REF_GOLDEN
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_VP9_REF_ALTREF
   :project: DEF_BREATHE_PROJECT

----------------------
VPPFieldProcessingMode
----------------------

The VPPFieldProcessingMode enumerator is used to control VPP field processing
algorithm.

.. doxygenenumvalue:: MFX_VPP_COPY_FRAME
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_VPP_COPY_FIELD
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_VPP_SWAP_FIELDS
   :project: DEF_BREATHE_PROJECT

------------
WeightedPred
------------

The WeightedPred enumerator itemizes weighted prediction modes.

.. doxygenenumvalue:: MFX_WEIGHTED_PRED_UNKNOWN
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_WEIGHTED_PRED_DEFAULT
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_WEIGHTED_PRED_EXPLICIT
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_WEIGHTED_PRED_IMPLICIT
   :project: DEF_BREATHE_PROJECT

--------------
FilmGrainFlags
--------------

The FilmGrainFlags enumerator itemizes flags in AV1 film grain parameters.

.. doxygenenumvalue:: MFX_FILM_GRAIN_NO
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_FILM_GRAIN_APPLY
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_FILM_GRAIN_UPDATE
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_FILM_GRAIN_CHROMA_SCALING_FROM_LUMA
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_FILM_GRAIN_OVERLAP
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_FILM_GRAIN_CLIP_TO_RESTRICTED_RANGE
   :project: DEF_BREATHE_PROJECT

------------
mfxHyperMode
------------

.. doxygenenum:: mfxHyperMode
   :project: DEF_BREATHE_PROJECT

-----------------------
mfxPoolAllocationPolicy
-----------------------

.. doxygenenum:: mfxPoolAllocationPolicy
   :project: DEF_BREATHE_PROJECT

--------------
mfxVPPPoolType
--------------

.. doxygenenum:: mfxVPPPoolType
   :project: DEF_BREATHE_PROJECT

------------------------
mfxAV1SegmentIdBlockSize
------------------------

The mfxAV1SegmentIdBlockSize enumerator indicates the block size represented by each segment_id in segmentation map.

.. doxygenenum:: mfxAV1SegmentIdBlockSize
   :project: DEF_BREATHE_PROJECT

-----------------
AV1SegmentFeature
-----------------

The AV1SegmentFeature enumerator indicates features enabled for the segment.

.. doxygenenumvalue:: MFX_AV1_SEGMENT_FEATURE_ALT_QINDEX
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_AV1_SEGMENT_FEATURE_ALT_LF_Y_VERT
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_AV1_SEGMENT_FEATURE_ALT_LF_Y_HORZ
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_AV1_SEGMENT_FEATURE_ALT_LF_U
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_AV1_SEGMENT_FEATURE_ALT_LF_V
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_AV1_SEGMENT_FEATURE_REFERENCE
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_AV1_SEGMENT_FEATURE_SKIP
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_AV1_SEGMENT_FEATURE_GLOBALMV
   :project: DEF_BREATHE_PROJECT

--------------------------
mfxEncodeBlkStatsMemLayout
--------------------------

.. doxygenenum:: mfxEncodeBlkStatsMemLayout
   :project: DEF_BREATHE_PROJECT

------------------
mfxEncodeStatsMode
------------------

.. doxygenenum:: mfxEncodeStatsMode
   :project: DEF_BREATHE_PROJECT

----------------
EncodeStatsLevel
----------------

Flags to specify what statistics will be reported by the implementation.

.. doxygenenumvalue:: MFX_ENCODESTATS_LEVEL_BLK
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_ENCODESTATS_LEVEL_FRAME
   :project: DEF_BREATHE_PROJECT


-------------------
mfxSurfaceComponent
-------------------

.. doxygenenum:: mfxSurfaceComponent
   :project: DEF_BREATHE_PROJECT


--------------
mfxSurfaceType
--------------

.. doxygenenum:: mfxSurfaceType
   :project: DEF_BREATHE_PROJECT

----------------
mfxStructureType
----------------

.. doxygenenum:: mfxStructureType
   :project: DEF_BREATHE_PROJECT

------------------------
mfxAISuperResolutionMode
------------------------

.. doxygenenum:: mfxAISuperResolutionMode
   :project: DEF_BREATHE_PROJECT

-----------------------------
mfxAISuperResolutionAlgorithm
-----------------------------

.. doxygenenum:: mfxAISuperResolutionAlgorithm
   :project: DEF_BREATHE_PROJECT

---------------------------
mfxAIFrameInterpolationMode
---------------------------

.. doxygenenum:: mfxAIFrameInterpolationMode
   :project: DEF_BREATHE_PROJECT

------------------------
mfxQualityInfoMode
------------------------

.. doxygenenum:: mfxQualityInfoMode
   :project: DEF_BREATHE_PROJECT

----------------
AlphaChannelMode
----------------

The AlphaChannelMode enumerator specifies alpha is pre-multiplied or straight.

.. doxygenenumvalue:: MFX_ALPHA_MODE_PREMULTIPLIED
   :project: DEF_BREATHE_PROJECT

.. doxygenenumvalue:: MFX_ALPHA_MODE_STRAIGHT
   :project: DEF_BREATHE_PROJECT
