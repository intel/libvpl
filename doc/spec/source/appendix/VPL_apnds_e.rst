.. SPDX-FileCopyrightText: 2019-2020 Intel Corporation
..
.. SPDX-License-Identifier: CC-BY-4.0

========================================
Working Directly with VA API for Linux\*
========================================

|msdk_full_name| takes care of all memory and synchronization related operations in the VA API.
The application may need to extend |msdk_full_name| functionality by working directly
with the VA API for Linux\*, for example to implement a customized external allocator.
This section describes basic memory management and synchronization techniques.

To create the VA surface pool, the application should call the vaCreateSurfaces
function:

.. literalinclude:: ../snippets/appnd_e.c
   :language: c++
   :start-after: /*beg1*/
   :end-before: /*end1*/
   :lineno-start: 1

To destroy the surface pool, the application should call the vaDestroySurfaces
function:

.. literalinclude:: ../snippets/appnd_e.c
   :language: c++
   :start-after: /*beg2*/
   :end-before: /*end2*/
   :lineno-start: 1

If the application works with hardware acceleration through |msdk_full_name|, then it can
access surface data immediately after successful completion of
the :cpp:func:`MFXVideoCORE_SyncOperation` call. If the application works with
hardware acceleration directly, then it must check surface status before
accessing data in video memory. This check can be done asynchronously by calling
the vaQuerySurfaceStatus function or synchronously by calling the vaSyncSurface
function.

After successful synchronization, the application can access surface data.
Accessing surface data is performed in two steps:

#. Create VAImage from surface.
#. Map image buffer to system memory.

After mapping, the VAImage.offsets[3] array holds offsets to each color
plain in a mapped buffer and the VAImage.pitches[3] array holds color plain
pitches in bytes. For packed data formats, only first entries in these
arrays are valid. The following example shows how to access data in a NV12 surface:

.. literalinclude:: ../snippets/appnd_e.c
   :language: c++
   :start-after: /*beg3*/
   :end-before: /*end3*/
   :lineno-start: 1


After processing data in a VA surface, the application should release resources
allocated for the mapped buffer and VAImage object:

.. literalinclude:: ../snippets/appnd_e.c
   :language: c++
   :start-after: /*beg4*/
   :end-before: /*end4*/
   :lineno-start: 1

In some cases, in order to retrieve encoded bitstream data from video memory,
the application must use the VABuffer to store data. The following example shows
how to create, use, and destroy the VABuffer:

.. literalinclude:: ../snippets/appnd_e.c
   :language: c++
   :start-after: /*beg5*/
   :end-before: /*end5*/
   :lineno-start: 1

Note that the vaMapBuffer function returns pointers to different objects
depending on the mapped buffer type. The VAImage is a plain data buffer and the
encoded bitstream is a VACodedBufferSegment structure. The application cannot use
VABuffer for synchronization. If encoding, it is recommended to synchronize
using the VA surface as described above.
