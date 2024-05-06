.. SPDX-FileCopyrightText: 2019-2020 Intel Corporation
..
.. SPDX-License-Identifier: CC-BY-4.0
..
  Intel(r) Video Processing Library (Intel(r) VPL)

.. _struct_vpp:

==============
VPP Structures
==============

.. _struct_vpp_begin:

Structures used by VPP only.

.. _struct_vpp_end:

---
API
---

.. contents::
   :local:
   :depth: 1

mfxExtColorConversion
---------------------

.. doxygenstruct:: mfxExtColorConversion
   :project: DEF_BREATHE_PROJECT
   :members:
   :protected-members:

ChromaSiting is applied on input or output surface depending on the scenario:

+-------------------------+-------------------------+--------------------------------------+
| VPP Input               | VPP Output              | ChromaSiting Indicates               |
+=========================+=========================+======================================+
| MFX_CHROMAFORMAT_YUV420 | MFX_CHROMAFORMAT_YUV444 | Chroma location for input            |
|                         |                         |                                      |
| MFX_CHROMAFORMAT_YUV422 |                         |                                      |
+-------------------------+-------------------------+--------------------------------------+
| MFX_CHROMAFORMAT_YUV444 | MFX_CHROMAFORMAT_YUV420 | Chroma location for output           |
|                         |                         |                                      |
|                         | MFX_CHROMAFORMAT_YUV422 |                                      |
+-------------------------+-------------------------+--------------------------------------+
| MFX_CHROMAFORMAT_YUV420 | MFX_CHROMAFORMAT_YUV420 | Chroma location for input and output |
+-------------------------+-------------------------+--------------------------------------+
| MFX_CHROMAFORMAT_YUV420 | MFX_CHROMAFORMAT_YUV422 | Horizontal location for input and    |
|                         |                         | output, vertical location for input  |
+-------------------------+-------------------------+--------------------------------------+

mfxExtDecVideoProcessing
------------------------

.. doxygenstruct:: mfxExtDecVideoProcessing
   :project: DEF_BREATHE_PROJECT
   :members:
   :protected-members:

mfxExtEncodedSlicesInfo
-----------------------

.. doxygenstruct:: mfxExtEncodedSlicesInfo
   :project: DEF_BREATHE_PROJECT
   :members:
   :protected-members:

mfxExtVppAuxData
----------------

.. doxygenstruct:: mfxExtVppAuxData
   :project: DEF_BREATHE_PROJECT
   :members:
   :protected-members:

mfxExtVPPColorFill
------------------

.. doxygenstruct:: mfxExtVPPColorFill
   :project: DEF_BREATHE_PROJECT
   :members:
   :protected-members:

mfxExtVPPComposite
------------------

.. doxygenstruct:: mfxExtVPPComposite
   :project: DEF_BREATHE_PROJECT
   :members:
   :protected-members:

mfxExtVPPDeinterlacing
----------------------

.. doxygenstruct:: mfxExtVPPDeinterlacing
   :project: DEF_BREATHE_PROJECT
   :members:
   :protected-members:
   :undoc-members:

mfxExtVPPDenoise
----------------

.. doxygenstruct:: mfxExtVPPDenoise
   :project: DEF_BREATHE_PROJECT
   :members:
   :protected-members:
   :undoc-members:

mfxExtVPPDenoise2
-----------------

.. doxygenstruct:: mfxExtVPPDenoise2
   :project: DEF_BREATHE_PROJECT
   :members:
   :protected-members:
   :undoc-members:

mfxExtVPPDetail
---------------

.. doxygenstruct:: mfxExtVPPDetail
   :project: DEF_BREATHE_PROJECT
   :members:
   :protected-members:
   :undoc-members:

mfxExtVPPDoNotUse
-----------------

.. doxygenstruct:: mfxExtVPPDoNotUse
   :project: DEF_BREATHE_PROJECT
   :members:
   :protected-members:
   :undoc-members:

mfxExtVPPDoUse
--------------

.. doxygenstruct:: mfxExtVPPDoUse
   :project: DEF_BREATHE_PROJECT
   :members:
   :protected-members:
   :undoc-members:

mfxExtVPPFieldProcessing
------------------------

.. doxygenstruct:: mfxExtVPPFieldProcessing
   :project: DEF_BREATHE_PROJECT
   :members:
   :protected-members:

mfxExtVPPFrameRateConversion
----------------------------

.. doxygenstruct:: mfxExtVPPFrameRateConversion
   :project: DEF_BREATHE_PROJECT
   :members:
   :protected-members:

mfxExtVPPImageStab
------------------

.. doxygenstruct:: mfxExtVPPImageStab
   :project: DEF_BREATHE_PROJECT
   :members:
   :protected-members:

mfxExtVppMctf
-------------

.. doxygenstruct:: mfxExtVppMctf
   :project: DEF_BREATHE_PROJECT
   :members:
   :protected-members:

mfxExtVPPMirroring
------------------

.. doxygenstruct:: mfxExtVPPMirroring
   :project: DEF_BREATHE_PROJECT
   :members:
   :protected-members:

mfxExtVPPProcAmp
----------------

.. doxygenstruct:: mfxExtVPPProcAmp
   :project: DEF_BREATHE_PROJECT
   :members:
   :protected-members:
   :undoc-members:

mfxExtVPPRotation
-----------------

.. doxygenstruct:: mfxExtVPPRotation
   :project: DEF_BREATHE_PROJECT
   :members:
   :protected-members:

mfxExtVPPScaling
----------------

.. doxygenstruct:: mfxExtVPPScaling
   :project: DEF_BREATHE_PROJECT
   :members:
   :protected-members:

mfxChannel
----------

.. doxygenstruct:: mfxChannel
   :project: DEF_BREATHE_PROJECT
   :members:
   :protected-members:

mfx3DLutSystemBuffer
--------------------

.. doxygenstruct:: mfx3DLutSystemBuffer
   :project: DEF_BREATHE_PROJECT
   :members:
   :protected-members:

mfx3DLutVideoBuffer
-------------------

.. doxygenstruct:: mfx3DLutVideoBuffer
   :project: DEF_BREATHE_PROJECT
   :members:
   :protected-members:

mfxExtVPP3DLut
--------------

.. doxygenstruct:: mfxExtVPP3DLut
   :project: DEF_BREATHE_PROJECT
   :members:
   :protected-members:

mfxExtVPPVideoSignalInfo
------------------------

.. doxygenstruct:: mfxExtVPPVideoSignalInfo
   :project: DEF_BREATHE_PROJECT
   :members:
   :protected-members:

mfxInfoVPP
----------

.. doxygenstruct:: mfxInfoVPP
   :project: DEF_BREATHE_PROJECT
   :members:
   :protected-members:

mfxVPPCompInputStream
---------------------

.. doxygenstruct:: mfxVPPCompInputStream
   :project: DEF_BREATHE_PROJECT
   :members:
   :protected-members:

mfxVPPStat
----------

.. doxygenstruct:: mfxVPPStat
   :project: DEF_BREATHE_PROJECT
   :members:
   :protected-members:

mfxExtVPPPercEncPrefilter
-------------------------

.. doxygenstruct:: mfxExtVPPPercEncPrefilter
   :project: DEF_BREATHE_PROJECT
   :members:
   :protected-members:

mfxExtVPPAISuperResolution
--------------------------

.. doxygenstruct:: mfxExtVPPAISuperResolution
   :project: DEF_BREATHE_PROJECT
   :members:
   :protected-members:

mfxExtVPPAIFrameInterpolation
-----------------------------

.. doxygenstruct:: mfxExtVPPAIFrameInterpolation
   :project: DEF_BREATHE_PROJECT
   :members:
   :protected-members:
