.. SPDX-FileCopyrightText: 2019-2020 Intel Corporation
..
.. SPDX-License-Identifier: CC-BY-4.0

.. _stream_vid_conf_features:

=========================================
Streaming and Video Conferencing Features
=========================================

The following sections address some aspects of additional requirements that
streaming or video conferencing applications may use in the encoding or
transcoding process. See the :ref:`config-change` section for additional
information.

----------------------
Dynamic Bitrate Change
----------------------

The oneVPL encoder supports dynamic bitrate change according to bitrate control mode
and HRD conformance requirements. If HRD conformance is required, for example if
the application sets the ``NalHrdConformance`` option in the
:cpp:struct:`mfxExtCodingOption` structure to ON, the only allowed
bitrate control mode is VBR. In this mode, the application can change the
``TargetKbps`` and ``MaxKbps`` values of the :cpp:struct:`mfxInfoMFX` structure
by calling the  :cpp:func:`MFXVideoENCODE_Reset` function. This sort of change in
bitrate usually results in the generation of a new keyframe and sequence header.
There are exceptions, such as if HRD information is absent in the stream.
In this scenario, the change of ``TargetKbps`` does not require a change in the
sequence header and as a result the encoder does not insert a keyframe.

If HRD conformance is not required, for example if the application turns off
the ``NalHrdConformance`` option in the :cpp:struct:`mfxExtCodingOption`
structure, all bitrate control modes are available. In CBR and AVBR modes the
application can change ``TargetKbps``. In VBR mode the application can change
``TargetKbps`` and ``MaxKbps`` values. This sort of change in bitrate will not
result in the generation of a new keyframe or sequence header.

The oneVPL encoder may change some initialization parameters provided by the
application during initialization. That in turn may lead to incompatibility
between the parameters provided by the application during reset and the working set
of parameters used by the encoder. For this reason, it is strongly recommended
to retrieve the actual working parameters using the
:cpp:func:`MFXVideoENCODE_GetVideoParam` function before making any changes to
bitrate settings.

In all modes, oneVPL encoders will respond to the bitrate changes as quickly as
the underlying algorithm allows, without breaking other encoding restrictions
such as HRD compliance if it is enabled. How quickly the actual bitrate can catch
up with the specified bitrate is implementation dependent.

Alternatively, the application may use the :term:`CQP` encoding mode to perform
customized bitrate adjustment on a per-frame base. The application may use any
of the encoded or display order modes to use per-frame CQP.

-------------------------
Dynamic Resolution Change
-------------------------

The oneVPL encoder supports dynamic resolution change in all bitrate control modes.
The application may change resolution by calling the :cpp:func:`MFXVideoENCODE_Reset`
function. The application may decrease or increase resolution up to the size
specified during encoder initialization.

Resolution change always results in the insertion of a key IDR frame and a new
sequence parameter set in the header. The only exception is the oneVPL VP9 encoder
(see section for `Dynamic reference frame scaling`_). The oneVPL encoder does not
guarantee HRD conformance across the resolution change point.

The oneVPL encoder may change some initialization parameters provided by the
application during initialization. That in turn may lead to incompatibility of
parameters provide by the application during reset and working set of parameters
used by the encoder. Due to this potential incompatibility, it is strongly
recommended to retrieve the actual working parameters set by
:cpp:func:`MFXVideoENCODE_GetVideoParam` function before making any resolution
change.

-------------------------------
Dynamic Reference Frame Scaling
-------------------------------

The VP9 standard allows changing the resolution without the insertion of a
keyframe. This is possible because the VP9 encoder has the built-in capability
to upscale and downscale reference frames to match the resolution of the frame
being encoded. By default the oneVPL VP9 encoder inserts a keyframe when the
application does `Dynamic Resolution Change`_. In this case, the first frame
with a new resolution is encoded using inter prediction from the scaled reference
frame of the previous resolution. Dynamic scaling has the following limitations,
described in the VP9 specification:

* The resolution of any active reference frame cannot exceed 2x the resolution
  of the current frame.
* The resolution of any active reference frame cannot be smaller than 1/16 of
  the current frame resolution.

