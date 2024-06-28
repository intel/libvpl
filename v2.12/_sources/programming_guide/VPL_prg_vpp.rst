.. SPDX-FileCopyrightText: 2019-2020 Intel Corporation
..
.. SPDX-License-Identifier: CC-BY-4.0

.. _vid_process_procedure:

===========================
Video Processing Procedures
===========================

The following pseudo code shows the video processing procedure:

.. literalinclude:: ../snippets/prg_vpp.c
   :language: c++
   :start-after: /*beg1*/
   :end-before: /*end1*/
   :lineno-start: 1

Note the following key points about the example:

- The application uses the :cpp:func:`MFXVideoVPP_QueryIOSurf` function to
  obtain the number of frame surfaces needed for input and output. The application
  must allocate two frame surface pools: one for the input and one for the output.
- The video processing function :cpp:func:`MFXVideoVPP_RunFrameVPPAsync` is
  asynchronous. The application must use the :cpp:func:`MFXVideoCORE_SyncOperation`
  function to synchronize in order to make the output result ready.
- The body of the video processing procedure covers the following three scenarios:

  - If the number of frames consumed at input is equal to the number of frames
    generated at output, :term:`VPP` returns :cpp:enumerator:`mfxStatus::MFX_ERR_NONE` when an
    output is ready. The application must process the output frame after
    synchronization, as the :cpp:func:`MFXVideoVPP_RunFrameVPPAsync` function is
    asynchronous. The application must provide a NULL input at the end of the
    sequence to drain any remaining frames.
  - If the number of frames consumed at input is more than the number of frames
    generated at output, VPP returns :cpp:enumerator:`mfxStatus::MFX_ERR_MORE_DATA` for
    additional input until an output is ready. When the output is ready, VPP
    returns :cpp:enumerator:`mfxStatus::MFX_ERR_NONE`. The application must process the
    output frame after synchronization and provide a NULL input at the end of the
    sequence to drain any remaining frames.
  - If the number of frames consumed at input is less than the number of frames
    generated at output, VPP returns either :cpp:enumerator:`mfxStatus::MFX_ERR_MORE_SURFACE`
    (when more than one output is ready), or :cpp:enumerator:`mfxStatus::MFX_ERR_NONE`
    (when one output is ready and VPP expects new input). In both cases, the
    application must process the output frame after synchronization and provide
    a NULL input at the end of the sequence to drain any remaining frames.

-------------
Configuration
-------------

|vpl_short_name| configures the video processing pipeline operation based on the
difference between the input and output formats, specified in the
:cpp:struct:`mfxVideoParam` structure. The following list shows several examples:

- When the input color format is :term:`YUY2` and the output color format is
  :term:`NV12`, |vpl_short_name| enables color conversion from YUY2 to NV12.
- When the input is interleaved and the output is progressive, |vpl_short_name| enables
  deinterlacing.
- When the input is single field and the output is interlaced or progressive,
  |vpl_short_name| enables field weaving, optionally with deinterlacing.
- When the input is interlaced and the output is single field, |vpl_short_name| enables
  field splitting.

In addition to specifying the input and output formats, the application can
provide hints to fine-tune the video processing pipeline operation. The
application can disable filters in the pipeline by using the
:cpp:struct:`mfxExtVPPDoNotUse` structure, enable filters by using the
:cpp:struct:`mfxExtVPPDoUse` structure, and configure filters by using dedicated
configuration structures. See the :ref:`Configurable VPP Filters table <vpp-filters-table>`
for a complete list of configurable video processing filters, their IDs, and
configuration structures. See the :ref:`ExtendedBufferID enumerator <extendedbufferid>`
for more details.

|vpl_short_name| ensures that all filters necessary to convert the input format to the
output format are included in the pipeline. |vpl_short_name| may skip some optional
filters even if they are explicitly requested by the application, for example
due to limitations of the underlying hardware. To notify the application about
skipped optional filters, |vpl_short_name| returns the :cpp:enumerator:`mfxStatus::MFX_WRN_FILTER_SKIPPED`
warning. The application can retrieve the list of active filters by attaching
the :cpp:struct:`mfxExtVPPDoUse` structure to the :cpp:struct:`mfxVideoParam`
structure and calling the :cpp:func:`MFXVideoVPP_GetVideoParam` function. The
application must allocate enough memory for the filter list.

