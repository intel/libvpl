.. SPDX-FileCopyrightText: 2019-2020 Intel Corporation
..
.. SPDX-License-Identifier: CC-BY-4.0

.. _hw-acceleration:

=====================
Hardware Acceleration
=====================

oneVPL provides a new model for working with hardware acceleration while
continuing to support hardware acceleration in legacy mode.

--------------------------------------------
New Model to Work with Hardware Acceleration
--------------------------------------------

oneVPL API version 2.0 introduces a new memory model: internal allocation where
oneVPL is responsible for video memory allocation. In this mode,
an application is not dependent on a low-level video framework API, such as
DirectX\* or the VA API, and does not need to create and set corresponding
low-level oneVPL primitives such as `ID3D11Device` or `VADisplay`. Instead,
oneVPL creates all required objects to work with hardware acceleration and video
surfaces internally. An application can get access to these objects using
:cpp:func:`MFXVideoCORE_GetHandle` or with help of the
:cpp:struct:`mfxFrameSurfaceInterface` interface.

This approach simplifies the oneVPL initialization, making calls to the
:cpp:func:`MFXVideoENCODE_QueryIOSurf`, :cpp:func:`MFXVideoDECODE_QueryIOSurf`,
or :cpp:func:`MFXVideoVPP_QueryIOSurf` functions optional. See
:ref:`Internal Memory Management <internal-mem-manage>`.

.. note:: Applications can set device handle before session creation through :cpp:func:`MFXSetConfigFilterProperty` 
          like shown in the code below:

.. literalinclude:: ../snippets/prg_hw.cpp
   :language: c++
   :start-after: /*beg3*/
   :end-before: /*end3*/
   :lineno-start: 1


----------------------------------------------
Work with Hardware Acceleration in Legacy Mode
----------------------------------------------

Work with Multiple Media Devices
--------------------------------

If your system has multiple graphics adapters, you may need hints on which
adapter is better suited to process a particular workload. The legacy mode of
oneVPL provides a helper API to select the most suitable adapter for your
workload based on the provided workload description.

.. important:: :cpp:func:`MFXQueryAdapters`, :cpp:func:`MFXQueryAdaptersDecode`,
               and :cpp:func:`MFXQueryAdaptersNumber` are deprecated starting
               from API 2.9. Applications should use :cpp:func:`MFXEnumImplementations`
               and :cpp:func:`MFXSetConfigFilterProperty` to query adapter
               capabilities and to select a suitable adapter for the input workload.

The following example shows workload initialization on a discrete adapter
in legacy mode:


.. literalinclude:: ../snippets/prg_hw.cpp
   :language: c++
   :start-after: /*beg1*/
   :end-before: /*end1*/
   :lineno-start: 1

The example shows that after obtaining the adapter list with
:cpp:func:`MFXQueryAdapters`, further initialization of :cpp:type:`mfxSession`
is performed in the regular way. The specific adapter is selected using
the :cpp:enumerator:`MFX_IMPL_HARDWARE`, :cpp:enumerator:`MFX_IMPL_HARDWARE2`,
:cpp:enumerator:`MFX_IMPL_HARDWARE3`, or :cpp:enumerator:`MFX_IMPL_HARDWARE4`
values of :cpp:type:`mfxIMPL`.

The following example shows the use of :cpp:func:`MFXQueryAdapters` for querying
the most suitable adapter for a particular encode workload:

.. literalinclude:: ../snippets/prg_hw.cpp
   :language: c++
   :start-after: /*beg2*/
   :end-before: /*end2*/
   :lineno-start: 1

See the :cpp:func:`MFXQueryAdapters` description for adapter priority rules.

Work with Video Memory
----------------------

To fully utilize the oneVPL acceleration capability, the application should support
OS specific infrastructures. If using Microsoft\* Windows\*, the application
should support Microsoft DirectX\*. If using Linux\*, the application should
support the VA API for Linux.

The hardware acceleration support in an application consists of video memory
support and acceleration device support.

Depending on the usage model, the application can use video memory at different
stages in the pipeline. Three major scenarios are shown in the following
diagrams:

.. graphviz::

  digraph {
    rankdir=LR;
    labelloc="t";
    label="oneVPL functions interconnection";
    F1 [shape=octagon label="oneVPL Function"];
    F2 [shape=octagon label="oneVPL Function"];
    F1->F2 [ label="Video Memory" ];
  }

|

.. graphviz::

  digraph {
    rankdir=LR;
    labelloc="t";
    label="Video memory as output";
    F3 [shape=octagon label="oneVPL Function"];
    F4 [shape=octagon label="Application" fillcolor=lightgrey];
    F3->F4 [ label="Video Memory" ];
  }

|

.. graphviz::

  digraph {
    rankdir=LR;
    labelloc="t";
    label="Video memory as input";
    F5 [shape=octagon label="Application"];
    F6 [shape=octagon label="oneVPL Function"];
    F5->F6 [ label="Video Memory" ];
  }

|

The application must use the :cpp:member:`mfxVideoParam::IOPattern` field to
indicate the I/O access pattern during initialization. Subsequent function calls
must follow this access pattern. For example, if a function operates on video
memory surfaces at both input and output, the
application must specify the access pattern `IOPattern` at initialization in
:cpp:enumerator:`MFX_IOPATTERN_IN_VIDEO_MEMORY` for input and
:cpp:enumerator:`MFX_IOPATTERN_OUT_VIDEO_MEMORY` for output. This particular I/O
access pattern must not change inside the **Init** **-** **Close** sequence.

