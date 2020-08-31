/*###########################################################################
  # Copyright (C) 2005-2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ###########################################################################*/

#ifndef __PIPELINE_USER_H__
#define __PIPELINE_USER_H__

#include "pipeline_encode.h"
#include "vm/so_defs.h"

#ifndef MFX_VERSION
    #error MFX_VERSION not defined
#endif

/* This class implements the following pipeline: user plugin (frame rotation) -> mfxENCODE */
class CUserPipeline : public CEncodingPipeline {
public:
    CUserPipeline();
    virtual ~CUserPipeline();

    virtual mfxStatus Init(sInputParams* pParams);
    virtual mfxStatus Run();
    virtual void Close();
    virtual mfxStatus ResetMFXComponents(sInputParams* pParams);
    virtual void PrintInfo();
    virtual mfxStatus FillBuffers();

protected:
    virtual mfxStatus AllocFrames();
    virtual void DeleteFrames();
};

#endif // __PIPELINE_USER_H__
