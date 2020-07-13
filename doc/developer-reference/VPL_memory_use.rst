=========================================
Memory Allocation and External Allocators
=========================================

There are two models of memory management: internal and external.  Internal is not 
available in the current release and mentioned here as a preview.  External
memory management is demonstrated in the example code.


--------------------------
External Memory Management
--------------------------

In the external memory model the application must allocate sufficient memory for:

 *  input/output frames
 *  intermediate work surfaces 
 *  extra frames required for AsyncDepth>1
 *  additional frames as required by application-level locking requirements

The application is responsible for the lifecycle of these surfaces as well as 
checking the locked state when searching for usable surfaces to submit to API 
operations.

Allocation management with callbacks will be supported in future releases.
However, this is not required for system memory.  For CPU operation surfaces can 
be allocated as any other CPU memory.  To best align with internal assumptions 
about memory layout, it is recommended to allocate memory for each frame as a 
contiguous unit.


Frame Surface Locking
---------------------

During encoding, decoding, or video processing there are many scenerios where 
input does not result in immediate output.  Frame dependencies, asynchronous 
operation, etc. require frame surface locking, so managing surface locking is 
a normal part of working with external surfaces. 

The mfxFrameData struct (part of mfxFrameSurface1, the struct used to represent
frames) contains a Locked member.  This can be used by the application to 
query the internal lock status of the surface.  If the value of Locked is not 
zero the surface should not be used.

The example below illustrates a search for a free surface in a surface pool.

.. code-block:: c++

   // Return index of free surface in given pool
   int GetFreeSurfaceIndex(mfxFrameSurface1 *SurfacesPool, mfxU16 nPoolSize) {
     for (mfxU16 i = 0; i < nPoolSize; i++) {
           if (0 == SurfacesPool[i].Data.Locked)
               return i;
      }
      return MFX_ERR_NOT_FOUND;
   }


The Locked value should be treated as a black box by the application.  The 
application can read this value but should never modify it.


.. attention:: Modifying the Locked counter is not recommended.

Another note: the value of Locked is not intended to be used for synchronization.
MFXVideoCORE_SyncOperation should be used at the end of pipelines to be sure 
the SDK is done writing the result.



Surface Pool Allocation
-----------------------

When using external allocation applications must set up pools of sufficient size.
This means that developers writing applications must use a mental model of how 
the surfaces will flow through the pipeline to set up correct accounting.

For example, in a decode->VPP->encode pipeline there are two pools:

* decode->VPP in 
* VPP out -> encode 

The SDK provides query functions to tell the application the number of surfaces 
required based on input parameters.

.. code-block:: 

   MFXVideoDecode_QueryIOSurf
   MFXVideoVPP_QueryIOSurf
   MFXVideoEncode_QueryIOSurf


The size of the decode->VPP pool is the sum of the response.NumFrameSuggested values 
from decode and VPP in.  The size of the VPP out->encode pool is the sum of the 
NumFrameSuggested values from VPP out and Encode.

Note: AsyncDepth>1 and any external locking add additional complexity to calculating 
the pool size.  
* AsyncDepth is a parameter for each operation, which can lead to overcounting 
* Any additional locking done by the application must be added to the pool size 

In general, unless memory is tight it is better for the pool size to be too big than 
too small.


--------------------------
Internal Memory Management
--------------------------

.. attention:: Internal is not available in the current release and mentioned here as a preview.

In the internal memory management model, the SDK provides interface functions for
frame allocation:

:cpp:func:`MFXMemory_GetSurfaceForVPP`

:cpp:func:`MFXMemory_GetSurfaceForEncode`

:cpp:func:`MFXMemory_GetSurfaceForDecode`

These functions are used together with :cpp:struct:`mfxFrameSurfaceInterface`
for surface management. The surface returned by these function is a reference
counted object and the application must call :cpp:member:`mfxFrameSurfaceInterface::Release`
after finishing all operations with the surface. In this model the application
doesn't need to handle external allocation details.

Another method to obtain an internally allocated surface is to call
:cpp:func:`MFXVideoDECODE_DecodeFrameAsync` with a working surface equal to NULL. In
this scenario, the Decoder will allocate a new refcountable
:cpp:struct:`mfxFrameSurface1` and return it to the user. All assumed contracts
with the user are similar to functions MFXMemory_GetSurfaceForXXX.





mfxFrameSurfaceInterface
------------------------
 
oneVPL API version 2.0 introduces :cpp:struct:`mfxFrameSurfaceInterface`. This
interface is a set of callback functions to manage the lifetime of allocated
surfaces, get access to pixel data, and obtain native handles and device
abstractions (if suitable). 

This interface defines mfxFrameSurface1 as a reference counted object which can
be allocated by the SDK or application. The application must follow the general
rules of operations with reference counted objects. For example, when surfaces
are allocated by the SDK during MFXVideoDECODE_DecodeFrameAsync or with help of
MFXMemory_GetSurfaceForVPP or MFXMemory_GetSurfaceForEncode, the application must
call the corresponding mfxFrameSurfaceInterface->(\*Release) for the surfaces
that are no longer in use.

Using mfxFrameSurface1::mfxFrameSurfaceInterface is recommended if present, 
instead of directly accessing :cpp:struct:`mfxFrameSurface1` structure
members or call external allocator callback functions if set.

The following example shows the usage of :cpp:struct:`mfxFrameSurfaceInterface`
for memory sharing:

.. code-block:: c++

    // let decode frame and try to access output optimal way.
    sts = MFXVideoDECODE_DecodeFrameAsync(session, NULL, NULL, &outsurface, &syncp);
    if (MFX_ERR_NONE == sts)
    {
        outsurface->FrameInterface->(*GetDeviceHandle)(outsurface, &device_handle, &device_type);

        // if application or component can use the target device_type 
        // and sharing memory created by device_handle is possible
        if (isDeviceTypeCompatible(device_type) && isPossibleForMemorySharing(device_handle)) {

            // get native handle and type
            outsurface->FrameInterface->(*GetNativeHandle)(outsurface, &resource, &resource_type);

            if (isResourceTypeCompatible(resource_type)) {
                //use memory directly
                ProcessNativeMemory(resource);
                outsurface->FrameInterface->(*Release)(outsurface);
            }

        }

        // Application or component is not aware about DeviceHandle 
        // or Resource type need to map to system memory.
        outsurface->FrameInterface->(*Map)(outsurface, MFX_MAP_READ);
        ProcessSystemMemory(outsurface);
        outsurface->FrameInterface->(*Unmap)(outsurface);
        outsurface->FrameInterface->(*Release)(outsurface);
    }

