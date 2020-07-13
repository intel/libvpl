===============================
Asynchronous Pipeline Operation
===============================

Pipelines can be constructed from decode, VPP, and encode operations.
Pipeline construction is done at runtime and can be dynamically changed, as shown in the
following example:

.. code-block:: c++

   mfxSyncPoint sp_d, sp_e;
   MFXVideoDECODE_DecodeFrameAsync(session,bs,work,&vin, &sp_d);
   if (going_through_vpp) {
      MFXVideoVPP_RunFrameVPPAsync(session,vin,vout, &sp_d);
      MFXVideoENCODE_EncodeFrameAsync(session,NULL,vout,bits2,&sp_e);
   } else {
      MFXVideoENCODE_EncodeFrameAsync(session,NULL,vin,bits2,&sp_e);
   }
   MFXVideoCORE_SyncOperation(session,sp_e,INFINITE);


The application only needs to synchronize after the last SDK function. 
Synchronizing after each operation is not required and can slow performance.

Any frame input must be synchronized before starting a pipeline.

Asynchronous pipelines can only be constructed from SDK operations.  If a pipeline
includes non-SDK components the operation delivering the data to the non-SDK 
component must be synchronized to ensure that results handed to the non-SDK 
operation are complete.

During the execution of an asynchronous pipeline, the application must consider
the input data in use and must not change it until the execution has completed.
The application must also consider output data unavailable until the execution
has finished. In addition, for encoders, the application must consider extended
and payload buffers in use while the input surface is locked.


The following pseudo code shows asynchronous **VPP** -> **ENCODE** pipeline
construction:

.. code-block:: c++

   mfxENCInput enc_in = ...;
   mfxENCOutput enc_out = ...;
   mfxSyncPoint sp_e, sp_n;
   mfxFrameSurface1* surface = get_frame_to_encode();
   mfxExtBuffer dependency;
   dependency.BufferId = MFX_EXTBUFF_TASK_DEPENDENCY;
   dependency.BufferSz = sizeof(mfxExtBuffer);

   enc_in.InSurface = surface;
   enc_out.ExtParam[enc_out.NumExtParam++] = &dependency;
   MFXVideoENC_ProcessFrameAsync(session, &enc_in, &enc_out, &sp_e);

   surface->Data.ExtParam[surface->Data.NumExtParam++] = &dependency;
   MFXVideoENCODE_EncodeFrameAsync(session, NULL, surface, &bs, &sp_n);

   MFXVideoCORE_SyncOperation(session, sp_n, INFINITE);
   surface->Data.NumExtParam--;

------------------------
Pipeline Error Reporting
------------------------

Each stage SDK function will return a synchronization point (sync point). 
These synchronization points are useful in tracking errors during asynchronous 
pipeline operation.

For example, assume the following pipeline:

.. graphviz::

   digraph {
      rankdir=LR;
      A->B->C;
   }

The application synchronizes on sync point **C**. If the error occurs before SDK 
function **C**, then the synchronization operation returns MFX_ERR_ABORTED. 

