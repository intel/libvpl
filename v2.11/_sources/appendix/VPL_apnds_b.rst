.. SPDX-FileCopyrightText: 2019-2020 Intel Corporation
..
.. SPDX-License-Identifier: CC-BY-4.0

=========================
Multiple-segment Encoding
=========================

Multiple-segment encoding is useful in video editing applications during
production, for example when the encoder encodes multiple video clips according
to their time line. In general, one can define multiple-segment encoding as
dividing an input sequence of frames into segments and encoding them in different
encoding sessions with the same or different parameter sets. For example:

======================= =================== =====================
Segment Already Encoded Segment in Encoding Segment to be Encoded
======================= =================== =====================
0s                      200s                500s
======================= =================== =====================

.. note:: Different encoders can also be used.

The application must be able to:

- Extract encoding parameters from the bitstream of previously encoded segment.
- Import these encoding parameters to configure the encoder.

Encoding can then continue on the current segment using either the same or
similar encoding parameters.

Extracting the header that contains the encoding parameter set from the encoded
bitstream is usually the task of a format splitter (de-multiplexer). Alternatively,
the :cpp:func:`MFXVideoDECODE_DecodeHeader` function can export
the raw header if the application attaches the :cpp:struct:`mfxExtCodingOptionSPSPPS`
structure as part of the parameters.

The encoder can use the :cpp:struct:`mfxExtCodingOptionSPSPPS` structure to import
the encoding parameters during :cpp:func:`MFXVideoENCODE_Init`. The encoding
parameters are in the encoded bitstream format. Upon a successful import of the
header parameters, the encoder will generate bitstreams with a compatible (not
necessarily bit-exact) header. The
:ref:`Header Import Functions table <header-import-func-table>` shows all
functions that can import a header and their error codes if there are
unsupported parameters in the header or the encoder is unable to achieve
compatibility with the imported header.

.. _header-import-func-table:

.. list-table:: Header Import Functions
   :header-rows: 1
   :widths: 50 50

   * - **Function Name**
     - **Error Code if Import Fails**
   * - :cpp:func:`MFXVideoENCODE_Init`
     - :cpp:enumerator:`MFX_ERR_INCOMPATIBLE_VIDEO_PARAM`
   * - :cpp:func:`MFXVideoENCODE_QueryIOSurf`
     - :cpp:enumerator:`MFX_ERR_INCOMPATIBLE_VIDEO_PARAM`
   * - :cpp:func:`MFXVideoENCODE_Reset`
     - :cpp:enumerator:`MFX_ERR_INCOMPATIBLE_VIDEO_PARAM`
   * - :cpp:func:`MFXVideoENCODE_Query`
     - :cpp:enumerator:`MFX_ERR_UNSUPPORTED`


The encoder must encode frames to a GOP sequence starting with an IDR frame for
H.264 (or I frame for MPEG-2) to ensure that the current segment encoding does
not refer to any frames in the previous segment. This ensures that the encoded
segment is self-contained, allowing the application to insert the segment anywhere
in the final bitstream. After encoding, each encoded segment is HRD compliant.
Concatenated segments may not be HRD compliant.

The following example shows the encoder initialization procedure that imports
H.264 sequence and picture parameter sets:

.. literalinclude:: ../snippets/appnd_b.c
   :language: c++
   :start-after: /*beg*/
   :end-before: /*end*/
   :lineno-start: 1


