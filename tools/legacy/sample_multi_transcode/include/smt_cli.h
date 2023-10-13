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
#include "smt_cli_params.h"

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

void PrintError(const wchar_t* strErrorMessage, ...);
void PrintError(const char* strErrorMessage, ...);
void PrintStreamInfo(mfxU32 session_number, sInputParams* pParams, mfxVersion* pVer);

class CmdProcessor {
public:
    CmdProcessor();
    virtual ~CmdProcessor();
    mfxStatus ParseCmdLine(int argc, char* argv[]);
    bool GetNextSessionParams(TranscodingSample::sInputParams& InputParams);
    std::string GetPerformanceFile() {
        return performance_file_name;
    };
    std::string GetParameterFile() {
        return parameter_file_name;
    };
    std::vector<std::string> GetSessionDescriptions() {
        return session_descriptions;
    };

    mfxU32 GetParameterSurfaceWaitInterval() {
        return m_surface_wait_interval;
    };

protected:
    mfxStatus ParseParFile(const std::string& filename);
    mfxStatus TokenizeLine(const std::string& line);
    size_t GetStringLength(char* pTempLine, size_t length);

    static bool isspace(char a);
    static bool is_not_allowed_char(char a);
    bool ParseROIFile(const std::string& roi_file_name, std::vector<mfxExtEncoderROI>& m_ROIData);

    mfxStatus ParseParamsForOneSession(mfxU32 argc, char* argv[]);
    mfxStatus ParseOption__set(char* strCodecType, char* strPluginPath);
    mfxStatus VerifyAndCorrectInputParams(TranscodingSample::sInputParams& InputParams);
    mfxU32 m_SessionParamId;
    std::vector<TranscodingSample::sInputParams> m_SessionArray;
    std::map<mfxU32, sPluginParams> m_decoderPlugins;
    std::map<mfxU32, sPluginParams> m_encoderPlugins;
    std::string performance_file_name;
    std::string parameter_file_name;
    mfxU32 statisticsWindowSize;
    FILE* statisticsLogFile;
    //store a name of a Logfile
    std::string DumpLogFileName;
    mfxU32 m_nTimeout;
    mfxU32 m_surface_wait_interval;
    bool bRobustFlag;
    bool bSoftRobustFlag;
    bool shouldUseGreedyFormula;
    std::vector<std::string> session_descriptions;

private:
    DISALLOW_COPY_AND_ASSIGN(CmdProcessor);
};
} // namespace TranscodingSample
#endif //__TRANSCODE_UTILS_H__
