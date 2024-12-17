.. SPDX-FileCopyrightText: 2019-2020 Intel Corporation
..
.. SPDX-License-Identifier: CC-BY-4.0

.. _decoding-proc:

===================
Decoding Procedures
===================

There are several approaches to decode video frames. The first one is based on 
the internal allocation mechanism presented here:

.. literalinclude:: ../snippets/prg_decoding.c
   :language: c++
   :start-after: /*beg1*/
   :end-before: /*end1*/
   :lineno-start: 1

Note the following key points about the example:

- The application calls the :cpp:func:`MFXVideoDECODE_DecodeFrameAsync` function
  for a decoding operation with the bitstream buffer (bits), frame surface is
  allocated internally by the library.

  .. attention:: As shown in the example above starting with API version 2.0, 
                 the application can provide NULL
                 as the working frame surface that leads to internal memory
                 allocation. 

- If decoding output is not available, the function returns a status code
  requesting additional bitstream input as follows:

  - :cpp:enumerator:`mfxStatus::MFX_ERR_MORE_DATA`: The function needs
    additional bitstream input. The existing buffer contains less than a frame's
    worth of bitstream data.

- Upon successful decoding, the :cpp:func:`MFXVideoDECODE_DecodeFrameAsync`
  function returns :cpp:enumerator:`mfxStatus::MFX_ERR_NONE`. However, the
  decoded frame data (identified by the surface_out pointer) is not yet
  available because the :cpp:func:`MFXVideoDECODE_DecodeFrameAsync` function is
  asynchronous. The application must use the
  :cpp:func:`MFXVideoCORE_SyncOperation` or
  :cpp:member:`mfxFrameSurfaceInterface::Synchronize`  to synchronize the
  decoding operation before retrieving the decoded frame data.

- At the end of the bitstream, the application continuously calls the
  :cpp:func:`MFXVideoDECODE_DecodeFrameAsync` function with a NULL bitstream
  pointer to drain any remaining frames cached within the decoder until the
  function returns :cpp:enumerator:`mfxStatus::MFX_ERR_MORE_DATA`.

- When application completes the work with frame surface, it must call release
  to avoid memory leaks.

The next example demonstrates how applications can use internally pre-allocated
chunk of video surfaces:

.. literalinclude:: ../snippets/prg_decoding.c
   :language: c++
   :start-after: /*beg2*/
   :end-before: /*end2*/
   :lineno-start: 1

Here the application should use the :cpp:func:`MFXVideoDECODE_QueryIOSurf`
function to obtain the number of working frame surfaces required to reorder
output frames. It is also required that
:cpp:func:`MFXMemory_GetSurfaceForDecode` call is done after decoder
initialization. In the :cpp:func:`MFXVideoDECODE_DecodeFrameAsync` the |vpl_short_name|
library increments reference counter of incoming surface frame so it is required
that the application releases frame surface after the call.  

Another approach to decode frames is to allocate video frames on-fly with help
of :cpp:func:`MFXMemory_GetSurfaceForDecode` function, feed the library and
release working surface after :cpp:func:`MFXVideoDECODE_DecodeFrameAsync` call.

  .. attention:: Please pay attention on two release calls for surfaces:
                 after :cpp:func:`MFXVideoDECODE_DecodeFrameAsync` to decrease
                 reference counter of working surface returned by 
                 :cpp:func:`MFXMemory_GetSurfaceForDecode`.
                 After :cpp:func:`MFXVideoCORE_SyncOperation` to decrease
                 reference counter of output surface returned by 
                 :cpp:func:`MFXVideoDECODE_DecodeFrameAsync`.

.. literalinclude:: ../snippets/prg_decoding.c
   :language: c++
   :start-after: /*beg6*/
   :end-before: /*end6*/
   :lineno-start: 1


The following pseudo code shows the decoding procedure according to the legacy
mode with external video frames allocation:

.. literalinclude:: ../snippets/prg_decoding.c
   :language: c++
   :start-after: /*beg3*/
   :end-before: /*end3*/
   :lineno-start: 1

Note the following key points about the example:


- The application can use the :cpp:func:`MFXVideoDECODE_DecodeHeader` function
  to retrieve decoding initialization parameters from the bitstream. This step
  is optional if the data is retrievable from other sources such as an
  audio/video splitter.
  
