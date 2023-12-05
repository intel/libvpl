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
   :project: Intel&reg; VPL
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
   :project: Intel&reg; VPL
   :members:
   :protected-members:

mfxExtEncodedSlicesInfo
-----------------------

.. doxygenstruct:: mfxExtEncodedSlicesInfo
   :project: Intel&reg; VPL
   :members:
   :protected-members:

mfxExtVppAuxData
----------------

.. doxygenstruct:: mfxExtVppAuxData
   :project: Intel&reg; VPL
   :members:
   :protected-members:

mfxExtVPPColorFill
------------------

.. doxygenstruct:: mfxExtVPPColorFill
   :project: Intel&reg; VPL
   :members:
   :protected-members:

mfxExtVPPComposite
------------------

.. doxygenstruct:: mfxExtVPPComposite
   :project: Intel&reg; VPL
   :members:
   :protected-members:

mfxExtVPPDeinterlacing
----------------------

.. doxygenstruct:: mfxExtVPPDeinterlacing
   :project: Intel&reg; VPL
   :members:
   :protected-members:
   :undoc-members:

mfxExtVPPDenoise
----------------

.. doxygenstruct:: mfxExtVPPDenoise
   :project: Intel&reg; VPL
   :members:
   :protected-members:
   :undoc-members:

mfxExtVPPDenoise2
-----------------

.. doxygenstruct:: mfxExtVPPDenoise2
   :project: Intel&reg; VPL
   :members:
   :protected-members:
   :undoc-members:

mfxExtVPPDetail
---------------

.. doxygenstruct:: mfxExtVPPDetail
   :project: Intel&reg; VPL
   :members:
   :protected-members:
   :undoc-members:

mfxExtVPPDoNotUse
-----------------

.. doxygenstruct:: mfxExtVPPDoNotUse
   :project: Intel&reg; VPL
   :members:
   :protected-members:
   :undoc-members:

mfxExtVPPDoUse
--------------

.. doxygenstruct:: mfxExtVPPDoUse
   :project: Intel&reg; VPL
   :members:
   :protected-members:
   :undoc-members:

mfxExtVPPFieldProcessing
------------------------

.. doxygenstruct:: mfxExtVPPFieldProcessing
   :project: Intel&reg; VPL
   :members:
   :protected-members:

mfxExtVPPFrameRateConversion
----------------------------

.. doxygenstruct:: mfxExtVPPFrameRateConversion
   :project: Intel&reg; VPL
   :members:
   :protected-members:

mfxExtVPPImageStab
------------------

.. doxygenstruct:: mfxExtVPPImageStab
   :project: Intel&reg; VPL
   :members:
   :protected-members:

mfxExtVppMctf
-------------

.. doxygenstruct:: mfxExtVppMctf
   :project: Intel&reg; VPL
   :members:
   :protected-members:

mfxExtVPPMirroring
------------------

.. doxygenstruct:: mfxExtVPPMirroring
   :project: Intel&reg; VPL
   :members:
   :protected-members:

mfxExtVPPProcAmp
----------------

.. doxygenstruct:: mfxExtVPPProcAmp
   :project: Intel&reg; VPL
   :members:
   :protected-members:
   :undoc-members:

mfxExtVPPRotation
-----------------

.. doxygenstruct:: mfxExtVPPRotation
   :project: Intel&reg; VPL
   :members:
   :protected-members:

mfxExtVPPScaling
----------------

.. doxygenstruct:: mfxExtVPPScaling
   :project: Intel&reg; VPL
   :members:
   :protected-members:

mfxChannel
----------

.. doxygenstruct:: mfxChannel
   :project: Intel&reg; VPL
   :members:
   :protected-members:

mfx3DLutSystemBuffer
--------------------

.. doxygenstruct:: mfx3DLutSystemBuffer
   :project: Intel&reg; VPL
   :members:
   :protected-members:

mfx3DLutVideoBuffer
-------------------

.. doxygenstruct:: mfx3DLutVideoBuffer
   :project: Intel&reg; VPL
   :members:
   :protected-members:

mfxExtVPP3DLut
--------------

.. doxygenstruct:: mfxExtVPP3DLut
   :project: Intel&reg; VPL
   :members:
   :protected-members:

mfxExtVPPVideoSignalInfo
------------------------

.. doxygenstruct:: mfxExtVPPVideoSignalInfo
   :project: Intel&reg; VPL
   :members:
   :protected-members:

mfxInfoVPP
----------

.. doxygenstruct:: mfxInfoVPP
   :project: Intel&reg; VPL
   :members:
   :protected-members:

mfxVPPCompInputStream
---------------------

.. doxygenstruct:: mfxVPPCompInputStream
   :project: Intel&reg; VPL
   :members:
   :protected-members:

mfxVPPStat
----------

.. doxygenstruct:: mfxVPPStat
   :project: Intel&reg; VPL
   :members:
   :protected-members:

mfxExtVPPPercEncPrefilter
-------------------------

.. doxygenstruct:: mfxExtVPPPercEncPrefilter
   :project: Intel&reg; VPL
   :members:
   :protected-members:
