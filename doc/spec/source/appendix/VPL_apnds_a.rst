.. SPDX-FileCopyrightText: 2019-2020 Intel Corporation
..
.. SPDX-License-Identifier: CC-BY-4.0

===================================
Configuration Parameter Constraints
===================================

The :cpp:struct:`mfxFrameInfo` structure is used by both the :cpp:struct:`mfxVideoParam`
structure during oneVPL class initialization and the :cpp:struct:`mfxFrameSurface1`
structure during the actual oneVPL class operation. The parameter constraints
described in the following tables apply.

-----------------------------------
DECODE, ENCODE, and VPP Constraints
-----------------------------------

The :ref:`DECODE, ENCODE, and VPP Constraints table <common-constraint-table>`
lists parameter constraints common to :term:`DECODE`, :term:`ENCODE`, and
:term:`VPP`.

.. _common-constraint-table:

.. list-table:: DECODE, ENCODE, and VPP Constraints
   :header-rows: 1
   :widths: 20 30 50

   * - **Parameters**
     - **Use During Initialization**
     - **Use During Operation**
   * - FourCC
     - Any valid value.
     - The value must be the same as the initialization value. The only exception is :term:`VPP` in composition mode, where in some cases it is allowed to mix RGB and NV12 surfaces. See :cpp:struct:`mfxExtVPPComposite` for more details.
   * - ChromaFormat
     - Any valid value.
     - The value must be the same as the initialization value.

------------------
DECODE Constraints
------------------

The :ref:`DECODE Constraints table <decode-constraint-table>` lists :term:`DECODE`
parameter constraints.

.. _decode-constraint-table:

.. list-table:: DECODE Constraints
   :header-rows: 1
   :widths: 20 30 50

   * - **Parameters**
     - **Use During Initialization**
     - **Use During Operation**
   * - Width, Height
     - Aligned frame size.
     - The values must be the equal to or larger than the initialization values.
   * - | CropX, CropY
       | CropW, CropH
     - Ignored.
     - :term:`DECODE` output. The cropping values are per-frame based.
   * - | AspectRatioW,
       | AspectRatioH
     - Any valid values or unspecified (zero); if unspecified, values from the input bitstream will be used. See note below the table.
     - DECODE output.
   * - | FrameRateExtN,
       | FrameRateExtD
     - If unspecified, values from the input bitstream will be used. See note below the table.
     - DECODE output.
   * - PicStruct
     - Ignored.
     - DECODE output.


.. note:: If the application explicitly sets FrameRateExtN/FrameRateExtD or
          AspectRatioW/AspectRatioH during initialization, then the decoder will
          use these values during decoding regardless of the values from bitstream
          and does not update them on new SPS. If the application sets them to 0,
          then the decoder uses values from the stream and updates them on each SPS.

------------------
ENCODE Constraints
------------------

The :ref:`ENCODE Constraints table <encode-constraint-table>` lists :term:`ENCODE`
parameter constraints.

.. _encode-constraint-table:

.. list-table:: ENCODE Constraints
   :header-rows: 1
   :widths: 20 40 40

   * - **Parameters**
     - **Use During Initialization**
     - **Use During Operation**
   * - Width, Height
     - Encoded frame size.
     - The values must be the equal to or larger than the initialization values.
   * - | CropX, CropY
       | CropW, CropH
     - | H.264: Cropped frame size
       | MPEG-2: CropW and CropH
       |
       | Specify the real width and height (may be unaligned) of the coded frames. CropX and CropY must be zero.
     - Ignored.
   * - | AspectRatioW,
       | AspectRatioH
     - Any valid values.
     - Ignored.
   * - | FrameRateExtN,
       | FrameRateExtD
     - Any valid values.
     - Ignored.
   * - PicStruct
     - | :cpp:enumerator:`MFX_PICSTRUCT_UNKNOWN`
       | :cpp:enumerator:`MFX_PICSTRUCT_PROGRESSIVE`
       | :cpp:enumerator:`MFX_PICSTRUCT_FIELD_TFF`
       | :cpp:enumerator:`MFX_PICSTRUCT_FIELD_BFF`
     - The base value must be the same as the initialization value unless :cpp:enumerator:`MFX_PICSTRUCT_UNKNOWN` is specified during initialization. Add other decorative picture structure flags to indicate additional display attributes. Use :cpp:enumerator:`MFX_PICSTRUCT_UNKNOWN` during initialization for field attributes and :cpp:enumerator:`MFX_PICSTRUCT_PROGRESSIVE` for frame attributes. See the :ref:`PicStruct` enumerator for details.


---------------
VPP Constraints
---------------

The :ref:`VPP Constraints table <vpp-constraint-table>` lists :term:`VPP`
parameter constraints.

.. _vpp-constraint-table:

.. list-table:: VPP Constraints
   :header-rows: 1
   :widths: 20 40 40

   * - **Parameters**
     - **During Initialization**
     - **During Operation**
   * - Width, Height
     - Any valid values
     - The values must be the equal to or larger than the initialization values.
   * - | CropX, CropY,
       | CropW, CropH
     - Ignored
     - These parameters specify the region of interest from input to output.
   * - | AspectRatioW,
       | AspectRatioH
     - Ignored
     - Aspect ratio values will be passed through from input to output.
   * - | FrameRateExtN,
       | FrameRateExtD
     - Any valid values
     - Frame rate values will be updated with the initialization value at output.
   * - PicStruct
     - | :cpp:enumerator:`MFX_PICSTRUCT_UNKNOWN`
       | :cpp:enumerator:`MFX_PICSTRUCT_PROGRESSIVE`
       | :cpp:enumerator:`MFX_PICSTRUCT_FIELD_TFF`
       | :cpp:enumerator:`MFX_PICSTRUCT_FIELD_BFF`
       | :cpp:enumerator:`MFX_PICSTRUCT_FIELD_SINGLE`
       | :cpp:enumerator:`MFX_PICSTRUCT_FIELD_TOP`
       | :cpp:enumerator:`MFX_PICSTRUCT_FIELD_BOTTOM`
     - The base value must be the same as the initialization value unless
       :cpp:enumerator:`MFX_PICSTRUCT_UNKNOWN` is specified during initialization.
       Other decorative picture structure flags are passed through or added as
       needed. See the :ref:`PicStruct` enumerator for details.

-----------------------------------
Specifying Configuration Parameters
-----------------------------------

The following :ref:`Configuration Parameters tables <config-param-table>`
summarize how to specify the configuration parameters during initialization,
encoding, decoding, and video processing.

.. _config-param-table:

.. list-table:: mfxVideoParam Configuration Parameters
   :header-rows: 1
   :widths: 15 14 14 14 14 14 15

   * - | **Structure**
       | **(param)**
     - | **ENCODE**
       | **Init**
     - | **ENCODE**
       | **Encoding**
     - | **DECODE**
       | **Init**
     - | **DECODE**
       | **Decoding**
     - | **VPP**
       | **Init**
     - | **VPP**
       | **Processing**
   * - Protected
     - R
     - -
     - R
     - -
     - R
     - -
   * - IOPattern
     - M
     - -
     - M
     - -
     - M
     - -
   * - ExtParam
     - O
     - -
     - O
     - -
     - O
     - -
   * - NumExtParam
     - O
     - -
     - O
     - -
     - O
     - -

.. list-table:: mfxInfoMFX Configuration Parameters
   :header-rows: 1
   :widths: 19 14 14 14 14 10 15

   * - | **Structure**
       | **(param)**
     - | **ENCODE**
       | **Init**
     - | **ENCODE**
       | **Encoding**
     - | **DECODE**
       | **Init**
     - | **DECODE**
       | **Decoding**
     - | **VPP**
       | **Init**
     - | **VPP**
       | **Processing**
   * - CodecId
     - M
     - -
     - M
     - -
     - -
     - -
   * - CodecProfile
     - O
     - -
     - O/M\*
     - -
     - -
     - -
   * - CodecLevel
     - O
     - -
     - O
     - -
     - -
     - -
   * - NumThread
     - O
     - -
     - O
     - -
     - -
     - -
   * - TargetUsage
     - O
     - -
     - -
     - -
     - -
     - -
   * - GopPicSize
     - O
     - -
     - -
     - -
     - -
     - -
   * - GopRefDist
     - O
     - -
     - -
     - -
     - -
     - -
   * - GopOptFlag
     - O
     - -
     - -
     - -
     - -
     - -
   * - IdrInterval
     - O
     - -
     - -
     - -
     - -
     - -
   * - RateControlMethod
     - O
     - -
     - -
     - -
     - -
     - -
   * - InitialDelayInKB
     - O
     - -
     - -
     - -
     - -
     - -
   * - BufferSizeInKB
     - O
     - -
     - -
     - -
     - -
     - -
   * - TargetKbps
     - M
     - -
     - -
     - -
     - -
     - -
   * - MaxKbps
     - O
     - -
     - -
     - -
     - -
     - -
   * - NumSlice
     - O
     - -
     - -
     - -
     - -
     - -
   * - NumRefFrame
     - O
     - -
     - -
     - -
     - -
     - -
   * - EncodedOrder
     - M
     - -
     - -
     - -
     - -
     - -

.. list-table:: mfxFrameInfo Configuration Parameters
   :header-rows: 1
   :widths: 19 14 14 14 14 10 15

   * - | **Structure**
       | **(param)**
     - | **ENCODE**
       | **Init**
     - | **ENCODE**
       | **Encoding**
     - | **DECODE**
       | **Init**
     - | **DECODE**
       | **Decoding**
     - | **VPP**
       | **Init**
     - | **VPP**
       | **Processing**
   * - FourCC
     - M
     - M
     - M
     - M
     - M
     - M
   * - Width
     - M
     - M
     - M
     - M
     - M
     - M
   * - Height
     - M
     - M
     - M
     - M
     - M
     - M
   * - CropX
     - M
     - Ign
     - Ign
     - U
     - Ign
     - M
   * - CropY
     - M
     - Ign
     - Ign
     - U
     - Ign
     - M
   * - CropW
     - M
     - Ign
     - Ign
     - U
     - Ign
     - M
   * - CropH
     - M
     - Ign
     - Ign
     - U
     - Ign
     - M
   * - FrameRateExtN
     - M
     - Ign
     - O
     - U
     - M
     - U
   * - FrameRateExtD
     - M
     - Ign
     - O
     - U
     - M
     - U
   * - AspectRatioW
     - O
     - Ign
     - O
     - U
     - Ign
     - PT
   * - AspectRatioH
     - O
     - Ign
     - O
     - U
     - Ign
     - PT
   * - PicStruct
     - O
     - M
     - Ign
     - U
     - M
     - M/U
   * - ChromaFormat
     - M
     - M
     - M
     - M
     - Ign
     - Ign

.. table:: Abbreviations used in configuration parameter tables
   :widths: auto

   ============ =================
   Abbreviation Meaning
   ============ =================
   Ign          Ignored
   PT           Pass Through
   -            Does Not Apply
   M            Mandated
   R            Reserved
   O            Optional
   U            Updated at output
   ============ =================


.. note:: :ref:`CodecProfile <codec_profile>` is mandated for HEVC REXT and SCC
          profiles and optional for other cases. If the application does not
          explicitly set CodecProfile during initialization, the HEVC decoder
          will use a profile up to Main10.