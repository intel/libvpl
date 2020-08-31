/*############################################################################
  # Copyright (C) 2017-2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef __MFXVIDEO_H__
#define __MFXVIDEO_H__
#include "mfxsession.h"
#include "mfxstructures.h"

#ifdef __cplusplus
extern "C"
{
#endif


MFX_PACK_BEGIN_STRUCT_W_PTR()
/*!
   The mfxFrameAllocator structure describes the callback functions Alloc, Lock, Unlock, GetHDL and Free that the SDK
   implementation might use for allocating internal frames. Applications that operate on OS-specific video surfaces must
   implement these callback functions.

   Using the default allocator implies that frame data passes in or out of SDK functions through pointers,
   as opposed to using memory IDs.

   The SDK behavior is undefined when using an incompletely defined external allocator. See the section Memory
   Allocation and External Allocators for additional information.
*/
typedef struct {
    mfxU32      reserved[4];
    mfxHDL      pthis;    /*!< Pointer to the allocator object. */

    /*!
       @brief This function allocates surface frames. For decoders, MFXVideoDECODE_Init calls Alloc only once. That call
              includes all frame allocation requests. For encoders, MFXVideoENCODE_Init calls Alloc twice: once for the
              input surfaces and again for the internal reconstructed surfaces.

              If two SDK components must share DirectX* surfaces, this function should pass the pre-allocated surface
              chain to SDK instead of allocating new DirectX surfaces. See the Surface Pool Allocation section for
              additional information.
       @param[in]  pthis    Pointer to the allocator object.
       @param[in]  request  Pointer to the mfxFrameAllocRequest structure that specifies the type and number of required frames.
       @param[out] response Pointer to the mfxFrameAllocResponse structure that retrieves frames actually allocated.
       @return
             MFX_ERR_NONE               The function successfully allocated the memory block. \n
             MFX_ERR_MEMORY_ALLOC       The function failed to allocate the video frames. \n
             MFX_ERR_UNSUPPORTED        The function does not support allocating the specified type of memory.
    */
    mfxStatus  (MFX_CDECL  *Alloc)    (mfxHDL pthis, mfxFrameAllocRequest *request, mfxFrameAllocResponse *response);

    /*!
       @brief This function locks a frame and returns its pointer.
       @param[in]  pthis    Pointer to the allocator object.
       @param[in]  mid      Memory block ID.
       @param[out] ptr      Pointer to the returned frame structure.
       @return
             MFX_ERR_NONE               The function successfully locked the memory block. \n
             MFX_ERR_LOCK_MEMORY        This function failed to lock the frame.
    */
    mfxStatus  (MFX_CDECL  *Lock)     (mfxHDL pthis, mfxMemId mid, mfxFrameData *ptr);

    /*!
       @brief This function unlocks a frame and invalidates the specified frame structure.
       @param[in]  pthis    Pointer to the allocator object.
       @param[in]  mid      Memory block ID.
       @param[out] ptr      Pointer to the frame structure; This pointer can be NULL.
       @return
             MFX_ERR_NONE               The function successfully locked the memory block.
    */
    mfxStatus  (MFX_CDECL  *Unlock)   (mfxHDL pthis, mfxMemId mid, mfxFrameData *ptr);

    /*!
       @brief This function returns the OS-specific handle associated with a video frame. If the handle is a COM interface,
              the reference counter must increase. The SDK will release the interface afterward.
       @param[in]  pthis    Pointer to the allocator object.
       @param[in]  mid      Memory block ID.
       @param[out] handle   Pointer to the returned OS-specific handle.
       @return
             MFX_ERR_NONE               The function successfully returned the OS-specific handle. \n
             MFX_ERR_UNSUPPORTED        The function does not support obtaining OS-specific handle..
    */
    mfxStatus  (MFX_CDECL  *GetHDL)   (mfxHDL pthis, mfxMemId mid, mfxHDL *handle);

    /*!
       @brief This function de-allocates all allocated frames.
       @param[in]  pthis    Pointer to the allocator object.
       @param[in]  response Pointer to the mfxFrameAllocResponse structure returned by the Alloc function.
       @return
             MFX_ERR_NONE               The function successfully de-allocated the memory block.
    */
    mfxStatus  (MFX_CDECL  *Free)     (mfxHDL pthis, mfxFrameAllocResponse *response);
} mfxFrameAllocator;
MFX_PACK_END()

/*!
   @brief 
      This function sets the external allocator callback structure for frame allocation.If the allocator argument is NULL, the SDK uses the 
      default allocator, which allocates frames from system memory or hardware devices.The behavior of the SDK is undefined if it uses this 
      function while the previous allocator is in use.A general guideline is to set the allocator immediately after initializing the session.
    
   @param[in]  session SDK session handle.
   @param[in] allocator   Pointer to the mfxFrameAllocator structure

   @return 
   MFX_ERR_NONE The function completed successfully. \n
*/
mfxStatus MFX_CDECL MFXVideoCORE_SetFrameAllocator(mfxSession session, mfxFrameAllocator *allocator);

/*!
   @brief 
    This function sets any essential system handle that SDK might use.
    If the specified system handle is a COM interface, the reference counter of the COM interface will increase. 
    The counter will decrease when the SDK session closes.
    
   @param[in] session SDK session handle.
   @param[in] type   Handle type
   @param[in] hdl   Handle to be set

   @return 
   MFX_ERR_NONE The function completed successfully. \n
   MFX_ERR_UNDEFINED_BEHAVIOR The same handle is redefined. 
                              For example, the function has been called twice with the same handle type or 
                              internal handle has been created by the SDK before this function call.
*/
mfxStatus MFX_CDECL MFXVideoCORE_SetHandle(mfxSession session, mfxHandleType type, mfxHDL hdl);

/*!
   @brief
    This function obtains system handles previously set by the MFXVideoCORE_SetHandle function.
    If the handler is a COM interface, the reference counter of the interface increases.
    The calling application must release the COM interface.

   @param[in] session SDK session handle.
   @param[in] type   Handle type
   @param[in] hdl  Pointer to the handle to be set

   @return
   MFX_ERR_NONE The function completed successfully. \n
   MFX_ERR_UNDEFINED_BEHAVIOR Specified handle type not found.
*/
mfxStatus MFX_CDECL MFXVideoCORE_GetHandle(mfxSession session, mfxHandleType type, mfxHDL *hdl);

/*!
   @brief
    This function returns information about current hardware platform.

   @param[in] session SDK session handle.
   @param[out] platform Pointer to the mfxPlatform structure

   @return
   MFX_ERR_NONE The function completed successfully. \n
*/
mfxStatus MFX_CDECL MFXVideoCORE_QueryPlatform(mfxSession session, mfxPlatform* platform);

/*!
   @brief
    This function initiates execution of an asynchronous function not already started and returns the status code after the specified asynchronous operation completes.
    If wait is zero, the function returns immediately

   @param[in] session SDK session handle.
   @param[in] syncp Sync point
   @param[in] wait  wait time in milliseconds

   @return
   MFX_ERR_NONE The function completed successfully. \n
   MFX_ERR_NONE_PARTIAL_OUTPUT   The function completed successfully, bitstream contains a portion of the encoded frame according to required granularity. \n
   MFX_WRN_IN_EXECUTION   The specified asynchronous function is in execution. \n
   MFX_ERR_ABORTED  The specified asynchronous function aborted due to data dependency on a previous asynchronous function that did not complete.   
*/
mfxStatus MFX_CDECL MFXVideoCORE_SyncOperation(mfxSession session, mfxSyncPoint syncp, mfxU32 wait);

/* MFXMemory */

/*!
   @brief 
      This function returns surface which can be used as input for VPP.  VPP should be initialized before this call. 
      Surface should be released with mfxFrameSurface1::FrameInterface.Release(...) after usage. Value of mfxFrameSurface1::Data.Locked for returned surface is 0.

    
   @param[in]  session SDK session handle.
   @param[out] surface   Pointer is set to valid mfxFrameSurface1 object.

   @return 
   MFX_ERR_NONE The function completed successfully. \n
   MFX_ERR_NULL_PTR If surface is NULL. \n
   MFX_ERR_INVALID_HANDLE If session was not initialized. \n
   MFX_ERR_NOT_INITIALIZED If VPP was not initialized (allocator needs to know surface size from somewhere). \n
   MFX_ERR_MEMORY_ALLOC In case of any other internal allocation error.

*/
mfxStatus MFX_CDECL MFXMemory_GetSurfaceForVPP(mfxSession session, mfxFrameSurface1** surface);

/*!
   @brief 
    This function returns surface which can be used as input for Encoder.  Encoder should be initialized before this call. 
    Surface should be released with mfxFrameSurface1::FrameInterface.Release(...) after usage. Value of mfxFrameSurface1::Data.Locked for returned surface is 0.


    
   @param[in]  session SDK session handle.
   @param[out] surface   Pointer is set to valid mfxFrameSurface1 object.

   @return 
   MFX_ERR_NONE The function completed successfully.\n
   MFX_ERR_NULL_PTR If surface is NULL.\n
   MFX_ERR_INVALID_HANDLE If session was not initialized.\n
   MFX_ERR_NOT_INITIALIZED If Encoder was not initialized (allocator needs to know surface size from somewhere).\n
   MFX_ERR_MEMORY_ALLOC In case of any other internal allocation error.

*/
mfxStatus MFX_CDECL MFXMemory_GetSurfaceForEncode(mfxSession session, mfxFrameSurface1** surface);

/*!
   @brief 
    This function returns surface which can be used as input for Decoder.  Decoder should be initialized before this call. 
    Surface should be released with mfxFrameSurface1::FrameInterface.Release(...) after usage. Value of mfxFrameSurface1::Data.Locked for returned surface is 0.'
    Note: this function was added to simplify transition from legacy surface management to proposed internal allocation approach. 
    Previously, user allocated surfaces for working pool and fed decoder with them in DecodeFrameAsync calls. With MFXMemory_GetSurfaceForDecode 
    it is possible to change the existing pipeline just changing source of work surfaces. 
    Newly developed applications should prefer direct usage of DecodeFrameAsync with internal allocation.'

   @param[in]  session SDK session handle.
   @param[out] surface   Pointer is set to valid mfxFrameSurface1 object.

   @return 
   MFX_ERR_NONE The function completed successfully.\n
   MFX_ERR_NULL_PTR If surface is NULL.\n
   MFX_ERR_INVALID_HANDLE If session was not initialized.\n
   MFX_ERR_NOT_INITIALIZED If Decoder was not initialized (allocator needs to know surface size from somewhere).\n
   MFX_ERR_MEMORY_ALLOC In case of any other internal allocation error.

*/
mfxStatus MFX_CDECL MFXMemory_GetSurfaceForDecode(mfxSession session, mfxFrameSurface1** surface);

