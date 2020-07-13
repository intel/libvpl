==============
Video Encoding
==============

The **ENCODE** class of functions takes raw frames as input and compresses them
into a bitstream.

Input frames usually come encoded in a repeated pattern called the Group of
Picture (GOP) sequence. For example, a GOP sequence can start from an I-frame,
followed by a few B-frames, a P-frame, and so on. **ENCODE** uses an MPEG-2 style
GOP sequence structure that can specify the length of the sequence and the
distance between two key frames: I- or P-frames. A GOP sequence ensures that the
segments of a bitstream do not completely depend upon each other. It also enables
decoding applications to reposition the bitstream.

An **ENCODE** output consists of one frame of a bitstream with the time stamp
passed from the input frame. The time stamp is used for multiplexing subsequent
video with other associated data such as audio. The API library provides only
pure video stream encoding. The application must provide its own multiplexing.

------------------
Encoding Procedure
------------------

There are two methods for shared memory allocation and handling in the API:
external and internal.

The following example shows the working code of the encoding procedure with
external memory (legacy mode):

.. code-block:: c++

    // codec: H.265 (HEVC)
    // spec:  Software Implementation
    //        VBR (Variable bitrate control) mode by default        
    //        4Mbps bitrate
    //        30fps
    //        GOP size 60
    //        Main profile, level auto
    // input: IYUV (I420), 1280 x 720, 200 frames
    
    #include <stdio.h>
    #include <string.h>
    #include "vpl/mfxvideo.h"

    mfxStatus LoadRawFrame(mfxFrameSurface1* pSurface, FILE* fSource);

    int main(int argc, char* argv[]) {
        mfxU16 i;
        FILE* fSource = fopen("./in.yuv", "rb");
        FILE* fSink = fopen("./out.h265", "wb");

        mfxU16 inputWidth  = 1280;
        mfxU16 inputHeight = 720;
        mfxU32 fourCC      = MFX_FOURCC_IYUV;
        mfxU32 codecID     = MFX_CODEC_HEVC;

        // Initialize Media SDK session
        mfxInitParam initPar   = { 0 };
        initPar.Version        = {{ 35, 1 }};
        initPar.Implementation = MFX_IMPL_SOFTWARE;

        mfxSession session;
        mfxStatus sts = MFXInitEx(initPar, &session);
        if (sts != MFX_ERR_NONE) return 1;

        // Initialize encoder parameters
        // Width must be a multiple of 16
        // Height must be a multiple of 16 in case of frame picture and a multiple of 32 in case of field picture
        // The default picture mode is progressive, so 16 bytes alignment is needed
        mfxVideoParam mfxEncParams = { 0 };
        mfxEncParams.mfx.CodecId                 = codecID;
        mfxEncParams.mfx.TargetKbps              = 4000; // 4Mbps
        mfxEncParams.mfx.FrameInfo.FrameRateExtN = 30;
        mfxEncParams.mfx.FrameInfo.FrameRateExtD = 1;
        mfxEncParams.mfx.FrameInfo.FourCC        = fourCC;
        mfxEncParams.mfx.FrameInfo.Width         = (((inputWidth + 15) >> 4) << 4);
        mfxEncParams.mfx.FrameInfo.Height        = (((inputHeight + 15) >> 4) << 4);
        mfxEncParams.IOPattern                   = MFX_IOPATTERN_IN_SYSTEM_MEMORY;

        // Query number required surfaces for encoder
        mfxFrameAllocRequest EncRequest = { 0 };
        sts = MFXVideoENCODE_QueryIOSurf(session, &mfxEncParams, &EncRequest);
        if (sts != MFX_ERR_NONE) return 1;

        // Determine the required number of surfaces for encoder
        mfxU16 nEncSurfNum = EncRequest.NumFrameSuggested;

        // Allocate surfaces for encoder
        // - Frame surface array keeps pointers all surface planes and general frame info
        mfxU32 surfaceSize = inputWidth*inputHeight*1.5;
        mfxU8* surfaceBuffers = new mfxU8[surfaceSize * nEncSurfNum];

        // Allocate surface headers (mfxFrameSurface1) for encoder
        // IYUV(I420)
        mfxFrameSurface1 *pEncSurfaces = new mfxFrameSurface1[nEncSurfNum];
        for (i = 0; i < nEncSurfNum; i++) {
            pEncSurfaces[i]            = {0};
            pEncSurfaces[i].Info       = mfxEncParams.mfx.FrameInfo;
            pEncSurfaces[i].Data.Y     = &surfaceBuffers[surfaceSize * i];
            pEncSurfaces[i].Data.U     = pEncSurfaces[i].Data.Y + inputWidth * inputHeight;
            pEncSurfaces[i].Data.V     = pEncSurfaces[i].Data.U + ((inputWidth / 2) * (inputHeight / 2));
            pEncSurfaces[i].Data.Pitch = inputWidth;
        }

        // Initialize the Media SDK encoder
        sts = MFXVideoENCODE_Init(session, &mfxEncParams);
        if (sts != MFX_ERR_NONE) return 1;

        // Prepare Media SDK bit stream buffer
        mfxBitstream mfxBS = { 0 };
        mfxBS.MaxLength    = 1000000;
        mfxBS.Data         = new mfxU8[mfxBS.MaxLength];

        mfxU16 nEncSurfIdx = 0;
        mfxSyncPoint syncp;
        mfxFrameSurface1* pSurface;
        
        // Main encoding loop
        while (MFX_ERR_NONE <= sts || MFX_ERR_MORE_DATA == sts) {
            // Find free frame surface
            for (i = 0; i < nEncSurfNum; i++) {
                if (0 == pEncSurfaces[i].Data.Locked) {
                    nEncSurfIdx = i;
                    break;
                }
            }

            if (i == nEncSurfNum) return MFX_ERR_NOT_FOUND;
            
            pSurface = &pEncSurfaces[i];

            // Load raw video data to frame surface
            sts = LoadRawFrame(pSurface, fSource);
            if (sts == MFX_ERR_MORE_DATA) {
                // Draining mode start. set pSurface to NULL.
                pSurface = NULL;
            } else if (sts != MFX_ERR_NONE) {
                break;
            }

            // Encode a frame asychronously (returns immediately)
            sts = MFXVideoENCODE_EncodeFrameAsync(session, NULL, pSurface,  &mfxBS, &syncp);

            if (MFX_ERR_NONE < sts && syncp) {
                sts = MFX_ERR_NONE; // Ignore warnings if output is available
            } else if(pSurface == NULL && sts == MFX_ERR_MORE_DATA) {
                break;  // done draining.
            }

            if (MFX_ERR_NONE == sts) {
                // Synchronize. Wait until encoded frame is ready
                sts = MFXVideoCORE_SyncOperation(session, syncp, 60000);
                
                // Write an encoded bitstream to file
                fwrite(mfxBS.Data + mfxBS.DataOffset, 1, mfxBS.DataLength, fSink);
                mfxBS.DataLength = 0;
            }
        }

        // Clean up resources
        //  - It is recommended to close Media SDK components first, before releasing allocated surfaces, since
        //    some surfaces may still be locked by internal Media SDK resources.
        MFXVideoENCODE_Close(session);

        fclose(fSource);
        fclose(fSink);

        delete[] mfxBS.Data;
        delete[] surfaceBuffers;
        delete[] pEncSurfaces;

        printf("done!\n");
        return 0;
    }

    // IYUV
    mfxStatus LoadRawFrame(mfxFrameSurface1* pSurface, FILE* fSource) {
        mfxStatus sts = MFX_ERR_NONE;
        mfxU16 w, h, i, pitch;
        mfxU32 nBytesRead;
        mfxU8* ptr;
        mfxFrameInfo* pInfo = &pSurface->Info;
        mfxFrameData* pData = &pSurface->Data;

        w = pInfo->Width;
        h = pInfo->Height;

        // read luminance plane (Y)
        pitch = pData->Pitch;
        ptr   = pData->Y;
        for (i = 0; i < h; i++) {
            nBytesRead = (mfxU32)fread(ptr + i * pitch, 1, w, fSource);
            if (w != nBytesRead)
                return MFX_ERR_MORE_DATA;
        }

        // read chrominance (U, V)
        pitch /= 2;
        h /= 2;
        w /= 2;
        ptr = pData->U;
        for (i = 0; i < h; i++) {
            nBytesRead = (mfxU32)fread(ptr + i * pitch, 1, w, fSource);
            if (w != nBytesRead)
                return MFX_ERR_MORE_DATA;
        }

        ptr = pData->V;
        for (i = 0; i < h; i++) {
            nBytesRead = (mfxU32)fread(ptr + i * pitch, 1, w, fSource);
            if (w != nBytesRead)
                return MFX_ERR_MORE_DATA;
        }
    
        return MFX_ERR_NONE;
    }

