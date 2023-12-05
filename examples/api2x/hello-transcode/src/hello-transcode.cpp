//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

///
/// A minimal Intel® Video Processing Library (Intel® VPL) transcode application
/// using Intel® VPL 2.2 API features including internal memory. For more information see:
/// https://software.intel.com/content/www/us/en/develop/articles/upgrading-from-msdk-to-onevpl.html
/// https://intel.github.io/libvpl
///
/// @file

#include "util.hpp"

#define TARGETKBPS                 4000
#define FRAMERATE                  30
#define OUTPUT_FILE                "out.h265"
#define BITSTREAM_BUFFER_SIZE      2000000
#define MAJOR_API_VERSION_REQUIRED 2
#define MINOR_API_VERSION_REQUIRED 5
#define MAX_TIMEOUT_COUNT          10

#define WAIT_5_MILLISECONDS 5
#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
    #define sleep(msec) Sleep(msec)
#else
    #include <unistd.h>
    #define sleep(msec) usleep(1000 * msec)
#endif

void Usage(void) {
    printf("\n");
    printf("   Usage  :  hello-transcode \n\n");
    printf("     -i             input file name (MJPEG elementary stream)\n\n");
    printf("   Example:  hello-transcode -i in.mjpeg\n");
    printf("   To view:  ffplay %s\n\n", OUTPUT_FILE);
    printf(" * Transcode HEVC/H265 elementary stream in %s\n\n", OUTPUT_FILE);
    return;
}

