.. SPDX-FileCopyrightText: 2019-2020 Intel Corporation
..
.. SPDX-License-Identifier: CC-BY-4.0

==============
Video Encoding
==============

The :term:`ENCODE` class of functions take raw frames as input and compresses
them into a bitstream.

Input frames usually come encoded in a repeated pattern called the Group of
Picture (GOP) sequence. For example, a GOP sequence can start with an
I-frame followed by a few B-frames, a P-frame, and so on. ENCODE uses an MPEG-2
style GOP sequence structure that can specify the length of the sequence and the
distance between two keyframes: I- or P-frames. A GOP sequence ensures that the
segments of a bitstream do not completely depend upon each other. It also enables
decoding applications to reposition the bitstream.

ENCODE processes input frames in two ways;

* **Display order:** ENCODE receives input frames in the display order. GOP structure
  parameters specify the GOP sequence during ENCODE initialization. Scene changes resulting from the video processing stage of a pipeline can alter the GOP
  sequence.

* **Encoded order:** ENCODE receives input frames in their encoding order. The
  application must specify the exact input frame type for encoding. ENCODE
  references GOP parameters to determine when to insert information, such as an end-of-sequence, into the bitstream.

An ENCODE output consists of one frame of a bitstream with the time stamp
passed from the input frame. The time stamp is used for multiplexing subsequent
video with other associated data such as audio. oneVPL provides only pure video
stream encoding. The application must provide its own multiplexing.

ENCODE supports the following bitrate control algorithms: constant bitrate,
variable bitrate (VBR), and constant quantization parameter (QP). In the
constant bitrate mode, ENCODE performs stuffing when the size of the
least-compressed frame is smaller than what is required to meet the hypothetical
reference decoder (HRD) buffer requirements (or VBR requirements). (Stuffing is a process that
appends zeros to the end of encoded frames.)