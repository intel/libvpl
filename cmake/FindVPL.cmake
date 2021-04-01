# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################

# Find project local libraries, only for use within project
#
# This module defines
#
# VPL_<component>_FOUND VPL_IMPORTED_TARGETS

if(NOT API_VERSION_MAJOR)
  set(VPL_api_FOUND 0)
  set(VPL_dispatcher_FOUND 0)
  set(VPL_FOUND FALSE)
  return()
endif()

set(VPL_INTERFACE_VERSION ${API_VERSION_MAJOR})

# VPL::dispatcher
add_library(VPL::dispatcher ALIAS VPL)
list(APPEND VPL_IMPORTED_TARGETS VPL::dispatcher)
set(VPL_dispatcher_FOUND 1)

add_library(VPL::api ALIAS vpl-api)
list(APPEND VPL_IMPORTED_TARGETS VPL::api)
set(VPL_api_FOUND 1)

set(VPL_FOUND 1)
