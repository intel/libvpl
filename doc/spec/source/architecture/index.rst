.. SPDX-FileCopyrightText: 2019-2020 Intel Corporation
..
.. SPDX-License-Identifier: CC-BY-4.0

============
Architecture
============

oneVPL functions fall into the following categories:

.. glossary::

   DECODE
      Functions that decode compressed video streams into raw video frames

   ENCODE
      Functions that encode raw video frames into compressed bitstreams

   VPP
      Functions that perform video processing on raw video frames
      
   DECODE_VPP
      Functions that perform combined operations of decoding and video processing

   CORE
      Auxiliary functions for synchronization

   Misc
      Global auxiliary functions

With the exception of the global auxiliary functions, oneVPL functions are named
after their functioning domain and category. oneVPL exposes video domain
functions.

.. figure:: ../images/sdk_function_naming_convention.png
   :alt: oneVPL function name notation

   oneVPL function name notation

Applications use oneVPL functions by linking with the oneVPL dispatcher library.

.. graphviz::
   :caption: oneVPL dispatching mechanism

   digraph {
     rankdir=TB;
     Application [shape=record label="Application" ];
     Sdk [shape=record  label="oneVPL dispatcher library"];
     Lib1 [shape=record  label="oneVPL library 1 (CPU)"];
     Lib2 [shape=record  label="oneVPL library 2 (platform 1)"];
     Lib3 [shape=record  label="oneVPL library 3 (platform 2)"];
     Application->Sdk;
     Sdk->Lib1;
     Sdk->Lib2;
     Sdk->Lib3;
   }


The dispatcher library identifies the hardware acceleration device on the
running platform, determines the most suitable platform library for the
identified hardware acceleration, and then redirects function calls accordingly.


.. toctree::
   :hidden:

   VPL_decoding
   VPL_encoding
   VPL_processing
   VPL_decoding_vpp
