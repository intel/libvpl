# ##############################################################################
# Copyright (C) 2021 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################

set(TARGET media_sdk_compatibility_headers)

add_library(${TARGET} INTERFACE)

target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