/* VideoENCODE */

/*!
   @brief 
     This function works in either of four modes:
     
     If the in pointer is zero, the function returns the class configurability in the output structure. A non-zero value in each field of the output structure 
     that the SDK implementation can configure the field with Init.
     
     If the in parameter is non-zero, the function checks the validity of the fields in the input structure. Then the function returns the corrected values in 
     the output structure. If there is insufficient information to determine the validity or correction is impossible, the function zeroes the fields. 
     This feature can verify whether the SDK implementation supports certain profiles, levels or bitrates.
    
     If the in parameter is non-zero and mfxExtEncoderResetOption structure is attached to it, then the function queries for the outcome of the MFXVideoENCODE_Reset function 
     and returns it in the mfxExtEncoderResetOption structure attached to out. The query function succeeds if such reset is possible and returns error otherwise. Unlike other 
     modes that are independent of the SDK encoder state, this one checks if reset is possible in the present SDK encoder state. 
     This mode also requires completely defined mfxVideoParam structure, unlike other modes that support partially defined configurations. 
     See mfxExtEncoderResetOption description for more details.

     If the in parameter is non-zero and mfxExtEncoderCapability structure is attached to it, then the function returns encoder capability in mfxExtEncoderCapability structure 
     attached to out. It is recommended to fill in mfxVideoParam structure and set hardware acceleration device handle before calling the function in this mode.

     The application can call this function before or after it initializes the encoder. The CodecId field of the output structure is a mandated field (to be filled by the 
     application) to identify the coding standard.

   @param[in] session SDK session handle.
   @param[in] in   Pointer to the mfxVideoParam structure as input
   @param[out] out  Pointer to the mfxVideoParam structure as output

   @return
   MFX_ERR_NONE  The function completed successfully. \n
   MFX_ERR_UNSUPPORTED  The function failed to identify a specific implementation for the required features. \n
   MFX_WRN_PARTIAL_ACCELERATION  The underlying hardware does not fully support the specified video parameters.
                                 The encoding may be partially accelerated. Only SDK hardware implementations may return this status code. \n
   MFX_WRN_INCOMPATIBLE_VIDEO_PARAM  The function detected some video parameters were incompatible with others; incompatibility resolved.
*/
mfxStatus MFX_CDECL MFXVideoENCODE_Query(mfxSession session, mfxVideoParam *in, mfxVideoParam *out);

/*!
   @brief 
    This function returns minimum and suggested numbers of the input frame surfaces required for encoding initialization and their type.
    Init will call the external allocator for the required frames with the same set of numbers.
    The use of this function is recommended. For more information, see the section Working with hardware acceleration.
    This function does not validate I/O parameters except those used in calculating the number of input surfaces.
    
   @param[in] session SDK session handle.
   @param[in] par     Pointer to the mfxVideoParam structure as input
   @param[in] request Pointer to the mfxFrameAllocRequest structure as output

   @return  
   MFX_ERR_NONE  The function completed successfully. \n
   MFX_ERR_INVALID_VIDEO_PARAM  The function detected invalid video parameters. These parameters may be out of the valid range, or the combination of them 
                                resulted in incompatibility. Incompatibility not resolved. \n
   MFX_WRN_PARTIAL_ACCELERATION  The underlying hardware does not fully support the specified video parameters.
                                 The encoding may be partially accelerated. Only SDK hardware implementations may return this status code. \n
   MFX_WRN_INCOMPATIBLE_VIDEO_PARAM  The function detected some video parameters were incompatible with others; incompatibility resolved.
*/
mfxStatus MFX_CDECL MFXVideoENCODE_QueryIOSurf(mfxSession session, mfxVideoParam *par, mfxFrameAllocRequest *request);

/*!
   @brief 
    This function allocates memory and prepares tables and necessary structures for encoding. This function also does extensive validation to ensure if the 
    configuration, as specified in the input parameters, is supported.
    
   @param[in] session SDK session handle.
   @param[in] par Pointer to the mfxVideoParam structure

   @return  
   MFX_ERR_NONE  The function completed successfully. \n
   MFX_ERR_INVALID_VIDEO_PARAM  The function detected invalid video parameters. These parameters may be out of the valid range, or the combination of them
                                resulted in incompatibility. Incompatibility not resolved. \n
   MFX_WRN_PARTIAL_ACCELERATION  The underlying hardware does not fully support the specified video parameters. 
                                 The encoding may be partially accelerated. Only SDK hardware implementations may return this status code. \n
   MFX_WRN_INCOMPATIBLE_VIDEO_PARAM  The function detected some video parameters were incompatible with others; incompatibility resolved. \n
   MFX_ERR_UNDEFINED_BEHAVIOR  The function is called twice without a close;
*/
mfxStatus MFX_CDECL MFXVideoENCODE_Init(mfxSession session, mfxVideoParam *par);

/*!
   @brief 
    This function stops the current encoding operation and restores internal structures or parameters for a new encoding operation, possibly with new parameters.
    
   @param[in] session SDK session handle.
   @param[in] par   Pointer to the mfxVideoParam structure
   
   @return  
   MFX_ERR_NONE  The function completed successfully. \n
   MFX_ERR_INVALID_VIDEO_PARAM  The function detected invalid video parameters. These parameters may be out of the valid range, or the combination of them
                                resulted in incompatibility. Incompatibility not resolved. \n
   MFX_ERR_INCOMPATIBLE_VIDEO_PARAM  The function detected that provided by the application video parameters are incompatible with initialization parameters. 
                                     Reset requires additional memory allocation and cannot be executed. The application should close the SDK 
                                     component and then reinitialize it. \n
   MFX_WRN_INCOMPATIBLE_VIDEO_PARAM  The function detected some video parameters were incompatible with others; incompatibility resolved.
*/
mfxStatus MFX_CDECL MFXVideoENCODE_Reset(mfxSession session, mfxVideoParam *par);

/*!
   @brief 
    This function terminates the current encoding operation and de-allocates any internal tables or structures.
    
   @param[in] session SDK session handle.

   @return 
   MFX_ERR_NONE  The function completed successfully. \n
*/
mfxStatus MFX_CDECL MFXVideoENCODE_Close(mfxSession session);

/*!
   @brief 
    This function retrieves current working parameters to the specified output structure. If extended buffers are to be returned, the
    application must allocate those extended buffers and attach them as part of the output structure.
    The application can retrieve a copy of the bitstream header, by attaching the mfxExtCodingOptionSPSPPS structure to the mfxVideoParam structure.
    
   @param[in] session SDK session handle.
   @param[in] par Pointer to the corresponding parameter structure

   @return 
   MFX_ERR_NONE The function completed successfully. \n
*/
mfxStatus MFX_CDECL MFXVideoENCODE_GetVideoParam(mfxSession session, mfxVideoParam *par);

/*!
   @brief 
    This function obtains statistics collected during encoding.
    
   @param[in] session SDK session handle.
   @param[in] stat  Pointer to the mfxEncodeStat structure

   @return MFX_ERR_NONE The function completed successfully.
*/
mfxStatus MFX_CDECL MFXVideoENCODE_GetEncodeStat(mfxSession session, mfxEncodeStat *stat);

/*!
   @brief 
    This function takes a single input frame in either encoded or display order and generates its output bitstream. In the case of encoded ordering the mfxEncodeCtrl 
    structure must specify the explicit frame type. In the case of display ordering, this function handles frame order shuffling according to the GOP structure 
    parameters specified during initialization.
    
    Since encoding may process frames differently from the input order, not every call of the function generates output and the function returns MFX_ERR_MORE_DATA.
    If the encoder needs to cache the frame, the function locks the frame. The application should not alter the frame until the encoder unlocks the frame. 
    If there is output (with return status MFX_ERR_NONE), the return is a frame worth of bitstream.
    
    It is the calling application's responsibility to ensure that there is sufficient space in the output buffer. The value BufferSizeInKB in the
    mfxVideoParam structure at encoding initialization specifies the maximum possible size for any compressed frames. This value can also be obtained from 
    MFXVideoENCODE_GetVideoParam after encoding initialization. 
   
    To mark the end of the encoding sequence, call this function with a NULL surface pointer. Repeat the call to drain any remaining internally cached bitstreams
    (one frame at a time) until MFX_ERR_MORE_DATA is returned.

    This function is asynchronous.

   @param[in] session SDK session handle.
   @param[in] ctrl  Pointer to the mfxEncodeCtrl structure for per-frame encoding control; this parameter is optional(it can be NULL) if the encoder works in the display order mode.
   @param[in] surface  Pointer to the frame surface structure
   @param[out] bs   Pointer to the output bitstream
   @param[out] syncp  Pointer to the returned sync point associated with this operation

   @return 
   MFX_ERR_NONE The function completed successfully. \n
   MFX_ERR_NOT_ENOUGH_BUFFER  The bitstream buffer size is insufficient. \n
   MFX_ERR_MORE_DATA   The function requires more data to generate any output. \n
   MFX_ERR_DEVICE_LOST Hardware device was lost; See Working with Microsoft* DirectX* Applications section for further information. \n
   MFX_WRN_DEVICE_BUSY  Hardware device is currently busy. Call this function again in a few milliseconds.  \n
   MFX_ERR_INCOMPATIBLE_VIDEO_PARAM  Inconsistent parameters detected not conforming to Appendix A.  
*/
mfxStatus MFX_CDECL MFXVideoENCODE_EncodeFrameAsync(mfxSession session, mfxEncodeCtrl *ctrl, mfxFrameSurface1 *surface, mfxBitstream *bs, mfxSyncPoint *syncp);

