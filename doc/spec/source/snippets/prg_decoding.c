/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "mfxdefs.h"
#include "mfxvideo.h"
#include "mfxmvc.h"

/* These macro required for code compilation. */
#define INFINITE 0x7FFFFFFF
#define UNUSED_PARAM(x) (void)(x)

mfxSession session;
mfxBitstream *bitstream, *bits, *bitstream_smem, *bitstream_vmem;
mfxVideoParam init_param, param;
mfxFrameAllocRequest request;
mfxStatus sts;
mfxFrameSurface1* work, *disp;
mfxSyncPoint syncp;
mfxU32 desired_fourcc;
mfxU16 desired_downscale_ratioh, desired_downscale_ratiow;

// Define a structure to encapsulate both the linked list node and the size of the list
typedef struct SyncPointList {
    mfxSyncPoint syncPoint;            // Store sync point
    mfxFrameSurface1* frameSurface;    // Store frame surface pointer
    struct SyncPointList* next;        // Pointer to the next node in the list
    int size;                       // Size of the linked list
} SyncPointList;

// Initialize the list
void init_sync_point_list(SyncPointList* list) {
    list->next = NULL;    // Set the next pointer to NULL (empty list)
    list->size = 0;       // Initialize the size to 0
}

// Add a sync point and frame surface to the end of the list
void push_sync_point(SyncPointList* list, mfxSyncPoint syncPoint, mfxFrameSurface1* frameSurface) {
    // Traverse the list to find the last node
    SyncPointList* temp = list;
    while (temp->next != NULL) {
        temp = temp->next;
    }

    // Allocate memory for the new node
    SyncPointList* newNode = (SyncPointList*)malloc(sizeof(SyncPointList));
    newNode->syncPoint = syncPoint;
    newNode->frameSurface = frameSurface;
    newNode->next = NULL;  // The new node's next pointer is NULL

    // Link the new node to the end of the list
    temp->next = newNode;

    // Update the size of the list
    list->size++;
}

// Remove a sync point and frame surface from the head of the list
void pop_sync_point(SyncPointList* list) {
    if (list->next != NULL) {  // If the list is not empty
        SyncPointList* temp = list->next;
        list->next = list->next->next;  // Update the head to the next node
        free(temp);  // Free the old head node

        // Update the size of the list
        list->size--;
    }
}

static void allocate_pool_of_frame_surfaces(int nFrames)
{
    UNUSED_PARAM(nFrames);
    return;
}

static void allocate_surface_pool(mfxFrameAllocRequest* request)
{
    UNUSED_PARAM(request);
    return;
}

static int end_of_stream()
{
    return 1;
}

static void append_more_bitstream(mfxBitstream *bs)
{
    UNUSED_PARAM(bs);
    return;
}

static void find_free_surface_from_the_pool(mfxFrameSurface1 **pool)
{
    UNUSED_PARAM(pool);
    return;
}

static void realloc_surface(mfxFrameSurface1* work, mfxFrameInfo FrameInfo)
{
    UNUSED_PARAM(work);
    UNUSED_PARAM(FrameInfo);
    return;
}

static void do_something_with_decoded_frame(mfxFrameSurface1* disp)
{
    UNUSED_PARAM(disp);
    return;
}

static void free_pool_of_frame_surfaces()
{
    return;
}

static void add_surface_to_pool(mfxFrameSurface1* surf)
{
    UNUSED_PARAM(surf);
    return;
}

static void get_next_surface_from_pool(mfxFrameSurface1 **surface)
{
    UNUSED_PARAM(surface);
    return;
}
/* end of internal stuff */

static void prg_decoding1 () {
/*beg1*/
MFXVideoDECODE_Init(session, &init_param);
sts=MFX_ERR_MORE_DATA;
for (;;) {
   if (sts==MFX_ERR_MORE_DATA && !end_of_stream())
      append_more_bitstream(bitstream);
   bits=(end_of_stream())?NULL:bitstream;
   sts=MFXVideoDECODE_DecodeFrameAsync(session,bits,NULL,&disp,&syncp);
   if (end_of_stream() && sts==MFX_ERR_MORE_DATA) break;
   // skipped other error handling
   if (sts==MFX_ERR_NONE) {
      disp->FrameInterface->Synchronize(disp, INFINITE); // or MFXVideoCORE_SyncOperation(session,syncp,INFINITE)
      do_something_with_decoded_frame(disp);
      disp->FrameInterface->Release(disp);
   }
}
MFXVideoDECODE_Close(session);
/*end1*/
}

