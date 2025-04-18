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
   :project: DEF_BREATHE_PROJECT

MFXInitEx
---------

.. doxygenfunction:: MFXInitEx
   :project: DEF_BREATHE_PROJECT

MFXInitialize
-------------

.. doxygenfunction:: MFXInitialize
   :project: DEF_BREATHE_PROJECT

.. important:: The :cpp:func:`MFXInitialize` function is mandatory for any implementation.

MFXClose
--------

.. doxygenfunction:: MFXClose
   :project: DEF_BREATHE_PROJECT

.. important:: The :cpp:func:`MFXClose` function is mandatory for any implementation.

MFXQueryIMPL
------------
.. doxygenfunction:: MFXQueryIMPL
   :project: DEF_BREATHE_PROJECT

MFXQueryVersion
---------------

.. doxygenfunction:: MFXQueryVersion
   :project: DEF_BREATHE_PROJECT

MFXJoinSession
--------------

.. doxygenfunction:: MFXJoinSession
   :project: DEF_BREATHE_PROJECT

MFXDisjoinSession
-----------------

.. doxygenfunction:: MFXDisjoinSession
   :project: DEF_BREATHE_PROJECT

MFXCloneSession
---------------

.. doxygenfunction:: MFXCloneSession
   :project: DEF_BREATHE_PROJECT

MFXSetPriority
--------------

.. doxygenfunction:: MFXSetPriority
   :project: DEF_BREATHE_PROJECT

MFXGetPriority
--------------

.. doxygenfunction:: MFXGetPriority
   :project: DEF_BREATHE_PROJECT
