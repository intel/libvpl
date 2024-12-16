.. SPDX-FileCopyrightText: 2019-2020 Intel Corporation
..
.. SPDX-License-Identifier: CC-BY-4.0
..
  Intel(r) Video Processing Library (Intel(r) VPL)

.. _struct_memory:

=================
Memory Structures
=================

.. _struct_memory_begin:

Structures used for memory.

.. _struct_memory_end:

---
API
---

.. contents::
   :local:
   :depth: 1

mfxBitstream
------------

.. doxygenstruct:: mfxBitstream
   :project: DEF_BREATHE_PROJECT
   :members:
   :protected-members:

mfxFrameAllocator
-----------------

.. doxygenstruct:: mfxFrameAllocator
   :project: DEF_BREATHE_PROJECT
   :members:
   :protected-members:

mfxFrameAllocRequest
--------------------

.. doxygenstruct:: mfxFrameAllocRequest
   :project: DEF_BREATHE_PROJECT
   :members:
   :protected-members:

mfxFrameAllocResponse
---------------------

.. doxygenstruct:: mfxFrameAllocResponse
   :project: DEF_BREATHE_PROJECT
   :members:
   :protected-members:

mfxFrameData
------------

.. doxygenstruct:: mfxY410
   :project: DEF_BREATHE_PROJECT
   :members:
   :protected-members:

.. doxygenstruct:: mfxY416
   :project: DEF_BREATHE_PROJECT
   :members:
   :protected-members:

.. doxygenstruct:: mfxA2RGB10
   :project: DEF_BREATHE_PROJECT
   :members:
   :protected-members:

.. doxygenstruct:: mfxFrameData
   :project: DEF_BREATHE_PROJECT
   :members:
   :protected-members:

mfxFrameInfo
------------

.. doxygenstruct:: mfxFrameInfo
   :project: DEF_BREATHE_PROJECT
   :members:
   :protected-members:

.. note::

   Example data alignment for Shift = 0:

   .. graphviz::

     digraph {
         abc [shape=none, margin=0, label=<
         <TABLE BORDER="0" CELLBORDER="1" CELLSPACING="0" CELLPADDING="4">
          <TR><TD>Bit</TD><TD>15</TD><TD>14</TD><TD>13</TD><TD>12</TD><TD>11</TD><TD>10</TD><TD>9</TD><TD>8</TD>
              <TD>7</TD><TD>6</TD><TD>5</TD><TD>4</TD><TD>3</TD><TD>2</TD><TD>1</TD><TD>0</TD>
          </TR>
          <TR><TD>Value</TD><TD>0</TD><TD>0</TD><TD>0</TD><TD>0</TD><TD>0</TD><TD>0</TD><TD COLSPAN="10">Valid data</TD>
          </TR>
            </TABLE>>];
     }

   |

   Example data alignment for Shift != 0:

   .. graphviz::

     digraph {
         abc [shape=none, margin=0, label=<
         <TABLE BORDER="0" CELLBORDER="1" CELLSPACING="0" CELLPADDING="4">
          <TR><TD>Bit</TD><TD>15</TD><TD>14</TD><TD>13</TD><TD>12</TD><TD>11</TD><TD>10</TD><TD>9</TD><TD>8</TD>
              <TD>7</TD><TD>6</TD><TD>5</TD><TD>4</TD><TD>3</TD><TD>2</TD><TD>1</TD><TD>0</TD>
          </TR>
          <TR><TD>Value</TD><TD COLSPAN="10">Valid data</TD><TD>0</TD><TD>0</TD><TD>0</TD><TD>0</TD><TD>0</TD><TD>0</TD>
          </TR>
            </TABLE>>];
     }


mfxFrameSurface1
----------------

.. doxygenstruct:: mfxFrameSurface1
   :project: DEF_BREATHE_PROJECT
   :members:
   :protected-members:

mfxFrameSurfaceInterface
------------------------

.. doxygenstruct:: mfxFrameSurfaceInterface
   :project: DEF_BREATHE_PROJECT
   :members:
   :protected-members:

mfxSurfacePoolInterface
------------------------

.. doxygenstruct:: mfxSurfacePoolInterface
   :project: DEF_BREATHE_PROJECT
   :members:
   :protected-members:

mfxMemoryInterface
------------------

.. doxygenstruct:: mfxMemoryInterface
   :project: DEF_BREATHE_PROJECT
   :members:
   :protected-members:

mfxSurfaceTypesSupported
------------------------

.. doxygenstruct:: mfxSurfaceTypesSupported
   :project: DEF_BREATHE_PROJECT
   :members:
   :protected-members:

mfxSurfaceHeader
----------------

.. doxygenstruct:: mfxSurfaceHeader
   :project: DEF_BREATHE_PROJECT
   :members:
   :protected-members:

mfxSurfaceInterface
-------------------

.. doxygenstruct:: mfxSurfaceInterface
   :project: DEF_BREATHE_PROJECT
   :members:
   :protected-members:

mfxSurfaceD3D11Tex2D
--------------------

.. doxygenstruct:: mfxSurfaceD3D11Tex2D
   :project: DEF_BREATHE_PROJECT
   :members:
   :protected-members:

mfxSurfaceVAAPI
---------------

.. doxygenstruct:: mfxSurfaceVAAPI
   :project: DEF_BREATHE_PROJECT
   :members:
   :protected-members:

mfxSurfaceOpenCLImg2D
---------------------

.. doxygenstruct:: mfxSurfaceOpenCLImg2D
   :project: DEF_BREATHE_PROJECT
   :members:
   :protected-members:

mfxExtSurfaceOpenCLImg2DExportDescription
-----------------------------------------

.. doxygenstruct:: mfxExtSurfaceOpenCLImg2DExportDescription
   :project: DEF_BREATHE_PROJECT
   :members:
   :protected-members:

mfxSurfaceD3D12Tex2D
---------------------

.. doxygenstruct:: mfxSurfaceD3D12Tex2D
   :project: DEF_BREATHE_PROJECT
   :members:
   :protected-members:

mfxExtSurfaceD3D12Tex2DExportDescription
-----------------------------------------

.. doxygenstruct:: mfxExtSurfaceD3D12Tex2DExportDescription
   :project: DEF_BREATHE_PROJECT
   :members:
   :protected-members:

mfxSurfaceVulkanImg2D
---------------------

.. doxygenstruct:: mfxSurfaceVulkanImg2D
   :project: DEF_BREATHE_PROJECT
   :members:
   :protected-members:

mfxExtSurfaceVulkanImg2DExportDescription
-----------------------------------------

.. doxygenstruct:: mfxExtSurfaceVulkanImg2DExportDescription
   :project: DEF_BREATHE_PROJECT
   :members:
   :protected-members: