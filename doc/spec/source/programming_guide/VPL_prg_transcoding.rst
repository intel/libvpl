.. SPDX-FileCopyrightText: 2019-2020 Intel Corporation
..
.. SPDX-License-Identifier: CC-BY-4.0

======================
Transcoding Procedures
======================

The application can use oneVPL encoding, decoding, and video processing
functions together for transcoding operations. This section describes the key
aspects of connecting two or more oneVPL functions together.

---------------------
Asynchronous Pipeline
---------------------

The application passes the output of an upstream oneVPL function to the input of
the downstream oneVPL function to construct an asynchronous pipeline. Pipeline
construction is done at runtime and can be dynamically changed, as shown in the
following example:

.. literalinclude:: ../snippets/prg_transcoding.c
   :language: c++
   :start-after: /*beg1*/
   :end-before: /*end1*/
   :lineno-start: 1

oneVPL simplifies the requirements for asynchronous pipeline synchronization.
The application only needs to synchronize after the last oneVPL function. Explicit
synchronization of intermediate results is not required and may slow performance.

oneVPL tracks dynamic pipeline construction and verifies dependency on input
and output parameters to ensure the execution order of the pipeline function.
In the previous example, oneVPL will ensure :cpp:func:`MFXVideoENCODE_EncodeFrameAsync`
does not begin its operation until :cpp:func:`MFXVideoDECODE_DecodeFrameAsync` or
:cpp:func:`MFXVideoVPP_RunFrameVPPAsync` has finished.

During the execution of an asynchronous pipeline, the application must consider
the input data as "in use" and must not change it until the execution has completed.
The application must also consider output data unavailable until the execution
has finished. In addition, for encoders, the application must consider extended
and payload buffers as "in use" while the input surface is locked.

oneVPL checks dependencies by comparing the input and output parameters of each
oneVPL function in the pipeline. Do not modify the contents of input and output
parameters before the previous asynchronous operation finishes. Doing so will
break the dependency check and can result in undefined behavior. An exception
occurs when the input and output parameters are structures, in which case
overwriting fields in the structures is allowed.

.. note:: The dependency check works on the pointers to the structures only.

There are two exceptions with respect to intermediate synchronization:

- If the input is from any asynchronous operation, the application must
  synchronize any input before calling the oneVPL :cpp:func:`MFXVideoDECODE_DecodeFrameAsync`
  function.
- When the application calls an asynchronous function to generate an output
  surface in video memory and passes that surface to a non-oneVPL component, it must
  explicitly synchronize the operation before passing the surface to the non-oneVPL
  component.

.. _surface_pool_alloc:

-----------------------
Surface Pool Allocation
-----------------------

When connecting API function **A** to API function **B**, the application must
take into account the requirements of both functions to calculate the number of
frame surfaces in the surface pool. Typically, the application can use the formula
**Na+Nb**, where **Na** is the frame surface requirements for oneVPL function **A**
output, and **Nb** is the frame surface requirements for oneVPL function **B** input.

For performance considerations, the application must submit multiple operations
and delay synchronization as much as possible, which gives oneVPL flexibility
to organize internal pipelining. For example, compare the following two operation
sequences, where the first sequence is the recommended order:

.. graphviz::
   :caption: Recommended operation sequence

   digraph {
      rankdir=LR;
      labelloc="t";
      label="Operation sequence 1";
      f1 [shape=record label="ENCODE(F1)" ];
      f2 [shape=record label="ENCODE(F2)" ];
      f3 [shape=record label="SYNC(F1)" ];
      f4 [shape=record label="SYNC(F2)" ];
      f1->f2->f3->f4;
   }


.. graphviz::
   :caption: Operation sequence - not recommended

   digraph {
      rankdir=LR;
      labelloc="t";
      label="Operation sequence 2";
      f1 [shape=record label="ENCODE(F1)" ];
      f2 [shape=record label="ENCODE(F2)" ];
      f3 [shape=record label="SYNC(F1)" ];
      f4 [shape=record label="SYNC(F2)" ];
      f1->f3->f2->f4;
   }



In this example, the surface pool needs additional surfaces to take into account
multiple asynchronous operations before synchronization. The application can use
the :cpp:member:`mfxVideoParam::AsyncDepth` field to inform a oneVPL function of
the number of asynchronous operations the application plans to perform before
synchronization. The corresponding oneVPL **QueryIOSurf** function will reflect
this number in the :cpp:member:`mfxFrameAllocRequest::NumFrameSuggested`
value. The following example shows a way of calculating the surface needs based
on :cpp:member:`mfxFrameAllocRequest::NumFrameSuggested` values:

.. literalinclude:: ../snippets/prg_transcoding.c
   :language: c++
   :start-after: /*beg2*/
   :end-before: /*end2*/
   :lineno-start: 1

------------------------
Pipeline Error Reporting
------------------------

During asynchronous pipeline construction, each pipeline stage function will
return a synchronization point (sync point). These synchronization points are
useful in tracking errors during the asynchronous pipeline operation.

For example, assume the following pipeline:

.. graphviz::

   digraph {
      rankdir=LR;
      A->B->C;
   }

The application synchronizes on sync point **C**. If the error occurs in
function **C**, then the synchronization returns the exact error code. If the
error occurs before function **C**, then the synchronization returns
:cpp:enumerator:`mfxStatus::MFX_ERR_ABORTED`. The application can then try to
synchronize on sync point **B**. Similarly, if the error occurs in function **B**,
the synchronization returns the exact error code, or else
:cpp:enumerator:`mfxStatus:: MFX_ERR_ABORTED`. The same logic applies if the
error occurs in function **A**.
