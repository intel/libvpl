==============
Video Decoding
==============

The **DECODE** class of functions take a compressed bitstream as input and
converts it to raw frames as output.

**DECODE** processes only pure or elementary video streams. The library cannot
process bitstreams that reside in a container format, such as MP4 or MPEG. The
application must first demultiplex the bitstreams. Demultiplexing extracts pure
video streams out of the container format. The application can provide the input
bitstream as one complete frame of data, a partial frame (less than one complete
frame), or multiple frames. If only a partial frame is provided, **DECODE**
internally constructs one frame of data before decoding it.

The time stamp of a bitstream buffer must be accurate to the first byte of the
frame data. For H.264, the first byte of the frame data comes from the NAL unit
in the video coding layer. For MPEG-2 or VC-1, the first byte of the frame data
comes from the picture header.

**DECODE** passes the time stamp to the output surface for audio and video
multiplexing or synchronization.

Decoding the first frame is a special case, since **DECODE** does not provide
enough configuration parameters to correctly process the bitstream. **DECODE**
searches for the sequence header (a sequence parameter set in H.264, or a
sequence header in MPEG-2 and VC-1) that contains the video configuration
parameters used to encode subsequent video frames. The decoder skips any
bitstream prior to that sequence header. In the case of multiple sequence headers
in the bitstream, **DECODE** adopts the new configuration parameters, ensuring
proper decoding of subsequent frames.

**DECODE** supports repositioning of the bitstream at any time during decoding.
Because there is no way to obtain the correct sequence header associated with
the specified bitstream position after a position change, the application must
supply **DECODE** with a sequence header before the decoder can process the next
frame at the new position. If the sequence header required to correctly decode
the bitstream at the new position is not provided by the application, **DECODE**
treats the new location as a new “first frame” and follows the procedure for
decoding first frames.

------------------
Decoding Procedure
------------------

The following pseudo code shows the decoding procedure:

.. code-block:: c++

   MFXVideoDECODE_DecodeHeader(session, bitstream, &init_param);
   MFXVideoDECODE_QueryIOSurf(session, &init_param, &request);
   allocate_pool_of_frame_surfaces(request.NumFrameSuggested);
   MFXVideoDECODE_Init(session, &init_param);
   sts=MFX_ERR_MORE_DATA;
   for (;;) {
      if (sts==MFX_ERR_MORE_DATA && !end_of_stream())
         append_more_bitstream(bitstream);
      find_unlocked_surface_from_the_pool(&work);
      bits=(end_of_stream())?NULL:bitstream;
      sts=MFXVideoDECODE_DecodeFrameAsync(session,bits,work,&disp,&syncp);
      if (sts==MFX_ERR_MORE_SURFACE) continue;
      if (end_of_bitstream() && sts==MFX_ERR_MORE_DATA) break;
      if (sts==MFX_ERR_REALLOC_SURFACE) {
         MFXVideoDECODE_GetVideoParam(session, &param);
         realloc_surface(work, param.mfx.FrameInfo);
         continue;
      }
      // skipped other error handling
      if (sts==MFX_ERR_NONE) {
         MFXVideoCORE_SyncOperation(session, syncp, INFINITE);
         do_something_with_decoded_frame(disp);
      }
   }
   MFXVideoDECODE_Close();
   free_pool_of_frame_surfaces();


Note the following key points about the example:

- The application can use the :cpp:func:`MFXVideoDECODE_DecodeHeader` function
  to retrieve decoding initialization parameters from the bitstream. This step
  is optional if the data is retrievable from other sources such as an
  audio/video splitter.
- The application uses the :cpp:func:`MFXVideoDECODE_QueryIOSurf` function to
  obtain the number of working frame surfaces required to reorder output frames. This call is optional and required when application uses external allocation.
- The application calls the :cpp:func:`MFXVideoDECODE_DecodeFrameAsync` function
  for a decoding operation, with the bitstream buffer (bits) and an unlocked
  working frame surface (work) as input parameters.

.. attention:: Starting with API version 2.0, the application can provide NULL
               as the working frame surface that leads to internal memory allocation.

If decoding output is not available, the function returns a status code requesting additional bitstream input or working frame surfaces as follows:

  - :cpp:enumerator:`MFX_ERR_MORE_DATA`: The function needs additional bitstream
    input. The existing buffer contains less than a frame's worth of bitstream data.
  - :cpp:enumerator:`MFX_ERR_MORE_SURFACE`: The function needs one more frame
    surface to produce any output.
  - :cpp:enumerator:`MFX_ERR_REALLOC_SURFACE`: Dynamic resolution change case -
    the function needs bigger working frame surface (work).

- Upon successful decoding, the :cpp:func:`MFXVideoDECODE_DecodeFrameAsync`
  function returns :cpp:enumerator:`MFX_ERR_NONE`. However, the decoded frame
  data (identified by the disp pointer) is not yet available because the
  :cpp:func:`MFXVideoDECODE_DecodeFrameAsync` function is asynchronous. The
  application must use the :cpp:func:`MFXVideoCORE_SyncOperation` or
  :cpp:struct:`mfxFrameSurfaceInterface` interface to synchronize the decoding operation before retrieving the decoded frame data.
- At the end of the bitstream, the application continuously calls the
  :cpp:func:`MFXVideoDECODE_DecodeFrameAsync` function with a NULL bitstream
  pointer to drain any remaining frames cached within the SDK decoder, until the
  function returns :cpp:enumerator:`MFX_ERR_MORE_DATA`.


The following example shows the simplified decoding procedure:

.. code-block:: c++

   sts=MFX_ERR_MORE_DATA;
   for (;;) {
      if (sts==MFX_ERR_MORE_DATA && !end_of_stream())
         append_more_bitstream(bitstream);
      bits=(end_of_stream())?NULL:bitstream;
      sts=MFXVideoDECODE_DecodeFrameAsync(session,bits,NULL,&disp,&syncp);
      if (sts==MFX_ERR_MORE_SURFACE) continue;
      if (end_of_bitstream() && sts==MFX_ERR_MORE_DATA) break;
      // skipped other error handling
      if (sts==MFX_ERR_NONE) {
         MFXVideoCORE_SyncOperation(session, syncp, INFINITE);
         do_something_with_decoded_frame(disp);
         release_surface(disp);
      }
   }

.. _simplified-decoding-procedure:

API version 2.0 introduces a new decoding approach. For simple use cases, when
the user wants to decode an elementary stream and doesn't want to set additional parameters, the simplified procedure of Decoder's initialization has been
proposed. For such situations it is possible to skip explicit stages of stream's header decoding and the decoder's initialization and instead to perform it
implicitly during decoding of the first frame. This change also requires
additional field in the mfxBitstream object to indicate codec type. In that mode
the decoder allocates mfxFrameSurface1 internally, so users should set input surface to zero.