- The :cpp:func:`MFXVideoDECODE_DecodeFrameAsync` function can return following
  status codes in addition to the described above:
  
  - :cpp:enumerator:`mfxStatus::MFX_ERR_MORE_SURFACE`: The function needs one
    more frame surface to produce any output.
  - :cpp:enumerator:`mfxStatus::MFX_ERR_REALLOC_SURFACE`: Dynamic resolution
    change case - the function needs a bigger working frame surface (work).

The following pseudo code shows the simplified decoding procedure:

.. literalinclude:: ../snippets/prg_decoding.c
   :language: c++
   :start-after: /*beg4*/
   :end-before: /*end4*/
   :lineno-start: 1

.. _simplified-decoding-procedure:

|vpl_short_name| API version 2.0 introduces a new decoding approach. For simple use cases,
when the user wants to decode a stream and does not want to set additional
parameters, a simplified procedure for the decoder's initialization has been
proposed. In this scenario it is possible to skip explicit stages of a
stream's header decoding and the decoder's initialization and instead to perform
these steps implicitly during decoding of the first frame. This change also 
requires setting the additional field :cpp:member:`mfxBitstream::CodecId` to
indicate codec type. In this mode the decoder allocates
:cpp:struct:`mfxFrameSurface1` internally, so users should set the input surface
to zero.

-----------------------
Bitstream Repositioning
-----------------------

The application can use the following procedure for bitstream reposition during
decoding:

#. Use the :cpp:func:`MFXVideoDECODE_Reset` function to reset the |vpl_short_name|
   decoder.
#. Optional: If the application maintains a sequence header that correctly
   decodes the bitstream at the new position, the application may insert the
   sequence header to the bitstream buffer.
#. Append the bitstream from the new location to the bitstream buffer.
#. Resume the decoding procedure. If the sequence header is not inserted in the
   previous steps, the |vpl_short_name| decoder searches for a new sequence header before
   starting decoding.

-----------------------
Broken Streams Handling
-----------------------

Robustness and the capability to handle a broken input stream is an important
part of the decoder.

First, the start code prefix (ITU-T\* H.264 3.148 and ITU-T H.265 3.142) is
used to separate NAL units. Then all syntax elements in the bitstream are parsed
and verified. If any of the elements violate the specification, the input
bitstream is considered invalid and the decoder tries to re-sync (find the next
start code). Subsequent decoder behavior is dependent on which syntax element is
broken:

* SPS header is broken: return
  :cpp:enumerator:`mfxStatus::MFX_ERR_INCOMPATIBLE_VIDEO_PARAM` (HEVC decoder
  only, AVC decoder uses last valid).
* PPS header is broken: re-sync, use last valid PPS for decoding.
* Slice header is broken: skip this slice, re-sync.
* Slice data is broken: corruption flags are set on output surface.

Many streams have IDR frames with ``frame_num != 0`` while the specification
says that “If the current picture is an IDR picture, frame_num shall be equal to
0” (ITU-T H.265 7.4.3).

VUI is also validated, but errors do not invalidate the whole SPS. The decoder
either does not use the corrupted VUI (AVC) or resets incorrect values to
default (HEVC).

.. note:: Some requirements are relaxed because there are many streams which
          violate the strict standard but can be decoded without errors.

Corruption at the reference frame is spread over all inter-coded pictures that
use the reference frame for prediction. To cope with this problem you must
either periodically insert I-frames (intra-coded) or use the intra-refresh
technique. The intra-refresh technique allows recovery from corruptions within a
predefined time interval. The main point of intra-refresh is to insert a cyclic
intra-coded pattern (usually a row) of macroblocks into the inter-coded
pictures, restricting motion vectors accordingly. Intra-refresh is often used in
combination with recovery point SEI, where the ``recovery_frame_cnt`` is derived
from the intra-refresh interval. The recovery point SEI message is well
described at ITU-T H.264 D.2.7 and ITU-T H.265 D.2.8. If decoding starts from AU
associated with this SEI message, then the message can be used by the decoder to
determine from which picture all subsequent pictures have no errors. In
comparison to IDR, the recovery point message does not mark reference pictures
as "unused for reference".

