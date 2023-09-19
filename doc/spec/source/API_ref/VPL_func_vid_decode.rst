.. SPDX-FileCopyrightText: 2019-2020 Intel Corporation
..
.. SPDX-License-Identifier: CC-BY-4.0

.. _func_video_decode:

===========
VideoDECODE
===========

.. _func_vid_decode_begin:

Functions that implement a complete decoder that decompresses input bitstreams
directly to output frame surfaces.

.. _func_vid_decode_end:

---
API
---

.. contents::
   :local:
   :depth: 1

MFXVideoDECODE_Query
--------------------

.. doxygenfunction:: MFXVideoDECODE_Query
   :project: oneVPL

.. important:: The :cpp:func:`MFXVideoDECODE_Query` is mandatory when implementing a decoder.

MFXVideoDECODE_DecodeHeader
---------------------------

.. doxygenfunction:: MFXVideoDECODE_DecodeHeader
   :project: oneVPL

MFXVideoDECODE_QueryIOSurf
--------------------------

.. doxygenfunction:: MFXVideoDECODE_QueryIOSurf
   :project: oneVPL

MFXVideoDECODE_Init
-------------------

.. doxygenfunction:: MFXVideoDECODE_Init
   :project: oneVPL

.. important:: The :cpp:func:`MFXVideoDECODE_Init` is mandatory when implementing a decoder.

MFXVideoDECODE_Reset
--------------------

.. doxygenfunction:: MFXVideoDECODE_Reset
   :project: oneVPL

MFXVideoDECODE_Close
--------------------

.. doxygenfunction:: MFXVideoDECODE_Close
   :project: oneVPL

.. important:: The :cpp:func:`MFXVideoDECODE_Close` is mandatory when implementing a decoder.

MFXVideoDECODE_GetVideoParam
----------------------------

.. doxygenfunction:: MFXVideoDECODE_GetVideoParam
   :project: oneVPL

MFXVideoDECODE_GetDecodeStat
----------------------------

.. doxygenfunction:: MFXVideoDECODE_GetDecodeStat
   :project: oneVPL

MFXVideoDECODE_SetSkipMode
--------------------------

.. doxygenfunction:: MFXVideoDECODE_SetSkipMode
   :project: oneVPL

MFXVideoDECODE_GetPayload
-------------------------

.. doxygenfunction:: MFXVideoDECODE_GetPayload
   :project: oneVPL

MFXVideoDECODE_DecodeFrameAsync
-------------------------------

.. doxygenfunction:: MFXVideoDECODE_DecodeFrameAsync
   :project: oneVPL

.. important:: The :cpp:func:`MFXVideoDECODE_DecodeFrameAsync` is mandatory when implementing a decoder.