See the :ref:`Configurable VPP Filters table <vpp-filters-table>` for a full
list of configurable filters.

.. _vpp-filters-table:

.. list-table:: Configurable VPP Filters
   :header-rows: 1
   :widths: 58 42

   * - **Filter ID**
     - **Configuration Structure**
   * - :cpp:enumerator:`MFX_EXTBUFF_VPP_DENOISE2`
     - :cpp:struct:`mfxExtVPPDenoise2`
   * - :cpp:enumerator:`MFX_EXTBUFF_VPP_MCTF`
     - :cpp:struct:`mfxExtVppMctf`
   * - :cpp:enumerator:`MFX_EXTBUFF_VPP_DETAIL`
     - :cpp:struct:`mfxExtVPPDetail`
   * - :cpp:enumerator:`MFX_EXTBUFF_VPP_FRAME_RATE_CONVERSION`
     - :cpp:struct:`mfxExtVPPFrameRateConversion`
   * - :cpp:enumerator:`MFX_EXTBUFF_VPP_IMAGE_STABILIZATION`
     - :cpp:struct:`mfxExtVPPImageStab`
   * - :cpp:enumerator:`MFX_EXTBUFF_VPP_PROCAMP`
     - :cpp:struct:`mfxExtVPPProcAmp`
   * - :cpp:enumerator:`MFX_EXTBUFF_VPP_FIELD_PROCESSING`
     - :cpp:struct:`mfxExtVPPFieldProcessing`
   * - :cpp:enumerator:`MFX_EXTBUFF_VPP_3DLUT`
     - :cpp:struct:`mfxExtVPP3DLut`

The following example shows video processing configuration:

.. literalinclude:: ../snippets/prg_vpp.c
   :language: c++
   :start-after: /*beg2*/
   :end-before: /*end2*/
   :lineno-start: 1

------------------
Region of Interest
------------------

During video processing operations, the application can specify a region of
interest for each frame as shown in the following figure:

.. figure:: ../images/vpp_region_of_interest_operation.png
   :alt: VPP region of interest operation

   VPP region of interest operation

Specifying a region of interest guides the resizing function to achieve special
effects, such as resizing from 16:9 to 4:3, while keeping the aspect ratio intact.
Use the ``CropX``, ``CropY``, ``CropW``, and ``CropH`` parameters in the
:cpp:struct:`mfxVideoParam` structure to specify a region of interest for each frame
when calling :cpp:func:`MFXVideoVPP_RunFrameVPPAsync`. Note: For per-frame dynamic
change, the application should set the ``CropX``, ``CropY``, ``CropW``, and ``CropH``
parameters when calling :cpp:func:`MFXVideoVPP_RunFrameVPPAsync` per frame.

The :ref:`VPP Region of Interest Operations table <vpp-region-op-table>` shows
examples of VPP operations applied to a region of interest.

.. _vpp-region-op-table:

.. list-table:: VPP Region of Interest Operations
   :header-rows: 1
   :widths: 30 15 20 15 20

   * - | **Operation**
       |
       |
     - | **VPP Input**
       | *Width X Height*
       |
     - | **VPP Input**
       | *CropX, CropY,*
       | *CropW, CropH*
     - | **VPP Output**
       | *Width X Height*
       |
     - | **VPP Output**
       | *CropX, CropY,*
       | *CropW, CropH*
   * - Cropping
     - 720 x 480
     - 16, 16, 688, 448
     - 720 x 480
     - 16, 16, 688, 448
   * - Resizing
     - 720 x 480
     - 0, 0, 720, 480
     - 1440 x 960
     - 0, 0, 1440, 960
   * - Horizontal stretching
     - 720 x 480
     - 0, 0, 720, 480
     - 640 x 480
     - 0, 0, 640, 480
   * - 16:9 4:3 with letter boxing at the top and bottom
     - 1920 x 1088
     - 0, 0, 1920, 1088
     - 720 x 480
     - 0, 36, 720, 408
   * - 4:3 16:9 with pillar boxing at the left and right
     - 720 x 480
     - 0, 0, 720, 480
     - 1920 x 1088
     - 144, 0, 1632, 1088