/*!
   @brief
     This function works in one of two modes:

     1.If the in pointer is zero, the function returns the class configurability in the output structure. A non-zero value in each field of the output structure 
     indicates that the field is configurable by the SDK implementation with the MFXVideoDECODE_Init function).

     2.If the in parameter is non-zero, the function checks the validity of the fields in the input structure. Then the function returns the corrected values to 
     the output structure. If there is insufficient information to determine the validity or correction is impossible, the function zeros the fields. This 
     feature can verify whether the SDK implementation supports certain profiles, levels or bitrates.

     The application can call this function before or after it initializes the decoder. The CodecId field of the output structure is a mandated field 
     (to be filled by the application) to identify the coding standard.

   @param[in] session SDK session handle.
   @param[in] in   Pointer to the mfxVideoParam structure as input
   @param[out] out  Pointer to the mfxVideoParam structure as output

   @return  
   MFX_ERR_NONE  The function completed successfully. \n
   MFX_ERR_UNSUPPORTED  The function failed to identify a specific implementation for the required features. \n
   MFX_WRN_PARTIAL_ACCELERATION  The underlying hardware does not fully support the specified video parameters. 
                                 The decoding may be partially accelerated. Only SDK hardware implementations may return this status code. \n
   MFX_WRN_INCOMPATIBLE_VIDEO_PARAM  The function detected some video parameters were incompatible with others; incompatibility resolved.
*/

mfxStatus MFX_CDECL MFXVideoDECODE_Query(mfxSession session, mfxVideoParam *in, mfxVideoParam *out);

/*!
   @brief 
    This function parses the input bitstream and fills the mfxVideoParam structure with appropriate values, such as resolution and frame rate, for the Init function. 
    The application can then pass the resulting structure to the MFXVideoDECODE_Init function for decoder initialization.

    An application can call this function at any time before or after decoder initialization. If the SDK finds a sequence header in the bitstream, the function 
    moves the bitstream pointer to the first bit of the sequence header. Otherwise, the function moves the bitstream pointer close to the end of the bitstream buffer but leaves enough data in the buffer to avoid possible loss of start code.

    The CodecId field of the mfxVideoParam structure is a mandated field (to be filled by the application) to identify the coding standard.

    The application can retrieve a copy of the bitstream header, by attaching the mfxExtCodingOptionSPSPPS structure to the mfxVideoParam structure.

   @param[in] session SDK session handle.
   @param[in] bs   Pointer to the bitstream
   @param[in] par  Pointer to the mfxVideoParam structure

   @return 
   MFX_ERR_NONE The function successfully filled structure. It does not mean that the stream can be decoded by SDK. 
                The application should call MFXVideoDECODE_Query function to check if decoding of the stream is supported. \n
   MFX_ERR_MORE_DATA   The function requires more bitstream data \n
   MFX_ERR_UNSUPPORTED  CodecId field of the mfxVideoParam structure indicates some unsupported codec. \n
   MFX_ERR_INVALID_HANDLE  session is not initialized \n
   MFX_ERR_NULL_PTR  bs or par pointer is NULL.
*/   
mfxStatus MFX_CDECL MFXVideoDECODE_DecodeHeader(mfxSession session, mfxBitstream *bs, mfxVideoParam *par);

/*!
   @brief
    This function returns minimum and suggested numbers of the output frame surfaces required for decoding initialization and their type.
    Init will call the external allocator for the required frames with the same set of numbers.
    The use of this function is recommended. For more information, see the section Working with hardware acceleration.
    The CodecId field of the mfxVideoParam structure is a mandated field (to be filled by the application) to identify the coding standard.
    This function does not validate I/O parameters except those used in calculating the number of output surfaces.

   @param[in] session SDK session handle.
   @param[in] par     Pointer to the mfxVideoParam structure as input
   @param[in] request Pointer to the mfxFrameAllocRequest structure as output

   @return  
   MFX_ERR_NONE  The function completed successfully. \n
   MFX_ERR_INVALID_VIDEO_PARAM  The function detected invalid video parameters. These parameters may be out of the valid range, or the combination of them
                                resulted in incompatibility. Incompatibility not resolved. \n
   MFX_WRN_PARTIAL_ACCELERATION  The underlying hardware does not fully support the specified video parameters.
                                 The encoding may be partially accelerated. Only SDK hardware implementations may return this status code. \n
   MFX_WRN_INCOMPATIBLE_VIDEO_PARAM  The function detected some video parameters were incompatible with others; incompatibility resolved.
*/
mfxStatus MFX_CDECL MFXVideoDECODE_QueryIOSurf(mfxSession session, mfxVideoParam *par, mfxFrameAllocRequest *request);

/*!
   @brief
    This function allocates memory and prepares tables and necessary structures for encoding. This function also does extensive validation to ensure if the
    configuration, as specified in the input parameters, is supported.

   @param[in] session SDK session handle.
   @param[in] par Pointer to the mfxVideoParam structure

   @return  
   MFX_ERR_NONE  The function completed successfully. \n
   MFX_ERR_INVALID_VIDEO_PARAM  The function detected invalid video parameters. These parameters may be out of the valid range, or the combination of them
                                resulted in incompatibility. Incompatibility not resolved. \n
   MFX_WRN_PARTIAL_ACCELERATION  The underlying hardware does not fully support the specified video parameters.
                                 The encoding may be partially accelerated. Only SDK hardware implementations may return this status code. \n
   MFX_WRN_INCOMPATIBLE_VIDEO_PARAM  The function detected some video parameters were incompatible with others; incompatibility resolved. \n
   MFX_ERR_UNDEFINED_BEHAVIOR  The function is called twice without a close;
*/
mfxStatus MFX_CDECL MFXVideoDECODE_Init(mfxSession session, mfxVideoParam *par);