static void prg_decoding2 () {
/*beg2*/
MFXVideoDECODE_QueryIOSurf(session, &init_param, &request);
MFXVideoDECODE_Init(session, &init_param);
for (int i = 0; i < request.NumFrameSuggested; i++) {
    MFXMemory_GetSurfaceForDecode(session, &work);
    add_surface_to_pool(work);
}
sts=MFX_ERR_MORE_DATA;
for (;;) {
   if (sts==MFX_ERR_MORE_DATA && !end_of_stream())
      append_more_bitstream(bitstream);
   bits=(end_of_stream())?NULL:bitstream;
   // application logic to distinguish free and busy surfaces
   find_free_surface_from_the_pool(&work);
   sts=MFXVideoDECODE_DecodeFrameAsync(session,bits,work,&disp,&syncp);
   if (end_of_stream() && sts==MFX_ERR_MORE_DATA) break;
   // skipped other error handling
   if (sts==MFX_ERR_NONE) {
      disp->FrameInterface->Synchronize(disp, INFINITE); // or MFXVideoCORE_SyncOperation(session,syncp,INFINITE)
      do_something_with_decoded_frame(disp);
      disp->FrameInterface->Release(disp);
   }
}
for (int i = 0; i < request.NumFrameSuggested; i++) {
    get_next_surface_from_pool(&work);
    work->FrameInterface->Release(work);
}
MFXVideoDECODE_Close(session);
/*end2*/
}


static void prg_decoding3 () {
/*beg3*/
MFXVideoDECODE_DecodeHeader(session, bitstream, &init_param);
MFXVideoDECODE_QueryIOSurf(session, &init_param, &request);
allocate_pool_of_frame_surfaces(request.NumFrameSuggested);
MFXVideoDECODE_Init(session, &init_param);
sts=MFX_ERR_MORE_DATA;
for (;;) {
   if (sts==MFX_ERR_MORE_DATA && !end_of_stream())
      append_more_bitstream(bitstream);
   find_free_surface_from_the_pool(&work);
   bits=(end_of_stream())?NULL:bitstream;
   sts=MFXVideoDECODE_DecodeFrameAsync(session,bits,work,&disp,&syncp);
   if (sts==MFX_ERR_MORE_SURFACE) continue;
   if (end_of_stream() && sts==MFX_ERR_MORE_DATA) break;
   if (sts==MFX_ERR_REALLOC_SURFACE) {
      MFXVideoDECODE_GetVideoParam(session, &param);
      realloc_surface(work, param.mfx.FrameInfo);
      continue;
   }
   // skipped other error handling
   if (sts==MFX_ERR_NONE) {
      disp->FrameInterface->Synchronize(disp, INFINITE); // or MFXVideoCORE_SyncOperation(session,syncp,INFINITE)
      do_something_with_decoded_frame(disp);
   }
}
MFXVideoDECODE_Close(session);
free_pool_of_frame_surfaces();
/*end3*/
}

static void prg_decoding4 () {
/*beg4*/
sts=MFX_ERR_MORE_DATA;
for (;;) {
   if (sts==MFX_ERR_MORE_DATA && !end_of_stream())
      append_more_bitstream(bitstream);
   bits=(end_of_stream())?NULL:bitstream;
   sts=MFXVideoDECODE_DecodeFrameAsync(session,bits,NULL,&disp,&syncp);
   if (sts==MFX_ERR_MORE_SURFACE) continue;
   if (end_of_stream() && sts==MFX_ERR_MORE_DATA) break;
   // skipped other error handling
   if (sts==MFX_ERR_NONE) {
      disp->FrameInterface->Synchronize(disp, INFINITE); // or MFXVideoCORE_SyncOperation(session,syncp,INFINITE)
      do_something_with_decoded_frame(disp);
      disp->FrameInterface->Release(disp);
   }
}
/*end4*/
}

