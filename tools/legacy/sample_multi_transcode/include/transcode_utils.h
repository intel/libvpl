/*############################################################################
  # Copyright (C) 2005 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef __TRANSCODE_UTILS_H__
#define __TRANSCODE_UTILS_H__

#if defined(_WIN32) || defined(_WIN64)
    #include <process.h>
    #pragma warning(disable : 4201)
    #include <d3d9.h>
    #include <dxva2api.h>
#endif

#include <map>
#include <vector>
#include "pipeline_transcode.h"

struct D3DAllocatorParams;

#ifdef _MSC_VER
    #pragma warning(disable : 4127) // constant expression
#endif

// Extensions for internal use, normally these macros are blank
#ifdef MOD_SMT
    #include "extension_macros.h"
#else
    #define MOD_SMT_CREATE_PIPELINE
    #define MOD_SMT_PRINT_HELP
    #define MOD_SMT_PARSE_INPUT
#endif

#ifndef MFX_VERSION
    #error MFX_VERSION not defined
#endif

namespace TranscodingSample {
struct sInputParams;

msdk_tick GetTick();
mfxF64 GetTime(msdk_tick start);

void PrintHelp();
void PrintError(const msdk_char* strErrorMessage, ...);
void PrintInfo(mfxU32 session_number, sInputParams* pParams, mfxVersion* pVer);

bool PrintDllInfo(msdk_char* buf, mfxU32 buf_size, sInputParams* pParams);

class CmdProcessor {
public:
    CmdProcessor();
    virtual ~CmdProcessor();
    mfxStatus ParseCmdLine(int argc, msdk_char* argv[]);
    bool GetNextSessionParams(TranscodingSample::sInputParams& InputParams);
    FILE* GetPerformanceFile() {
        return m_PerfFILE;
    };
    void PrintParFileName();
    msdk_string GetLine(mfxU32 n);

protected:
    mfxStatus ParseParFile(FILE* file);
    mfxStatus TokenizeLine(msdk_char* pLine, mfxU32 length);
    mfxU32 GetStringLength(msdk_char* pTempLine, mfxU32 length);

    static bool isspace(char a);
    static bool is_not_allowed_char(char a);
    bool ParseROIFile(msdk_char const* roi_file_name, std::vector<mfxExtEncoderROI>& m_ROIData);

    mfxStatus ParseParamsForOneSession(mfxU32 argc, msdk_char* argv[]);
    mfxStatus ParseOption__set(msdk_char* strCodecType, msdk_char* strPluginPath);
    mfxStatus VerifyAndCorrectInputParams(TranscodingSample::sInputParams& InputParams);
    mfxU32 m_SessionParamId;
    std::vector<TranscodingSample::sInputParams> m_SessionArray;
    std::map<mfxU32, sPluginParams> m_decoderPlugins;
    std::map<mfxU32, sPluginParams> m_encoderPlugins;
    FILE* m_PerfFILE;
    msdk_char* m_parName;
    mfxU32 statisticsWindowSize;
    FILE* statisticsLogFile;
    //store a name of a Logfile
    msdk_tstring DumpLogFileName;
    mfxU32 m_nTimeout;
    bool bRobustFlag;
    bool bSoftRobustFlag;
    bool shouldUseGreedyFormula;
    std::vector<msdk_string> m_lines;

private:
    DISALLOW_COPY_AND_ASSIGN(CmdProcessor);
};
} // namespace TranscodingSample
#endif //__TRANSCODE_UTILS_H__