/*!
   @brief
    This function stops the current decoding operation and restores internal structures or parameters for a new decoding operation
    Reset serves two purposes:
    * It recovers the decoder from errors.
    * It restarts decoding from a new position
    The function resets the old sequence header (sequence parameter set in H.264, or sequence header in MPEG-2 and VC-1). The decoder will expect a new sequence header 
    before it decodes the next frame and will skip any bitstream before encountering the new sequence header.
   
   @param[in] session SDK session handle.
   @param[in] par   Pointer to the mfxVideoParam structure

   @return  
   MFX_ERR_NONE  The function completed successfully. \n
   MFX_ERR_INVALID_VIDEO_PARAM  The function detected that video parameters are wrong or they conflict with initialization parameters. Reset is impossible. \n
   MFX_ERR_INCOMPATIBLE_VIDEO_PARAM  The function detected that provided by the application video parameters are incompatible with initialization parameters.
                                     Reset requires additional memory allocation and cannot be executed. The application should close the SDK
                                     component and then reinitialize it. \n
   MFX_WRN_INCOMPATIBLE_VIDEO_PARAM  The function detected some video parameters were incompatible with others; incompatibility resolved.
*/
mfxStatus MFX_CDECL MFXVideoDECODE_Reset(mfxSession session, mfxVideoParam *par);

/*!
   @brief
    This function terminates the current decoding operation and de-allocates any internal tables or structures.

   @param[in] session SDK session handle.

   @return 
   MFX_ERR_NONE  The function completed successfully. \n
*/
mfxStatus MFX_CDECL MFXVideoDECODE_Close(mfxSession session);

/*!
   @brief
    This function retrieves current working parameters to the specified output structure. If extended buffers are to be returned, the
    application must allocate those extended buffers and attach them as part of the output structure.
    The application can retrieve a copy of the bitstream header, by attaching the mfxExtCodingOptionSPSPPS structure to the mfxVideoParam structure.

   @param[in] session SDK session handle.
   @param[in] par Pointer to the corresponding parameter structure

   @return 
   MFX_ERR_NONE The function completed successfully. \n
*/
mfxStatus MFX_CDECL MFXVideoDECODE_GetVideoParam(mfxSession session, mfxVideoParam *par);

/*!
   @brief
    This function obtains statistics collected during decoding.

   @param[in] session SDK session handle.
   @param[in] stat  Pointer to the mfxDecodeStat structure

   @return 
   MFX_ERR_NONE The function completed successfully. \n
*/
mfxStatus MFX_CDECL MFXVideoDECODE_GetDecodeStat(mfxSession session, mfxDecodeStat *stat);

/*!
   @brief 
    This function sets the decoder skip mode. The application may use it to increase decoding performance by sacrificing output quality. The rising of skip 
    level firstly results in skipping of some decoding operations like deblocking and then leads to frame skipping; firstly, B then P. Particular details are platform dependent.
    
   @param[in] session SDK session handle.
   @param[in] mode   Decoder skip mode. See the mfxSkipMode enumerator for details.

   @return 
   MFX_ERR_NONE The function completed successfully and the output surface is ready for decoding \n
   MFX_WRN_VALUE_NOT_CHANGED   The skip mode is not affected as the maximum or minimum skip range is reached.
*/
mfxStatus MFX_CDECL MFXVideoDECODE_SetSkipMode(mfxSession session, mfxSkipMode mode);

/*!
   @brief 
    This function extracts user data (MPEG-2) or SEI (H.264) messages from the bitstream. Internally, the decoder implementation stores encountered user data or 
    SEI messages. The application may call this function multiple times to retrieve the user data or SEI messages, one at a time.

    If there is no payload available, the function returns with payload->NumBit=0.
    
   @param[in] session SDK session handle.
   @param[in] ts  Pointer to the user data time stamp in units of 90 KHz; divide ts by 90,000 (90 KHz) to obtain the time in seconds; the time stamp matches the payload 
                  with a specific decoded frame.
   @param[in] payload  Pointer to the mfxPayload structure; the payload contains user data in MPEG-2 or SEI messages in H.264.

   @return 
   MFX_ERR_NONE The function completed successfully and the output buffer is ready for decoding \n
   MFX_ERR_NOT_ENOUGH_BUFFER  The payload buffer size is insufficient.
*/
mfxStatus MFX_CDECL MFXVideoDECODE_GetPayload(mfxSession session, mfxU64 *ts, mfxPayload *payload);

/*!
   @brief 
   This function decodes the input bitstream to a single output frame.

   The surface_work parameter provides a working frame buffer for the decoder. The application should allocate the working frame buffer, which stores decoded frames. 
   If the function requires caching frames after decoding, the function locks the frames and the application must provide a new frame buffer in the next call.

   If, and only if, the function returns MFX_ERR_NONE, the pointer surface_out points to the output frame in the display order. If there are no further frames, 
   the function will reset the pointer to zero and return the appropriate status code.

   Before decoding the first frame, a sequence header(sequence parameter set in H.264 or sequence header in MPEG-2 and VC-1) must be present. The function skips any 
   bitstreams before it encounters the new sequence header.

   The input bitstream bs can be of any size. If there are not enough bits to decode a frame, the function returns MFX_ERR_MORE_DATA, and consumes all input bits except if 
   a partial start code or sequence header is at the end of the buffer. In this case, the function leaves the last few bytes in the bitstream buffer. 
   If there is more incoming bitstream, the application should append the incoming bitstream to the bitstream buffer. Otherwise, the application should ignore the 
   remaining bytes in the bitstream buffer and apply the end of stream procedure described below.

   The application must set bs to NULL to signal end of stream. The application may need to call this function several times to drain any internally cached frames until the 
   function returns MFX_ERR_MORE_DATA.

   If more than one frame is in the bitstream buffer, the function decodes until the buffer is consumed. The decoding process can be interrupted for events such as if the 
   decoder needs additional working buffers, is readying a frame for retrieval, or encountering a new header. In these cases, the function returns appropriate status code 
   and moves the bitstream pointer to the remaining data.
   
   The decoder may return MFX_ERR_NONE without taking any data from the input bitstream buffer. If the application appends additional data to the bitstream buffer, it 
   is possible that the bitstream buffer may contain more than 1 frame. It is recommended that the application invoke the function repeatedly until the function 
   returns MFX_ERR_MORE_DATA, before appending any more data to the bitstream buffer. 
   This function is asynchronous.
 
   @param[in] session SDK session handle.
   @param[in] bs  Pointer to the input bitstream
   @param[in] surface_work  Pointer to the working frame buffer for the decoder
   @param[out] surface_out   Pointer to the output frame in the display order
   @param[out] syncp   Pointer to the sync point associated with this operation

   @return 
   MFX_ERR_NONE The function completed successfully and the output surface is ready for decoding \n
   MFX_ERR_MORE_DATA The function requires more bitstream at input before decoding can proceed. \n
   MFX_ERR_MORE_SURFACE The function requires more frame surface at output before decoding can proceed. \n
   MFX_ERR_DEVICE_LOST  Hardware device was lost; See the Working with Microsoft* DirectX* Applications section for further information. \n
   MFX_WRN_DEVICE_BUSY  Hardware device is currently busy. Call this function again in a few milliseconds. \n
   MFX_WRN_VIDEO_PARAM_CHANGED  The decoder detected a new sequence header in the bitstream. Video parameters may have changed. \n
   MFX_ERR_INCOMPATIBLE_VIDEO_PARAM  The decoder detected incompatible video parameters in the bitstream and failed to follow them. \n
   MFX_ERR_REALLOC_SURFACE  Bigger surface_work required. May be returned only if mfxInfoMFX::EnableReallocRequest was set to ON during initialization.
*/
mfxStatus MFX_CDECL MFXVideoDECODE_DecodeFrameAsync(mfxSession session, mfxBitstream *bs, mfxFrameSurface1 *surface_work, mfxFrameSurface1 **surface_out, mfxSyncPoint *syncp);
 
/* VideoVPP */

/*!
   @brief
     This function works in one of two modes:

     1.If the in pointer is zero, the function returns the class configurability in the output structure. A non-zero value in a field indicates that the 
       SDK implementation can configure it with Init.

     2.If the in parameter is non-zero, the function checks the validity of the fields in the input structure. Then the function returns the corrected values to
     the output structure. If there is insufficient information to determine the validity or correction is impossible, the function zeroes the fields.

     The application can call this function before or after it initializes the preprocessor.

   @param[in] session SDK session handle.
   @param[in] in   Pointer to the mfxVideoParam structure as input
   @param[out] out  Pointer to the mfxVideoParam structure as output

   @return  
   MFX_ERR_NONE  The function completed successfully. \n
   MFX_ERR_UNSUPPORTED  The SDK implementation does not support the specified configuration. \n 
   MFX_WRN_PARTIAL_ACCELERATION  The underlying hardware does not fully support the specified video parameters.
                                 The video processing may be partially accelerated. Only SDK hardware implementations may return this status code. \n
   MFX_WRN_INCOMPATIBLE_VIDEO_PARAM  The function detected some video parameters were incompatible with others; incompatibility resolved.
*/
mfxStatus MFX_CDECL MFXVideoVPP_Query(mfxSession session, mfxVideoParam *in, mfxVideoParam *out);

/*!
   @brief
    This function returns minimum and suggested numbers of the input frame surfaces required for video processing initialization and their type.
    The parameter request[0] refers to the input requirements; request[1] refers to output requirements. Init will call the external allocator for the 
    required frames with the same set of numbers.
    The use of this function is recommended. For more information, see the section Working with hardware acceleration.
    This function does not validate I/O parameters except those used in calculating the number of input surfaces.

   @param[in] session SDK session handle.
   @param[in] par     Pointer to the mfxVideoParam structure as input
   @param[in] request Pointer to the mfxFrameAllocRequest structure; use request[0] for input requirements and request[1] for output requirements for video processing.

   @return  
   MFX_ERR_NONE  The function completed successfully. \n
   MFX_ERR_INVALID_VIDEO_PARAM  The function detected invalid video parameters. These parameters may be out of the valid range, or the combination of them
                                resulted in incompatibility. Incompatibility not resolved. \n
   MFX_WRN_PARTIAL_ACCELERATION  The underlying hardware does not fully support the specified video parameters.
                                 The video processing may be partially accelerated. Only SDK hardware implementations may return this status code. \n
   MFX_WRN_INCOMPATIBLE_VIDEO_PARAM  The function detected some video parameters were incompatible with others; incompatibility resolved.
*/
mfxStatus MFX_CDECL MFXVideoVPP_QueryIOSurf(mfxSession session, mfxVideoParam *par, mfxFrameAllocRequest request[2]);

