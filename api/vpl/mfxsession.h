/*############################################################################
  # Copyright (C) 2017-2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef __MFXSESSION_H__
#define __MFXSESSION_H__
#include "mfxcommon.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Global Functions */

/*! SDK session handle. */
typedef struct _mfxSession *mfxSession;

/*!
   @brief
      This function creates and initializes an SDK session. Call this function before calling
      any other SDK function. If the desired implementation specified by impl is MFX_IMPL_AUTO,
      the function will search for the platform-specific SDK implementation.
      If the function cannot find it, it will use the software implementation.

      The ``ver`` argument indicates the desired version of the library implementation.
      The loaded SDK will have an API version compatible to the specified version (equal in
      the major version number, and no less in the minor version number.) If the desired version
      is not specified, the default is to use the API version from the SDK release, with
      which an application is built.

      We recommend that production applications always specify the minimum API version that meets their
      functional requirements. For example, if an application uses only H.264 decoding as described
      in API v1.0, have the application initialize the library with API v1.0. This ensures
      backward compatibility.

   @param[in] impl     mfxIMPL enumerator that indicates the desired SDK implementation.
   @param[in] ver      Pointer to the minimum library version or zero, if not specified.
   @param[out] session Pointer to the SDK session handle.

   @return
      MFX_ERR_NONE        The function completed successfully. The output parameter contains the handle of the session.\n
      MFX_ERR_UNSUPPORTED The function cannot find the desired SDK implementation or version.
*/
mfxStatus MFX_CDECL MFXInit(mfxIMPL impl, mfxVersion *ver, mfxSession *session);

/*!
   @brief
      This function creates and initializes an SDK session. Call this function before calling any other SDK functions.
      If the desired implementation specified by par. Implementation is MFX_IMPL_AUTO, the function will search for
      the platform-specific SDK implementation. If the function cannot find it, it will use the software implementation.

      The argument ``par.Version`` indicates the desired version of the library implementation. The loaded SDK will have an API
      version compatible to the specified version (equal in the major version number, and no less in the minor version number.)
      If the desired version is not specified, the default is to use the API version from the SDK release, with
      which an application is built.

      We recommend that production applications always specify the minimum API version that meets their functional requirements.
      For example, if an application uses only H.264 decoding as described in API v1.0, have the application initialize
      the library with API v1.0. This ensures backward compatibility.

      The argument ``par.ExternalThreads`` specifies threading mode. Value 0 means that SDK should internally create and
      handle work threads (this essentially equivalent of regular MFXInit). I

   @param[in]  par     mfxInitParam structure that indicates the desired SDK implementation, minimum library version and desired threading mode.
   @param[out] session Pointer to the SDK session handle.

   @return
      MFX_ERR_NONE        The function completed successfully. The output parameter contains the handle of the session.\n
      MFX_ERR_UNSUPPORTED The function cannot find the desired SDK implementation or version.
*/
mfxStatus MFX_CDECL MFXInitEx(mfxInitParam par, mfxSession *session);

/*!
   @brief This function completes and deinitializes an SDK session. Any active tasks in execution or
    in queue are aborted. The application cannot call any SDK function after this function.

   All child sessions must be disjoined before closing a parent session.
   @param[in] session SDK session handle.

   @return MFX_ERR_NONE The function completed successfully.
*/
mfxStatus MFX_CDECL MFXClose(mfxSession session);

/*!
   @brief This function returns the implementation type of a given session.

   @param[in]  session SDK session handle.
   @param[out] impl    Pointer to the implementation type

   @return MFX_ERR_NONE The function completed successfully.
*/
mfxStatus MFX_CDECL MFXQueryIMPL(mfxSession session, mfxIMPL *impl);

/*!
   @brief This function returns the SDK implementation version.
    
   @param[in]  session SDK session handle.
   @param[out] version Pointer to the returned implementation version.

   @return MFX_ERR_NONE The function completed successfully.
*/
mfxStatus MFX_CDECL MFXQueryVersion(mfxSession session, mfxVersion *version);

/*!
   @brief This function joins the child session to the current session.

   After joining, the two sessions share thread and resource scheduling for asynchronous
   operations. However, each session still maintains its own device manager and buffer/frame
   allocator. Therefore, the application must use a compatible device manager and buffer/frame
   allocator to share data between two joined sessions.

   The application can join multiple sessions by calling this function multiple times. When joining
   the first two sessions, the current session becomes the parent responsible for thread and
   resource scheduling of any later joined sessions.

   Joining of two parent sessions is not supported.

   @param[in,out] session    The current session handle.
   @param[in]     child      The child session handle to be joined

   @return MFX_ERR_NONE         The function completed successfully. \n
           MFX_WRN_IN_EXECUTION Active tasks are executing or in queue in one of the
                                sessions. Call this function again after all tasks are completed. \n
           MFX_ERR_UNSUPPORTED  The child session cannot be joined with the current session.
*/
mfxStatus MFX_CDECL MFXJoinSession(mfxSession session, mfxSession child);

/*!
   @brief This function removes the joined state of the current session. After disjoining, the current
      session becomes independent. The application must ensure there is no active task running
      in the session before calling this function.

   @param[in,out] session    The current session handle.

   @return MFX_ERR_NONE                The function completed successfully. \n
           MFX_WRN_IN_EXECUTION        Active tasks are executing or in queue in one of the
                                       sessions. Call this function again after all tasks are completed. \n
           MFX_ERR_UNDEFINED_BEHAVIOR  The session is independent, or this session is the parent of all joined sessions.
*/
mfxStatus MFX_CDECL MFXDisjoinSession(mfxSession session);

/*!
   @brief This function creates a clean copy of the current session. The cloned session is an independent session.
          It does not inherit any user-defined buffer, frame allocator, or device manager handles from the current session.
          This function is a light-weight equivalent of MFXJoinSession after MFXInit.

   @param[in] session    The current session handle.
   @param[out] clone     Pointer to the cloned session handle.

   @return MFX_ERR_NONE                The function completed successfully.
*/
mfxStatus MFX_CDECL MFXCloneSession(mfxSession session, mfxSession *clone);

/*!
   @brief This function sets the current session priority.

   @param[in] session    The current session handle.
   @param[in] priority   Priority value.

   @return MFX_ERR_NONE                The function completed successfully.
*/
mfxStatus MFX_CDECL MFXSetPriority(mfxSession session, mfxPriority priority);

/*!
   @brief This function returns the current session priority.

   @param[in] session    The current session handle.
   @param[out] priority   Pointer to the priority value.

   @return MFX_ERR_NONE                The function completed successfully.
*/
mfxStatus MFX_CDECL MFXGetPriority(mfxSession session, mfxPriority *priority);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

