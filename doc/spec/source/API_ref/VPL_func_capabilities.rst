.. SPDX-FileCopyrightText: 2019-2020 Intel Corporation
..
.. SPDX-License-Identifier: CC-BY-4.0

.. _func_impl_capabilities:

===========================
Implementation Capabilities
===========================

.. _func_impl_cap_begin:

Functions to report capabilities of available implementations and create
user-requested library implementations.

.. _func_impl_cap_end:

---
API
---

.. contents::
   :local:
   :depth: 1

MFXQueryImplsDescription
------------------------

.. doxygenfunction:: MFXQueryImplsDescription
   :project: oneVPL

.. important:: The :cpp:func:`MFXQueryImplsDescription` function is mandatory for any implementation.

MFXReleaseImplDescription
-------------------------

.. doxygenfunction:: MFXReleaseImplDescription
   :project: oneVPL

.. important:: The :cpp:func:`MFXReleaseImplDescription` function is mandatory for any implementation.