static void prg_decoding5 () {
/*beg5*/
mfxExtBuffer *eb[2];
mfxExtMVCSeqDesc  seq_desc;
mfxVideoParam init_param;

init_param.ExtParam=(mfxExtBuffer **)&eb;
init_param.NumExtParam=1;
eb[0]=(mfxExtBuffer *)&seq_desc;
MFXVideoDECODE_DecodeHeader(session, bitstream, &init_param);

/* select views to decode */
mfxExtMVCTargetViews tv;
init_param.NumExtParam=2;
eb[1]=(mfxExtBuffer *)&tv;

/* initialize decoder */
MFXVideoDECODE_Init(session, &init_param);

/* perform decoding */
for (;;) {
    MFXVideoDECODE_DecodeFrameAsync(session, bits, work, &disp, &syncp);
    disp->FrameInterface->Synchronize(disp, INFINITE); // or MFXVideoCORE_SyncOperation(session,syncp,INFINITE)
}

/* close decoder */
MFXVideoDECODE_Close(session);
/*end5*/
}

static void prg_decoding6 () {
/*beg6*/
MFXVideoDECODE_Init(session, &init_param);
sts=MFX_ERR_MORE_DATA;
for (;;) {
   if (sts==MFX_ERR_MORE_DATA && !end_of_stream())
      append_more_bitstream(bitstream);
   bits=(end_of_stream())?NULL:bitstream;
   MFXMemory_GetSurfaceForDecode(session, &work);
   sts=MFXVideoDECODE_DecodeFrameAsync(session,bits,work,&disp,&syncp);
   work->FrameInterface->Release(work);
   if (end_of_stream() && sts==MFX_ERR_MORE_DATA) break;
   // skipped other error handling
   if (sts==MFX_ERR_NONE) {
      disp->FrameInterface->Synchronize(disp, INFINITE); // or MFXVideoCORE_SyncOperation(session,syncp,INFINITE)
      do_something_with_decoded_frame(disp);
      disp->FrameInterface->Release(disp);
   }
}
MFXVideoDECODE_Close(session);
/*end6*/
}

static void prg_decoding7 () {
/*beg7*/
SyncPointList syncPointsList;
init_sync_point_list(&syncPointsList); // Initialize the list
/* initialize decoder */
MFXVideoDECODE_Init(session, &init_param);
/* perform decoding */
for (;;) {
   // Asynchronously decode a frame
   sts = MFXVideoDECODE_DecodeFrameAsync(session, bits, work, &disp, &syncp);
   if (sts == MFX_ERR_NONE) {

        // Add the sync point to the linked list
        if(disp != NULL)
             push_sync_point(&syncPointsList, syncp, disp);

        // If the number of elements in the list exceeds AsyncDepth or there is no more data to decode
        if (syncPointsList.size >= init_param.AsyncDepth || !bits) {
            while (syncPointsList.next != NULL) {
                // Wait for the sync point to complete
                sts = MFXVideoCORE_SyncOperation(session, syncPointsList.next->syncPoint, MFX_INFINITE);
                if (sts == MFX_ERR_NONE) {
                    pop_sync_point(&syncPointsList);  // Remove the processed item
                } else {
                    // If synchronization fails, exit the loop
                    break;
                }
            }
        }
    }
}
/* close decoder */
MFXVideoDECODE_Close(session);
/*end7*/
}

/*beg8*/
static mfxBitstream* get_bitstream_buffer(mfxMemoryInterface *memoryInterface)
{
    if (memoryInterface && memoryInterface->GetBitstreamBuffer && memoryInterface->GetBitstreamBuffer(memoryInterface, bitstream_vmem) == MFX_ERR_NONE)
    {
        //bitstream buffer in video memory will be allocated and update to bitstream_vmem.
        //MFX_BITSTREAM_IN_VIDEO_MEMORY bit of bitstream_vmem->DataFlag will be set by runtime.
        //MFX_BITSTREAM_COMPLETE_FRAME should be set by App. App should be aware about this requirement.
        //This will achieve optimal performance for the supported-codec.
        bitstream_vmem->DataFlag |= MFX_BITSTREAM_COMPLETE_FRAME;
        return bitstream_vmem;
    }
    else
        return bitstream_smem; //fallback to system memory if video memory is not supported, managed by App.
}
/*end8*/

