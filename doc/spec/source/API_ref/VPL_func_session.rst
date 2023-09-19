.. SPDX-FileCopyrightText: 2019-2020 Intel Corporation
..
.. SPDX-License-Identifier: CC-BY-4.0

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
   :project: oneVPL

MFXInitEx
---------

.. doxygenfunction:: MFXInitEx
   :project: oneVPL

MFXInitialize
-------------

.. doxygenfunction:: MFXInitialize
   :project: oneVPL

.. important:: The :cpp:func:`MFXInitialize` function is mandatory for any implementation.

MFXClose
--------

.. doxygenfunction:: MFXClose
   :project: oneVPL

.. important:: The :cpp:func:`MFXClose` function is mandatory for any implementation.

MFXQueryIMPL
------------
.. doxygenfunction:: MFXQueryIMPL
   :project: oneVPL

MFXQueryVersion
---------------

.. doxygenfunction:: MFXQueryVersion
   :project: oneVPL

MFXJoinSession
--------------

.. doxygenfunction:: MFXJoinSession
   :project: oneVPL

MFXDisjoinSession
-----------------

.. doxygenfunction:: MFXDisjoinSession
   :project: oneVPL

MFXCloneSession
---------------

.. doxygenfunction:: MFXCloneSession
   :project: oneVPL

MFXSetPriority
--------------

.. doxygenfunction:: MFXSetPriority
   :project: oneVPL

MFXGetPriority
--------------

.. doxygenfunction:: MFXGetPriority
   :project: oneVPL