Besides validation of syntax elements and their constraints, the decoder also
uses various hints to handle broken streams:

* If there are no valid slices for the current frame, then the whole frame is
  skipped.
* The slices which violate slice segment header semantics (ITU-T H.265 7.4.7.1)
  are skipped. Only the ``slice_temporal_mvp_enabled_flag`` is checked for now.
* Since LTR (Long Term Reference) stays at DPB until it is explicitly
  cleared by IDR or MMCO, the incorrect LTR could cause long standing visual
  artifacts. AVC decoder uses the following approaches to handle this:

  * When there is a DPB overflow in the case of an incorrect MMCO command that
    marks the reference picture as LT, the operation is rolled back.
  * An IDR frame with ``frame_num != 0`` can’t be LTR.

* If the decoder detects frame gapping, it inserts "fake"’" (marked as
  non-existing) frames, updates FrameNumWrap (ITU-T H.264 8.2.4.1) for reference
  frames, and applies the Sliding Window (ITU-T H.264 8.2.5.3) marking process.
  Fake frames are marked as reference, but since they are marked as
  non-existing, they are not used for inter-prediction.

--------------------
VP8 Specific Details
--------------------

Unlike other |vpl_short_name| supported decoders, VP8 can accept only a complete frame as
input. The application should provide the complete frame accompanied by the
:cpp:enumerator:`MFX_BITSTREAM_COMPLETE_FRAME` flag. This is the single specific
difference.

----
JPEG
----

The application can use the same decoding procedures for JPEG/motion JPEG
decoding, as shown in the following pseudo code:

.. code-block:: c++

   // optional; retrieve initialization parameters
   MFXVideoDECODE_DecodeHeader(...);
   // decoder initialization
   MFXVideoDECODE_Init(...);
   // single frame/picture decoding
   MFXVideoDECODE_DecodeFrameAsync(...);
   MFXVideoCORE_SyncOperation(...);
   // optional; retrieve meta-data
   MFXVideoDECODE_GetUserData(...);
   // close
   MFXVideoDECODE_Close(...);


The :cpp:func:`MFXVideoDECODE_Query` function will return
:cpp:enumerator:`mfxStatus::MFX_ERR_UNSUPPORTED` if the input bitstream contains
unsupported features.

For still picture JPEG decoding, the input can be any JPEG bitstreams that
conform to the ITU-T Recommendation T.81 with an EXIF or JFIF header. For
motion JPEG decoding, the input can be any JPEG bitstreams that conform to the
ITU-T Recommendation T.81.

Unlike other |vpl_short_name| decoders, JPEG decoding supports three different output
color formats: :term:`NV12`, :term:`YUY2`, and :term:`RGB32`. This support
sometimes requires internal color conversion and more complicated
initialization. The color format of the input bitstream is described by the
:cpp:member:`mfxInfoMFX::JPEGChromaFormat` and
:cpp:member:`mfxInfoMFX::JPEGColorFormat` fields. The
:cpp:func:`MFXVideoDECODE_DecodeHeader` function usually fills them in. If the
JPEG bitstream does not contains color format information, the application
should provide it. Output color format is described by general |vpl_short_name|
parameters: the :cpp:member:`mfxFrameInfo::FourCC` and
:cpp:member:`mfxFrameInfo::ChromaFormat` fields.

Motion JPEG supports interlaced content by compressing each field (a half-height
frame) individually. This behavior is incompatible with the rest of the |vpl_short_name|
transcoding pipeline, where |vpl_short_name| requires fields to be in odd and even lines
of the same frame surface. The decoding procedure is modified as follows:

- The application calls the :cpp:func:`MFXVideoDECODE_DecodeHeader` function
  with the first field JPEG bitstream to retrieve initialization parameters.
- The application initializes the |vpl_short_name| JPEG decoder with the following
  settings:

  - The ``PicStruct`` field of the :cpp:struct:`mfxVideoParam` structure set to
    the correct interlaced type, :cpp:enumerator:`MFX_PICSTRUCT_FIELD_TFF` or
    :cpp:enumerator:`MFX_PICSTRUCT_FIELD_BFF`, from the motion JPEG header.
  - Double the ``Height`` field in the :cpp:struct:`mfxVideoParam` structure as
    the value returned by the :cpp:func:`MFXVideoDECODE_DecodeHeader` function
    describes only the first field. The actual frame surface should contain both
    fields.

- During decoding, the application sends both fields for decoding in the
  same :cpp:struct:`mfxBitstream`. The application should also set
  :cpp:member:`mfxBitstream::DataFlag` to
  :cpp:enumerator:`MFX_BITSTREAM_COMPLETE_FRAME`. |vpl_short_name| decodes both fields
  and combines them into odd and even lines according to |vpl_short_name| convention.

By default, the :cpp:func:`MFXVideoDECODE_DecodeHeader` function returns the
``Rotation`` parameter so that after rotation, the pixel at the
first row and first column is at the top left. The application can overwrite the
default rotation before calling :cpp:func:`MFXVideoDECODE_Init`.

The application may specify Huffman and quantization tables during decoder
initialization by attaching :cpp:struct:`mfxExtJPEGQuantTables` and
:cpp:struct:`mfxExtJPEGHuffmanTables` buffers to the :cpp:struct:`mfxVideoParam`
structure. In this case, the decoder ignores tables from bitstream and uses
the tables specified by the application. The application can also retrieve these
tables by attaching the same buffers to :cpp:struct:`mfxVideoParam` and calling
:cpp:func:`MFXVideoDECODE_GetVideoParam` or
:cpp:func:`MFXVideoDECODE_DecodeHeader` functions.

-------------------------
Multi-view Video Decoding
-------------------------

The |vpl_short_name| MVC decoder operates on complete MVC streams that contain all
view and temporal configurations. The application can configure the |vpl_short_name|
decoder to generate a subset at the decoding output. To do this, the application
must understand the stream structure and use the stream information to configure
the decoder for target views.

The decoder initialization procedure is as follows:

#. The application calls the :cpp:func:`MFXVideoDECODE_DecodeHeader` function to
   obtain the stream structural information. This is done in two steps:

   #. The application calls the :cpp:func:`MFXVideoDECODE_DecodeHeader` function
      with the :cpp:struct:`mfxExtMVCSeqDesc` structure attached to the
      :cpp:struct:`mfxVideoParam` structure. At this point, do not allocate
      memory for the arrays in the :cpp:struct:`mfxExtMVCSeqDesc` structure. Set
      the ``View``, ``ViewId``, and ``OP`` pointers to NULL and set
      ``NumViewAlloc``, ``NumViewIdAlloc``, and ``NumOPAlloc`` to zero. The
      function parses the bitstream and returns
      :cpp:enumerator:`mfxStatus::MFX_ERR_NOT_ENOUGH_BUFFER` with the correct
      values for ``NumView``, ``NumViewId``, and ``NumOP``. This step can be
      skipped if the application is able to obtain the ``NumView``,
      ``NumViewId``, and ``NumOP`` values from other sources.
   #. The application allocates memory for the ``View``, ``ViewId``, and ``OP``
      arrays and calls the :cpp:func:`MFXVideoDECODE_DecodeHeader` function
      again. The function returns the MVC structural information in the
      allocated arrays.

#. The application fills the :cpp:struct:`mfxExtMVCTargetViews` structure to
   choose the target views, based on information described in the
   :cpp:struct:`mfxExtMVCSeqDesc` structure.
#. The application initializes the |vpl_short_name| decoder using the
   :cpp:func:`MFXVideoDECODE_Init` function. The application must attach both
   the :cpp:struct:`mfxExtMVCSeqDesc` structure and the
   :cpp:struct:`mfxExtMVCTargetViews` structure to the
   :cpp:struct:`mfxVideoParam` structure.

In the above steps, do not modify the values of the
:cpp:struct:`mfxExtMVCSeqDesc` structure after the
:cpp:func:`MFXVideoDECODE_DecodeHeader` function, as the |vpl_short_name| decoder uses the
values in the structure for internal memory allocation. Once the application
configures the |vpl_short_name| decoder, the rest of the decoding procedure remains
unchanged. As shown in the pseudo code below, the application calls the
:cpp:func:`MFXVideoDECODE_DecodeFrameAsync` function multiple times to obtain
all target views of the current frame picture, one target view at a time. The
target view is identified by the ``FrameID`` field of the
:cpp:struct:`mfxFrameInfo` structure.

.. literalinclude:: ../snippets/prg_decoding.c
   :language: c++
   :start-after: /*beg5*/
   :end-before: /*end5*/
   :lineno-start: 1

---------------------------------------------------
Combined Decode with Multi-channel Video Processing
---------------------------------------------------

The |vpl_short_name| exposes interface for making decode and video processing operations
in one call. Users can specify a number of output processing channels and
multiple video filters per each channel. This interface supports only internal
memory allocation model and returns array of processed frames through
:cpp:struct:`mfxSurfaceArray` reference object as shown by the example:

.. literalinclude:: ../snippets/prg_decoding_vpp.c
   :language: c++
   :start-after: /*beg1*/
   :end-before: /*end1*/
   :lineno-start: 1

It's possible that different video processing channels may have different
latency:

.. literalinclude:: ../snippets/prg_decoding_vpp.c
   :language: c++
   :start-after: /*beg2*/
   :end-before: /*end2*/
   :lineno-start: 1

Application can match decoded frame w/ specific VPP channels using
:cpp:member:`mfxFrameData::TimeStamp`, :cpp:member:mfxFrameData::FrameOrder` and
:cpp:member:`mfxFrameInfo::ChannelId`.

Application can skip some or all channels including decoding output with help of
`skip_channels` and `num_skip_channels` parameters as follows: application fills
`skip_channels` array with `ChannelId`s to disable output of correspondent
channels. In that case  :cpp:member:`surf_array_out` would contain only surfaces
for the remaining channels. If the decoder's channel and/or impacted VPP
channels don't have output frame(s) for the current call (for instance, input
bitstream doesn't contain complete frame or deinterlacing/FRC filter have delay)
`skip_channels` parameter is ignored for this channel. 

If application disables all channels the SDK returns NULL as
:cpp:struct:`mfxSurfaceArray`.

If application doesn't need to disable any channels it sets `num_skip_channels`
to zero, `skip_channels` is ignored when `num_skip_channels` is zero.

If application doesn't need to make scaling or cropping operations it has to set the following fields  
:cpp:member:`mfxFrameInfo::Width`, :cpp:member:`mfxFrameInfo::Height`, 
:cpp:member:`mfxFrameInfo::CropX`, :cpp:member:`mfxFrameInfo::CropY`
:cpp:member:`mfxFrameInfo::CropW`, :cpp:member:`mfxFrameInfo::CropH` of the VPP channel to zero. 
In that case output surfaces have the original decoded resolution and cropping. The operation supports
bitstreams with resolution change without need of :cpp:func:`MFXVideoDECODE_VPP_Reset` call.

.. note:: Even if more than one input compressed frame is consumed, the
  :cpp:func:`MFXVideoDECODE_VPP_DecodeFrameAsync` produces only one decoded
  frame and correspondent frames from VPP channels.

---------------------------------------------------
AsyncDepth Specific Details
---------------------------------------------------

`AsyncDepth` means how many asynchronous operations an application performs before the application explicitly
synchronizes the result, application will set this parameter and call :cpp:func:`MFXVideoDECODE_DecodeFrameAsync`,
the number of submitted frames is specified by `AsyncDepth`, then call :cpp:func:`MFXVideoCORE_SyncOperation` to synchronize
the result of all frames. 

The following pseudo code shows the simplified asynchronous and synchronous procedure:

.. literalinclude:: ../snippets/prg_decoding.c
   :language: c++
   :start-after: /*beg7*/
   :end-before: /*end7*/
   :lineno-start: 1

According to memory utilization and performance, two modes are recommend for decoder pipeline:

- For smallest memory footprint: the application should set AsyncDepth to 1 and then call :cpp:func:`MFXVideoCORE_SyncOperation`
  after each frame. This will have poor performance, since the GPU will not be kept fully occupied.
- For best performance: the application should set AsyncDepth to 4 and plan to :cpp:func:`MFXVideoCORE_SyncOperation` after every
  4th frame, which generally will keep the GPU close to fully occupied.