static void prg_decoding9 () {
/*beg9*/
/*get memory interface with GetBitstreamBuffer function*/
mfxMemoryInterface *memoryInterface = NULL;
MFXGetMemoryInterface(session, &memoryInterface);

/* initialize decoder */
MFXVideoDECODE_Init(session, &init_param);

sts=MFX_ERR_MORE_DATA;

/* perform decoding */
for (;;) {
    if ((sts==MFX_ERR_MORE_DATA || !bitstream || !bitstream->Data) && !end_of_stream())
    {
      bitstream = get_bitstream_buffer(memoryInterface);
      //Same process for updating the bitstream for video memory and system memory.
      //The video memory bitstream buffer has been mapped for CPU access in the runtime.
      append_more_bitstream(bitstream);
    }
   // Asynchronously decode a frame
   sts = MFXVideoDECODE_DecodeFrameAsync(session, bitstream, work, &disp, &syncp);
   if (sts == MFX_ERR_NONE) {
       //if bitstream is in video memory, bitstream->Data will be set to nullptr by MFXVideoDECODE_DecodeFrameAsync.
       //do other things such as MFXVideoCORE_SyncOperation.
    }
}
/* close decoder */
MFXVideoDECODE_Close(session);
/*end9*/
}

/*beg10*/
static void update_csc_scaling_param(mfxVideoParam *p_video_param, mfxExtBuffer **pp_ext_params, mfxExtDecVideoProcessing *p_dec_vp_param)
{
    /*****check whether desired_fourcc is supported*****/
    /*init the output fourcc to original default foucc*/
    mfxU32 output_fourcc = p_video_param->mfx.FrameInfo.FourCC;
    mfxU32 output_chromaformat = p_video_param->mfx.FrameInfo.ChromaFormat;
    mfxU32 csc_caps = (p_video_param->mfx.OutputCscCapsHigh << 16) + p_video_param->mfx.OutputCscCapsLow;
    bool is_fourcc_supported = false;

    switch (desired_fourcc)
    {
        case MFX_FOURCC_Y216:
            is_fourcc_supported = csc_caps & DEC_CSC_Y216_SUPPORTED_BIT;
            if (is_fourcc_supported)
            {
                output_fourcc = MFX_FOURCC_Y216;
                output_chromaformat = MFX_CHROMAFORMAT_YUV422;
            }
            break;
        case MFX_FOURCC_YUY2:
            is_fourcc_supported = csc_caps & DEC_CSC_YUY2_SUPPORTED_BIT;
            if (is_fourcc_supported)
            {
                output_fourcc = MFX_FOURCC_YUY2;
                output_chromaformat = MFX_CHROMAFORMAT_YUV422;
            }
            break;
        //...other FourCC
        default:
            is_fourcc_supported = false;
            break;
    }

    /*****check whether desired_downscale_ratio is supported*****/
    bool is_downscale_ratio_supported = false;
    /*init the ratio to 1 which means no scaling is needed*/
    mfxU16 output_downscale_ratiow = 1;
    mfxU16 output_downscale_ratioh = 1;

    if (desired_downscale_ratiow == 2 && desired_downscale_ratioh == 2)
    {
        is_downscale_ratio_supported = p_video_param->mfx.OutputScalingRatioCaps & DEC_DOWNSCALING_RATIO_W2_H2_SUPPORTED_BIT;
        if (is_downscale_ratio_supported)
        {
            output_downscale_ratiow = 2;
            output_downscale_ratioh = 2;
        }
    }
    else if (desired_downscale_ratiow == 4 && desired_downscale_ratioh == 4)
    {
        is_downscale_ratio_supported = p_video_param->mfx.OutputScalingRatioCaps & DEC_DOWNSCALING_RATIO_W4_H4_SUPPORTED_BIT;
        if (is_downscale_ratio_supported)
        {
            output_downscale_ratiow = 4;
            output_downscale_ratioh = 4;
        }
    }
    //....other ratio check

    /*****Set the desired fourcc and desired downscale ratio after the caps check*****/
    /*Csc or/and downscaling is needed. video_param should be updated. The desired fourcc and ratio should be set to mfxExtDecVideoProcessing extension buffer to tell runtime*/
    if (is_fourcc_supported || is_downscale_ratio_supported)
    {
        //input is original size
        p_dec_vp_param->In.CropW = p_video_param->mfx.FrameInfo.CropW;
        p_dec_vp_param->In.CropH = p_video_param->mfx.FrameInfo.CropH;
        p_dec_vp_param->In.CropX = p_video_param->mfx.FrameInfo.CropX;
        p_dec_vp_param->In.CropY = p_video_param->mfx.FrameInfo.CropY;

        //output is set according to the above check
        p_dec_vp_param->Out.FourCC = output_fourcc;
        p_dec_vp_param->Out.ChromaFormat = output_chromaformat;
        p_dec_vp_param->Out.CropW = p_video_param->mfx.FrameInfo.CropW / output_downscale_ratiow;
        p_dec_vp_param->Out.CropH = p_video_param->mfx.FrameInfo.CropH / output_downscale_ratioh;
        p_dec_vp_param->Out.CropX = p_video_param->mfx.FrameInfo.CropX / output_downscale_ratiow;
        p_dec_vp_param->Out.CropY = p_video_param->mfx.FrameInfo.CropY / output_downscale_ratioh;
        p_dec_vp_param->Out.Width = p_video_param->mfx.FrameInfo.Width / output_downscale_ratiow;
        p_dec_vp_param->Out.Height = p_video_param->mfx.FrameInfo.Height / output_downscale_ratioh;
        p_dec_vp_param->Header.BufferId = MFX_EXTBUFF_DEC_VIDEO_PROCESSING;
        p_dec_vp_param->Header.BufferSz = sizeof(mfxExtDecVideoProcessing);

        pp_ext_params[0] = ((mfxExtBuffer*)(p_dec_vp_param));
        p_video_param->NumExtParam = 1;
        p_video_param->ExtParam = &(pp_ext_params[0]);
    }
    //else do nothing as no csc and scaling are supported or needed. video_param keeps unchanged.
}
/*end10*/

