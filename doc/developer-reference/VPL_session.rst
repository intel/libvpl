===========
API Session
===========

Before calling any API functions, the application must initialize the API
library and create an API session. An API session maintains context for the use
of any of **DECODE**, **ENCODE**, or **VPP** functions.

------------------------------------
|msdk_full_name| Dispatcher (Legacy)
------------------------------------

The function :cpp:func:`MFXInit` starts (initializes) an API session.
:cpp:func:`MFXClose` closes (de-initializes) the API session. To avoid memory
leaks, always call :cpp:func:`MFXClose` after :cpp:func:`MFXInit`.

The application can initialize a session as a software-based session
(:cpp:enumerator:`MFX_IMPL_SOFTWARE`) or a hardware-based session
(:cpp:enumerator:`MFX_IMPL_HARDWARE`). In the software scenario, the API
functions execute on a CPU, and in the hardware scenario, the API functions
use platform acceleration capabilities. For platforms that expose multiple
graphic devices, the application can initialize the API session on any
alternative graphic device (:cpp:enumerator:`MFX_IMPL_HARDWARE1`,..., :cpp:enumerator:`MFX_IMPL_HARDWARE4`).

The application can also initialize a session to be automatic (:cpp:enumerator:`MFX_IMPL_AUTO`
or :cpp:enumerator:`MFX_IMPL_AUTO_ANY`), instructing the dispatcher library to
detect the platform capabilities and choose the best API library available. After
initialization, the API returns the actual implementation through the
:cpp:func:`MFXQueryIMPL` function.

Internally, the dispatcher works as follows:

#. It searches for the shared library with the specific name:

   ========= =============== ====================================
   OS        Name            Description
   ========= =============== ====================================
   Linux\*   libmfxsw64.so.1 64-bit software-based implementation
   Linux     libmfxsw32.so.1 32-bit software-based implementation
   Linux     libmfxhw64.so.1 64-bit hardware-based implementation
   Linux     libmfxhw32.so.1 32-bit hardware-based implementation
   Windows\* libmfxsw64.dll  64-bit software-based implementation
   Windows   libmfxsw32.dll  32-bit software-based implementation
   Windows   libmfxhw64.dll  64-bit hardware-based implementation
   Windows   libmfxhw32.dll  32-bit hardware-based implementation
   ========= =============== ====================================

#. Once the library is loaded, the dispatcher obtains addresses of each API
   function. See table with the list of functions to export.

-----------------
oneVPL Dispatcher
-----------------

The oneVPL dispatcher extends the legacy dispatcher by providing additional
ability to select the appropriate implementation based on the implementation
capabilities. Implementation capabilities include information about supported
decoders, encoders, and VPP filters. For each supported encoder, decoder, and
filter, capabilities include information about supported memory types, color
formats, and image (frame) size in pixels.

The recommended approach to configure the dispatcher's capabilities
search filters and to create a session based on suitable implementation is as
follows:

#. Create loader (dispatcher function :cpp:func:`MFXLoad`).
#. Create loader's configuration (dispatcher function :cpp:func:`MFXCreateConfig`).
#. Add configuration properties (dispatcher function :cpp:func:`MFXSetConfigFilterProperty`).
#. Explore available implementations according (dispatcher function
   :cpp:func:`MFXEnumImplementations`).
#. Create suitable session (dispatcher function :cpp:func:`MFXCreateSession`).

The procedure to terminate an application is as follows:

#. Destroy session (function :cpp:func:`MFXClose`).
#. Destroy loader (dispatcher function :cpp:func:`MFXUnload`).

.. note:: Multiple loader instances can be created.

.. note:: Each loader may have multiple configuration objects associated with it.

.. important:: One configuration object can handle only one filter property.

.. note:: Multiple sessions can be created by using one loader object.

When the dispatcher searches for the implementation, it uses the following
priority rules:

#. Hardware implementation has priority over software implementation.
#. General hardware implementation has priority over VSI hardware implementation.
#. Highest API version has higher priority over lower API version.

.. note:: Implementation has priority over the API version. In other words, the
          dispatcher must return the implementation with the highest API
          priority (greater or equal to the implementation requested).

Dispatcher searches implementation in the following folders at runtime (in
priority order):

#. User-defined search folders.
#. oneVPL package.
#. Standalone |msdk_full_name| package (or driver).

A user has the ability to develop their own implementation and guide the oneVPL
dispatcher to load their implementation by providing a list of search folders.
The specific steps depend on which OS is used.

* Linux: User can provide colon separated list of folders in
  ONEVPL_SEARCH_PATH environmental variable.
* Windows: User can provide semicolon separated list of folders in
  ONEVPL_SEARCH_PATH environmental variable. Alternatively, the user can use the
  Windows registry.

The dispatcher supports different software implementations. The user can use
field :cpp:member:`mfxImplDescription::VendorID` or
:cpp:member:`mfxImplDescription::VendorImplID` or :cpp:member:`mfxImplDescription::ImplName`
to search for the particular implementation.

Internally, the dispatcher works as follows:

#. Dispatcher loads any shared library within the given search folders.
#. For each loaded library, the dispatcher tries to resolve address of the
   :cpp:func:`MFXQueryImplCapabilities` function to collect the implementation's
   capabilities.
#. Once the user has requested to create the session based on this implementation,
   the dispatcher obtains addresses of each API function. See table with the
   list of functions to export.

This table summarizes the list of environmental variables to control the
dispatcher behavior:

================== =============================================================
Variable           Purpose
================== =============================================================
ONEVPL_SEARCH_PATH List of user-defined search folders.
================== =============================================================


.. note:: Each implementation must support both dispatchers for backward
          compatibility with existing applications.

