.. SPDX-FileCopyrightText: 2019-2020 Intel Corporation
..
.. SPDX-License-Identifier: CC-BY-4.0
..
  Intel(r) Video Processing Library (Intel(r) VPL)

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
   :project: DEF_BREATHE_PROJECT

.. important:: The :cpp:func:`MFXQueryImplsDescription` function is mandatory for any implementation.

MFXQueryImplsProperties
------------------------

.. doxygenfunction:: MFXQueryImplsProperties
   :project: DEF_BREATHE_PROJECT

MFXReleaseImplDescription
-------------------------

.. doxygenfunction:: MFXReleaseImplDescription
   :project: DEF_BREATHE_PROJECT

.. important:: The :cpp:func:`MFXReleaseImplDescription` function is mandatory for any implementation.