In the case of dynamic scaling, the oneVPL VP9 encoder always uses a single active
reference frame for the first frame after a resolution change. The VP9
encoder has the following limitations for dynamic resolution change:

* The new resolution should not exceed 16x the resolution of the current frame.
* The new resolution should be less than 1/2 of current frame resolution.

The application may force insertion of a keyframe at the point of resolution
change by invoking encoder reset with :cpp:member:`mfxExtEncoderResetOption::StartNewSequence`
set to :cpp:enumerator:`MFX_CODINGOPTION_ON`. If a keyframe is inserted, the
dynamic resolution limitations are not enforced.

Note that resolution change with dynamic reference scaling is compatible with
multiref (:cpp:member:`mfxInfoMFX::NumRefFrame` > 1). For multiref
configuration, the oneVPL VP9 encoder uses multiple references within stream pieces
of the same resolution and uses a single reference at the place of resolution
change.

---------------------------
Forced Keyframe Generation
---------------------------

oneVPL supports forced keyframe generation during encoding. The application
can set the FrameType parameter of the :cpp:struct:`mfxEncodeCtrl` structure to
control how the current frame is encoded, as follows:

- If the oneVPL encoder works in the display order, the application can enforce any
  current frame to be a keyframe. The application cannot change the frame type
  of already buffered frames inside the encoder.
- If the oneVPL encoder works in the encoded order, the application must specify
  exact frame type for every frame. In this way, the application can enforce the
  current frame to have any frame type that the particular coding standard allows.

.. _sec_reference_list_selection:

------------------------
Reference List Selection
------------------------

During streaming or video conferencing, if the application can obtain feedback
about how well the client receives certain frames, the application may need to
adjust the encoding process to use or not use certain frames as reference. This
section describes how to fine-tune the encoding process based on client feedback.

The application can specify the reference window size by specifying the
:cpp:member:`mfxInfoMFX::NumRefFrame` parameter during encoding initialization.
Certain platforms may have limits on the the size of the reference window. Use the
:cpp:func:`MFXVideoENCODE_GetVideoParam` function to retrieve the current working
set of parameters.

During encoding, the application can specify the actual reference list lengths
by attaching the :cpp:struct:`mfxExtAVCRefListCtrl` structure to the
:cpp:func:`MFXVideoENCODE_EncodeFrameAsync` function. ``NumRefIdxL0Active``
specifies the length of the reference list L0 and ``NumRefIdxL1Active`` specifies
the length of the reference list L1. These two numbers must be less than or equal
to the :cpp:member:`mfxInfoMFX::NumRefFrame` parameter during encoding initialization.

The application can instruct the oneVPL encoder to use or not use certain reference
frames. To do this, there is a prerequisite that the application uniquely
identify each input frame by setting the :cpp:member:`mfxFrameData::FrameOrder`
parameter. The application then specifies the preferred reference frame list
``PreferredRefList`` and/or the rejected frame list ``RejectedRefList``, and
attaches the :cpp:struct:`mfxExtAVCRefListCtrl` structure to the
:cpp:func:`MFXVideoENCODE_EncodeFrameAsync` function. The two lists fine-tune how
the encoder chooses the reference frames for the current frame.
The encoder does not keep ``PreferredRefList`` and the application must send it
for each frame if necessary. There are limitations as follows:

- The frames in the lists are ignored if they are out of the reference window.
- If by going through the lists, the oneVPL encoder cannot find a reference frame
  for the current frame, the encoder will encode the current frame without
  using any reference frames.
- If the GOP pattern contains B-frames, the oneVPL encoder may not be able to follow
  the :cpp:struct:`mfxExtAVCRefListCtrl` instructions.

---------------------------------
Low Latency Encoding and Decoding
---------------------------------

The application can set :cpp:member:`mfxVideoParam::AsyncDepth` = 1 to disable
any decoder buffering of output frames, which is aimed to improve the transcoding
throughput. With :cpp:member:`mfxVideoParam::AsyncDepth` = 1, the application
must synchronize after the decoding or transcoding operation of each frame.

