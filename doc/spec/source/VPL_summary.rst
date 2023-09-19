.. SPDX-FileCopyrightText: 2019-2020 Intel Corporation
..
.. SPDX-License-Identifier: CC-BY-4.0

============================
Mandatory APIs and Functions
============================

----------
Disclaimer
----------

Developers can implement any subset of the oneVPL API.
The specification makes no claim about what encoder, decoder, VPP
filter, or any other underlying features are mandatory for the implementation.
The oneVPL API is designed such that users have several options
to discover capabilities exposed by the implementation:

#. Before or after session creation: Users can get a list of supported encoders, decoders,
   VPP filters, correspondent color formats, and memory types with the help of
   the :cpp:func:`MFXEnumImplementations` function. For more details, see
   :ref:`oneVPL Dispatcher Interactions <onevpl-dispatcher-interactions>`.
#. After session is created: Users can call **Query** functions to obtain
   low level implementation capabilities.

.. attention:: The legacy |msdk_full_name| implementation does not support the
    first approach to obtain capabilities.

------------------
Exported Functions
------------------

The :ref:`Exported Functions table <export-func-version-table-2x>` lists all
functions that must be exposed by any |vpl_full_name| implementation.
The realization of all listed functions is mandatory; most functions may return
:cpp:enumerator:`mfxStatus::MFX_ERR_NOT_IMPLEMENTED`.

.. note:: Functions :cpp:func:`MFXInit` and :cpp:func:`MFXInitEx` are not required to be
          exported.

See `Mandatory APIs`_ for details about which functions, in which conditions,
must not return :cpp:enumerator:`mfxStatus::MFX_ERR_NOT_IMPLEMENTED`.

.. _export-func-version-table-2x:

.. list-table:: Exported Functions
   :header-rows: 1
   :widths: 70 30

   * - **Function**
     - **API Version**
   * - :cpp:func:`MFXClose`
     - 1.0
   * - :cpp:func:`MFXQueryIMPL`
     - 1.0
   * - :cpp:func:`MFXQueryVersion`
     - 1.0
   * - :cpp:func:`MFXJoinSession`
     - 1.1
   * - :cpp:func:`MFXDisjoinSession`
     - 1.1
   * - :cpp:func:`MFXCloneSession`
     - 1.1
   * - :cpp:func:`MFXSetPriority`
     - 1.1
   * - :cpp:func:`MFXGetPriority`
     - 1.1
   * - :cpp:func:`MFXVideoCORE_SetFrameAllocator`
     - 1.0
   * - :cpp:func:`MFXVideoCORE_SetHandle`
     - 1.0
   * - :cpp:func:`MFXVideoCORE_GetHandle`
     - 1.0
   * - :cpp:func:`MFXVideoCORE_SyncOperation`
     - 1.0
   * - :cpp:func:`MFXVideoENCODE_Query`
     - 1.0
   * - :cpp:func:`MFXVideoENCODE_QueryIOSurf`
     - 1.0
   * - :cpp:func:`MFXVideoENCODE_Init`
     - 1.0
   * - :cpp:func:`MFXVideoENCODE_Reset`
     - 1.0
   * - :cpp:func:`MFXVideoENCODE_Close`
     - 1.0
   * - :cpp:func:`MFXVideoENCODE_GetVideoParam`
     - 1.0
   * - :cpp:func:`MFXVideoENCODE_GetEncodeStat`
     - 1.0
   * - :cpp:func:`MFXVideoENCODE_EncodeFrameAsync`
     - 1.0
   * - :cpp:func:`MFXVideoDECODE_Query`
     - 1.0
   * - :cpp:func:`MFXVideoDECODE_DecodeHeader`
     - 1.0
   * - :cpp:func:`MFXVideoDECODE_QueryIOSurf`
     - 1.0
   * - :cpp:func:`MFXVideoDECODE_Init`
     - 1.0
   * - :cpp:func:`MFXVideoDECODE_Reset`
     - 1.0
   * - :cpp:func:`MFXVideoDECODE_Close`
     - 1.0
   * - :cpp:func:`MFXVideoDECODE_GetVideoParam`
     - 1.0
   * - :cpp:func:`MFXVideoDECODE_GetDecodeStat`
     - 1.0
   * - :cpp:func:`MFXVideoDECODE_SetSkipMode`
     - 1.0
   * - :cpp:func:`MFXVideoDECODE_GetPayload`
     - 1.0
   * - :cpp:func:`MFXVideoDECODE_DecodeFrameAsync`
     - 1.0
   * - :cpp:func:`MFXVideoVPP_Query`
     - 1.0
   * - :cpp:func:`MFXVideoVPP_QueryIOSurf`
     - 1.0
   * - :cpp:func:`MFXVideoVPP_Init`
     - 1.0
   * - :cpp:func:`MFXVideoVPP_Reset`
     - 1.0
   * - :cpp:func:`MFXVideoVPP_Close`
     - 1.0
   * - :cpp:func:`MFXVideoVPP_GetVideoParam`
     - 1.0
   * - :cpp:func:`MFXVideoVPP_GetVPPStat`
     - 1.0
   * - :cpp:func:`MFXVideoVPP_RunFrameVPPAsync`
     - 1.0
   * - :cpp:func:`MFXVideoCORE_QueryPlatform`
     - 1.19
   * - :cpp:func:`MFXQueryAdapters`
     - 1.31
   * - :cpp:func:`MFXQueryAdaptersDecode`
     - 1.31
   * - :cpp:func:`MFXQueryAdaptersNumber`
     - 1.31
   * - :cpp:func:`MFXMemory_GetSurfaceForVPP`
     - 2.0
   * - :cpp:func:`MFXMemory_GetSurfaceForEncode`
     - 2.0
   * - :cpp:func:`MFXMemory_GetSurfaceForDecode`
     - 2.0
   * - :cpp:func:`MFXQueryImplsDescription`
     - 2.0
   * - :cpp:func:`MFXReleaseImplDescription`
     - 2.0
   * - :cpp:func:`MFXInitialize`
     - 2.0
   * - :cpp:func:`MFXMemory_GetSurfaceForVPPOut`
     - 2.1
   * - :cpp:func:`MFXVideoVPP_ProcessFrameAsync`
     - 2.1
   * - :cpp:func:`MFXVideoDECODE_VPP_Init`
     - 2.1
   * - :cpp:func:`MFXVideoDECODE_VPP_DecodeFrameAsync`
     - 2.1
   * - :cpp:func:`MFXVideoDECODE_VPP_Reset`
     - 2.1
   * - :cpp:func:`MFXVideoDECODE_VPP_GetChannelParam`
     - 2.1
   * - :cpp:func:`MFXVideoDECODE_VPP_Close`
     - 2.1

