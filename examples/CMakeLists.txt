# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
# oneAPI Video Processing Library (oneVPL) examples

cmake_minimum_required(VERSION 3.10.2)

if(BUILD_EXAMPLES)
  add_subdirectory(coreAPI/legacy-decode)
  add_subdirectory(coreAPI/legacy-vpp)
  add_subdirectory(coreAPI/legacy-encode)
  add_subdirectory(hello/hello-createsession)
  add_subdirectory(hello/hello-decode)
  add_subdirectory(hello/hello-decvpp)
  add_subdirectory(hello/hello-encode)
  add_subdirectory(hello/hello-transcode)
  add_subdirectory(hello/hello-vpp)
endif()

if(INSTALL_EXAMPLE_CODE)
  install(
    DIRECTORY content
    DESTINATION ${ONEAPI_INSTALL_EXAMPLEDIR}
    COMPONENT dev)

  install(
    DIRECTORY coreAPI/legacy-decode coreAPI/legacy-vpp coreAPI/legacy-encode
    DESTINATION ${ONEAPI_INSTALL_EXAMPLEDIR}/coreAPI
    COMPONENT dev)

  install(
    DIRECTORY interop/vpl-infer interop/dpcpp-blur
    DESTINATION ${ONEAPI_INSTALL_EXAMPLEDIR}/interop
    COMPONENT dev)

  install(
    DIRECTORY hello/hello-createsession hello/hello-decode hello/hello-decvpp
              hello/hello-encode hello/hello-transcode hello/hello-vpp
    DESTINATION ${ONEAPI_INSTALL_EXAMPLEDIR}/hello
    COMPONENT dev)
endif()