Note the following key points about the example:

- The application uses the MFXVideoENCODE_QueryIOSurf function to obtain the
  number of working frame surfaces required for reordering input frames.
- The application calls the MFXVideoENCODE_EncodeFrameAsync function for the
  encoding operation. The input frame must be in an unlocked frame surface from
  the frame surface pool. If the encoding output is not available, the function
  returns the status code MFX_ERR_MORE_DATA to request additional input frames.
- Upon successful encoding, the MFXVideoENCODE_EncodeFrameAsync function returns
  MFX_ERR_NONE. However, the encoded bitstream is not yet available because the MFXVideoENCODE_EncodeFrameAsync function is asynchronous. The application must
  use the MFXVideoCORE_SyncOperation function to synchronize the encoding operation
  before retrieving the encoded bitstream.
- At the end of the stream, the application continuously calls the
  MFXVideoENCODE_EncodeFrameAsync function with NULL surface pointer to drain any
  remaining bitstreams cached within the API encoder, until the function
  returns MFX_ERR_MORE_DATA.

.. note:: It is the application's responsibility to fill pixels outside of the
          crop window when it is smaller than the frame to be encoded, especially
          in cases when crops are not aligned to minimum coding block size (16
          for AVC, 8 for HEVC and VP9).

Another approach is when the API allocates memory for shared objects internally,
as shown in the following example:

.. code-block:: c++

   MFXVideoENCODE_Init(session, &init_param);
   sts=MFX_ERR_MORE_DATA;
   for (;;) {
      if (sts==MFX_ERR_MORE_DATA && !end_of_stream()) {
         MFXMemory_GetSurfaceForEncode(&surface);
         fill_content_for_encoding(surface);
      }
      surface2=end_of_stream()?NULL:surface;
      sts=MFXVideoENCODE_EncodeFrameAsync(session,NULL,surface2,bits,&syncp);
      if (surface2) surface->FrameInterface->(*Release)(surface2);
      if (end_of_stream() && sts==MFX_ERR_MORE_DATA) break;
      // Skipped other error handling
      if (sts==MFX_ERR_NONE) {
         MFXVideoCORE_SyncOperation(session, syncp, INFINITE);
         do_something_with_encoded_bits(bits);
      }
   }
   MFXVideoENCODE_Close();

There are several key points which are different from legacy mode:

- The application doesn't need to call MFXVideoENCODE_QueryIOSurf function to
  obtain the number of working frame surfaces since allocation is done by the API.
- The application calls the MFXMemory_GetSurfaceForEncode function to get a free
  surface for the following encode operation.
- The application needs to call the FrameInterface->(\*Release) function to
  decrement the reference counter of the obtained surface after the MFXVideoENCODE_EncodeFrameAsync call.

--------------------
Configuration Change
--------------------

The application changes configuration during encoding by calling the MFXVideoENCODE_Reset function. Depending on the difference in configuration
parameters before and after the change, the API encoder either continues the
current sequence or starts a new one. If the API encoder starts a new sequence,
it completely resets internal state and begins a new sequence with IDR frame.

The application controls encoder behavior during parameter change by attaching
mfxExtEncoderResetOption to the mfxVideoParam structure during reset. By using
this structure, the application instructs encoder to start or not start a new sequence after reset. In some cases, the request to continue the current
sequence cannot be satisfied and the encoder fails during reset. To avoid such
cases the application may query the reset outcome before actual reset
by calling MFXVideoENCODE_Query function with mfxExtEncoderResetOption attached
to mfxVideoParam structure.

The application uses the following procedure to change encoding configurations:

#. The application retrieves any cached frames in the API encoder by calling the
   MFXVideoENCODE_EncodeFrameAsync function with a NULL input frame pointer until the function returns MFX_ERR_MORE_DATA.

.. note:: The application must set the initial encoding configuration flag
   EndOfStream of the mfxExtCodingOption structure to OFF to avoid inserting
   an End of Stream (EOS) marker into the bitstream. An EOS marker causes
   the bitstream to terminate before encoding is complete.

#. The application calls the MFXVideoENCODE_Reset function with the new
   configuration:

   - If the function successfully set the configuration, the application can
     continue encoding as usual.
   - If the new configuration requires a new memory allocation, the function
     returns MFX_ERR_INCOMPATIBLE_VIDEO_PARAM. The application must close the
     API encoder and reinitialize the encoding procedure with the new configuration.

