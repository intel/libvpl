.. SPDX-FileCopyrightText: 2019-2020 Intel Corporation
..
.. SPDX-License-Identifier: CC-BY-4.0
..
  Intel(r) Video Processing Library (Intel(r) VPL)

.. _func_session_mgmt:

==================
Session Management
==================

.. _func_session_begin:

Functions to manage sessions.

.. _func_session_end:

---
API
---

.. contents::
   :local:
   :depth: 1

MFXInit
-------

.. doxygenfunction:: MFXInit
   :project: Intel&reg; VPL

MFXInitEx
---------

.. doxygenfunction:: MFXInitEx
   :project: Intel&reg; VPL

MFXInitialize
-------------

.. doxygenfunction:: MFXInitialize
   :project: Intel&reg; VPL

.. important:: The :cpp:func:`MFXInitialize` function is mandatory for any implementation.

MFXClose
--------

.. doxygenfunction:: MFXClose
   :project: Intel&reg; VPL

.. important:: The :cpp:func:`MFXClose` function is mandatory for any implementation.

MFXQueryIMPL
------------
.. doxygenfunction:: MFXQueryIMPL
   :project: Intel&reg; VPL

MFXQueryVersion
---------------

.. doxygenfunction:: MFXQueryVersion
   :project: Intel&reg; VPL

MFXJoinSession
--------------

.. doxygenfunction:: MFXJoinSession
   :project: Intel&reg; VPL

MFXDisjoinSession
-----------------

.. doxygenfunction:: MFXDisjoinSession
   :project: Intel&reg; VPL

MFXCloneSession
---------------

.. doxygenfunction:: MFXCloneSession
   :project: Intel&reg; VPL

MFXSetPriority
--------------

.. doxygenfunction:: MFXSetPriority
   :project: Intel&reg; VPL

MFXGetPriority
--------------

.. doxygenfunction:: MFXGetPriority
   :project: Intel&reg; VPL
