.. SPDX-FileCopyrightText: 2019-2020 Intel Corporation
..
.. SPDX-License-Identifier: CC-BY-4.0

===================
Encoding Procedures
===================

There are two methods for shared memory allocation and handling in |vpl_short_name|:
external and internal.

---------------
External Memory
---------------

The following pseudo code shows the encoding procedure with external memory
(legacy mode):

.. literalinclude:: ../snippets/prg_encoding.c
   :language: c++
   :start-after: /*beg1*/
   :end-before: /*end1*/
   :lineno-start: 1

Note the following key points about the example:

- The application uses the :cpp:func:`MFXVideoENCODE_QueryIOSurf` function to
  obtain the number of working frame surfaces required for reordering input
  frames.
- The application calls the :cpp:func:`MFXVideoENCODE_EncodeFrameAsync` function
  for the encoding operation. The input frame must be in an unlocked frame
  surface from the frame surface pool. If the encoding output is not available,
  the function returns the :cpp:enumerator:`mfxStatus::MFX_ERR_MORE_DATA` status
  code to request additional input frames.
- Upon successful encoding, the :cpp:func:`MFXVideoENCODE_EncodeFrameAsync`
  function returns :cpp:enumerator:`mfxStatus::MFX_ERR_NONE`. At this point, the
  encoded bitstream is not yet available because the
  :cpp:func:`MFXVideoENCODE_EncodeFrameAsync` function is asynchronous. The
  application must use the :cpp:func:`MFXVideoCORE_SyncOperation` function to
  synchronize the encoding operation before retrieving the encoded bitstream.
- At the end of the stream, the application continuously calls the
  :cpp:func:`MFXVideoENCODE_EncodeFrameAsync` function with a NULL surface
  pointer to drain any remaining bitstreams cached within the |vpl_short_name| encoder,
  until the function returns :cpp:enumerator:`mfxStatus::MFX_ERR_MORE_DATA`.

.. note:: It is the application's responsibility to fill pixels outside of the
          crop window when it is smaller than the frame to be encoded,
          especially in cases when crops are not aligned to minimum coding block
          size (16 for AVC and 8 for HEVC and VP9).

---------------
Internal Memory
---------------

The following pseudo code shows the encoding procedure with internal memory:

.. literalinclude:: ../snippets/prg_encoding.c
   :language: c++
   :start-after: /*beg2*/
   :end-before: /*end2*/
   :lineno-start: 1

There are several key differences in this example, compared to external memory
(legacy mode):

- The application does not need to call the
  :cpp:func:`MFXVideoENCODE_QueryIOSurf` function to obtain the number of
  working frame surfaces since allocation is done by |vpl_short_name|.
- The application calls the :cpp:func:`MFXMemory_GetSurfaceForEncode` function
  to get a free surface for the subsequent encode operation.
- The application must call the
  :cpp:member:`mfxFrameSurfaceInterface::Release`
  function to decrement the reference counter of the obtained surface after
  the call to the :cpp:func:`MFXVideoENCODE_EncodeFrameAsync` function.


.. _config-change:

--------------------
Configuration Change
--------------------

The application changes configuration during encoding by calling the
:cpp:func:`MFXVideoENCODE_Reset` function. Depending on the difference in
configuration parameters before and after the change, the |vpl_short_name| encoder will
either continue the current sequence or start a new one. If the encoder starts a
new sequence, it completely resets internal state and begins a new sequence with
the IDR frame.

The application controls encoder behavior during parameter change by attaching
the :cpp:struct:`mfxExtEncoderResetOption` structure to the
:cpp:struct:`mfxVideoParam` structure during reset. By using this structure, the
application instructs the encoder to start or not start a new sequence after
reset. In some cases, the request to continue the current sequence cannot be
satisfied and the encoder will fail during reset. To avoid this scenario, the
application may query the reset outcome before the actual reset by calling the
:cpp:func:`MFXVideoENCODE_Query` function with the
:cpp:struct:`mfxExtEncoderResetOption` attached to the
:cpp:struct:`mfxVideoParam` structure.

The application uses the following procedure to change encoding configurations:

#. The application retrieves any cached frames in the |vpl_short_name| encoder by calling
   the :cpp:func:`MFXVideoENCODE_EncodeFrameAsync` function with a NULL input
   frame pointer until the function returns
   :cpp:enumerator:`mfxStatus::MFX_ERR_MORE_DATA`.

