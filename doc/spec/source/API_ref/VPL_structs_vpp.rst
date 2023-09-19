.. SPDX-FileCopyrightText: 2019-2020 Intel Corporation
..
.. SPDX-License-Identifier: CC-BY-4.0

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
   :project: oneVPL
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
   :project: oneVPL
   :members:
   :protected-members:

mfxExtEncodedSlicesInfo
-----------------------

.. doxygenstruct:: mfxExtEncodedSlicesInfo
   :project: oneVPL
   :members:
   :protected-members:

mfxExtVppAuxData
----------------

.. doxygenstruct:: mfxExtVppAuxData
   :project: oneVPL
   :members:
   :protected-members:

mfxExtVPPColorFill
------------------

.. doxygenstruct:: mfxExtVPPColorFill
   :project: oneVPL
   :members:
   :protected-members:

mfxExtVPPComposite
------------------

.. doxygenstruct:: mfxExtVPPComposite
   :project: oneVPL
   :members:
   :protected-members:

mfxExtVPPDeinterlacing
----------------------

.. doxygenstruct:: mfxExtVPPDeinterlacing
   :project: oneVPL
   :members:
   :protected-members:
   :undoc-members:

mfxExtVPPDenoise
----------------

.. doxygenstruct:: mfxExtVPPDenoise
   :project: oneVPL
   :members:
   :protected-members:
   :undoc-members:

mfxExtVPPDenoise2
-----------------

.. doxygenstruct:: mfxExtVPPDenoise2
   :project: oneVPL
   :members:
   :protected-members:
   :undoc-members:

mfxExtVPPDetail
---------------

.. doxygenstruct:: mfxExtVPPDetail
   :project: oneVPL
   :members:
   :protected-members:
   :undoc-members:

mfxExtVPPDoNotUse
-----------------

.. doxygenstruct:: mfxExtVPPDoNotUse
   :project: oneVPL
   :members:
   :protected-members:
   :undoc-members:

mfxExtVPPDoUse
--------------

.. doxygenstruct:: mfxExtVPPDoUse
   :project: oneVPL
   :members:
   :protected-members:
   :undoc-members:

mfxExtVPPFieldProcessing
------------------------

.. doxygenstruct:: mfxExtVPPFieldProcessing
   :project: oneVPL
   :members:
   :protected-members:

mfxExtVPPFrameRateConversion
----------------------------

.. doxygenstruct:: mfxExtVPPFrameRateConversion
   :project: oneVPL
   :members:
   :protected-members:

mfxExtVPPImageStab
------------------

.. doxygenstruct:: mfxExtVPPImageStab
   :project: oneVPL
   :members:
   :protected-members:

mfxExtVppMctf
-------------

.. doxygenstruct:: mfxExtVppMctf
   :project: oneVPL
   :members:
   :protected-members:

mfxExtVPPMirroring
------------------

.. doxygenstruct:: mfxExtVPPMirroring
   :project: oneVPL
   :members:
   :protected-members:

mfxExtVPPProcAmp
----------------

.. doxygenstruct:: mfxExtVPPProcAmp
   :project: oneVPL
   :members:
   :protected-members:
   :undoc-members:

mfxExtVPPRotation
-----------------

.. doxygenstruct:: mfxExtVPPRotation
   :project: oneVPL
   :members:
   :protected-members:

mfxExtVPPScaling
----------------

.. doxygenstruct:: mfxExtVPPScaling
   :project: oneVPL
   :members:
   :protected-members:

mfxChannel
----------

.. doxygenstruct:: mfxChannel
   :project: oneVPL
   :members:
   :protected-members:

mfx3DLutSystemBuffer
--------------------

.. doxygenstruct:: mfx3DLutSystemBuffer
   :project: oneVPL
   :members:
   :protected-members:

mfx3DLutVideoBuffer
-------------------

.. doxygenstruct:: mfx3DLutVideoBuffer
   :project: oneVPL
   :members:
   :protected-members:

mfxExtVPP3DLut
--------------

.. doxygenstruct:: mfxExtVPP3DLut
   :project: oneVPL
   :members:
   :protected-members:

mfxExtVPPVideoSignalInfo
------------------------

.. doxygenstruct:: mfxExtVPPVideoSignalInfo
   :project: oneVPL
   :members:
   :protected-members:

mfxInfoVPP
----------

.. doxygenstruct:: mfxInfoVPP
   :project: oneVPL
   :members:
   :protected-members:

mfxVPPCompInputStream
---------------------

.. doxygenstruct:: mfxVPPCompInputStream
   :project: oneVPL
   :members:
   :protected-members:

mfxVPPStat
----------

.. doxygenstruct:: mfxVPPStat
   :project: oneVPL
   :members:
   :protected-members:

mfxExtVPPPercEncPrefilter
-------------------------

.. doxygenstruct:: mfxExtVPPPercEncPrefilter
   :project: oneVPL
   :members:
   :protected-members:
