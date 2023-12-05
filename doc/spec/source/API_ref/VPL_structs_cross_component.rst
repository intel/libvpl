.. SPDX-FileCopyrightText: 2019-2020 Intel Corporation
..
.. SPDX-License-Identifier: CC-BY-4.0
..
  Intel(r) Video Processing Library (Intel(r) VPL)

.. _struct_cross_component:

==========================
Cross-component Structures
==========================

.. _struct_cross_comp_begin:

Structures used across library components.

.. _struct_cross_comp_end:

---
API
---

.. contents::
   :local:
   :depth: 1

mfxComponentInfo
----------------

.. doxygenstruct:: mfxComponentInfo
   :project: Intel&reg; VPL
   :members:
   :protected-members:

mfxExtHEVCParam
---------------

.. doxygenstruct:: mfxExtHEVCParam
   :project: Intel&reg; VPL
   :members:
   :protected-members:

mfxExtJPEGHuffmanTables
-----------------------

.. doxygenstruct:: mfxExtJPEGHuffmanTables
   :project: Intel&reg; VPL
   :members:
   :protected-members:

+------------------+---------+------+
| Table ID         | 0       | 1    |
+------------------+---------+------+
| Number of tables |         |      |
+==================+=========+======+
| 0                | Y, U, V |      |
+------------------+---------+------+
| 1                | Y       | U, V |
+------------------+---------+------+


mfxExtJPEGQuantTables
---------------------

.. doxygenstruct:: mfxExtJPEGQuantTables
   :project: Intel&reg; VPL
   :members:
   :protected-members:

+------------------+---------+------+---+
| Table ID         | 0       | 1    | 2 |
+------------------+---------+------+---+
| Number of tables |         |      |   |
+==================+=========+======+===+
| 0                | Y, U, V |      |   |
+------------------+---------+------+---+
| 1                | Y       | U, V |   |
+------------------+---------+------+---+
| 2                | Y       | U    | V |
+------------------+---------+------+---+



mfxExtMVCSeqDesc
----------------

.. doxygenstruct:: mfxExtMVCSeqDesc
   :project: Intel&reg; VPL
   :members:
   :protected-members:

mfxExtMVCTargetViews
--------------------

.. doxygenstruct:: mfxExtMVCTargetViews
   :project: Intel&reg; VPL
   :members:
   :protected-members:

mfxExtVideoSignalInfo
---------------------

.. doxygenstruct:: mfxExtVideoSignalInfo
   :project: Intel&reg; VPL
   :members:
   :protected-members:
   :undoc-members:

mfxExtVP9Param
--------------

.. doxygenstruct:: mfxExtVP9Param
   :project: Intel&reg; VPL
   :members:
   :protected-members:

mfxFrameId
----------

.. doxygenstruct:: mfxFrameId
   :project: Intel&reg; VPL
   :members:
   :protected-members:

mfxInfoMFX
----------

.. doxygenstruct:: mfxInfoMFX
   :project: Intel&reg; VPL
   :members:
   :protected-members:

.. note::

   The :cpp:member:`mfxInfoMFX::InitialDelayInKB`, :cpp:member:`mfxInfoMFX::TargetKbps`,
   :cpp:member:`mfxInfoMFX::MaxKbps` parameters are used by the constant bitrate
   (CBR), variable bitrate control (VBR), and CQP HRD algorithms.

   Encoders follow the Hypothetical Reference Decoding (HRD) model. The
   HRD model assumes that data flows into a buffer of the fixed size
   BufferSizeInKB with a constant bitrate of TargetKbps. (Estimate the targeted
   frame size by dividing frame rate by bitrate.)

   The decoder starts decoding after the buffer reaches the initial size
   InitialDelayInKB, which is equivalent to reaching an initial delay of
   InitialDelayInKB*8000/TargetKbpsms.
   *In this context, KB is 1000 bytes and Kbps is 1000 bps.*

   If InitialDelayInKB or BufferSizeInKB is equal to zero, the value is
   calculated using bitrate, frame rate, profile, level, and so on.

   TargetKbps must be specified for encoding initialization.

   For variable bitrate control, the MaxKbps parameter specifies the maximum
   bitrate at which the encoded data enters the Video Buffering Verifier (VBV)
   buffer. If MaxKbps is equal to zero, the value is calculated from bitrate,
   frame rate, profile, and level.

.. note::

   The :cpp:member:`mfxInfoMFX::TargetKbps`, :cpp:member:`mfxInfoMFX::Accuracy`,
   :cpp:member:`mfxInfoMFX::Convergence` parameters are used by the average variable
   bitrate control (AVBR) algorithm. The algorithm focuses on overall encoding
   quality while meeting the specified bitrate, TargetKbps, within the accuracy
   range, Accuracy, after a Convergence period. This method does not follow HRD
   and the instant bitrate is not capped or padded.


mfxMVCOperationPoint
--------------------

.. doxygenstruct:: mfxMVCOperationPoint
   :project: Intel&reg; VPL
   :members:
   :protected-members:

mfxMVCViewDependency
--------------------

.. doxygenstruct:: mfxMVCViewDependency
   :project: Intel&reg; VPL
   :members:
   :protected-members:

mfxPayload
----------

.. doxygenstruct:: mfxPayload
   :project: Intel&reg; VPL
   :members:
   :protected-members:

+-----------+-------------------------------------------+
| **Codec** | **Supported Types**                       |
+===========+===========================================+
| MPEG2     | 0x01B2 //User Data                        |
+-----------+-------------------------------------------+
| AVC       | 02 //pan_scan_rect                        |
|           |                                           |
|           | 03 //filler_payload                       |
|           |                                           |
|           | 04 //user_data_registered_itu_t_t35       |
|           |                                           |
|           | 05 //user_data_unregistered               |
|           |                                           |
|           | 06 //recovery_point                       |
|           |                                           |
|           | 09 //scene_info                           |
|           |                                           |
|           | 13 //full_frame_freeze                    |
|           |                                           |
|           | 14 //full_frame_freeze_release            |
|           |                                           |
|           | 15 //full_frame_snapshot                  |
|           |                                           |
|           | 16 //progressive_refinement_segment_start |
|           |                                           |
|           | 17 //progressive_refinement_segment_end   |
|           |                                           |
|           | 19 //film_grain_characteristics           |
|           |                                           |
|           | 20 //deblocking_filter_display_preference |
|           |                                           |
|           | 21 //stereo_video_info                    |
|           |                                           |
|           | 45 //frame_packing_arrangement            |
+-----------+-------------------------------------------+
| HEVC      | All                                       |
+-----------+-------------------------------------------+



mfxVideoParam
-------------

.. doxygenstruct:: mfxVideoParam
   :project: Intel&reg; VPL
   :members:
   :protected-members:

mfxVP9SegmentParam
------------------

.. doxygenstruct:: mfxVP9SegmentParam
   :project: Intel&reg; VPL
   :members:
   :protected-members:

mfxExtAV1FilmGrainParam
-----------------------

.. doxygenstruct:: mfxExtAV1FilmGrainParam
   :project: Intel&reg; VPL
   :members:
   :protected-members:

mfxAV1FilmGrainPoint
--------------------

.. doxygenstruct:: mfxAV1FilmGrainPoint
   :project: Intel&reg; VPL
   :members:
   :protected-members:

mfxRect
-------

.. doxygenstruct:: mfxRect
   :project: Intel&reg; VPL
   :members:
   :protected-members:

mfxExtHyperModeParam
--------------------

.. doxygenstruct:: mfxExtHyperModeParam
   :project: Intel&reg; VPL
   :members:
   :protected-members:

mfxGUID
-------

.. doxygenstruct:: mfxGUID
   :project: Intel&reg; VPL
   :members:
   :protected-members:

mfxExtAllocationHints
---------------------
.. doxygenstruct:: mfxExtAllocationHints
   :project: Intel&reg; VPL
   :members:
   :protected-members:

mfxRefInterface
---------------
.. doxygenstruct:: mfxRefInterface
   :project: Intel&reg; VPL
   :members:
   :protected-members:

mfxExtMasteringDisplayColourVolume
----------------------------------

.. doxygenstruct:: mfxExtMasteringDisplayColourVolume
   :project: Intel&reg; VPL
   :members:
   :protected-members:


mfxExtContentLightLevelInfo
---------------------------

.. doxygenstruct:: mfxExtContentLightLevelInfo
   :project: Intel&reg; VPL
   :members:
   :protected-members:

mfxExtSyncSubmission
--------------------

.. doxygenstruct:: mfxExtSyncSubmission
   :project: Intel&reg; VPL
   :members:
   :protected-members:

mfxExtTuneEncodeQuality
-----------------------

.. doxygenstruct:: mfxExtTuneEncodeQuality
   :project: Intel&reg; VPL
   :members:
   :protected-members:

mfxConfigInterface
------------------

.. doxygenstruct:: mfxConfigInterface
   :project: Intel&reg; VPL
   :members:
   :protected-members:
