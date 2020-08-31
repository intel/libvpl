# ##############################################################################
# Copyright (C) 2020 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
cmake_minimum_required(VERSION 3.10.2)

set(TARGET ffmpeg-svt)

add_library(${TARGET} INTERFACE)

if(NOT DEFINED ENV{VPL_BUILD_DEPENDENCIES})
  message(FATAL_ERROR "VPL_BUILD_DEPENDENCIES not defined in environment")
else()
  set(VPL_DEP_DIR $ENV{VPL_BUILD_DEPENDENCIES})
  message(STATUS "Using VPL_BUILD_DEPENDENCIES located at ${VPL_DEP_DIR}")
endif()

option(BUILD_GPL_X264 "Build X264 with GPL License" OFF)

set(SVTHEVCENC_LIB ${VPL_DEP_DIR}/lib/libSvtHevcEnc.a)
set(SVTAV1ENC_LIB ${VPL_DEP_DIR}/lib/libSvtAv1Enc.a)
set(DAV1D_LIB ${VPL_DEP_DIR}/lib/libdav1d.a)
set(AVCODEC_LIB ${VPL_DEP_DIR}/lib/libavcodec.a)
set(AVUTIL_LIB ${VPL_DEP_DIR}/lib/libavutil.a)
set(AVFILTER_LIB ${VPL_DEP_DIR}/lib/libavfilter.a)
set(SWSCALE_LIB ${VPL_DEP_DIR}/lib/libswscale.a)

if(BUILD_GPL_X264)
  set(X264_LIB ${VPL_DEP_DIR}/lib/libx264.a)
  if(NOT EXISTS ${X264_LIB})
    message(FATAL_ERROR "Could not find expected FFmpeg libraries")
  endif()
endif(BUILD_GPL_X264)

if(NOT EXISTS ${SVTHEVCENC_LIB}
   OR NOT EXISTS ${SVTHEVCENC_LIB}
   OR NOT EXISTS ${DAV1D_LIB}
   OR NOT EXISTS ${AVCODEC_LIB}
   OR NOT EXISTS ${AVUTIL_LIB}
   OR NOT EXISTS ${AVFILTER_LIB}
   OR NOT EXISTS ${SWSCALE_LIB})
  message(FATAL_ERROR "Could not find expected FFmpeg libraries")
endif()

if(MSVC)
  if(NOT DEFINED ENV{MSYS_ROOT})
    message(STATUS "MSYS_ROOT not defined, assuming C:/tools/msys64")
    set(MSYS_ROOT C:/tools/msys64)
  else()
    set(MSYS_ROOT $ENV{MSYS_ROOT})
  endif()
  execute_process(COMMAND ${MSYS_ROOT}/mingw64/bin/gcc.exe --version
                  OUTPUT_VARIABLE gcc_version_text)
  string(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" gcc_version ${gcc_version_text})

  set(GCC_LIB
      ${MSYS_ROOT}/mingw64/lib/gcc/x86_64-w64-mingw32/${gcc_version}/libgcc.a)
  set(GCC_EH_LIB
      ${MSYS_ROOT}/mingw64/lib/gcc/x86_64-w64-mingw32/${gcc_version}/libgcc_eh.a
  )
  set(BCRYPT_LIB ${MSYS_ROOT}/mingw64/x86_64-w64-mingw32/lib/libbcrypt.a)
  set(MINGW32_LIB ${MSYS_ROOT}/mingw64/x86_64-w64-mingw32/lib/libmingw32.a)
  set(MINGWEX_LIB ${MSYS_ROOT}/mingw64/x86_64-w64-mingw32/lib/libmingwex.a)
  set(PTHREAD_IMPLIB
      ${MSYS_ROOT}/mingw64/x86_64-w64-mingw32/lib/libpthread.dll.a)
  set(PTHREAD_DLL ${MSYS_ROOT}/mingw64/bin/libwinpthread-1.dll)

  if(NOT EXISTS ${GCC_LIB}
     OR NOT EXISTS ${GCC_EH_LIB}
     OR NOT EXISTS ${BCRYPT_LIB}
     OR NOT EXISTS ${MINGW32_LIB}
     OR NOT EXISTS ${MINGWEX_LIB}
     OR NOT EXISTS ${PTHREAD_DLL})
    message(FATAL_ERROR "Could not find expected MinGW runtime libraries")
  endif()
  target_link_libraries(
    ${TARGET} INTERFACE ${GCC_LIB} ${GCC_EH_LIB} ${BCRYPT_LIB} ${MINGW32_LIB}
                        ${MINGWEX_LIB})

  if(NOT EXISTS ${PTHREAD_IMPLIB})
    message(FATAL_ERROR "Could not find expected MinGW runtime libraries")
  endif()
  target_link_libraries(${TARGET} INTERFACE ${PTHREAD_IMPLIB})

  install(
    PROGRAMS ${PTHREAD_DLL}
    DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT runtime)
  # copy pthread dll to output directory so bins work without setting paths
  add_custom_target(mingw-pthread)
  add_dependencies(${TARGET} mingw-pthread)
  add_custom_command(
    TARGET mingw-pthread
    COMMAND ${CMAKE_COMMAND} -E copy ${PTHREAD_DLL}
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR})

endif()

set_property(TARGET ${TARGET} PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                                       ${VPL_DEP_DIR}/include)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

if(MSVC)
  target_link_libraries(
    ${TARGET}
    INTERFACE ${AVCODEC_LIB}
              ${AVUTIL_LIB}
              ${AVFILTER_LIB}
              ${SWSCALE_LIB}
              ${SVTHEVCENC_LIB}
              ${SVTAV1ENC_LIB}
              ${DAV1D_LIB}
              Threads::Threads)
else()
  target_link_libraries(
    ${TARGET}
    INTERFACE ${AVCODEC_LIB}
              ${AVUTIL_LIB}
              ${AVFILTER_LIB}
              ${SWSCALE_LIB}
              ${SVTHEVCENC_LIB}
              ${SVTAV1ENC_LIB}
              ${DAV1D_LIB}
              Threads::Threads
              dl)
endif()

if(BUILD_GPL_X264)
  if(MSVC)
    target_link_libraries(${TARGET} INTERFACE ${X264_LIB} Threads::Threads)
  else()
    target_link_libraries(${TARGET} INTERFACE ${X264_LIB} Threads::Threads dl)
  endif()
endif(BUILD_GPL_X264)