int main(int argc, char *argv[]) {
    bool isDrainingDec                = false;
    bool isDrainingEnc                = false;
    bool isStillgoing                 = true;
    bool isFailed                     = false;
    FILE *sink                        = NULL;
    FILE *source                      = NULL;
    mfxBitstream bs_dec_in            = {};
    mfxBitstream bs_enc_out           = {};
    mfxFrameSurface1 *dec_surface_out = NULL;
    mfxSession session                = NULL;
    mfxStatus sts                     = MFX_ERR_NONE;
    mfxStatus sts_r                   = MFX_ERR_NONE;
    mfxSyncPoint syncp                = {};
    mfxU32 framenum                   = 0;
    mfxVideoParam encodeParams        = {};
    mfxVideoParam stream_info         = {};
    Params cliParams                  = {};

    // variables used only in 2.x version
    mfxConfig cfg[4];
    mfxVariant cfgVal[4];
    mfxLoader loader = NULL;
    mfxU8 timeout_count;

    //Parse command line args to cliParams
    if (ParseArgsAndValidate(argc, argv, &cliParams, PARAMS_DECODE) == false) {
        Usage();
        return 1; // return 1 as error code
    }

    source = fopen(cliParams.infileName, "rb");
    VERIFY(source, "Could not open input file");

    sink = fopen(OUTPUT_FILE, "wb");
    VERIFY(sink, "Could not create output file");

    // Initialize session
    loader = MFXLoad();
    VERIFY(NULL != loader, "MFXLoad failed -- is implementation in path?");

    // Implementation used must be the type requested from command line
    cfg[0] = MFXCreateConfig(loader);
    VERIFY(NULL != cfg[0], "MFXCreateConfig failed")
    cfgVal[0].Type     = MFX_VARIANT_TYPE_U32;
    cfgVal[0].Data.U32 = MFX_IMPL_TYPE_HARDWARE;

    sts = MFXSetConfigFilterProperty(cfg[0], (mfxU8 *)"mfxImplDescription.Impl", cfgVal[0]);
    VERIFY(MFX_ERR_NONE == sts, "MFXSetConfigFilterProperty failed for Impl");

    // Implementation must provide a JPEG decoder
    cfg[1] = MFXCreateConfig(loader);
    VERIFY(NULL != cfg[1], "MFXCreateConfig failed")
    cfgVal[1].Type     = MFX_VARIANT_TYPE_U32;
    cfgVal[1].Data.U32 = MFX_CODEC_JPEG;
    sts                = MFXSetConfigFilterProperty(
        cfg[1],
        (mfxU8 *)"mfxImplDescription.mfxDecoderDescription.decoder.CodecID",
        cfgVal[1]);
    VERIFY(MFX_ERR_NONE == sts, "MFXSetConfigFilterProperty failed for decoder CodecID");

    // Implementation must provide an HEVC encoder
    cfg[2] = MFXCreateConfig(loader);
    VERIFY(NULL != cfg[2], "MFXCreateConfig failed")
    cfgVal[2].Type     = MFX_VARIANT_TYPE_U32;
    cfgVal[2].Data.U32 = MFX_CODEC_HEVC;
    sts                = MFXSetConfigFilterProperty(
        cfg[2],
        (mfxU8 *)"mfxImplDescription.mfxEncoderDescription.encoder.CodecID",
        cfgVal[2]);
    VERIFY(MFX_ERR_NONE == sts, "MFXSetConfigFilterProperty failed for encoder CodecID");

    // Implementation must provide equal to or higher API version than MAJOR_API_VERSION_REQUIRED.MINOR_API_VERSION_REQUIRED
    cfg[3] = MFXCreateConfig(loader);
    VERIFY(NULL != cfg[3], "MFXCreateConfig failed")
    cfgVal[3].Type     = MFX_VARIANT_TYPE_U32;
    cfgVal[3].Data.U32 = VPLVERSION(MAJOR_API_VERSION_REQUIRED, MINOR_API_VERSION_REQUIRED);
    sts                = MFXSetConfigFilterProperty(cfg[3],
                                     (mfxU8 *)"mfxImplDescription.ApiVersion.Version",
                                     cfgVal[3]);
    VERIFY(MFX_ERR_NONE == sts, "MFXSetConfigFilterProperty failed for API version");

    sts = MFXCreateSession(loader, 0, &session);
    VERIFY(MFX_ERR_NONE == sts,
           "Cannot create session -- no implementations meet selection criteria");

    // Print info about implementation loaded
    ShowImplementationInfo(loader, 0);

    // Initiate bs_dec_in for decode input stream and parse basic stream information (input width and height)
    //   in : session, source
    //   out: bs_dec_in, stream_info

    // Prepare input bitstream and start decoding
    bs_dec_in.MaxLength = BITSTREAM_BUFFER_SIZE;
    bs_dec_in.Data      = (mfxU8 *)calloc(bs_dec_in.MaxLength, sizeof(mfxU8));
    VERIFY(bs_dec_in.Data, "Not able to allocate input buffer");
    bs_dec_in.CodecId = MFX_CODEC_JPEG;

    //Pre-parse input stream
    sts = ReadEncodedStream(bs_dec_in, source);
    VERIFY(MFX_ERR_NONE == sts, "Error reading bitstream\n");

    stream_info.mfx.CodecId = MFX_CODEC_JPEG;
    stream_info.IOPattern   = MFX_IOPATTERN_OUT_SYSTEM_MEMORY;
    sts                     = MFXVideoDECODE_DecodeHeader(session, &bs_dec_in, &stream_info);
    VERIFY(MFX_ERR_NONE == sts, "Error decoding header\n");

    // Initialize the decoder
    sts = MFXVideoDECODE_Init(session, &stream_info);
    VERIFY(MFX_ERR_NONE == sts, "Error initializing decode\n");

    printf("Transcoding %s -> %s\n", cliParams.infileName, OUTPUT_FILE);

    // Prepare bitstream for encode output and encode params
    //   in : stream_info.mfx.FrameInfo.Width, stream_info.mfx.FrameInfo.Height
    //   out: bs_enc_out, encodeParams
    bs_enc_out.MaxLength = BITSTREAM_BUFFER_SIZE;
    bs_enc_out.Data      = (mfxU8 *)calloc(bs_enc_out.MaxLength, sizeof(mfxU8));
    VERIFY(bs_dec_in.Data, "Not able to allocate input buffer");

    encodeParams.mfx.CodecId                 = MFX_CODEC_HEVC;
    encodeParams.mfx.TargetUsage             = MFX_TARGETUSAGE_BALANCED;
    encodeParams.mfx.TargetKbps              = TARGETKBPS;
    encodeParams.mfx.RateControlMethod       = MFX_RATECONTROL_VBR;
    encodeParams.mfx.FrameInfo.FrameRateExtN = FRAMERATE;
    encodeParams.mfx.FrameInfo.FrameRateExtD = 1;
    encodeParams.mfx.FrameInfo.FourCC        = stream_info.mfx.FrameInfo.FourCC;
    encodeParams.mfx.FrameInfo.ChromaFormat  = stream_info.mfx.FrameInfo.ChromaFormat;
    encodeParams.mfx.FrameInfo.CropW         = stream_info.mfx.FrameInfo.Width;
    encodeParams.mfx.FrameInfo.CropH         = stream_info.mfx.FrameInfo.Height;
    encodeParams.mfx.FrameInfo.Width         = ALIGN16(encodeParams.mfx.FrameInfo.CropW);
    encodeParams.mfx.FrameInfo.Height        = ALIGN16(encodeParams.mfx.FrameInfo.CropH);

    encodeParams.IOPattern = MFX_IOPATTERN_IN_SYSTEM_MEMORY;

    // Validate video encode parameters
    // - In this example the validation result is written to same structure
    // - MFX_WRN_INCOMPATIBLE_VIDEO_PARAM is returned if some of the video parameters are not supported,
    //   instead the encoder will select suitable parameters closest matching the requested configuration,
    //   and it's ignorable.
    sts = MFXVideoENCODE_Query(session, &encodeParams, &encodeParams);
    if (sts == MFX_WRN_INCOMPATIBLE_VIDEO_PARAM)
        sts = MFX_ERR_NONE;
    VERIFY(MFX_ERR_NONE == sts, "Encode query failed");

    // Initialize the encoder
    sts = MFXVideoENCODE_Init(session, &encodeParams);
    VERIFY(MFX_ERR_NONE == sts, "Could not initialize Encode");

    printf("Transcoding %s -> %s\n", cliParams.infileName, OUTPUT_FILE);

    while (isStillgoing == true) {
        sts = MFX_ERR_NONE;

        // Read input stream for decode
        if (isDrainingDec == false) {
            sts = ReadEncodedStream(bs_dec_in, source);
            if (sts != MFX_ERR_NONE) // No more data to read, start decode draining mode
                isDrainingDec = true;
        }

        // Decode MJPEG stream
        if (isDrainingEnc == false) {
            timeout_count = 0;
            do {
                sts = MFXVideoDECODE_DecodeFrameAsync(session,
                                                      (isDrainingDec == true) ? NULL : &bs_dec_in,
                                                      NULL,
                                                      &dec_surface_out,
                                                      &syncp);
                // From API version 2.5,
                // When the internal memory model is used,
                // MFX_WRN_ALLOC_TIMEOUT_EXPIRED is returned when all the surfaces are currently in use and timeout set by mfxExtAllocationHints for allocation of new surfaces through functions DecodeFrameAsync expired.
                // Repeat the call in a few milliseconds.
                // For more information, please check Intel® VPL API documentation.
                if (sts == MFX_WRN_ALLOC_TIMEOUT_EXPIRED) {
                    if (timeout_count > MAX_TIMEOUT_COUNT) {
                        sts = MFX_ERR_DEVICE_FAILED;
                        break;
                    }
                    else {
                        timeout_count++;
                        sleep(WAIT_5_MILLISECONDS);
                        continue;
                    }
                }
                else
                    break;
            } while (1);
        }

        switch (sts) {
            case MFX_ERR_NONE: // Got 1 decoded frame
                break;
            case MFX_ERR_MORE_DATA: // The function requires more bitstream at input before decoding can proceed
                if (isDrainingDec == true)
                    isDrainingEnc =
                        true; // No more data to drain from decoder, start encode draining mode
                else
                    continue; // read more data
                break;
            default:
                printf("unknown status %d\n", sts);
                isStillgoing = false;
                continue;
        }

        // Encode to H265 stream
        sts = MFXVideoENCODE_EncodeFrameAsync(session,
                                              NULL,
                                              (isDrainingEnc == true) ? NULL : dec_surface_out,
                                              &bs_enc_out,
                                              &syncp);

        if (isDrainingEnc == false && dec_surface_out) {
            sts_r = dec_surface_out->FrameInterface->Release(dec_surface_out);
            VERIFY(MFX_ERR_NONE == sts_r, "mfxFrameSurfaceInterface->Release failed");
        }

        switch (sts) {
            case MFX_ERR_NONE:
                // MFX_ERR_NONE and syncp indicate output is available
                if (syncp) {
                    // Encode output is not available on CPU until sync operation completes
                    sts = MFXVideoCORE_SyncOperation(session, syncp, WAIT_100_MILLISECONDS);
                    VERIFY(MFX_ERR_NONE == sts, "MFXVideoCORE_SyncOperation error");
                    WriteEncodedStream(bs_enc_out, sink);
                    framenum++;
                }
                break;
            case MFX_ERR_MORE_DATA: // The function requires more data to generate any output
                if (isDrainingEnc == true)
                    isStillgoing = false; // No more data to drain from encoder, exit loop
                else
                    continue;
                break;
            default:
                printf("unknown status %d\n", sts);
                isStillgoing = false;
                break;
        }
    }

end:
    printf("Transcoded %d frames\n", framenum);

    // Clean up resources - It is recommended to close components first, before
    // releasing allocated surfaces, since some surfaces may still be locked by
    // internal resources.

    if (bs_enc_out.Data)
        free(bs_enc_out.Data);

    if (bs_dec_in.Data)
        free(bs_dec_in.Data);

    if (source)
        fclose(source);

    if (sink)
        fclose(sink);

    if (loader)
        MFXUnload(loader);

    if (isFailed) {
        return -1;
    }
    else {
        return 0;
    }
}
