.. SPDX-FileCopyrightText: 2019-2020 Intel Corporation
..
.. SPDX-License-Identifier: CC-BY-4.0

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
   :project: oneVPL

MFXVideoCORE_SetHandle
----------------------

.. doxygenfunction:: MFXVideoCORE_SetHandle
   :project: oneVPL

MFXVideoCORE_GetHandle
----------------------

.. doxygenfunction:: MFXVideoCORE_GetHandle
   :project: oneVPL

MFXVideoCORE_QueryPlatform
--------------------------

.. doxygenfunction:: MFXVideoCORE_QueryPlatform
   :project: oneVPL

MFXVideoCORE_SyncOperation
--------------------------

.. doxygenfunction:: MFXVideoCORE_SyncOperation
   :project: oneVPL

.. important:: The :cpp:func:`MFXVideoCORE_SyncOperation` function is mandatory for
   any implementation.