static void prg_decoding11 () {
/*beg11*/
/*get memory interface with GetBitstreamBuffer function*/
mfxMemoryInterface *memoryInterface = NULL;
MFXGetMemoryInterface(session, &memoryInterface);

//get bitstream for MFXVideoDECODE_DecodeHeader
bitstream = get_bitstream_buffer(memoryInterface);
append_more_bitstream(bitstream);

mfxExtBuffer *ext_buffer[1];
mfxExtDecVideoProcessing  dec_vp_param;
mfxVideoParam init_param = {0};

/*need set IOPatern before MFXVideoDECODE_DecodeHeader to get the correct CSC and Scaling Ratio capabilities*/
/*MFX_IOPATTERN_OUT_VIDEO_MEMORY and MFX_IOPATTERN_OUT_SYSTEM_MEMORY may have different capabilities*/
init_param.IOPattern = MFX_IOPATTERN_OUT_VIDEO_MEMORY; // or MFX_IOPATTERN_OUT_SYSTEM_MEMORY
MFXVideoDECODE_DecodeHeader(session, bitstream, &init_param);

//init_param should be updated if the desired_fourcc and desired_downscale_ratio are supported.
update_csc_scaling_param(&init_param, ext_buffer, &dec_vp_param);
MFXVideoDECODE_Query(session, &init_param, &init_param);

//the request will be updated by the runtime according to the information of init_param
//if there is extension buffer mfxExtDecVideoProcessing in it, the request will be updated according to the mfxExtDecVideoProcessing which includes the desired_fourcc and desired resolution.
//if no extension buffer in it, the request is default fourcc and original resolution.
MFXVideoDECODE_QueryIOSurf(session, &init_param, &request);

//allocate surfaces according to the request from MFXVideoDECODE_QueryIOSurf
allocate_surface_pool(&request);

/* initialize decoder */
MFXVideoDECODE_Init(session, &init_param);

sts = MFX_ERR_NONE;

/* perform decoding */
for (;;) {
    if ((sts==MFX_ERR_MORE_DATA || !bitstream || !bitstream->Data) && !end_of_stream())
    {
      bitstream = get_bitstream_buffer(memoryInterface);
      append_more_bitstream(bitstream);
    }
    // Asynchronously decode a frame
    sts = MFXVideoDECODE_DecodeFrameAsync(session, bitstream, work, &disp, &syncp);
    if (sts == MFX_ERR_NONE) {
    //do other things such as MFXVideoCORE_SyncOperation.
    }
}
/* close decoder */
MFXVideoDECODE_Close(session);
/*end11*/
}