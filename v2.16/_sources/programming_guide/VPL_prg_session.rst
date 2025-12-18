.. SPDX-FileCopyrightText: 2019-2020 Intel Corporation
..
.. SPDX-License-Identifier: CC-BY-4.0
..
  Intel® Video Processing Library (Intel® VPL)
.. _sdk-session:

========================
|vpl_short_name| Session
========================

Before calling any |vpl_short_name| functions, the application must initialize the
library and create a |vpl_short_name| session. A |vpl_short_name| session maintains context for the
use of any of :term:`DECODE`, :term:`ENCODE`, :term:`VPP`, :term:`DECODE_VPP` 
functions.

------------------------------------
|msdk_full_name| Dispatcher (Legacy)
------------------------------------

The :cpp:func:`MFXInit` or :cpp:func:`MFXInitEx` function starts (initializes)
a session. The :cpp:func:`MFXClose` function closes (de-initializes) the
session. To avoid memory leaks, always call :cpp:func:`MFXClose` after
:cpp:func:`MFXInit`.

.. important:: :cpp:func:`MFXInit` and :cpp:func:`MFXInitEx` are deprecated 
               starting from API 2.0. Applications must use :cpp:func:`MFXLoad`
               and :cpp:func:`MFXCreateSession` to initialize implementations.

.. important:: For backward compatibility with existent |msdk_full_name|
               applications |vpl_short_name| session can be created and initialized by
               the legacy dispatcher through :cpp:func:`MFXInit` or
               :cpp:func:`MFXInitEx` calls. In this case, the reported API version
               will be 1.255 on |intel_r| platforms with X\ :sup:`e` architecture.

The application can initialize a session as a software-based session
(:cpp:enumerator:`MFX_IMPL_SOFTWARE`) or a hardware-based session
(:cpp:enumerator:`MFX_IMPL_HARDWARE`). In a software-based session, the SDK
functions execute on a CPU. In a hardware-base session, the SDK functions
use platform acceleration capabilities. For platforms that expose multiple
graphic devices, the application can initialize a session on any
alternative graphic device using the :cpp:enumerator:`MFX_IMPL_HARDWARE`,
:cpp:enumerator:`MFX_IMPL_HARDWARE2`, :cpp:enumerator:`MFX_IMPL_HARDWARE3`, or
:cpp:enumerator:`MFX_IMPL_HARDWARE4` values of :cpp:type:`mfxIMPL`.

The application can also initialize a session to be automatic (
:cpp:enumerator:`MFX_IMPL_AUTO` or :cpp:enumerator:`MFX_IMPL_AUTO_ANY`),
instructing the dispatcher library to detect the platform capabilities and
choose the best SDK library available. After initialization, the SDK returns the
actual implementation through the :cpp:func:`MFXQueryIMPL` function.

Internally, the dispatcher works as follows:

#. Dispatcher searches for the shared library with the specific name:

   ========= =============== ====================================
   **OS**    **Name**        **Description**
   ========= =============== ====================================
   Linux\*   libmfxsw64.so.1 64-bit software-based implementation
   Linux     libmfxsw32.so.1 32-bit software-based implementation
   Linux     libmfxhw64.so.1 64-bit hardware-based implementation
   Linux     libmfxhw64.so.1 32-bit hardware-based implementation
   Windows\* libmfxsw32.dll  64-bit software-based implementation
   Windows   libmfxsw32.dll  32-bit software-based implementation
   Windows   libmfxhw64.dll  64-bit hardware-based implementation
   Windows   libmfxhw64.dll  32-bit hardware-based implementation
   ========= =============== ====================================

#. Once the library is loaded, the dispatcher obtains addresses for each SDK
   function. See the
   :ref:`Exported Functions/API Version table <export-func-version-table-2x>` for
   the list of functions to expose.

.. _legacy_search_order:

How the shared library is identified using the implementation search strategy
will vary according to the OS.

* On Windows, the dispatcher searches the following locations, in the specified
  order, to find the correct implementation library:

  #. The :file:`Driver Store` directory for the current adapter.
     All types of graphics drivers can install libraries in this directory. `Learn more about Driver Store <https://docs.microsoft.com/en-us/windows-hardware/drivers/install/driver-store>`__.
  #. The directory specified for the current hardware under the registry key
     ``HKEY_CURRENT_USER\Software\Intel\MediaSDK\Dispatch``.
  #. The directory specified for the current hardware under the registry key
     ``HKEY_LOCAL_MACHINE\Software\Intel\MediaSDK\Dispatch``.
  #. The directory that is stored in these registry keys: :file:`C:\Program Files\Intel\Media SDK`.
     This directory is where legacy graphics drivers install libraries.
  #. The directory where the current module (the module that links the dispatcher)
     is located (only if the current module is a dll).

  After the dispatcher completes the main search, it additionally checks:

  #. The directory of the exe file of the current process, where it looks for
     software implementation only, regardless of which implementation the
     application requested.
  #. Default dll search. This provides loading from the directory of the
     application's exe file and from the :file:`System32` and :file:`SysWOW64`
     directories. `Learn more about default dll search order <https://docs.microsoft.com/en-us/windows/win32/dlls/dynamic-link-library-search-order?redirectedfrom=MSDN#search-order-for-desktop-applications>`__.
  #. The :file:`System32` and :file:`SysWOW64` directories, which is where DCH
     graphics drivers install libraries.

* On Linux, the dispatcher searches the following locations, in the specified
  order, to find the correct implementation library:

  #. Directories provided by the environment variable ``LD_LIBRARY_PATH``.
  #. Content of the :file:`/etc/ld.so.cache` cache file.
  #. Default path :file:`/lib`, then :file:`/usr/lib` or :file:`/lib64`, and then
     :file:`/usr/lib64` on some 64 bit OSs. On Debian: :file:`/usr/lib/x86_64-linux-gnu`.
  #. SDK installation folder.


.. _vpl-dispatcher:

---------------------------
|vpl_short_name| Dispatcher
---------------------------

The |vpl_short_name| dispatcher extends the legacy dispatcher by providing additional
ability to select the appropriate implementation based on the implementation
capabilities. Implementation capabilities include information about supported
decoders, encoders, and VPP filters. For each supported encoder, decoder, and
filter, capabilities include information about supported memory types, color
formats, and image (frame) size in pixels.

The recommended approach to configure the dispatcher's capabilities
search filters and to create a session based on a suitable implementation is as
follows:

#. Create loader with :cpp:func:`MFXLoad`.
#. Create loader's configuration with :cpp:func:`MFXCreateConfig`.
#. Add configuration properties with :cpp:func:`MFXSetConfigFilterProperty`.
#. Explore available implementations with :cpp:func:`MFXEnumImplementations`.
#. Create a suitable session with :cpp:func:`MFXCreateSession`.

The procedure to terminate an application is as follows:

#. Destroy session with :cpp:func:`MFXClose`.
#. Destroy loader with :cpp:func:`MFXUnload`.

.. note:: Multiple loader instances can be created.

.. note:: Each loader may have multiple configuration objects associated with it.
          When a configuration object is modified through :cpp:func:`MFXSetConfigFilterProperty`
          it implicitly impacts the state and configuration of the associated loader.

.. important:: One configuration object can handle only one filter property.

.. note:: Multiple sessions can be created by using one loader object.

When the dispatcher searches for the implementation, it uses the following
priority rules:

#. Hardware implementation has priority over software implementation.
#. General hardware implementation has priority over VSI hardware implementation.
#. Highest API version has higher priority over lower API version.

.. note:: Implementation has priority over the API version. In other words, the
          dispatcher must return the implementation with the highest API
          priority (greater than or equal to the implementation requested).

How the shared library is identified using the implementation search strategy
will vary according to the OS.

* On Windows, the dispatcher searches the following locations, in the specified
  order, to find the correct implementation library:

  #. The :file:`Driver Store` directory for all available adapters.
     All types of graphics drivers can install libraries in this directory. `Learn more about Driver Store <https://docs.microsoft.com/en-us/windows-hardware/drivers/install/driver-store>`__.
     Applicable only for Intel implementations.
  #. The directory of the exe file of the current process.
  #. `PATH` environmental variable.
  #. For backward compatibility with older spec versions, dispatcher also checks
     user-defined search folders which are provided by `ONEVPL_SEARCH_PATH`
     environmental variable.

* On Linux, the dispatcher searches the following locations, in the specified
  order, to find the correct implementation library:

  #. Directories provided by the environment variable ``LD_LIBRARY_PATH``.
  #. Default path :file:`/lib`, then :file:`/usr/lib` or :file:`/lib64`, and then
     :file:`/usr/lib64` on some 64 bit OSs. On Debian: :file:`/usr/lib/x86_64-linux-gnu`.
  #. For backward compatibility with older spec versions, dispatcher also checks
     user-defined search folders which are provided by `ONEVPL_SEARCH_PATH`
     environmental variable.
  #. Optional: Library installation directory specified by ``CMAKE_INSTALL_FULL_LIBDIR``,
     if and only if the project is configured with ``-D ENABLE_LIBDIR_IN_RUNTIME_SEARCH=ON``
     during cmake configuration (default = OFF).

.. important:: To prioritize loading of custom |vpl_short_name| library, users may set environment variable
               `ONEVPL_PRIORITY_PATH` with the path to the user-defined folder. 
               All libraries found in the ONEVPL_PRIORITY_PATH have the same priority 
               (higher than any others, and HW/SW or API version rules are not applied) and 
               should be loaded/filtered according to :cpp:func:`MFXSetConfigFilterProperty`.

When |vpl_short_name| dispatcher searchers for the legacy |msdk_full_name|
implementation it uses :ref:`legacy dispatcher search order <legacy_search_order>`, 
excluding the current working directory and :file:`/etc/ld.so.cache`.

The dispatcher supports different software implementations. The user can use
the :cpp:member:`mfxImplDescription::VendorID` field, the
:cpp:member:`mfxImplDescription::VendorImplID` field, or the
:cpp:member:`mfxImplDescription::ImplName` field to search for the specific
implementation.

Internally, the dispatcher works as follows:

#. Dispatcher loads all shared libraries in the given search folders, whose names
   match any of the patterns in the following table:

   ================== ================== ================= =========================================
   Windows 64-bit     Windows 32-bit     Linux 64-bit      Description
   ================== ================== ================= =========================================
   libvpl\*.dll       libvpl\*.dll       libvpl\*.so\*     Runtime library for any platform
   libmfx64-gen.dll   libmfx32-gen.dll   libmfx-gen.so.1.2 Runtime library for |vpl_short_name| for |intel_r| platforms with X\ :sup:`e` architecture
   libmfxhw64.dll     libmfxhw32.dll     libmfxhw64.so.1   Runtime library for |msdk_full_name|
   ================== ================== ================= =========================================

#. For each loaded library, the dispatcher tries to resolve address of the
   :cpp:func:`MFXQueryImplsDescription` function to collect the implementation's
   capabilities.
#. Once the user has requested to create the session based on this implementation,
   the dispatcher obtains addresses of each |vpl_short_name| function. See the
   :ref:`Exported Functions/API Version table <export-func-version-table-2x>` for
   the list of functions to export.

----------------------------------------------------
|vpl_short_name| Dispatcher Configuration Properties
----------------------------------------------------

The :ref:`Dispatcher Configuration Properties Table <dsp-conf-prop-table>` shows property strings
supported by the dispatcher. Table organized in the hierarchy way, to create the string, go from the
left to right from column to column and concatenate strings by using `.` (dot) as the separator.

.. _dsp-conf-prop-table:

.. container:: stripe-table

   .. table:: Dispatcher Configuration Properties
      :widths: 25 25 30 20

      +---------------------------------------+----------------------------+----------------------+---------------------------+
      | Structure name                        | Property                   | Value Data Type      | Comment                   |
      +=======================================+============================+======================+===========================+
      | :cpp:struct:`mfxImplDescription`      | | mfxImplDescription       | MFX_VARIANT_TYPE_U32 |                           |
      |                                       | | .Impl                    |                      |                           |
      |                                       +----------------------------+----------------------+---------------------------+
      |                                       | | mfxImplDescription       | MFX_VARIANT_TYPE_U32 | The mode will be used for |
      |                                       | | .AccelerationMode        |                      | session initialization    |
      |                                       +----------------------------+----------------------+---------------------------+
      |                                       | | mfxImplDescription       | MFX_VARIANT_TYPE_U32 |                           |
      |                                       | | .ApiVersion              |                      |                           |
      |                                       | | .Version                 |                      |                           |
      |                                       +----------------------------+----------------------+---------------------------+
      |                                       | | mfxImplDescription       | MFX_VARIANT_TYPE_U16 |                           |
      |                                       | | .ApiVersion              |                      |                           |
      |                                       | | .Major                   |                      |                           |
      |                                       +----------------------------+----------------------+---------------------------+
      |                                       | | mfxImplDescription       | MFX_VARIANT_TYPE_U16 |                           |
      |                                       | | .ApiVersion              |                      |                           |
      |                                       | | .Minor                   |                      |                           |
      |                                       +----------------------------+----------------------+---------------------------+
      |                                       | | mfxImplDescription       | MFX_VARIANT_TYPE_PTR | Pointer to the            |
      |                                       | | .ImplName                |                      | null-terminated string.   |
      |                                       +----------------------------+----------------------+---------------------------+
      |                                       | | mfxImplDescription       | MFX_VARIANT_TYPE_PTR | Pointer to the            |
      |                                       | | .License                 |                      | null-terminated string.   |
      |                                       +----------------------------+----------------------+---------------------------+
      |                                       | | mfxImplDescription       | MFX_VARIANT_TYPE_PTR | Pointer to the            |
      |                                       | | .Keywords                |                      | null-terminated string.   |
      |                                       +----------------------------+----------------------+---------------------------+
      |                                       | | mfxImplDescription       | MFX_VARIANT_TYPE_U32 |                           |
      |                                       | | .VendorID                |                      |                           |
      |                                       +----------------------------+----------------------+---------------------------+
      |                                       | | mfxImplDescription       | MFX_VARIANT_TYPE_U32 |                           |
      |                                       | | .VendorImplID            |                      |                           |
      |                                       +----------------------------+----------------------+---------------------------+
      |                                       | | mfxImplDescription       | MFX_VARIANT_TYPE_U32 |                           |
      |                                       | | .mfxSurfacePoolMode      |                      |                           |
      |                                       +----------------------------+----------------------+---------------------------+
      |                                       | | mfxImplDescription       | MFX_VARIANT_TYPE_PTR | Pointer to the            |
      |                                       | | .mfxDeviceDescription    |                      | null-terminated string.   |
      |                                       | | .device                  |                      |                           |
      |                                       | | .DeviceID                |                      |                           |
      |                                       +----------------------------+----------------------+---------------------------+
      |                                       | | mfxImplDescription       | MFX_VARIANT_TYPE_U16 |                           |
      |                                       | | .mfxDeviceDescription    |                      |                           |
      |                                       | | .device                  |                      |                           |
      |                                       | | .MediaAdapterType        |                      |                           |
      |                                       +----------------------------+----------------------+---------------------------+
      |                                       | | mfxImplDescription       | MFX_VARIANT_TYPE_U32 |                           |
      |                                       | | .mfxDecoderDescription   |                      |                           |
      |                                       | | .decoder                 |                      |                           |
      |                                       | | .CodecID                 |                      |                           |
      |                                       +----------------------------+----------------------+---------------------------+
      |                                       | | mfxImplDescription       | MFX_VARIANT_TYPE_U16 |                           |
      |                                       | | .mfxDecoderDescription   |                      |                           |
      |                                       | | .decoder                 |                      |                           |
      |                                       | | .MaxcodecLevel           |                      |                           |
      |                                       +----------------------------+----------------------+---------------------------+
      |                                       | | mfxImplDescription       | MFX_VARIANT_TYPE_U32 |                           |
      |                                       | | .mfxDecoderDescription   |                      |                           |
      |                                       | | .decoder                 |                      |                           |
      |                                       | | .decprofile              |                      |                           |
      |                                       | | .Profile                 |                      |                           |
      |                                       +----------------------------+----------------------+---------------------------+
      |                                       | | mfxImplDescription       | MFX_VARIANT_TYPE_U32 |                           |
      |                                       | | .mfxDecoderDescription   |                      |                           |
      |                                       | | .decoder                 |                      |                           |
      |                                       | | .decprofile              |                      |                           |
      |                                       | | .Profile                 |                      |                           |
      |                                       | | .decmemdesc              |                      |                           |
      |                                       | | .MemHandleType           |                      |                           |
      |                                       +----------------------------+----------------------+---------------------------+
      |                                       | | mfxImplDescription       | MFX_VARIANT_TYPE_PTR | Pointer to the            |
      |                                       | | .mfxDecoderDescription   |                      | :cpp:struct:`mfxRange32U` |
      |                                       | | .decoder                 |                      | object                    |
      |                                       | | .decprofile              |                      |                           |
      |                                       | | .Profile                 |                      |                           |
      |                                       | | .decmemdesc              |                      |                           |
      |                                       | | .Width                   |                      |                           |
      |                                       +----------------------------+----------------------+---------------------------+
      |                                       | | mfxImplDescription       | MFX_VARIANT_TYPE_PTR | Pointer to the            |
      |                                       | | .mfxDecoderDescription   |                      | :cpp:struct:`mfxRange32U` |
      |                                       | | .decoder                 |                      | object                    |
      |                                       | | .decprofile              |                      |                           |
      |                                       | | .Profile                 |                      |                           |
      |                                       | | .decmemdesc              |                      |                           |
      |                                       | | .Height                  |                      |                           |
      |                                       +----------------------------+----------------------+---------------------------+
      |                                       | | mfxImplDescription       | MFX_VARIANT_TYPE_U32 |                           |
      |                                       | | .mfxDecoderDescription   |                      |                           |
      |                                       | | .decoder                 |                      |                           |
      |                                       | | .decprofile              |                      |                           |
      |                                       | | .Profile                 |                      |                           |
      |                                       | | .decmemdesc              |                      |                           |
      |                                       | | .ColorFormats            |                      |                           |
      |                                       +----------------------------+----------------------+---------------------------+
      |                                       | | mfxImplDescription       | MFX_VARIANT_TYPE_U32 |                           |
      |                                       | | .mfxEncoderDescription   |                      |                           |
      |                                       | | .encoder                 |                      |                           |
      |                                       | | .CodecID                 |                      |                           |
      |                                       +----------------------------+----------------------+---------------------------+
      |                                       | | mfxImplDescription       | MFX_VARIANT_TYPE_U16 |                           |
      |                                       | | .mfxEncoderDescription   |                      |                           |
      |                                       | | .encoder                 |                      |                           |
      |                                       | | .MaxcodecLevel           |                      |                           |
      |                                       +----------------------------+----------------------+---------------------------+
      |                                       | | mfxImplDescription       | MFX_VARIANT_TYPE_U16 |                           |
      |                                       | | .mfxEncoderDescription   |                      |                           |
      |                                       | | .encoder                 |                      |                           |
      |                                       | | .BiDirectionalPrediction |                      |                           |
      |                                       +----------------------------+----------------------+---------------------------+
      |                                       | | mfxImplDescription       | MFX_VARIANT_TYPE_U32 |                           |
      |                                       | | .mfxEncoderDescription   |                      |                           |
      |                                       | | .encoder                 |                      |                           |
      |                                       | | .encprofile              |                      |                           |
      |                                       | | .Profile                 |                      |                           |
      |                                       +----------------------------+----------------------+---------------------------+
      |                                       | | mfxImplDescription       | MFX_VARIANT_TYPE_U32 |                           |
      |                                       | | .mfxEncoderDescription   |                      |                           |
      |                                       | | .encoder                 |                      |                           |
      |                                       | | .encprofile              |                      |                           |
      |                                       | | .Profile                 |                      |                           |
      |                                       | | .encmemdesc              |                      |                           |
      |                                       | | .MemHandleType           |                      |                           |
      |                                       +----------------------------+----------------------+---------------------------+
      |                                       | | mfxImplDescription       | MFX_VARIANT_TYPE_PTR | Pointer to the            |
      |                                       | | .mfxEncoderDescription   |                      | :cpp:struct:`mfxRange32U` |
      |                                       | | .encoder                 |                      | object                    |
      |                                       | | .encprofile              |                      |                           |
      |                                       | | .Profile                 |                      |                           |
      |                                       | | .encmemdesc              |                      |                           |
      |                                       | | .Width                   |                      |                           |
      |                                       +----------------------------+----------------------+---------------------------+
      |                                       | | mfxImplDescription       | MFX_VARIANT_TYPE_PTR | Pointer to the            |
      |                                       | | .mfxEncoderDescription   |                      | :cpp:struct:`mfxRange32U` |
      |                                       | | .encoder                 |                      | object                    |
      |                                       | | .encprofile              |                      |                           |
      |                                       | | .Profile                 |                      |                           |
      |                                       | | .encmemdesc              |                      |                           |
      |                                       | | .Height                  |                      |                           |
      |                                       +----------------------------+----------------------+---------------------------+
      |                                       | | mfxImplDescription       | MFX_VARIANT_TYPE_U32 |                           |
      |                                       | | .mfxEncoderDescription   |                      |                           |
      |                                       | | .encoder                 |                      |                           |
      |                                       | | .encprofile              |                      |                           |
      |                                       | | .Profile                 |                      |                           |
      |                                       | | .encmemdesc              |                      |                           |
      |                                       | | .ColorFormats            |                      |                           |
      |                                       +----------------------------+----------------------+---------------------------+
      |                                       | | mfxImplDescription       | MFX_VARIANT_TYPE_U32 |                           |
      |                                       | | .mfxVPPDescription       |                      |                           |
      |                                       | | .filter                  |                      |                           |
      |                                       | | .FilterFourCC            |                      |                           |
      |                                       +----------------------------+----------------------+---------------------------+
      |                                       | | mfxImplDescription       | MFX_VARIANT_TYPE_U16 |                           |
      |                                       | | .mfxVPPDescription       |                      |                           |
      |                                       | | .filter                  |                      |                           |
      |                                       | | .MaxDelayInFrames        |                      |                           |
      |                                       +----------------------------+----------------------+---------------------------+
      |                                       | | mfxImplDescription       | MFX_VARIANT_TYPE_U32 |                           |
      |                                       | | .mfxVPPDescription       |                      |                           |
      |                                       | | .filter                  |                      |                           |
      |                                       | | .memdesc                 |                      |                           |
      |                                       | | .MemHandleType           |                      |                           |
      |                                       +----------------------------+----------------------+---------------------------+
      |                                       | | mfxImplDescription       | MFX_VARIANT_TYPE_PTR | Pointer to the            |
      |                                       | | .mfxVPPDescription       |                      | :cpp:struct:`mfxRange32U` |
      |                                       | | .filter                  |                      | object                    |
      |                                       | | .memdesc                 |                      |                           |
      |                                       | | .Width                   |                      |                           |
      |                                       +----------------------------+----------------------+---------------------------+
      |                                       | | mfxImplDescription       | MFX_VARIANT_TYPE_PTR | Pointer to the            |
      |                                       | | .mfxVPPDescription       |                      | :cpp:struct:`mfxRange32U` |
      |                                       | | .filter                  |                      | object                    |
      |                                       | | .memdesc                 |                      |                           |
      |                                       | | .Height                  |                      |                           |
      |                                       +----------------------------+----------------------+---------------------------+
      |                                       | | mfxImplDescription       | MFX_VARIANT_TYPE_U32 |                           |
      |                                       | | .mfxVPPDescription       |                      |                           |
      |                                       | | .filter                  |                      |                           |
      |                                       | | .memdesc                 |                      |                           |
      |                                       | | .format                  |                      |                           |
      |                                       | | .InFormat                |                      |                           |
      |                                       +----------------------------+----------------------+---------------------------+
      |                                       | | mfxImplDescription       | MFX_VARIANT_TYPE_U32 |                           |
      |                                       | | .mfxVPPDescription       |                      |                           |
      |                                       | | .filter                  |                      |                           |
      |                                       | | .memdesc                 |                      |                           |
      |                                       | | .format                  |                      |                           |
      |                                       | | .OutFormats              |                      |                           |
      +---------------------------------------+----------------------------+----------------------+---------------------------+
      | :cpp:struct:`mfxImplementedFunctions` | | mfxImplementedFunctions  | MFX_VARIANT_TYPE_PTR | Pointer to the buffer     |
      |                                       | | .FunctionsName           |                      | with string               |
      +---------------------------------------+----------------------------+----------------------+---------------------------+
      | N/A                                   | | DXGIAdapterIndex         | MFX_VARIANT_TYPE_U32 | Adapter index according   |
      |                                       | |                          |                      | to                        |
      |                                       | |                          |                      | IDXGIFactory::EnumAdapters|
      +---------------------------------------+----------------------------+----------------------+---------------------------+

.. important:: DXGIAdapterIndex property is available for Windows only and filters only hardware implementations.

Examples of the property name strings:

- mfxImplDescription.mfxDecoderDescription.decoder.decprofile.Profile
- mfxImplDescription.mfxDecoderDescription.decoder.decprofile.decmemdesc.MemHandleType
- mfxImplementedFunctions.FunctionsName

Following properties are supported in a special manner: they are used to send
additional data to the implementation through the dispatcher. Application needs
to use :cpp:func:`MFXSetConfigFilterProperty` to set them up but they don't
influence on the implementation selection. They are used during the
:cpp:func:`MFXCreateSession` function call to fine tune the implementation.

.. list-table:: Dispatcher's Special Properties
   :header-rows: 1
   :widths: auto

   * - **Property Name**
     - **Property Value**
     - **Value data type**
   * - mfxHandleType
     - :cpp:enum:`mfxHandleType`
     - :cpp:enumerator:`mfxVariantType::MFX_VARIANT_TYPE_U32`
   * - mfxHDL
     - :cpp:type:`mfxHDL`
     - :cpp:enumerator:`mfxVariantType::MFX_VARIANT_TYPE_PTR`
   * - NumThread
     - Unsigned fixed-point integer value
     - :cpp:enumerator:`mfxVariantType::MFX_VARIANT_TYPE_U32`
   * - DeviceCopy
     - :ref:`Device copy <gpu_copy>`
     - :cpp:enumerator:`mfxVariantType::MFX_VARIANT_TYPE_U16`
   * - ExtBuffer
     - Pointer to the extension buffer
     - :cpp:enumerator:`mfxVariantType::MFX_VARIANT_TYPE_PTR`

.. _vpl-dispatcher-interactions:

----------------------------------------
|vpl_short_name| Dispatcher Interactions
----------------------------------------

This sequence diagram visualize how application communicates with implementations
via the dispatcher.

Dispatcher API
    This API is implemented in the dispatcher.

Implementation API
    This API is provided by the any implementation.

.. uml::

   @startuml
   actor Application as A
   participant "Intel® VPL Dispatcher" as D
   participant "Intel® VPL Implementation 1" as I1
   participant "Intel® VPL Implementation 2" as I2
   participant "Intel® VPL Implementation 3" as I3

   ref over A, D : Dispatcher API
   ref over D, I1, I2, I3 : Implementation API

   activate A
   == Initialization ==
   group Dispatcher API [Enumerate and load implementations]
      A -> D: mfxLoad()
      activate D

      D -> D: Search for the available runtimes

      A -> D: MFXCreateConfig()
      A -> D: MFXSetConfigProperty()
      A -> D: MFXCreateConfig()
      A -> D: MFXSetConfigProperty()
      A -> D: MFXEnumImplementations()

      note right of A
      MFXEnumImplementations() may also be called
      after MFXCreateSession().
      end note

      Activate I1

      D -> I1: MFXQueryImplsDescription()
      I1 --> D: mfxImplDescription

      Activate I2
      D -> I2: MFXQueryImplsDescription()
      I2 --> D: mfxImplDescription

      Activate I3
      D -> I3: MFXQueryImplsDescription()
      I3 --> D: mfxImplDescription

      D --> A: list of mfxImplDescription structures es for all implementations

      A -> D: MFXCreateSession(i=0)
      D -> I1: MFXInitilize()
      Activate I1 #DarkSalmon
      I1 --> D: mfxSession1
      D --> A: mfxSession1

      A -> D: MFXCreateSession(i=1)
      D -> I2: MFXInitilize()
      Activate I2 #DarkSalmon
      I2 --> D: mfxSession2
      D --> A: mfxSession2

      A -> D: MFXCreateSession(i=2)
      D -> I3: MFXInitilize()
      I3 --> D: mfxSession3
      Activate I3 #DarkSalmon
      D --> A: mfxSession3

      A -> D: MFXDispReleaseImplDescription(hdl=0)
      D -> I1: MFXReleaseImplDescription()

      A -> D: MFXDispReleaseImplDescription(hdl=1)
      D -> I2: MFXReleaseImplDescription()

      A -> D: MFXDispReleaseImplDescription(hdl=2)
      D -> I3: MFXReleaseImplDescription()
   end
   == Processing ==
   group Implementation API [Process the data]
      A -> I1: MFXVideoDECODE_Init()
      A -> I1: MFXVideoDECODE_Query()
      A -> I1: MFXVideoDECODE_DecodeFrameAsync()
      ...
      A -> I1: MFXVideoDECODE_Close()
      |||
      deactivate I1

      A -> I2: MFXVideoENCODE_Init()
      A -> I2: MFXVideoENCODE_Query()
      A -> I2: MFXVideoENCODE_EncodeFrameAsync()
      ...
      A -> I2: MFXVideoENCODE_Close()
      |||
      deactivate I2

      A -> I3: MFXVideoENCODE_Init()
      A -> I3: MFXVideoENCODE_Query()
      A -> I3: MFXVideoENCODE_EncodeFrameAsync()
      ...
      A -> I3: MFXVideoENCODE_Close()
      |||
      deactivate I3
   end
   == Finalization ==
   group Implementation API [Release the implementations]
      A -> I1: MFXClose()
      deactivate I1
      A -> I2: MFXClose()
      deactivate I2
      A -> I3: MFXClose()
      deactivate I3
   group Dispatcher API [Release the dispatcher's instance]
      A -> D: MFXUnload()
   end

   deactivate D
   @enduml

The |vpl_short_name| dispatcher is capable to load and initialize |msdk_full_name| legacy
library. The sequence diagram below demonstrates the approach.

.. uml::

   @startuml
   actor Application as A
   participant "Intel® VPL Dispatcher" as D
   participant "Intel® MediaSDK (legacy)" as M
   A -> D: MFXLoad
   activate D
   D -> D: Search for the available runtimes
   A -> D: MFXCreateConfig
   
   note left of D
   Setting properties to filter implementation.
   MediaSDK supports only general parameters,
   no filtering for Decode/VPP/Encoder details.
   end note

   group MediaSDK LegacyAPI
      D -> M: MFXInitEx
      activate M

      M --> D: mfxSession
      D -> M: MFXQueryIMPL
      M --> D: Implementation speciefic

      D -> M: MFXQueryVersion
      M --> D: mfxVersion
   end

   D -> D: Fill mfxImplDescription for MediaSDK impl

   A -> D: mfxEnumImplementations
   D --> A: MediaSDK caps description

   A -> D: MFXCreateSession
   D --> A: mfxSession

   A -> M: MFXVideoDECODE_Init()
   A -> M: MFXVideoDECODE_Query()
   A -> M: MFXVideoDECODE_DecodeFrameAsync()
   ...
   A -> M: MFXVideoDECODE_Close()
   A -> M: MFXClose()

   deactivate M

   A -> D: MFXUnload()
   deactivate D
   @enduml

.. important:: The dispatcher doesn't filter and report
               :cpp:struct:`mfxDeviceDescription`,
               :cpp:struct:`mfxDecoderDescription`,
               :cpp:struct:`mfxEncoderDescription`,
               :cpp:struct:`mfxVPPDescription` when enumerates or creates
               |msdk_full_name| implementation. Once |msdk_full_name| is loaded
               applications have to use legacy approach to query capabilities.

-------------------------------------
|vpl_short_name| Dispatcher Debug Log
-------------------------------------

The debug output of the dispatcher is controlled with the `ONEVPL_DISPATCHER_LOG`
environment variable. To enable log output, set the `ONEVPL_DISPATCHER_LOG`
environment variable value equals to "ON".

By default, |vpl_short_name| dispatcher prints all log messages to the console.
To redirect log output to the desired file, set the `ONEVPL_DISPATCHER_LOG_FILE`
environmental variable with the file name of the log file.

------------------------------
Examples of Dispatcher's Usage
------------------------------

This code illustrates simple usage of dispatcher to load first available
library:

.. literalinclude:: ../snippets/prg_disp.c
   :language: c++
   :start-after: /*beg1*/
   :end-before: /*end1*/
   :lineno-start: 1

This code illustrates simple usage of dispatcher to load first available HW
accelerated library:

.. literalinclude:: ../snippets/prg_disp.c
   :language: c++
   :start-after: /*beg2*/
   :end-before: /*end2*/
   :lineno-start: 1

This code illustrates how multiple sessions from multiple loaders can be
created:

.. literalinclude:: ../snippets/prg_disp.c
   :language: c++
   :start-after: /*beg3*/
   :end-before: /*end3*/
   :lineno-start: 1

This code illustrates how multiple decoders from single loader can be
created:

.. literalinclude:: ../snippets/prg_disp.c
   :language: c++
   :start-after: /*beg4*/
   :end-before: /*end4*/
   :lineno-start: 1

---------------------------------------
How To Check If Function is Implemented
---------------------------------------

There are two ways to check if particular function is implemented or not by the
implementation.

This code illustrates how application can iterate through the whole list of
implemented functions:

.. literalinclude:: ../snippets/prg_session.cpp
   :language: c++
   :start-after: /*beg1*/
   :end-before: /*end1*/
   :lineno-start: 1

This code illustrates how application can check that specific functions are
implemented:

.. literalinclude:: ../snippets/prg_session.cpp
   :language: c++
   :start-after: /*beg2*/
   :end-before: /*end2*/
   :lineno-start: 1

--------------------------------------------------------------
How To Search For The Available encoder/decoder implementation
--------------------------------------------------------------

The :ref:`CodecFormatFourCC <codec-format-fourcc>` enum specifies codec's FourCC
values. Application needs to assign this value to the field of
:cpp:member:`mfxDecoderDescription::decoder::CodecID` to search for the decoder
or :cpp:member:`mfxEncoderDescription::encoder::CodecID` to search for the
encoder.

This code illustrates decoder's implementation search procedure:

.. literalinclude:: ../snippets/prg_session.cpp
   :language: c++
   :start-after: /*beg3*/
   :end-before: /*end3*/
   :lineno-start: 1

---------------------------------------------------------
How To Search For The Available VPP Filter implementation
---------------------------------------------------------

Each VPP filter identified by the filter ID. Filter ID is defined by
corresponding to the filter extension buffer ID value which is defined in a form
of FourCC value.
Filter ID values are subset of the general :ref:`ExtendedBufferID <extendedbufferid>`
enum. The :ref:`table <vpp-filters-ids>` references available IDs of VPP filters
to search. Application needs to assign this value to the field of
:cpp:member:`mfxVPPDescription::filter::FilterFourCC` to search for the needed
VPP filter.

.. _vpp-filters-ids:

.. list-table:: VPP Filters ID
   :header-rows: 1
   :widths: 58 42

   * - **Filter ID**
     - **Description**
   * - :cpp:enumerator:`MFX_EXTBUFF_VPP_DENOISE2`
     - Denoise filter
   * - :cpp:enumerator:`MFX_EXTBUFF_VPP_MCTF`
     - Motion-Compensated Temporal Filter (MCTF).
   * - :cpp:enumerator:`MFX_EXTBUFF_VPP_DETAIL`
     - Detail/edge enhancement filter.
   * - :cpp:enumerator:`MFX_EXTBUFF_VPP_FRAME_RATE_CONVERSION`
     - Frame rate conversion filter
   * - :cpp:enumerator:`MFX_EXTBUFF_VPP_IMAGE_STABILIZATION`
     - Image stabilization filter
   * - :cpp:enumerator:`MFX_EXTBUFF_VPP_PROCAMP`
     - ProcAmp filter
   * - :cpp:enumerator:`MFX_EXTBUFF_VPP_FIELD_PROCESSING`
     - Field processing filter
   * - :cpp:enumerator:`MFX_EXTBUFF_VPP_COLOR_CONVERSION`
     - Color Conversion filter
   * - :cpp:enumerator:`MFX_EXTBUFF_VPP_SCALING`
     - Resize filter
   * - :cpp:enumerator:`MFX_EXTBUFF_VPP_COMPOSITE`
     - Surfaces composition filter
   * - :cpp:enumerator:`MFX_EXTBUFF_VPP_DEINTERLACING`
     - Deinterlace filter
   * - :cpp:enumerator:`MFX_EXTBUFF_VPP_ROTATION`
     - Rotation filter
   * - :cpp:enumerator:`MFX_EXTBUFF_VPP_MIRRORING`
     - Mirror filter
   * - :cpp:enumerator:`MFX_EXTBUFF_VPP_COLORFILL`
     - ColorFill filter

This code illustrates VPP mirror filter implementation search procedure:

.. literalinclude:: ../snippets/prg_session.cpp
   :language: c++
   :start-after: /*beg4*/
   :end-before: /*end4*/
   :lineno-start: 1

-------------------------------------------------------------
How To Get Path to the Shared Library With the Implementation
-------------------------------------------------------------

Sessions can be created from different implementations, each implementations can
be located in different shared libraries. To get path of the shared library with
the implementation from which session can be or was created, application can use
:cpp:func:`MFXEnumImplementations` and pass :cpp:enumerator:`MFX_IMPLCAPS_IMPLPATH`
value as the output data request.

This code illustrates collection and print out path of implementations's
shared library:

.. literalinclude:: ../snippets/prg_session.cpp
   :language: c++
   :start-after: /*beg5*/
   :end-before: /*end5*/
   :lineno-start: 1

--------------------------------------------------------------
How To Get Extended Capability Information for encoder/decoder
--------------------------------------------------------------

From |vpl_short_name| API 2.15 applications can get more capabilities information
for encoder/decoder with the help of the :cpp:func:`MFXEnumImplementations`
function, including list of supported extension buffers, list of supported
Bitrate Control for encoders, supported bit depth and chroma subsamplings.

:cpp:struct:`mfxDecoderDescription` is extended to contain
:cpp:struct:`mfxDecExtDescription` and :cpp:struct:`mfxDecMemExtDescription` from
the struct version 1.1. :cpp:struct:`mfxEncoderDescription` is extended to contain
:cpp:struct:`mfxEncExtDescription` and :cpp:struct:`mfxEncMemExtDescription` from
the struct version 1.1.

Applications can check the struct version and correspondent pointers in extended
:cpp:struct:`mfxDecoderDescription` and :cpp:struct:`mfxEncExtDescription` to
get the extended capability information.

This code illustrates how to get the extended capability information using
:cpp:func:`MFXEnumImplementations` before session creation:

.. literalinclude:: ../snippets/prg_session.cpp
   :language: c++
   :start-after: /*beg6*/
   :end-before: /*end6*/
   :lineno-start: 1

.. note:: :cpp:struct:`mfxEncExtDescription` and
          :cpp:struct:`mfxEncMemExtDescription` report capabilities only
          for VDEnc.

---------------------------------------------------------------
How To Do Property-Based Query For The Available Implementation
---------------------------------------------------------------

From |vpl_short_name| API 2.15 applications can use property-based query
to search for the available implementation. Applications can get only
shallow information for session creation without
filling out the following structures:
:cpp:struct:`mfxDecoderDescription`,
:cpp:struct:`mfxEncoderDescription`,
:cpp:struct:`mfxVPPDescription`.
Applications can also query specific encoder or decoder or VPP filter
before session creation. This will result in faster session creation
as compared with full query of all encoders, decoders and VPP filters.

Applications must use the data type
:cpp:enumerator:`mfxVariantType::MFX_VARIANT_TYPE_QUERY` as bitmask to
OR with other variant data types to do property-based query.

This code illustrates how application can use the shallow information for
session creation:

.. literalinclude:: ../snippets/prg_session.cpp
   :language: c++
   :start-after: /*beg7*/
   :end-before: /*end7*/
   :lineno-start: 1

Applications can get only shallow information with
:cpp:func:`MFXEnumImplementations`.

This code illustrates how application can use the shallow information to
explore available implementations:

.. literalinclude:: ../snippets/prg_session.cpp
   :language: c++
   :start-after: /*beg8*/
   :end-before: /*end8*/
   :lineno-start: 1

Applications can also use :cpp:enumerator:`mfxVariantType::MFX_VARIANT_TYPE_QUERY`
to find decoders or encoders or VPP filters.
This code illustrates how application can find one encoder for session creation:

.. literalinclude:: ../snippets/prg_session.cpp
   :language: c++
   :start-after: /*beg10*/
   :end-before: /*end10*/
   :lineno-start: 1

This code illustrates how application can find decoders for session creation:

.. literalinclude:: ../snippets/prg_session.cpp
   :language: c++
   :start-after: /*beg9*/
   :end-before: /*end9*/
   :lineno-start: 1

This code illustrates how application can find VPP filters for session
creation:

.. literalinclude:: ../snippets/prg_session.cpp
   :language: c++
   :start-after: /*beg11*/
   :end-before: /*end11*/
   :lineno-start: 1

Applications can use :cpp:enumerator:`mfxVariantType::MFX_VARIANT_TYPE_QUERY`
to get decoder or encoder or VPP filter information with
:cpp:func:`MFXEnumImplementations`.
This code illustrates how application can query one encoder using
:cpp:func:`MFXEnumImplementations`:

.. literalinclude:: ../snippets/prg_session.cpp
   :language: c++
   :start-after: /*beg12*/
   :end-before: /*end12*/
   :lineno-start: 1

The :ref:`Query-able Properties Table <query-able-prop-table>` shows property
strings supported by property-based query. Applying the bitmask
:cpp:enumerator:`mfxVariantType::MFX_VARIANT_TYPE_QUERY` to properties not
listed in this table will cause :cpp:func:`MFXSetConfigFilterProperty` to return
MFX_ERR_UNSUPPORTED.

.. _query-able-prop-table:

.. container:: stripe-table

   .. table:: Query-able Properties
      :widths: 25 30 20

      +----------------------------+------------------------+---------------------------+
      | Property                   | Value Data Type        | Comment                   |
      +============================+========================+===========================+
      | | mfxImplDescription       | MFX_VARIANT_TYPE_U32 | | Query for shallow         |
      |                            | MFX_VARIANT_TYPE_QUERY | information               |
      +----------------------------+------------------------+---------------------------+
      | | mfxImplDescription       | MFX_VARIANT_TYPE_U32 | | Query for decoder         |
      | | .mfxDecoderDescription   | MFX_VARIANT_TYPE_QUERY |                           |
      | | .decoder                 |                        |                           |
      | | .CodecID                 |                        |                           |
      +----------------------------+------------------------+---------------------------+
      | | mfxImplDescription       | MFX_VARIANT_TYPE_U32 | | Query for encoder         |
      | | .mfxEncoderDescription   | MFX_VARIANT_TYPE_QUERY |                           |
      | | .encoder                 |                        |                           |
      | | .CodecID                 |                        |                           |
      +----------------------------+------------------------+---------------------------+
      | | mfxImplDescription       | MFX_VARIANT_TYPE_U32 | | Query for VPP filter      |
      | | .mfxVPPDescription       | MFX_VARIANT_TYPE_QUERY |                           |
      | | .filter                  |                        |                           |
      | | .FilterFourCC            |                        |                           |
      +----------------------------+------------------------+---------------------------+
      | | mfxImplDescription       | MFX_VARIANT_TYPE_U32 | | Query for all decoders    |
      | | .mfxDecoderDescription   | MFX_VARIANT_TYPE_QUERY |                           |
      +----------------------------+------------------------+---------------------------+
      | | mfxImplDescription       | MFX_VARIANT_TYPE_U32 | | Query for all encoders    |
      | | .mfxEncoderDescription   | MFX_VARIANT_TYPE_QUERY |                           |
      +----------------------------+------------------------+---------------------------+
      | | mfxImplDescription       | MFX_VARIANT_TYPE_U32 | | Query for all VPP filters |
      | | .mfxVPPDescription       | MFX_VARIANT_TYPE_QUERY |                           |
      +----------------------------+------------------------+---------------------------+

Examples of valid property name strings for query:

- mfxImplDescription
- mfxImplDescription.mfxEncoderDescription.encoder.CodecID
- mfxImplDescription.mfxDecoderDescription

.. important::
   To use property-based query, the application needs to configure at least
   one of the properties in the
   :ref:`Query-able Properties Table <query-able-prop-table>`.
   For example, if application wants to query a particular AV1 profile,
   it needs to set at least:

   * mfxImplDescription.mfxEncoderDescription.encoder.CodecID and value data :cpp:enumerator:`MFX_CODEC_AV1` (to get all AV1 encoder properties)

   or

   * mfxImplDescription.mfxEncoderDescription (to get all encoder properties for all encoders)

No matter what property is queried, below shallow information will be filled in
:cpp:struct:`mfxImplDescription`:

.. code-block:: c++

   mfxStructVersion       Version;
   mfxImplType            Impl;
   mfxAccelerationMode    AccelerationMode;
   mfxVersion             ApiVersion;
   mfxChar                ImplName[MFX_IMPL_NAME_LEN];
   mfxChar                License[MFX_STRFIELD_LEN];
   mfxChar                Keywords[MFX_STRFIELD_LEN];
   mfxU32                 VendorID;
   mfxU32                 VendorImplID;
   mfxDeviceDescription   Dev;
   mfxAccelerationModeDescription   AccelerationModeDescription;
   mfxPoolPolicyDescription  PoolPolicies;

.. note:: When using property-based queries, all of the desired properties must be
          configured before the first call to either :cpp:func:`MFXCreateSession`
          or :cpp:func:`MFXEnumImplementations`.

.. _vpl_coexistense:

---------------------------------------------------------------------------------------------------------------------
|vpl_short_name| implementation on |intel_r| platforms with X\ :sup:`e` architecture and |msdk_full_name| Coexistence 
---------------------------------------------------------------------------------------------------------------------

|vpl_short_name| supersedes |msdk_full_name| and is partially binary compatible with
|msdk_full_name|. Both |vpl_short_name| and |msdk_full_name| includes own dispatcher and
implementation. Coexistence of |vpl_short_name| and |msdk_full_name| dispatchers and
implementations on single system is allowed until |msdk_full_name| is not EOL.

Usage of the following combinations of dispatchers and implementations within
the single application is permitted for the legacy purposes only. In that
scenario legacy applications developed with |msdk_full_name| will continue to
work on any HW supported either by |msdk_full_name| or by the |vpl_short_name|.

.. attention:: Any application to work with the |vpl_short_name| API starting from version
               2.0 must use only |vpl_short_name| dispatcher.

|msdk_full_name| API
    |msdk_full_name| API of 1.x version.

Removed API
    |msdk_full_name| :ref:`API <deprecated-api>` which is removed from |vpl_short_name|.

Core API
    |msdk_full_name| API without removed API.

|vpl_short_name| API
    New :ref:`API <new-api>` introduced in |vpl_short_name| only started from API 2.0
    version.

|vpl_short_name| Dispatcher API
    Dispatcher :ref:`API <dispatcher-api>` introduced in |vpl_short_name| in 2.0
    API version. This is subset of |vpl_short_name| API.

.. list-table:: |vpl_short_name| for |intel_r| platforms with X\ :sup:`e` architecture and |msdk_full_name|
   :header-rows: 1
   :widths: 25 25 25 25

   * - **Dispatcher**
     - **Installed on the device**
     - **Loaded**
     - **Allowed API**
   * - |vpl_short_name|
     - |vpl_short_name| for |intel_r| platforms with X\ :sup:`e` architecture
     - |vpl_short_name| for |intel_r| platforms with X\ :sup:`e` architecture
     - Usage of any API except removed API is allowed.
   * - |vpl_short_name|
     - |msdk_full_name|
     - |msdk_full_name|
     - Usage of core API plus dispatcher API is allowed only.
   * - |vpl_short_name|
     - |vpl_short_name| for |intel_r| platforms with X\ :sup:`e` architecture and |msdk_full_name|
     - |vpl_short_name| for |intel_r| platforms with X\ :sup:`e` architecture
     - Usage of any API except removed API is allowed.
   * - |msdk_full_name|
     - |vpl_short_name| for |intel_r| platforms with X\ :sup:`e` architecture
     - |vpl_short_name| for |intel_r| platforms with X\ :sup:`e` architecture
     - Usage of core API is allowed only.
   * - |msdk_full_name|
     - |vpl_short_name| for |intel_r| platforms with X\ :sup:`e` architecture and |msdk_full_name|
     - |msdk_full_name|
     - Usage of |msdk_full_name| API is allowed.
   * - |msdk_full_name|
     - |msdk_full_name|
     - |msdk_full_name|
     - Usage of |msdk_full_name| API is allowed.

.. note:: if system has multiple devices the logic of selection and loading implementations
          will be applied to each device accordingly to the system enumeration.

-----------------
Multiple Sessions
-----------------

Each |vpl_short_name| session can run exactly one instance of the DECODE, ENCODE, and
VPP functions. This is adequate for a simple transcoding operation. If the
application needs more than one instance of DECODE, ENCODE, or VPP
in a complex transcoding setting or needs more simultaneous transcoding
operations, the application can initialize multiple
|vpl_short_name| sessions created from one or several |vpl_short_name| implementations. 

The application can use multiple |vpl_short_name| sessions independently or run a “joined”
session. To join two sessions together, the application can use the function
:cpp:func:`MFXJoinSession`. Alternatively, the application can use the
:cpp:func:`MFXCloneSession` function to duplicate an existing session. Joined
|vpl_short_name| sessions work together as a single session, sharing all session
resources, threading control, and prioritization operations except hardware
acceleration devices and external allocators. When joined, the first session
(first join) serves as the parent session and will schedule execution resources
with all other child sessions. Child sessions rely on the parent session for
resource management.

.. important:: Applications can join sessions created from the same |vpl_short_name| 
               implementation only.

With joined sessions, the application can set the priority of session operations
through the :cpp:func:`MFXSetPriority` function. A lower priority session
receives fewer CPU cycles. Session priority does not affect hardware accelerated
processing.

After the completion of all session operations, the application can use the
:cpp:func:`MFXDisjoinSession` function to remove the joined state of a session.
Do not close the parent session until all child sessions are disjoined or
closed.