The application can adjust :cpp:member:`mfxExtCodingOption::MaxDecFrameBuffering`
during encoding initialization to improve decoding latency. It is recommended to
set this value equal to the number of reference frames.

------------------------------------------------
Reference Picture Marking Repetition SEI Message
------------------------------------------------

The application can request writing the reference picture marking repetition SEI
message during encoding initialization by setting ``RefPicMarkRep`` of the
:cpp:struct:`mfxExtCodingOption` structure. The reference picture marking
repetition SEI message repeats certain reference frame information in the output
bitstream for robust streaming.

The oneVPL decoder will respond to the reference picture marking repetition SEI
message if the message exists in the bitstream and compare it to the reference
list information specified in the sequence/picture headers. The decoder will
report any mismatch of the SEI message with the reference list information in
the :cpp:member:`mfxFrameData::Corrupted` field.

.. _sec_long_term_reference_frame:

-------------------------
Long Term Reference Frame
-------------------------

The application may use long term reference frames to improve coding efficiency
or robustness for video conferencing applications. The application controls the
long term frame marking process by attaching the :cpp:struct:`mfxExtAVCRefListCtrl`
extended buffer during encoding. The oneVPL encoder itself never marks a frame as
long term.

There are two control lists in the :cpp:struct:`mfxExtAVCRefListCtrl` extended
buffer. The ``LongTermRefList`` list contains the frame orders (the ``FrameOrder``
value in the :cpp:struct:`mfxFrameData` structure) of the frames that should be
marked as long term frames. The ``RejectedRefList`` list contains the frame order
of the frames that should be unmarked as long term frames. The application can
only mark or unmark the frames that are buffered inside the encoder. Because of
this, it is recommended that the application marks a frame when it is submitted
for encoding. The application can either explicitly unmark long term reference
frames or wait for the IDR frame. When the IDR frame is reached, all long term
reference frames will be unmarked.

The oneVPL encoder puts all long term reference frames at the end of a reference
frame list. If the number of active reference frames (the ``NumRefIdxL0Active``
and ``NumRefIdxL1Active`` values in the :cpp:struct:`mfxExtAVCRefListCtrl`
extended buffer) is less than than the total reference frame number (the
``NumRefFrame`` value in the :cpp:struct:`mfxInfoMFX` structure during the
encoding initialization), the encoder may ignore some or all long term
reference frames. The application may avoid this by providing a list of preferred
reference frames in the ``PreferredRefList`` list in the
:cpp:struct:`mfxExtAVCRefListCtrl` extended buffer. In this case, the encoder
reorders the reference list based on the specified list.

--------------------
Temporal Scalability
--------------------

The application may specify the temporal hierarchy of frames by using the
:cpp:struct:`mfxExtAvcTemporalLayers` extended buffer during the encoder
initialization in the display order encoding mode. oneVPL inserts the prefix
NAL unit before each slice with a unique temporal and priority ID. The temporal
ID starts from zero and the priority ID starts from the ``BaseLayerPID`` value.
oneVPL increases the temporal ID and priority ID value by one for each
consecutive layer.

If the application needs to specify a unique sequence or picture parameter set
ID, the application must use the :cpp:struct:`mfxExtCodingOptionSPSPPS`
extended buffer, with all pointers and sizes set to zero and valid
``SPSId`` and ``PPSId`` fields. The same SPS and PPS ID will be used for all
temporal layers.

Each temporal layer is a set of frames with the same temporal ID. Each layer is
defined by the ``Scale`` value. The scale for layer N is equal to the ratio
between the frame rate of subsequent temporal layers with a temporal ID less
than or equal to N and the frame rate of the base temporal layer. The application
may skip some temporal layers by specifying the ``Scale`` value as zero. The
application should use an integer ratio of the frame rates for two consecutive
temporal layers.

For example, a video sequence with 30 frames/second is typically separated by
three temporal layers that can be decoded as 7.5 fps (base layer), 15 fps (base
and first temporal layer) and 30 fps (all three layers). In this scenario,
``Scale`` should have the values {1,2,4,0,0,0,0,0}.