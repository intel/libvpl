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
   :project: Intel&reg; VPL

MFXVideoCORE_SetHandle
----------------------

.. doxygenfunction:: MFXVideoCORE_SetHandle
   :project: Intel&reg; VPL

MFXVideoCORE_GetHandle
----------------------

.. doxygenfunction:: MFXVideoCORE_GetHandle
   :project: Intel&reg; VPL

MFXVideoCORE_QueryPlatform
--------------------------

.. doxygenfunction:: MFXVideoCORE_QueryPlatform
   :project: Intel&reg; VPL

MFXVideoCORE_SyncOperation
--------------------------

.. doxygenfunction:: MFXVideoCORE_SyncOperation
   :project: Intel&reg; VPL

.. important:: The :cpp:func:`MFXVideoCORE_SyncOperation` function is mandatory for
   any implementation.
