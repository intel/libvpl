.. SPDX-FileCopyrightText: 2019-2020 Intel Corporation
..
.. SPDX-License-Identifier: CC-BY-4.0

.. _mem-alloc-ext-alloc:

=========================================
Memory Allocation and External Allocators
=========================================

There are two models of memory management in oneVPL: internal and external.

--------------------------
External Memory Management
--------------------------

In the external memory model, the application must allocate sufficient memory for
input and output parameters and buffers and deallocate it when oneVPL functions
complete their operations. During execution, the oneVPL functions use callback
functions to the application to manage memory for video frames through the
external allocator interface :cpp:struct:`mfxFrameAllocator`.

If an application needs to control the allocation of video frames, it can use
callback functions through the :cpp:struct:`mfxFrameAllocator` interface. If an
application does not specify an allocator, an internal allocator is used. However,
if an application uses video memory surfaces for input and output, it must specify
the hardware acceleration device and an external frame allocator using
:cpp:struct:`mfxFrameAllocator`.

The external frame allocator can allocate different frame types:

- In-system memory.
- In-video memory, as 'Decoder Render Targets' or 'Processor Render Targets.'
  See :ref:`Working with Hardware Acceleration <hw-acceleration>` for additional
  details.

The external frame allocator responds only to frame allocation requests for the
requested memory type and returns :cpp:enumerator:`mfxStatus::MFX_ERR_UNSUPPORTED`
for all other types. The allocation request uses flags (part of the memory type
field) to indicate which oneVPL class initiated the request so that the external
frame allocator can respond accordingly.

The following example shows a simple external frame allocator:

.. literalinclude:: ../snippets/prg_mem.c
   :language: c++
   :start-after: /*beg1*/
   :end-before: /*end1*/
   :lineno-start: 1

For system memory, it is highly recommended to allocate memory for all
planes of the same frame as a single buffer (using one single malloc call).

.. _internal-mem-manage:

--------------------------
Internal Memory Management
--------------------------

In the internal memory management model, oneVPL provides interface functions for
frames allocation:

- :cpp:func:`MFXMemory_GetSurfaceForVPP`

- :cpp:func:`MFXMemory_GetSurfaceForVPPOut`

- :cpp:func:`MFXMemory_GetSurfaceForEncode`

- :cpp:func:`MFXMemory_GetSurfaceForDecode`

These functions are used together with :cpp:struct:`mfxFrameSurfaceInterface`
for surface management. The surface returned by these functions is a reference
counted object and the application must call :cpp:member:`mfxFrameSurfaceInterface::Release`
after finishing all operations with the surface. In this model the application
does not need to create and set the external allocator to oneVPL.

Another method to obtain an internally allocated surface is to call
:cpp:func:`MFXVideoDECODE_DecodeFrameAsync` with a working surface equal to NULL
(see :ref:`Simplified decoding procedure <simplified-decoding-procedure>`). In
this scenario, the decoder will allocate a new refcountable
:cpp:struct:`mfxFrameSurface1` and return it to the user. All assumed contracts
with the user are similar to the **MFXMemory_GetSurfaceForXXX** functions.

------------------------
mfxFrameSurfaceInterface
------------------------

oneVPL API version 2.0 introduces :cpp:struct:`mfxFrameSurfaceInterface`. This
interface is a set of callback functions to manage the lifetime of allocated
surfaces, get access to pixel data, and obtain native handles and device
abstractions (if suitable). Instead of directly accessing
:cpp:struct:`mfxFrameSurface1` structure members, it's recommended to use
the :cpp:struct:`mfxFrameSurfaceInterface` if present or call external
allocator callback functions if set.

The following pseudo code shows the usage of :cpp:struct:`mfxFrameSurfaceInterface`
for memory sharing:

.. literalinclude:: ../snippets/prg_mem.c
   :language: c++
   :start-after: /*beg2*/
   :end-before: /*end2*/
   :lineno-start: 1
