.. SPDX-FileCopyrightText: 2019-2020 Intel Corporation
..
.. SPDX-License-Identifier: CC-BY-4.0
..
  Intel(r) Video Processing Library (Intel(r) VPL)

.. _func_video_vpp:

========
VideoVPP
========

.. _func_vid_vpp_begin:

Functions that perform video processing before encoding, rendering, or standalone.

.. _func_vid_vpp_end:

---
API
---

.. contents::
   :local:
   :depth: 1

MFXVideoVPP_Query
-----------------

.. doxygenfunction:: MFXVideoVPP_Query
   :project: DEF_BREATHE_PROJECT

.. important:: The :cpp:func:`MFXVideoVPP_Query` function is mandatory when implementing a VPP filter.

MFXVideoVPP_QueryIOSurf
-----------------------

.. doxygenfunction:: MFXVideoVPP_QueryIOSurf
   :project: DEF_BREATHE_PROJECT

MFXVideoVPP_Init
----------------

.. doxygenfunction:: MFXVideoVPP_Init
   :project: DEF_BREATHE_PROJECT

.. important:: The :cpp:func:`MFXVideoVPP_Init` function is mandatory when implementing a VPP filter.

MFXVideoVPP_Reset
-----------------

.. doxygenfunction:: MFXVideoVPP_Reset
   :project: DEF_BREATHE_PROJECT

MFXVideoVPP_Close
-----------------

.. doxygenfunction:: MFXVideoVPP_Close
   :project: DEF_BREATHE_PROJECT

.. important:: The :cpp:func:`MFXVideoVPP_Close` function is mandatory when implementing a VPP filter.

MFXVideoVPP_GetVideoParam
-------------------------

.. doxygenfunction:: MFXVideoVPP_GetVideoParam
   :project: DEF_BREATHE_PROJECT

MFXVideoVPP_GetVPPStat
----------------------

.. doxygenfunction:: MFXVideoVPP_GetVPPStat
   :project: DEF_BREATHE_PROJECT

MFXVideoVPP_RunFrameVPPAsync
----------------------------

.. doxygenfunction:: MFXVideoVPP_RunFrameVPPAsync
   :project: DEF_BREATHE_PROJECT

MFXVideoVPP_ProcessFrameAsync
-----------------------------

.. doxygenfunction:: MFXVideoVPP_ProcessFrameAsync
   :project: DEF_BREATHE_PROJECT

.. important:: Either :cpp:func:`MFXVideoVPP_RunFrameVPPAsync` or  :cpp:func:`MFXVideoVPP_ProcessFrameAsync` function is                  mandatory when implementing a VPP filter.
