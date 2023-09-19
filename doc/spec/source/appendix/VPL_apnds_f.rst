.. SPDX-FileCopyrightText: 2019-2020 Intel Corporation
..
.. SPDX-License-Identifier: CC-BY-4.0

=====================
CQP HRD Mode Encoding
=====================

The application can configure an AVC encoder to work in CQP rate control mode with
HRD model parameters. oneVPL will place HRD information to SPS/VUI and choose the
appropriate profile/level. It’s the responsibility of the application to provide
per-frame QP, track HRD conformance, and insert required SEI messages to the
bitstream.

The following example shows how to enable CQP HRD mode. The application should
set :ref:`RateControlMethod` to CQP,
:cpp:member:`mfxExtCodingOption::VuiNalHrdParameters` to ON,
:cpp:member:`mfxExtCodingOption::NalHrdConformance` to OFF, and
set rate control parameters similar to CBR or VBR modes (instead of QPI, QPP,
and QPB). oneVPL will choose CBR or VBR HRD mode based on the ``MaxKbps``
parameter. If ``MaxKbps`` is set to zero, oneVPL will use CBR HRD model
(write cbr_flag = 1 to VUI), otherwise the VBR model will be used
(and cbr_flag = 0 is written to VUI).

.. note:: For CQP, if implementation does not support individual QPI, QPP and QPB
   parameters, then QPI parameter should be used as a QP parameter across all frames.

.. literalinclude:: ../snippets/appnd_f.c
   :language: c++
   :start-after: /*beg1*/
   :end-before: /*end1*/
   :lineno-start: 1


