# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
# oneAPI Video Processing Library (oneVPL) examples

cmake_minimum_required(VERSION 3.10.2)

add_subdirectory(util)
add_subdirectory(dpcpp-blur)
add_subdirectory(hello-createsession)
add_subdirectory(hello-decode)
add_subdirectory(hello-decvpp)
add_subdirectory(hello-encode)
add_subdirectory(hello-transcode)
add_subdirectory(hello-vpp)

if(OPTION_COMPILE_PREVIEW_EXAMPLES)
  add_subdirectory(preview/legacy-createsession)
  add_subdirectory(preview/legacy-decode)
  add_subdirectory(preview/legacy-vpp)
endif()

install(
  DIRECTORY content
            dpcpp-blur
            hello-createsession
            hello-decode
            hello-decvpp
            hello-encode
            hello-transcode
            hello-vpp
            util
  DESTINATION ${CMAKE_INSTALL_EXAMPLEDIR}
  COMPONENT dev)
