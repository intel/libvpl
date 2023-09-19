.. SPDX-FileCopyrightText: 2019-2020 Intel Corporation
..
.. SPDX-License-Identifier: CC-BY-4.0

========
Glossary
========

The oneVPL API and documentation uses a standard set of acronyms and terms.
This section describes these conventions.

.. contents::
   :local:
   :depth: 1

------------------
Acronyms and Terms
------------------

.. glossary::

	AVC
		Advanced video codec (same as H.264 and MPEG-4, part 10).

	BRC
		Bit rate control.

	CQP
		Constant quantization parameter.

	DRM
		Digital rights management.

	DXVA2
		Microsoft DirectX\* Video Acceleration standard 2.0.

	GOP
		Group of pictures. In video coding, a group of frames in a specific order.
		In the H.264 standard, a group of I-frames, B-frames and P-frames.

	GPB
		Generalized P/B picture. B-picture, containing only forward references
		in both L0 and L1.

	H.264
		Video coding standard. See ISO\*/IEC\* 14496-10 and ITU-T\* H.264,
		MPEG-4 Part 10, Advanced Video Coding, May 2005.

	HDR
		High dynamic range.

	HRD
		Hypothetical reference decoder, a term used in the H.264 specification.

	IDR
		Instantaneous decoding fresh picture, a term used in the H.264 specification.

	LA
		Look ahead. Special encoding mode where encoder performs pre-analysis of
		several frames before actual encoding starts.

	MCTF
		Motion compensated temporal filter. Special type of noise reduction
		filter which utilizes motion to improve efficiency of video denoising.

	NAL
		Network abstraction layer.

	PPS
		Picture parameter set.

	QP
		Quantization parameter.

	SEI
		Supplemental enhancement information.

	SPS
		Sequence parameter set.

	VA API
		Video acceleration API.

	VBR
		Variable bit rate.

	VBV
		Video buffering verifier.

	Video memory
		Memory used by a hardware acceleration device, also known as GPU, to
		hold frame and other types of video data.

	VUI
		Video usability information.

-------------
Video Formats
-------------

.. glossary::

	MPEG
		Moving Picture Experts Group video file.

	MPEG-2
		Moving Picture Experts Group video file. See ISO/IEC 13818-2 and ITU-T
		H.262, MPEG-2 Part 2, Information Technology- Generic Coding of Moving
		Pictures and Associate Audio Information: Video, 2000.

	NV12
		YUV 4:2:0 video format, 12 bits per pixel.

	NV16
		YUV 4:2:2 video format, 16 bits per pixel.

	P010
		YUV 4:2:0 video format, extends NV12, 10 bits per pixel.

	P210
		YUV 4:2:2 video format, 10 bits per pixel.

	UYVY
		YUV 4:2:2 video format, 16 bits per pixel.

	VC-1
		Video coding format. See SMPTE\* 421M, SMPTE Standard for Television:
		VC-1 Compressed Video Bitstream Format and Decoding Process, August 2005.

-------------
Color Formats
-------------

.. glossary::

	I010
		Color format for raw video frames, extends IYUV/I420 for 10 bit.

	IYUV
		A color format for raw video frames, also known as I420.

	RGB32
		Thirty-two-bit RGB color format.

	RGB4
		Thirty-two-bit RGB color format. Also known as RGB32.

	YUY2
		A color format for raw video frames.

	YV12
		A color format for raw video frames, similar to IYUV with U and V
		reversed.
