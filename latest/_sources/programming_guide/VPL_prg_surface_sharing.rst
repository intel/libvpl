.. SPDX-FileCopyrightText: 2019-2020 Intel Corporation
..
.. SPDX-License-Identifier: CC-BY-4.0

.. _surface-sharing:

=======================================
Importing and Exporting Shared Surfaces
=======================================

|vpl_short_name| API 2.10 introduces new interfaces for sharing surfaces between
|vpl_short_name| runtime and other applications, frameworks, and graphics
APIs. This functionality is only supported when using the :ref:`internal memory
management<internal-mem-manage>` model.

**Importing** a surface enables |vpl_short_name| to access raw video data as input to encode or VPP
operations without first mapping the data to system memory and then copying it to 
a surface allocated by |vpl_short_name| runtime using :cpp:member:`mfxFrameSurfaceInterface::Map`.

**Exporting** a surface similarly enables the application to access raw video
data which is the output of decode or VPP operations and which was allocated by
|vpl_short_name| runtime, without first mapping to system memory using
:cpp:member:`mfxFrameSurfaceInterface::Map`.

--------------
Import Example
--------------

The following code snippet shows an example of importing a shared surface.

.. literalinclude:: ../snippets/prg_surface_sharing.cpp
   :language: c++
   :start-after: /*beg1*/
   :end-before: /*end1*/
   :lineno-start: 1

--------------
Export Example
--------------

The following code snippet shows an example of exporting a shared surface.

.. literalinclude:: ../snippets/prg_surface_sharing.cpp
   :language: c++
   :start-after: /*beg2*/
   :end-before: /*end2*/
   :lineno-start: 1