--------------
Mandatory APIs
--------------

All implementations must implement the APIs listed in the
:ref:`Mandatory APIs table <mandatory-apis-table>`:

.. _mandatory-apis-table:

.. list-table:: Mandatory APIs
   :header-rows: 1
   :widths: 50 50

   * - **Functions**
     - **Description**
   * - | :cpp:func:`MFXInitialize`
       | :cpp:func:`MFXClose`
     - Required functions for the dispatcher to create a session.
   * - | :cpp:func:`MFXQueryImplsDescription`
       | :cpp:func:`MFXReleaseImplDescription`
     - Required functions for the dispatcher to return implementation capabilities.
   * - :cpp:func:`MFXVideoCORE_SyncOperation`
     - Required function for synchronization of asynchronous operations.


If the implementation exposes any encoder, decoder, or VPP filter, it must
implement the corresponding mandatory APIs, as described in the
:ref:`Mandatory Encode <mandatory-enc-apis-table>`, :ref:`Decode <mandatory-dec-apis-table>`,
:ref:`VPP <mandatory-vpp-apis-table>` and :ref:`Decode+VPP <mandatory-dec-vpp-apis-table>` APIs
tables:

.. _mandatory-enc-apis-table:

.. list-table:: Mandatory Encode APIs
   :header-rows: 1
   :widths: 50 50

   * - **Functions**
     - **Description**
   * - | :cpp:func:`MFXVideoENCODE_Init`
       | :cpp:func:`MFXVideoENCODE_Close`
       | :cpp:func:`MFXVideoENCODE_Query`
       | :cpp:func:`MFXVideoENCODE_EncodeFrameAsync`
     - Required functions if the implementation implements any encoder.

.. _mandatory-dec-apis-table:

.. list-table:: Mandatory Decode APIs
   :header-rows: 1
   :widths: 50 50

   * - **Functions**
     - **Description**
   * - | :cpp:func:`MFXVideoDECODE_Init`
       | :cpp:func:`MFXVideoDECODE_Close`
       | :cpp:func:`MFXVideoDECODE_Query`
       | :cpp:func:`MFXVideoDECODE_DecodeFrameAsync`
     - Required functions if the implementation implements any decoder.

.. _mandatory-vpp-apis-table:

.. list-table:: Mandatory VPP APIs
   :header-rows: 1
   :widths: 50 50

   * - **Functions**
     - **Description**
   * - | :cpp:func:`MFXVideoVPP_Init`
       | :cpp:func:`MFXVideoVPP_Close`
       | :cpp:func:`MFXVideoVPP_Query`
       | :cpp:func:`MFXVideoVPP_RunFrameVPPAsync` or :cpp:func:`MFXVideoVPP_ProcessFrameAsync`
     - Required functions if the implementation implements any VPP filter.

.. _mandatory-dec-vpp-apis-table:

.. list-table:: Mandatory Decode+VPP APIs
   :header-rows: 1
   :widths: 50 50

   * - **Functions**
     - **Description**
   * - | :cpp:func:`MFXVideoDECODE_VPP_Init`
       | :cpp:func:`MFXVideoDECODE_VPP_DecodeFrameAsync`
       | :cpp:func:`MFXVideoDECODE_VPP_Close`
     - Required functions if the implementation implements any Decode+VPP component.

.. note:: Mandatory functions must not return the
          :cpp:enumerator:`MFX_ERR_NOT_IMPLEMENTED` status.

If at least one of the encoder, decoder, or VPP filter functions is implemented,
the :cpp:func:`MFXQueryImplsDescription` function must return a valid
:cpp:struct:`mfxImplDescription` structure instance with mandatory capabilities
of the implementation, including decoder, encoder, or VPP capabilities information.

If the implementation supports internal memory allocation by exposing at least one of the
function from that family: :ref:`internal memory allocation and management API <func_memory>` then
implementation of whole scope of the :cpp:struct:`mfxFrameSurfaceInterface` structure as a part of
the :cpp:struct:`mfxFrameSurface1` is mandatory.

Any other functions or extension buffers are optional for the implementation.


