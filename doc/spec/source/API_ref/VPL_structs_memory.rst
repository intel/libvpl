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
   :project: Intel&reg; VPL
   :members:
   :protected-members:

mfxFrameAllocator
-----------------

.. doxygenstruct:: mfxFrameAllocator
   :project: Intel&reg; VPL
   :members:
   :protected-members:

mfxFrameAllocRequest
--------------------

.. doxygenstruct:: mfxFrameAllocRequest
   :project: Intel&reg; VPL
   :members:
   :protected-members:

mfxFrameAllocResponse
---------------------

.. doxygenstruct:: mfxFrameAllocResponse
   :project: Intel&reg; VPL
   :members:
   :protected-members:

mfxFrameData
------------

.. doxygenstruct:: mfxY410
   :project: Intel&reg; VPL
   :members:
   :protected-members:

.. doxygenstruct:: mfxY416
   :project: Intel&reg; VPL
   :members:
   :protected-members:

.. doxygenstruct:: mfxA2RGB10
   :project: Intel&reg; VPL
   :members:
   :protected-members:

.. doxygenstruct:: mfxFrameData
   :project: Intel&reg; VPL
   :members:
   :protected-members:

mfxFrameInfo
------------

.. doxygenstruct:: mfxFrameInfo
   :project: Intel&reg; VPL
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
   :project: Intel&reg; VPL
   :members:
   :protected-members:

mfxFrameSurfaceInterface
------------------------

.. doxygenstruct:: mfxFrameSurfaceInterface
   :project: Intel&reg; VPL
   :members:
   :protected-members:

mfxSurfacePoolInterface
------------------------

.. doxygenstruct:: mfxSurfacePoolInterface
   :project: Intel&reg; VPL
   :members:
   :protected-members:

mfxMemoryInterface
------------------

.. doxygenstruct:: mfxMemoryInterface
   :project: Intel&reg; VPL
   :members:
   :protected-members:

mfxSurfaceTypesSupported
------------------------

.. doxygenstruct:: mfxSurfaceTypesSupported
   :project: Intel&reg; VPL
   :members:
   :protected-members:

mfxSurfaceHeader
----------------

.. doxygenstruct:: mfxSurfaceHeader
   :project: Intel&reg; VPL
   :members:
   :protected-members:

mfxSurfaceInterface
-------------------

.. doxygenstruct:: mfxSurfaceInterface
   :project: Intel&reg; VPL
   :members:
   :protected-members:

mfxSurfaceD3D11Tex2D
--------------------

.. doxygenstruct:: mfxSurfaceD3D11Tex2D
   :project: Intel&reg; VPL
   :members:
   :protected-members:

mfxSurfaceVAAPI
---------------

.. doxygenstruct:: mfxSurfaceVAAPI
   :project: Intel&reg; VPL
   :members:
   :protected-members:

mfxSurfaceOpenCLImg2D
---------------------

.. doxygenstruct:: mfxSurfaceOpenCLImg2D
   :project: Intel&reg; VPL
   :members:
   :protected-members:

mfxExtSurfaceOpenCLImg2DExportDescription
-----------------------------------------

.. doxygenstruct:: mfxExtSurfaceOpenCLImg2DExportDescription
   :project: Intel&reg; VPL
   :members:
   :protected-members:
