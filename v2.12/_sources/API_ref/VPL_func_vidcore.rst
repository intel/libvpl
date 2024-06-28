.. SPDX-FileCopyrightText: 2019-2020 Intel Corporation
..
.. SPDX-License-Identifier: CC-BY-4.0
..
  Intel(r) Video Processing Library (Intel(r) VPL)

.. _func_video_core:

=========
VideoCORE
=========

.. _func_vidcore_begin:

Functions to perform external device and memory management and synchronization.

.. _func_vidcore_end:

---
API
---

.. contents::
   :local:
   :depth: 1

MFXVideoCORE_SetFrameAllocator
------------------------------

.. doxygenfunction:: MFXVideoCORE_SetFrameAllocator
   :project: DEF_BREATHE_PROJECT

MFXVideoCORE_SetHandle
----------------------

.. doxygenfunction:: MFXVideoCORE_SetHandle
   :project: DEF_BREATHE_PROJECT

MFXVideoCORE_GetHandle
----------------------

.. doxygenfunction:: MFXVideoCORE_GetHandle
   :project: DEF_BREATHE_PROJECT

MFXVideoCORE_QueryPlatform
--------------------------

.. doxygenfunction:: MFXVideoCORE_QueryPlatform
   :project: DEF_BREATHE_PROJECT

MFXVideoCORE_SyncOperation
--------------------------

.. doxygenfunction:: MFXVideoCORE_SyncOperation
   :project: DEF_BREATHE_PROJECT

.. important:: The :cpp:func:`MFXVideoCORE_SyncOperation` function is mandatory for
   any implementation.
