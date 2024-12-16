.. SPDX-FileCopyrightText: 2019-2020 Intel Corporation
..
.. SPDX-License-Identifier: CC-BY-4.0

.. _config-interface:

=======================
Parameter Configuration
=======================

|vpl_short_name| API 2.10 introduces a new interface for configuring
|vpl_short_name| for encode, decode, or video processing. Applications may
optionally use the new function :cpp:member:`mfxConfigInterface::SetParameter`
to fill in data structures used for initialization, including
:cpp:struct:`mfxVideoParam` and extension buffers of type
:cpp:struct:`mfxExtBuffer`.

:cpp:member:`mfxConfigInterface::SetParameter` accepts as input key-value pairs
of ``char *`` strings, converts these strings to appropriate C data types, and writes the
results into the application-provided initialization structures. This can provide
a simpler and more flexible initialization method for applications which accept
user input in the form of strings, or which store configuration information in
formats such as XML, YAML, or JSON.

Applications may freely mix use of
:cpp:member:`mfxConfigInterface::SetParameter` with standard C-style
initialization of the same structures. Additionally, the use of
:cpp:member:`mfxConfigInterface::SetParameter` facilitates support of new
parameters which may be added to |vpl_short_name| API in the future. When new
extension buffers are added to |vpl_short_name| API,
:cpp:member:`mfxConfigInterface::SetParameter` will enable the application to
allocate buffers of the appropriate size, and initialize them with the required
values, without recompiling the application.

------------------------------------
Setting a parameter in mfxVideoParam
------------------------------------

The following code snippet shows an example of setting a parameter in the structure :cpp:struct:`mfxVideoParam`.

.. literalinclude:: ../snippets/prg_config.cpp
   :language: c++
   :start-after: /*beg1*/
   :end-before: /*end1*/
   :lineno-start: 1

------------------------------------------
Setting a parameter in an extension buffer
------------------------------------------

The following code snippet shows an example of setting a parameter in the extension buffer
:cpp:struct:`mfxExtHEVCParam` and attaching it to the structure :cpp:struct:`mfxVideoParam`.

When setting a parameter which maps to an extension buffer, the function first
checks whether the required extension buffer has been attached to the provided
:cpp:struct:`mfxVideoParam`.  If so, |vpl_short_name| will update the
corresponding field in the extension buffer and return MFX_ERR_NONE.

If the required extension buffer is not attached, |vpl_short_name| will instead
return MFX_ERR_MORE_EXTBUFFER.  If this happens, the application is required to
allocate an extension buffer whose size and buffer ID are indicated by the
ext_buffer parameter returned from the call to
:cpp:member:`mfxConfigInterface::SetParameter`. This extension buffer must then
be attached to :cpp:struct:`mfxVideoParam`, then
:cpp:member:`mfxConfigInterface::SetParameter` should be called again with the
same arguments. If the key and value strings represent a valid parameter in the
newly-attached extension buffer, the function will now return MFX_ERR_NONE.

.. literalinclude:: ../snippets/prg_config.cpp
   :language: c++
   :start-after: /*beg2*/
   :end-before: /*end2*/
   :lineno-start: 1

