.. SPDX-FileCopyrightText: 2019-2020 Intel Corporation
..
.. SPDX-License-Identifier: CC-BY-4.0

==============================
Hardware Device Error Handling
==============================

For implementations that accelerate decoding, encoding, and video processing 
through a hardware device, API functions may return errors or warnings 
if the hardware device encounters errors. See the
:ref:`Hardware Device Errors and Warnings table <hw-device-errors-table>` for
detailed information about the errors and warnings.

.. _hw-device-errors-table:

.. list-table:: Hardware Device Errors and Warnings
   :header-rows: 1
   :widths: 60 40

   * - **Status**
     - **Description**
   * - :cpp:enumerator:`mfxStatus::MFX_ERR_DEVICE_FAILED`
     - Hardware device returned unexpected errors. oneVPL was unable to restore operation.
   * - :cpp:enumerator:`mfxStatus::MFX_ERR_DEVICE_LOST`
     - Hardware device was lost due to system lock or shutdown.
   * - :cpp:enumerator:`mfxStatus::MFX_WRN_PARTIAL_ACCELERATION`
     - The hardware does not fully support the specified configuration. The encoding, decoding, or video processing operation may be partially accelerated.
   * - :cpp:enumerator:`mfxStatus::MFX_WRN_DEVICE_BUSY`
     - Hardware device is currently busy.


oneVPL **Query**, **QueryIOSurf**, and **Init** functions return
:cpp:enumerator:`mfxStatus::MFX_WRN_PARTIAL_ACCELERATION` to indicate that the encoding,
decoding, or video processing operation can be partially hardware accelerated or
not hardware accelerated at all. The application can ignore this warning and
proceed with the operation. (Note that oneVPL functions may return
errors or other warnings overwriting
:cpp:enumerator:`mfxStatus::MFX_WRN_PARTIAL_ACCELERATION`, as it is a lower priority warning.)

oneVPL functions return :cpp:enumerator:`mfxStatus::MFX_WRN_DEVICE_BUSY` to indicate that the
hardware device is busy and unable to receive commands at this time. The recommended approach is:

   * If the asynchronous operation returns synchronization point along with :cpp:enumerator:`mfxStatus::MFX_WRN_DEVICE_BUSY` - call the :cpp:func:`MFXVideoCORE_SyncOperation` with it.
   * If application has buffered synchronization point(s) obtained from previous asynchronous operations - call :cpp:func:`MFXVideoCORE_SyncOperation` with the oldest one.
   * If no synchronization point(s) available - wait for a few milliseconds.
   * Resume the operation by resubmitting the request.


.. literalinclude:: ../snippets/prg_err.c
   :language: c++
   :start-after: /*beg1*/
   :end-before: /*end1*/
   :lineno-start: 1

The same procedure applies to encoding and video processing.

oneVPL functions return :cpp:enumerator:`mfxStatus::MFX_ERR_DEVICE_LOST` or
:cpp:enumerator:`mfxStatus::MFX_ERR_DEVICE_FAILED` to indicate that there is a complete
failure in hardware acceleration. The application must close and reinitialize
the oneVPL function class. If the application has provided a hardware acceleration
device handle to oneVPL, the application must reset the device.



