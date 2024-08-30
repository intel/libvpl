.. SPDX-FileCopyrightText: 2019-2020 Intel Corporation
..
.. SPDX-License-Identifier: CC-BY-4.0
..
  Intel(r) Video Processing Library (Intel(r) VPL)

=================
Programming Guide
=================

This chapter describes the concepts used in programming with |vpl_short_name|.

The application must use the include file :file:`mfx.h` for C/C++ programming
and link the |vpl_short_name| dispatcher library :file:`libvpl.so`.

Include these files:

.. code-block:: c++

   #include "mfx.h"    /* Intel® VPL include file */

Link this library:

.. code-block:: c++

   libvpl.so                /* Intel® VPL dynamic dispatcher library (Linux\*) */

.. toctree::
   :hidden:

   VPL_prg_stc
   VPL_prg_session
   VPL_prg_frame
   VPL_prg_decoding
   VPL_prg_encoding
   VPL_prg_vpp
   VPL_prg_transcoding
   VPL_prg_config
   VPL_prg_hw
   VPL_prg_mem
   VPL_prg_surface_sharing
   VPL_prg_err
