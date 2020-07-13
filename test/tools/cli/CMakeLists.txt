# ##############################################################################
# Copyright (C) 2020 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
cmake_minimum_required(VERSION 3.10.2)

add_executable(vpl-encode vpl-encode.cpp)
target_link_libraries(vpl-encode mfx)

add_executable(vpl-decode vpl-decode.cpp)
target_link_libraries(vpl-decode mfx)

add_executable(vpl-decode-newloader vpl-decode-newloader.cpp)
target_link_libraries(vpl-decode-newloader mfx)
