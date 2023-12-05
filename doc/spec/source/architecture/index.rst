.. SPDX-FileCopyrightText: 2019-2020 Intel Corporation
..
.. SPDX-License-Identifier: CC-BY-4.0
..
  Intel(r) Video Processing Library (Intel(r) VPL)

============
Architecture
============

|vpl_short_name| functions fall into the following categories:

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

With the exception of the global auxiliary functions, |vpl_short_name| functions are named
after their functioning domain and category. |vpl_short_name| exposes video domain
functions.

.. figure:: ../images/sdk_function_naming_convention.png

   |vpl_short_name| function name notation

Applications use |vpl_short_name| functions by linking with the |vpl_short_name| dispatcher library.

.. graphviz::
   :caption: |vpl_short_name| dispatching mechanism

   digraph {
     rankdir=TB;
     Application [shape=record label="Application" ];
     Sdk [shape=record  label="Intel® VPL dispatcher library"];
     Lib1 [shape=record  label="Intel® VPL library 1 (CPU)"];
     Lib2 [shape=record  label="Intel® VPL library 2 (platform 1)"];
     Lib3 [shape=record  label="Intel® VPL library 3 (platform 2)"];
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