Initialization of any hardware accelerated oneVPL component requires the
acceleration device handle. This handle is also used by the oneVPL component to
query hardware capabilities. The application can share its device with oneVPL
by passing the device handle through the :cpp:func:`MFXVideoCORE_SetHandle`
function. It is recommended to share the handle before any actual usage of oneVPL.

.. _work_ms_directx_app:

Work with Microsoft DirectX\* Applications
------------------------------------------

oneVPL supports two different infrastructures for hardware acceleration on the
Microsoft Windows OS: the Direct3D\* 9 DXVA2 and Direct3D 11 Video API. If Direct3D 9
DXVA2 is used for hardware acceleration, the application should use the
`IDirect3DDeviceManager9` interface as the acceleration device handle. If the
Direct3D 11 Video API is used for hardware acceleration, the application should
use the `ID3D11Device` interface as the acceleration device handle.

The application should share one of these interfaces with oneVPL through the
:cpp:func:`MFXVideoCORE_SetHandle` function. If the application does not provide
the interface, then oneVPL creates its own internal acceleration device. As a result, 
oneVPL input and output will be limited to system memory only for the external allocation mode, which will reduce oneVPL performance. If oneVPL fails to create a valid acceleration device, 
then oneVPL cannot proceed with hardware acceleration and returns an error status to the
application.

.. note:: It is recommended to work in the internal allocation mode if the application does not provide the `IDirect3DDeviceManager9` or `ID3D11Device` interface.

The application must create the Direct3D 9 device with the flag
``D3DCREATE_MULTITHREADED``. The flag ``D3DCREATE_FPU_PRESERVE`` is also
recommended. This influences floating-point calculations, including PTS values.

The application must also set multi-threading mode for the Direct3D 11 device.
The following example shows how to set multi-threading mode for a Direct3D 11
device:

.. code-block:: c++
   :lineno-start: 1

   ID3D11Device            *pD11Device;
   ID3D11DeviceContext     *pD11Context;
   ID3D10Multithread       *pD10Multithread;

   pD11Device->GetImmediateContext(&pD11Context);
   pD11Context->QueryInterface(IID_ID3D10Multithread, &pD10Multithread);
   pD10Multithread->SetMultithreadProtected(true);

During hardware acceleration, if a Direct3D “device lost” event occurs, the oneVPL
operation terminates with the :cpp:enumerator:`mfxStatus::MFX_ERR_DEVICE_LOST`
return status. If the application provided the Direct3D device handle, the
application must reset the Direct3D device.

When the oneVPL decoder creates auxiliary devices for hardware acceleration, it
must allocate the list of Direct3D surfaces for I/O access, also known as the
surface chain, and pass the surface chain as part of the device creation command.
In most cases, the surface chain is the frame surface pool mentioned in the
:ref:`Frame Surface Locking <frame-surface-manag>` section.

The application passes the surface chain to the oneVPL component **Init** function
through a oneVPL external allocator callback. See the
:ref:`Memory Allocation and External Allocators <mem-alloc-ext-alloc>` section
for details.

Only the decoder **Init** function requests the external surface chain from the
application and uses it for auxiliary device creation. Encoder and VPP **Init**
functions may only request internal surfaces. See the
:ref:`ExtMemFrameType enumerator <extmemframetype>` for more details about
different memory types.

Depending on configuration parameters, oneVPL requires different surface types.
It is strongly recommended to call the :cpp:func:`MFXVideoENCODE_QueryIOSurf`
function, the :cpp:func:`MFXVideoDECODE_QueryIOSurf` function, or the
:cpp:func:`MFXVideoVPP_QueryIOSurf` function to determine the appropriate type in 
the external allocation mode.

Work with VA API Applications
-----------------------------

oneVPL supports the VA API infrastructure for hardware acceleration on Linux.
The application should use the `VADisplay` interface as the acceleration device
handle for this infrastructure and share it with oneVPL through the
:cpp:func:`MFXVideoCORE_SetHandle` function.

The following example shows how to obtain the VA display from the X Window System:

.. code-block::
   :lineno-start: 1

   Display   *x11_display;
   VADisplay va_display;

   x11_display = XOpenDisplay(current_display);
   va_display  = vaGetDisplay(x11_display);

   MFXVideoCORE_SetHandle(session, MFX_HANDLE_VA_DISPLAY, (mfxHDL) va_display);

The following example shows how to obtain the VA display from the Direct
Rendering Manager:

.. code-block::
   :lineno-start: 1

   int card;
   VADisplay va_display;

   card = open("/dev/dri/card0", O_RDWR); /* primary card */
   va_display = vaGetDisplayDRM(card);
   vaInitialize(va_display, &major_version, &minor_version);

   MFXVideoCORE_SetHandle(session, MFX_HANDLE_VA_DISPLAY, (mfxHDL) va_display);

When the oneVPL decoder creates a hardware acceleration device, it must allocate
the list of video memory surfaces for I/O access, also known as the surface chain,
and pass the surface chain as part of the device creation command. The
application passes the surface chain to the oneVPL component **Init** function
through a oneVPL external allocator callback.  See the
:ref:`Memory Allocation and External Allocators <mem-alloc-ext-alloc>` section
for details.
Starting from oneVPL API version 2.0, oneVPL creates its own surface chain if an
external allocator is not set. See the :ref`New Model to work with Hardware Acceleration <hw-acceleration>`
section for details.


.. note:: The VA API does not define any surface types and the application can
          use either :cpp:enumerator:`MFX_MEMTYPE_VIDEO_MEMORY_DECODER_TARGET`
          or :cpp:enumerator:`MFX_MEMTYPE_VIDEO_MEMORY_PROCESSOR_TARGET` to indicate data in video
          memory.