#. The application calls the :cpp:func:`MFXVideoENCODE_Reset` function with the
   new configuration:

   - If the function successfully sets the configuration, the application can
     continue encoding as usual.
   - If the new configuration requires a new memory allocation, the function
     returns :cpp:enumerator:`mfxStatus::MFX_ERR_INCOMPATIBLE_VIDEO_PARAM`. The
     application must close the |vpl_short_name| encoder and reinitialize the encoding
     procedure with the new configuration.

------------------------
External Bitrate Control
------------------------

The application can make the encoder use the external Bitrate Control (BRC)
instead of the native bitrate control. To make the encoder use the external BRC,
the application should attach the :cpp:struct:`mfxExtCodingOption2` structure
with ``ExtBRC = MFX_CODINGOPTION_ON`` and the :cpp:struct:`mfxExtBRC` callback
structure to the :cpp:struct:`mfxVideoParam` structure during encoder
initialization. The **Init**, **Reset**, and **Close** callbacks will be invoked
inside their corresponding functions: :cpp:func:`MFXVideoENCODE_Init`,
:cpp:func:`MFXVideoENCODE_Reset`, and :cpp:func:`MFXVideoENCODE_Close`. The
following figure shows asynchronous encoding flow with external BRC (using
``GetFrameCtrl`` and ``Update``):

.. figure:: ../images/extbrc_async.png
   :alt: Asynchronous encoding flow with external BRC

   Asynchronous encoding flow with external BRC

.. note:: ``IntAsyncDepth`` is the |vpl_short_name| max internal asynchronous encoding
   queue size. It is always less than or equal to
   :cpp:member:`mfxVideoParam::AsyncDepth`.

The following pseudo code shows use of the external BRC:

.. literalinclude:: ../snippets/prg_encoding.c
   :language: c++
   :start-after: /*beg3*/
   :end-before: /*end3*/
   :lineno-start: 1

----
JPEG
----

The application can use the same encoding procedures for JPEG/motion JPEG
encoding, as shown in the following pseudo code:

.. code-block:: c++

   // encoder initialization
   MFXVideoENCODE_Init (...);
   // single frame/picture encoding
   MFXVideoENCODE_EncodeFrameAsync (...);
   MFXVideoCORE_SyncOperation(...);
   // close down
   MFXVideoENCODE_Close(...);

The application may specify Huffman and quantization tables during encoder
initialization by attaching :cpp:struct:`mfxExtJPEGQuantTables` and
:cpp:struct:`mfxExtJPEGHuffmanTables` buffers to the :cpp:struct:`mfxVideoParam`
structure. If the application does not define tables, then the |vpl_short_name| encoder
uses tables recommended in ITU-T\* Recommendation T.81. If the application does
not define a quantization table it must specify the
:cpp:member:`mfxInfoMFX::Quality` parameter. In this case, the |vpl_short_name| encoder
scales the default quantization table according to the specified
:cpp:member:`mfxInfoMFX::Quality` parameter value.

The application should properly configure chroma sampling format and color
format using the :cpp:member:`mfxFrameInfo::FourCC` and
:cpp:member:`mfxFrameInfo::ChromaFormat` fields. For example, to encode a 4:2:2
vertically sampled YCbCr picture, the application should set
:cpp:member:`mfxFrameInfo::FourCC` to :cpp:enumerator:`MFX_FOURCC_YUY2` and
:cpp:member:`mfxFrameInfo::ChromaFormat` to
:cpp:enumerator:`MFX_CHROMAFORMAT_YUV422V`. To encode a 4:4:4 sampled RGB
picture, the application should set :cpp:member:`mfxFrameInfo::FourCC` to
:cpp:enumerator:`MFX_FOURCC_RGB4` and :cpp:member:`mfxFrameInfo::ChromaFormat`
to :cpp:enumerator:`MFX_CHROMAFORMAT_YUV444`.

The |vpl_short_name| encoder supports different sets of chroma sampling and color formats
on different platforms. The application must call the
:cpp:func:`MFXVideoENCODE_Query` function to check if the required color format
is supported on a given platform and then initialize the encoder with proper
values of :cpp:member:`mfxFrameInfo::FourCC` and
:cpp:member:`mfxFrameInfo::ChromaFormat`.

The application should not define the number of scans and number of components.
These numbers are derived by the |vpl_short_name| encoder from the
:cpp:member:`mfxInfoMFx::Interleaved` flag and from chroma type. If interleaved
coding is specified, then one scan is encoded that contains all image
components. Otherwise, the number of scans is equal to number of components.
The encoder uses the following component IDs: “1” for luma (Y), “2” for chroma
Cb (U), and “3” for chroma Cr (V).

The application should allocate a buffer that is big enough to hold the encoded
picture. A rough upper limit may be calculated using the following equation
where **Width** and **Height** are width and height of the picture in pixel and
**BytesPerPx** is the number of bytes for one pixel:

.. code-block:: c++

   BufferSizeInKB = 4 + (Width * Height * BytesPerPx + 1023) / 1024;

The equation equals 1 for a monochrome picture, 1.5 for NV12 and YV12 color
formats, 2 for YUY2 color format, and 3 for RGB32 color format (alpha channel is
not encoded).

-------------------------
Multi-view Video Encoding
-------------------------

Similar to the decoding and video processing initialization procedures, the
application attaches the :cpp:struct:`mfxExtMVCSeqDesc` structure to the
:cpp:struct:`mfxVideoParam` structure for encoding initialization. The
:cpp:struct:`mfxExtMVCSeqDesc` structure configures the |vpl_short_name| MVC encoder to
work in three modes:

- **Default dependency mode:** The application specifies
  :cpp:member:`mfxExtMVCSeqDesc::NumView` and all other fields to zero. The
  |vpl_short_name| encoder creates a single operation point with all views (view
  identifier 0 : NumView-1) as target views. The first view (view identifier 0)
  is the base view. Other views depend on the base view.

- **Explicit dependency mode:** The application specifies
  :cpp:member:`mfxExtMVCSeqDesc::NumView` and the view dependency array, and
  sets all other fields to zero. The |vpl_short_name| encoder creates a single operation
  point with all views (view identifier View[0 : NumView-1].ViewId) as target
  views. The first view (view identifier View[0].ViewId) is the base view. View
  dependencies are defined as :cpp:struct:`mfxMVCViewDependency` structures.

- **Complete mode:** The application fully specifies the views and their
  dependencies. The |vpl_short_name| encoder generates a bitstream with corresponding
  stream structures.

During encoding, the |vpl_short_name| encoding function
:cpp:func:`MFXVideoENCODE_EncodeFrameAsync` accumulates input frames until
encoding of a picture is possible. The function returns
:cpp:enumerator:`mfxStatus::MFX_ERR_MORE_DATA` for more data at input or
:cpp:enumerator:`mfxStatus::MFX_ERR_NONE` if it successfully accumulated enough
data for encoding a picture. The generated bitstream contains the complete
picture (multiple views). The application can change this behavior and instruct
the encoder to output each view in a separate bitstream buffer. To do so, the
application must turn on the :cpp:member:`mfxExtCodingOption::ViewOutput` flag.
In this case, the encoder returns
:cpp:enumerator:`mfxStatus::MFX_ERR_MORE_BITSTREAM` if it needs more bitstream
buffers at output and :cpp:enumerator:`mfxStatus::MFX_ERR_NONE` when processing
of the picture (multiple views) has been finished. It is recommended that the
application provide a new input frame each time the |vpl_short_name| encoder requests a
new bitstream buffer. The application must submit view data for encoding in the
order they are described in the :cpp:struct:`mfxExtMVCSeqDesc` structure.
Particular view data can be submitted for encoding only when all views that it
depends upon have already been submitted.

The following pseudo code shows the encoding procedure:

.. literalinclude:: ../snippets/prg_encoding.c
   :language: c++
   :start-after: /*beg4*/
   :end-before: /*end4*/
   :lineno-start: 1

---------------------------
Encoder Quality Information
---------------------------

Video encoder supports outputting quality information. Currently, it only 
reports the mean squared error (MSE).

- The application uses the :cpp:struct:`mfxExtQualityInfoMode` structure with 
  ``mfxQualityInfoMode = MFX_QUALITY_INFO_LEVEL_FRAME`` to initiate the request.
- The application uses the :cpp:struct:`mfxExtQualityInfoOutput` structure to obtain the 
  MSE result.

The following pseudo code shows an example of gathering encoder quality information:

.. literalinclude:: ../snippets/prg_encoding.c
   :language: c++
   :start-after: /*beg9*/
   :end-before: /*end9*/
   :lineno-start: 1

Note the following key points about the example:

- The application must attach the :cpp:struct:`mfxExtQualityInfoMode` structure to the 
  :cpp:struct:`mfxVideoParam` structure and call the :cpp:func:`MFXVideoENCODE_Query` to 
  check the support status of the request. The function returns 
  :cpp:enumerator:`mfxStatus::MFX_ERR_NONE` if support is successful, or issues a warning 
  :cpp:enumerator:`mfxStatus::MFX_WRN_INCOMPATIBLE_VIDEO_PARAM` if not supported, and 
  sets :cpp:struct:`mfxExtQualityInfoMode` to 
  ``mfxQualityInfoMode = MFX_QUALITY_INFO_DISABLE``.