/*!
   @brief
    This function allocates memory and prepares tables and necessary structures for video processing. This function also does extensive validation to ensure if the
    configuration, as specified in the input parameters, is supported.

   @param[in] session SDK session handle.
   @param[in] par Pointer to the mfxVideoParam structure

   @return  
   MFX_ERR_NONE  The function completed successfully. \n
   MFX_ERR_INVALID_VIDEO_PARAM  The function detected invalid video parameters. These parameters may be out of the valid range, or the combination of them
                                resulted in incompatibility. Incompatibility not resolved. \n
   MFX_WRN_PARTIAL_ACCELERATION  The underlying hardware does not fully support the specified video parameters.
                                 The video processing may be partially accelerated. Only SDK hardware implementations may return this status code. \n
   MFX_WRN_INCOMPATIBLE_VIDEO_PARAM  The function detected some video parameters were incompatible with others; incompatibility resolved. \n
   MFX_ERR_UNDEFINED_BEHAVIOR  The function is called twice without a close. \n
   MFX_WRN_FILTER_SKIPPED    The VPP skipped one or more filters requested by the application.
*/
mfxStatus MFX_CDECL MFXVideoVPP_Init(mfxSession session, mfxVideoParam *par);

/*!
   @brief
    This function stops the current video processing operation and restores internal structures or parameters for a new operation

   @param[in] session SDK session handle.
   @param[in] par   Pointer to the mfxVideoParam structure

   @return  
   MFX_ERR_NONE  The function completed successfully. \n
   MFX_ERR_INVALID_VIDEO_PARAM  The function detected that video parameters are wrong or they conflict with initialization parameters. Reset is impossible. \n
   MFX_ERR_INCOMPATIBLE_VIDEO_PARAM  The function detected that provided by the application video parameters are incompatible with initialization parameters.
                                     Reset requires additional memory allocation and cannot be executed. The application should close the SDK
                                     component and then reinitialize it. \n
   MFX_WRN_INCOMPATIBLE_VIDEO_PARAM  The function detected some video parameters were incompatible with others; incompatibility resolved.
*/
mfxStatus MFX_CDECL MFXVideoVPP_Reset(mfxSession session, mfxVideoParam *par);

/*!
   @brief
    This function terminates the current video processing operation and de-allocates any internal tables or structures.

   @param[in] session SDK session handle.

   @return MFX_ERR_NONE  
   The function completed successfully. \n
*/
mfxStatus MFX_CDECL MFXVideoVPP_Close(mfxSession session);

/*!
   @brief
    This function retrieves current working parameters to the specified output structure. If extended buffers are to be returned, the
    application must allocate those extended buffers and attach them as part of the output structure.

   @param[in] session SDK session handle.
   @param[in] par Pointer to the corresponding parameter structure

   @return 
   MFX_ERR_NONE The function completed successfully. \n
*/
mfxStatus MFX_CDECL MFXVideoVPP_GetVideoParam(mfxSession session, mfxVideoParam *par);

/*!
   @brief
    This function obtains statistics collected during video processing.

   @param[in] session SDK session handle.
   @param[in] stat  Pointer to the mfxVPPStat structure

   @return 
   MFX_ERR_NONE The function completed successfully. \n
*/
mfxStatus MFX_CDECL MFXVideoVPP_GetVPPStat(mfxSession session, mfxVPPStat *stat);

/*!
   @brief
    This function processes a single input frame to a single output frame. Retrieval of the auxiliary data is optional; the encoding process may use it.
    The video processing process may not generate an instant output given an input. See section Video Processing Procedures for details on how to
    correctly send input and retrieve output.
    At the end of the stream, call this function with the input argument in=NULL to retrieve any remaining frames, until the function returns MFX_ERR_MORE_DATA.
    This function is asynchronous.

   @param[in] session SDK session handle.
   @param[in] in  Pointer to the input video surface structure
   @param[out] out  Pointer to the output video surface structure
   @param[in] aux  Optional pointer to the auxiliary data structure
   @param[out] syncp  Pointer to the output sync point

   @return 
   MFX_ERR_NONE The output frame is ready after synchronization. \n
   MFX_ERR_MORE_DATA Need more input frames before VPP can produce an output \n
   MFX_ERR_MORE_SURFACE The output frame is ready after synchronization. Need more surfaces at output for additional output frames available. \n
   MFX_ERR_DEVICE_LOST  Hardware device was lost; See the Working with Microsoft* DirectX* Applications section for further information. \n
   MFX_WRN_DEVICE_BUSY  Hardware device is currently busy. Call this function again in a few milliseconds.
*/
mfxStatus MFX_CDECL MFXVideoVPP_RunFrameVPPAsync(mfxSession session, mfxFrameSurface1 *in, mfxFrameSurface1 *out, mfxExtVppAuxData *aux, mfxSyncPoint *syncp);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
