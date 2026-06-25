.. SPDX-FileCopyrightText: 2019-2020 Intel Corporation
..
.. SPDX-License-Identifier: CC-BY-4.0

=============================================
Video Decoding with multiple video processing
=============================================

The :term:`DECODE_VPP` class of functions take a compressed bitstream as input,
converts it to raw frames and applies video processing filters to raw frames.
Users can set several output channels where each channel represents a list
of video processing filters applied for decoded frames.

The :term:`DECODE_VPP` supports only internal allocation.
