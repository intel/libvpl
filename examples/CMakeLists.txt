# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
# oneAPI Video Processing Library (oneVPL) examples

cmake_minimum_required(VERSION 3.10.2)

add_subdirectory(coreAPI/legacy-decode)
add_subdirectory(coreAPI/legacy-vpp)
add_subdirectory(coreAPI/legacy-encode)
add_subdirectory(hello/hello-decvpp)
add_subdirectory(hello/hello-createsession)
add_subdirectory(hello/hello-transcode)

install(
  DIRECTORY content
  DESTINATION ${ONEAPI_INSTALL_EXAMPLEDIR}
  COMPONENT dev)

install(
  DIRECTORY coreAPI/legacy-decode coreAPI/legacy-vpp coreAPI/legacy-encode
  DESTINATION ${ONEAPI_INSTALL_EXAMPLEDIR}/coreAPI
  COMPONENT dev)

install(
  DIRECTORY interop/hello-decode-infer interop/legacy-decode-infer
            interop/advanced-decvpp-infer
  DESTINATION ${ONEAPI_INSTALL_EXAMPLEDIR}/interop
  COMPONENT dev)

install(
  DIRECTORY hello/hello-decvpp hello/hello-createsession hello/hello-transcode
  DESTINATION ${ONEAPI_INSTALL_EXAMPLEDIR}/hello
  COMPONENT dev)
