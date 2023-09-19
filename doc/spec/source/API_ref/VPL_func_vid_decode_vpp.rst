.. SPDX-FileCopyrightText: 2019-2020 Intel Corporation
..
.. SPDX-License-Identifier: CC-BY-4.0


.. _func_video_decode_vpp:

===============
VideoDECODE_VPP
===============

.. _func_vid_decode_vpp_begin:

Functions that implement combined operation of decoding and video processing 
with multiple output frame surfaces. 

.. _func_vid_decode_vpp_end:

---
API
---

.. contents::
   :local:
   :depth: 1

MFXVideoDECODE_VPP_Init
-----------------------

.. doxygenfunction:: MFXVideoDECODE_VPP_Init
   :project: oneVPL

.. important:: The :cpp:func:`MFXVideoDECODE_VPP_Init` is mandatory when implementing a combined decode plus vpp.

MFXVideoDECODE_VPP_Reset
------------------------

.. doxygenfunction:: MFXVideoDECODE_VPP_Reset
   :project: oneVPL

MFXVideoDECODE_VPP_GetChannelParam
----------------------------------

.. doxygenfunction:: MFXVideoDECODE_VPP_GetChannelParam
   :project: oneVPL

MFXVideoDECODE_VPP_DecodeFrameAsync
-----------------------------------

.. doxygenfunction:: MFXVideoDECODE_VPP_DecodeFrameAsync
   :project: oneVPL

.. important:: The :cpp:func:`MFXVideoDECODE_VPP_DecodeFrameAsync` is mandatory when implementing a combined decode plus vpp.

MFXVideoDECODE_VPP_Close
------------------------

.. doxygenfunction:: MFXVideoDECODE_VPP_Close
   :project: oneVPL

.. important:: The :cpp:func:`MFXVideoDECODE_VPP_Close` is mandatory when implementing a combined decode plus vpp.

