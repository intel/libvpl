.. SPDX-FileCopyrightText: 2019-2020 Intel Corporation
..
.. SPDX-License-Identifier: CC-BY-4.0

=========================================
Switchable Graphics and Multiple Monitors
=========================================

The following sections discuss support for switchable graphics and multiple
monitor configurations.

-------------------
Switchable Graphics
-------------------

Switchable Graphics refers to the machine configuration that multiple graphic
devices are available (integrated device for power saving and discrete
devices for performance.) Usually at one time or instance, one of the graphic
devices drives display and becomes the active device, and others become
inactive. There are different variations of software or hardware mechanisms to
switch between the graphic devices. In one of the switchable graphics variations,
it is possible to register an application in an affinity list to certain graphic
device so that the launch of the application automatically triggers a switch. The
actual techniques to enable such a switch are outside the scope of this document.
This section discusses the implication of switchable graphics to |msdk_full_name| and
|msdk_full_name| applications.

As |msdk_full_name| performs hardware acceleration through graphic devices, it is
critical that |msdk_full_name| can access the graphic device in the switchable
graphics setting. It is recommended to add the application to the graphic device
affinity list. If this is not possible, the application should handle the
following cases:

- By design, during legacy |msdk_full_name| library initialization, the
  :cpp:func:`MFXInit` function searches for graphic devices. If a |msdk_full_name|
  implementation is successfully loaded, the :cpp:func:`MFXInit` function returns :cpp:enumerator:`mfxStatus::MFX_ERR_NONE`
  and the :cpp:func:`MFXQueryIMPL` function returns the actual implementation
  type. If no |msdk_full_name| implementation is loaded, the :cpp:func:`MFXInit` function
  returns :cpp:enumerator:`mfxStatus::MFX_ERR_UNSUPPORTED`. In the switchable graphics
  environment, if the application is not in the graphic device affinity
  list, it is possible that the graphic device will not be accessible
  during the library initialization. The fact that the :cpp:func:`MFXInit`
  function returns :cpp:enumerator:`mfxStatus::MFX_ERR_UNSUPPORTED` does not mean that
  hardware acceleration is permanently impossible. The user may switch the
  graphics later and the graphic device will become accessible. It
  is recommended that the application initialize the library right before
  the actual decoding, video processing, and encoding operations to determine
  the hardware acceleration capability.
- During decoding, video processing, and encoding operations, if the application
  is not in the graphic device affinity list, the previously accessible
  graphic device may become inaccessible due to a switch event. The |msdk_full_name|
  functions will return :cpp:enumerator:`mfxStatus::MFX_ERR_DEVICE_LOST` or
  :cpp:enumerator:`mfxStatus::MFX_ERR_DEVICE_FAILED`, depending on when the switch occurs
  and what stage the |msdk_full_name| functions operate. The application should handle these
  errors and exit gracefully.

-----------------
Multiple Monitors
-----------------

Multiple monitors refer to the machine configuration that multiple graphic devices
are available. Some graphic devices connect to a display and become active
and accessible under the Microsoft\* DirectX\* infrastructure. Graphic devices
that are not connected to a display are inactive. Using the Microsoft Direct3D\* 9
infrastructure, devices that are not connected to a display are not accessible.

The legacy |msdk_full_name| uses the adapter number to access a specific graphic
device. Usually, the graphic device driving the main desktop becomes the primary
adapter. Other graphic devices take subsequent adapter numbers after the primary
adapter. Under the Microsoft Direct3D 9 infrastructure, only active adapters are
accessible and have an adapter number.

|msdk_full_name| extends the :cpp:type:`mfxIMPL` implementation type as shown in the
:ref:`Intel® Media SDK mfxIMPL Implementation Type Definitions table <mfxIMPL-def-1-table>`:

.. _mfxIMPL-def-1-table:

.. list-table:: Intel® Media SDK mfxIMPL Implementation Type Definitions
   :header-rows: 1
   :widths: 40 60

   * - **Implementation Type**
     - **Definition**
   * - :cpp:enumerator:`MFX_IMPL_HARDWARE`
     - |msdk_full_name| should initialize on the primary adapter
   * - :cpp:enumerator:`MFX_IMPL_HARDWARE2`
     - |msdk_full_name| should initialize on the 2nd graphic adapter
   * - :cpp:enumerator:`MFX_IMPL_HARDWARE3`
     - |msdk_full_name| should initialize on the 3rd graphic adapter
   * - :cpp:enumerator:`MFX_IMPL_HARDWARE4`
     - |msdk_full_name| should initialize on the 4th graphic adapter
   * - :cpp:enumerator:`MFX_IMPL_HARDWARE_ANY`
     - |msdk_full_name| should initialize on any graphic adapter.
   * - :cpp:enumerator:`MFX_IMPL_AUTO_ANY`
     - |msdk_full_name| should initialize on any graphic adapter. If not successful,
       load the software implementation.

The application can use the first four definitions shown in the
:ref:`Intel® Media SDK mfxIMPL Implementation Type Definitions table <mfxIMPL-def-1-table>`
to instruct the legacy |msdk_full_name| library to initialize on a specific
graphic device. The application can use the definitions for
:cpp:enumerator:`MFX_IMPL_HARDWARE_ANY` and
:cpp:enumerator:`MFX_IMPL_AUTO_ANY` for automatic detection.

If the application uses the Microsoft DirectX surfaces for I/O, it is critical
that the application and |msdk_full_name| work on the same graphic device. It is
recommended that the application use the following procedure:

#. The application uses the :cpp:func:`MFXInit` function to initialize the legacy
   |msdk_full_name|, with option :cpp:enumerator:`MFX_IMPL_HARDWARE_ANY` or
   :cpp:enumerator:`MFX_IMPL_AUTO_ANY`. The :cpp:func:`MFXInit` function returns
   :cpp:enumerator:`mfxStatus::MFX_ERR_NONE` if successful.
#. The application uses the :cpp:func:`MFXQueryIMPL` function to check the actual
   implementation type. The implementation type :cpp:enumerator:`MFX_IMPL_HARDWARE`,
   :cpp:enumerator:`MFX_IMPL_HARDWARE2`, :cpp:enumerator:`MFX_IMPL_HARDWARE3`,
   or :cpp:enumerator:`MFX_IMPL_HARDWARE4` indicates the graphic adapter the
   |msdk_full_name| works on.
#. The application creates the Direct3D device on the respective graphic adapter
   and passes it to |msdk_full_name| through the :cpp:func:`MFXVideoCORE_SetHandle` function.

Similar to the switchable graphics cases, interruption may result if the user
disconnects monitors from the graphic devices or remaps the primary adapter. If
the interruption occurs during the |msdk_full_name| library initialization, the
:cpp:func:`MFXInit` function may return :cpp:enumerator:`mfxStatus::MFX_ERR_UNSUPPORTED`.
This means hardware acceleration is currently not available. It is recommended
that the application initialize |msdk_full_name| right before the actual decoding,
video processing, and encoding operations to determine the hardware acceleration
capability.

If the interruption occurs during decoding, video processing, or encoding
operations, oneVPL functions will return :cpp:enumerator:`mfxStatus::MFX_ERR_DEVICE_LOST`
or :cpp:enumerator:`mfxStatus::MFX_ERR_DEVICE_FAILED`. The application should handle these
errors and exit gracefully.