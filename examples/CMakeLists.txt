# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
# oneAPI Video Processing Library (oneVPL) examples

cmake_minimum_required(VERSION 3.10.2)

if(BUILD_EXAMPLES)
  add_subdirectory(api1x_core/legacy-decode)
  add_subdirectory(api1x_core/legacy-vpp)
  add_subdirectory(api1x_core/legacy-encode)
  add_subdirectory(api2x/hello-createsession)
  add_subdirectory(api2x/hello-decode)
  add_subdirectory(api2x/hello-decvpp)
  add_subdirectory(api2x/hello-encode)
  add_subdirectory(api2x/hello-transcode)
  add_subdirectory(api2x/hello-vpp)
endif()

if(INSTALL_EXAMPLE_CODE)
  install(
    DIRECTORY content
    DESTINATION ${ONEAPI_INSTALL_EXAMPLEDIR}
    COMPONENT dev)

  install(
    DIRECTORY api1x_core/legacy-decode api1x_core/legacy-vpp
              api1x_core/legacy-encode
    DESTINATION ${ONEAPI_INSTALL_EXAMPLEDIR}/api1x_core
    COMPONENT dev)

  install(
    DIRECTORY interop/vpl-infer interop/dpcpp-blur
    DESTINATION ${ONEAPI_INSTALL_EXAMPLEDIR}/interop
    COMPONENT dev)

  install(
    DIRECTORY api2x/hello-createsession api2x/hello-decode api2x/hello-decvpp
              api2x/hello-encode api2x/hello-transcode api2x/hello-vpp
    DESTINATION ${ONEAPI_INSTALL_EXAMPLEDIR}/api2x
    COMPONENT dev)
endif()
