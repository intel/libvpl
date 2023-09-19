.. SPDX-FileCopyrightText: 2019-2020 Intel Corporation
..
.. SPDX-License-Identifier: CC-BY-4.0

================
Video Processing
================

Video processing functions (:term:`VPP`) take raw frames as input and provide
raw frames as output.

The actual conversion process is a chain operation with many single-function
filters.

.. graphviz::
   :caption: Video processing operation pipeline

   digraph {
     rankdir=LR;
     F1 [shape=record label="Function 1" ];
     F2 [shape=record  label="Function 2"];
     F3 [shape=record  label="Additional filters"];
     F4 [shape=record label="Function N-1" ];
     F5 [shape=record  label="Function N"];
     F1->F2->F3->F4->F5;
   }

The application specifies the input and output format; oneVPL configures the
pipeline according to the specified input and output formats. The application
can also attach one or more hint structures
to configure individual filters or turn them on and off. Unless specifically
instructed, oneVPL builds the pipeline in a way that best utilizes hardware
acceleration or generates the best video processing quality.

The :ref:`Video Processing Features table <vid-processing-feat-table>` shows oneVPL
video processing features. The application can configure supported video
processing features through the video processing I/O parameters. The application
can also configure optional features through hints.
See :ref:`Video Processing Procedures <vid_process_procedure>` for more details
on how to configure optional filters.

.. _vid-processing-feat-table:

.. list-table:: Video Processing Features
   :header-rows: 1
   :widths: 70 30

   * - **Video Processing Features**
     - **Configuration**
   * - Convert color format from input to output
     - I/O parameters
   * - De-interlace to produce progressive frames at the output
     - I/O parameters
   * - Crop and resize the input frames
     - I/O parameters
   * - Convert input frame rate to match the output
     - I/O parameters
   * - Perform inverse telecine operations
     - I/O parameters
   * - Fields weaving
     - I/O parameters
   * - Fields splitting
     - I/O parameters
   * - Remove noise
     - Hint (optional feature)
   * - Enhance picture details/edges
     - Hint (optional feature)
   * - Adjust the brightness, contrast, saturation, and hue settings
     - Hint (optional feature)
   * - Perform image stabilization
     - Hint (optional feature)
   * - Convert input frame rate to match the output, based on frame interpolation
     - Hint (optional feature)
   * - Perform detection of picture structure
     - Hint (optional feature)