- The application uses the :cpp:func:`MFXVideoENCODE_Init` to initialize the encoder.
- The application can attach the :cpp:struct:`mfxExtQualityInfoOutput` structure to 
  the encoded bitstream structure before calling 
  :cpp:func:`MFXVideoENCODE_EncodeFrameAsync` function.
- The application must use the :cpp:func:`MFXVideoCORE_SyncOperation` function to 
  synchronize the encoding operation before retrieving the encoded bitstream and MSE 
  result.

------------------------
AV1 Screen Content Tools
------------------------

AV1 video encoder supports Palette Prediction and Intra Block Copy mode.
The following pseudo code shows an example of encoding with AV1 Screen Content tools:

.. literalinclude:: ../snippets/prg_encoding.c
   :language: c++
   :start-after: /*beg10*/
   :end-before: /*end10*/
   :lineno-start: 1

Note the following key points about the example:

- The application must attach the :cpp:struct:`mfxExtAV1ScreenContentTools` structure to 
  the :cpp:struct:`mfxVideoParam` structure and call the :cpp:func:`MFXVideoENCODE_Query` 
  to check the support status of the request. The function returns 
  :cpp:enumerator:`mfxStatus::MFX_ERR_NONE` if support is successful, issues a warning   
  :cpp:enumerator:`mfxStatus::MFX_WRN_INCOMPATIBLE_VIDEO_PARAM` due to platform 
  limitations and updates the :cpp:struct:`mfxExtAV1ScreenContentTools` structure values 
  to the default, or returns the error :cpp:enumerator:`mfxStatus::MFX_ERR_UNSUPPORTED` if 
  not support.
- The application uses the :cpp:func:`MFXVideoENCODE_Init` to initialize the encoder.
- The application calls the :cpp:func:`MFXVideoENCODE_EncodeFrameAsync` function to 
  perform the encoding operation.
- The application must use the :cpp:func:`MFXVideoCORE_SyncOperation` function to 
  synchronize the encoding operation before retrieving the encoded bitstream.

----------------------
Alpha Channel Encoding
----------------------

The following pseudo code shows several examples of alpha channel encoding
with multiple input data formats:

.. literalinclude:: ../snippets/prg_encoding.c
   :language: c++
   :start-after: /*beg11*/
   :end-before: /*end11*/
   :lineno-start: 1

Note the following key points about the example:

API Definition:

- :cpp:struct:`mfxExtAlphaChannelEncCtrl`: This structure configures the alpha channel encoding. It includes fields like EnableAlphaChannelEncoding, AlphaChannelMode, and AlphaChannelBitrateRatio.
- :cpp:member:`mfxExtAlphaChannelEncCtrl::EnableAlphaChannelEncoding`: Set to :cpp:enumerator:`MFX_CODINGOPTION_ON` to enable alpha channel encoding.
- :cpp:member:`mfxExtAlphaChannelEncCtrl::AlphaChannelMode`: Defines whether the alpha is straight or pre-multiplied.

   - :cpp:enumerator:`MFX_ALPHA_MODE_PREMULTIPLIED`: RGB and alpha are independent.
   - :cpp:enumerator:`MFX_ALPHA_MODE_STRAIGHT`: RGB and alpha are linked.
- :cpp:member:`mfxExtAlphaChannelEncCtrl::AlphaChannelBitrateRatio`: Indicates the percentage of the bitrate allocated to the alpha channel.

Usage Instructions:

- At present, Internal Memory Allocation as described in `Memory Allocation and External Allocators <https://intel.github.io/libvpl/latest/programming_guide/VPL_prg_mem.html>`_ is not yet supported.
- The application queries encoding capabilities (:cpp:func:`MFXVideoENCODE_Query`) and surface requirements (:cpp:func:`MFXVideoENCODE_QueryIOSurf`).
- The application initializes(:cpp:func:`MFXVideoENCODE_Init`) the encoder with alpha channel encoding enabled.
- Alpha channel data can be provided separately or embedded within the main surface data depending on the format.
- For each input data type, the application configures the alpha surface accordingly before calling :cpp:func:`MFXVideoENCODE_EncodeFrameAsync` for encoding.

   - NV12_VIDEO: Uses video memory for NV12 data, with a mfxExtAlphaChannelSurface structure for video memory handle.
   - NV12_SYS: Uses system memory for NV12 data, with a mfxExtAlphaChannelSurface structure attached to the surface.
   - RGBA_VIDEO: Directly uses shared resources for RGBA video textures.
   - RGBA_SYS: Not yet supported.

- The application must use the :cpp:func:`MFXVideoCORE_SyncOperation` function to synchronize the encoding operation before retrieving the encoded bitstream.