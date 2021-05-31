# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
# oneAPI Video Processing Library (oneVPL) main build script

cmake_minimum_required(VERSION 3.10.2)

add_subdirectory(sample_common)
add_subdirectory(sample_decode)
add_subdirectory(sample_vpp)
add_subdirectory(sample_encode)
add_subdirectory(sample_misc/wayland)
