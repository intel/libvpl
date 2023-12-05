.. SPDX-FileCopyrightText: 2019-2020 Intel Corporation
..
.. SPDX-License-Identifier: CC-BY-4.0
..
  Intel(r) Video Processing Library (Intel(r) VPL)

.. _func_video_encode:

===========
VideoENCODE
===========

.. _func_vid_encode_begin:

Functions that perform the entire encoding pipeline from the input video frames
to the output bitstream.

.. _func_vid_encode_end:

---
API
---

.. contents::
   :local:
   :depth: 1

MFXVideoENCODE_Query
--------------------

.. doxygenfunction:: MFXVideoENCODE_Query
   :project: Intel&reg; VPL

.. important:: The :cpp:func:`MFXVideoENCODE_Query` function is mandatory when
   implementing an encoder.

MFXVideoENCODE_QueryIOSurf
--------------------------

.. doxygenfunction:: MFXVideoENCODE_QueryIOSurf
   :project: Intel&reg; VPL

MFXVideoENCODE_Init
-------------------

.. doxygenfunction:: MFXVideoENCODE_Init
   :project: Intel&reg; VPL

.. important:: The :cpp:func:`MFXVideoENCODE_Init` function is mandatory when
               implementing an encoder.

MFXVideoENCODE_Reset
--------------------

.. doxygenfunction:: MFXVideoENCODE_Reset
   :project: Intel&reg; VPL

MFXVideoENCODE_Close
--------------------

.. doxygenfunction:: MFXVideoENCODE_Close
   :project: Intel&reg; VPL

.. important:: The :cpp:func:`MFXVideoENCODE_Close` function is mandatory when
               implementing an encoder.

MFXVideoENCODE_GetVideoParam
----------------------------

.. doxygenfunction:: MFXVideoENCODE_GetVideoParam
   :project: Intel&reg; VPL

MFXVideoENCODE_GetEncodeStat
----------------------------

.. doxygenfunction:: MFXVideoENCODE_GetEncodeStat
   :project: Intel&reg; VPL

MFXVideoENCODE_EncodeFrameAsync
-------------------------------

.. doxygenfunction:: MFXVideoENCODE_EncodeFrameAsync
   :project: Intel&reg; VPL

.. important:: The :cpp:func:`MFXVideoENCODE_EncodeFrameAsync` function is mandatory when
               implementing an encoder.
