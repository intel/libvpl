/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

/* This file contains API definitions which are deprected or removed.  Add
 * removed enum values, functions or structure definitions to this file.  If
 * APIs located here reappear, header compilation tests will fail.  For each
 * removed API entry - please keep original name of the header file.
*/

#ifndef API_TESTS_COMPILE_HEADERS_REMOVED_API_H_
#define API_TESTS_COMPILE_HEADERS_REMOVED_API_H_

#include "./mfxstructures.h"

/* Don't expose this file publically without special approval. */

/* Enums */
enum {
    /*!
       This extended buffer allow to specify multi-frame submission parameters.
    */
    MFX_EXTBUFF_MULTI_FRAME_PARAM               = MFX_MAKEFOURCC('M', 'F', 'R', 'P'),
    /*!
       This extended buffer allow to manage multi-frame submission in runtime.
    */
    MFX_EXTBUFF_MULTI_FRAME_CONTROL             = MFX_MAKEFOURCC('M', 'F', 'R', 'C'),
};


/*`Unions */

/* Structs */

/* Functions*/

#endif  /* API_TESTS_COMPILE_HEADERS_REMOVED_API_H_ */
