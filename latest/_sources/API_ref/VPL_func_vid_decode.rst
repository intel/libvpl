.. SPDX-FileCopyrightText: 2019-2020 Intel Corporation
..
.. SPDX-License-Identifier: CC-BY-4.0
..
  Intel(r) Video Processing Library (Intel(r) VPL)

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
   :project: DEF_BREATHE_PROJECT

.. important:: The :cpp:func:`MFXVideoDECODE_Query` is mandatory when implementing a decoder.

MFXVideoDECODE_DecodeHeader
---------------------------

.. doxygenfunction:: MFXVideoDECODE_DecodeHeader
   :project: DEF_BREATHE_PROJECT

MFXVideoDECODE_QueryIOSurf
--------------------------

.. doxygenfunction:: MFXVideoDECODE_QueryIOSurf
   :project: DEF_BREATHE_PROJECT

MFXVideoDECODE_Init
-------------------

.. doxygenfunction:: MFXVideoDECODE_Init
   :project: DEF_BREATHE_PROJECT

.. important:: The :cpp:func:`MFXVideoDECODE_Init` is mandatory when implementing a decoder.

MFXVideoDECODE_Reset
--------------------

.. doxygenfunction:: MFXVideoDECODE_Reset
   :project: DEF_BREATHE_PROJECT

MFXVideoDECODE_Close
--------------------

.. doxygenfunction:: MFXVideoDECODE_Close
   :project: DEF_BREATHE_PROJECT

.. important:: The :cpp:func:`MFXVideoDECODE_Close` is mandatory when implementing a decoder.

MFXVideoDECODE_GetVideoParam
----------------------------

.. doxygenfunction:: MFXVideoDECODE_GetVideoParam
   :project: DEF_BREATHE_PROJECT

MFXVideoDECODE_GetDecodeStat
----------------------------

.. doxygenfunction:: MFXVideoDECODE_GetDecodeStat
   :project: DEF_BREATHE_PROJECT

MFXVideoDECODE_SetSkipMode
--------------------------

.. doxygenfunction:: MFXVideoDECODE_SetSkipMode
   :project: DEF_BREATHE_PROJECT

MFXVideoDECODE_GetPayload
-------------------------

.. doxygenfunction:: MFXVideoDECODE_GetPayload
   :project: DEF_BREATHE_PROJECT

MFXVideoDECODE_DecodeFrameAsync
-------------------------------

.. doxygenfunction:: MFXVideoDECODE_DecodeFrameAsync
   :project: DEF_BREATHE_PROJECT

.. important:: The :cpp:func:`MFXVideoDECODE_DecodeFrameAsync` is mandatory when implementing a decoder.
