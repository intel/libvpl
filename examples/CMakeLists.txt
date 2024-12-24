# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################

cmake_minimum_required(VERSION 3.13.0)

if(BUILD_EXAMPLES)
  add_subdirectory(api1x_core/legacy-decode)
  add_subdirectory(api1x_core/legacy-vpp)
  add_subdirectory(api1x_core/legacy-encode)
  add_subdirectory(api2x/hello-decode)
  if(WIN32)
    add_subdirectory(api2x/hello-sharing-dx11)
    add_subdirectory(api2x/hello-sharing-ocl)
  else()
    add_subdirectory(api2x/hello-sharing-vaapi)
  endif()
  add_subdirectory(api2x/hello-decvpp)
  add_subdirectory(api2x/hello-encode)
  add_subdirectory(api2x/hello-encode-jpeg)
  add_subdirectory(api2x/hello-transcode)
  add_subdirectory(api2x/hello-vpp)
  add_subdirectory(tutorials/01_transition/VPL)
endif()

if(INSTALL_EXAMPLES)
  install(
    DIRECTORY content
    DESTINATION ${VPL_INSTALL_EXAMPLEDIR}
    COMPONENT ${VPL_COMPONENT_DEV})

  install(
    DIRECTORY api1x_core/legacy-decode api1x_core/legacy-vpp
              api1x_core/legacy-encode
    DESTINATION ${VPL_INSTALL_EXAMPLEDIR}/api1x_core
    COMPONENT ${VPL_COMPONENT_DEV})

  install(
    DIRECTORY interop/vpl-infer
    DESTINATION ${VPL_INSTALL_EXAMPLEDIR}/interop
    COMPONENT ${VPL_COMPONENT_DEV})

  install(
    DIRECTORY api2x/hello-decode api2x/hello-decvpp api2x/hello-encode
              api2x/hello-transcode api2x/hello-vpp api2x/hello-encode-jpeg
    DESTINATION ${VPL_INSTALL_EXAMPLEDIR}/api2x
    COMPONENT ${VPL_COMPONENT_DEV})

  install(
    DIRECTORY tutorials/01_transition
    DESTINATION ${VPL_INSTALL_EXAMPLEDIR}/tutorials
    COMPONENT ${VPL_COMPONENT_DEV})

  if(WIN32)
    install(
      DIRECTORY api2x/hello-sharing-dx11 api2x/hello-sharing-ocl
      DESTINATION ${VPL_INSTALL_EXAMPLEDIR}/api2x
      COMPONENT ${VPL_COMPONENT_DEV})
  else()
    install(
      DIRECTORY api2x/hello-sharing-vaapi
      DESTINATION ${VPL_INSTALL_EXAMPLEDIR}/api2x
      COMPONENT ${VPL_COMPONENT_DEV})
  endif()
endif()