---------------------------
Multi-view Video Processing
---------------------------

|vpl_short_name| video processing supports processing multiple views. For video processing
initialization, the application needs to attach the :cpp:struct:`mfxExtMVCSeqDesc`
structure to the :cpp:struct:`mfxVideoParam` structure and call the
:cpp:func:`MFXVideoVPP_Init` function. The function saves the view identifiers.
During video processing, |vpl_short_name| processes each view individually. |vpl_short_name| refers
to the ``FrameID`` field of the :cpp:struct:`mfxFrameInfo` structure to configure
each view according to its processing pipeline. If the video processing source
frame is not the output from the |vpl_short_name| MVC decoder, then the application needs to
fill the the ``FrameID`` field before calling the :cpp:func:`MFXVideoVPP_RunFrameVPPAsync`
function. This is shown in the following pseudo code:

.. literalinclude:: ../snippets/prg_vpp.c
   :language: c++
   :start-after: /*beg3*/
   :end-before: /*end3*/
   :lineno-start: 1

----------------------
Video Processing 3DLUT
----------------------

|vpl_short_name| video processing supports 3DLUT with Intel HW specific memory layout. The following pseudo code
shows how to create a :cpp:enumerator:`MFX_3DLUT_MEMORY_LAYOUT_INTEL_65LUT` 3DLUT surface.

.. literalinclude:: ../snippets/prg_vpp.c
   :language: c++
   :start-after: /*beg4*/
   :end-before: /*end4*/
   :lineno-start: 1
   
The following pseudo code shows how to create a system memory :cpp:struct:`mfx3DLutSystemBuffer` 3DLUT surface.

.. literalinclude:: ../snippets/prg_vpp.c
   :language: c++
   :start-after: /*beg5*/
   :end-before: /*end5*/
   :lineno-start: 1

The following pseudo code shows how to specify 3DLUT interpolation method :cpp:enum:`mfx3DLutInterpolationMethod`.

.. literalinclude:: ../snippets/prg_vpp.c
   :language: c++
   :start-after: /*beg11*/
   :end-before: /*end11*/
   :lineno-start: 1

----------------
HDR Tone Mapping
----------------

|vpl_short_name| video processing supports HDR Tone Mapping with Intel HW. The following pseudo code
shows how to perform HDR Tone Mapping.

The following pseudo code shows HDR to SDR.

.. literalinclude:: ../snippets/prg_vpp.c
   :language: c++
   :start-after: /*beg6*/
   :end-before: /*end6*/
   :lineno-start: 1
   
The following pseudo code shows SDR to HDR.

.. literalinclude:: ../snippets/prg_vpp.c
   :language: c++
   :start-after: /*beg7*/
   :end-before: /*end7*/
   :lineno-start: 1
   
The following pseudo code shows HDR to HDR.

.. literalinclude:: ../snippets/prg_vpp.c
   :language: c++
   :start-after: /*beg8*/
   :end-before: /*end8*/
   :lineno-start: 1

-----------------------
Camera RAW acceleration
-----------------------

|vpl_short_name| supports camera raw format processing with Intel HW. The following pseudo code
shows how to perform camera raw hardware acceleration. For pipeline processing initialization,
the application needs to attach the camera structures to the :cpp:struct:`mfxVideoParam` structure
and call the :cpp:func:`MFXVideoVPP_Init` function.

The following pseudo code shows camera raw processing.

.. literalinclude:: ../snippets/prg_vpp.c
   :language: c++
   :start-after: /*beg9*/
   :end-before: /*end9*/
   :lineno-start: 1

|vpl_short_name| supports AI based super resolution with Intel HW. The following pseudo code
shows how to perform AI based super resolution.

---------------------------
AI Powered Video Processing
---------------------------

The following pseudo code shows AI super resolution video processing.

.. literalinclude:: ../snippets/prg_vpp.c
   :language: c++
   :start-after: /*beg12*/
   :end-before: /*end12*/
   :lineno-start: 1

The following pseudo code shows AI frame interpolation video processing.

.. literalinclude:: ../snippets/prg_vpp.c
   :language: c++
   :start-after: /*beg13*/
   :end-before: /*end13*/
   :lineno-start: 1
