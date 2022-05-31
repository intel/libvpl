# ##############################################################################
# Copyright (C) 2018 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
# oneAPI Video Processing Library (oneVPL) main build script

add_subdirectory(media_sdk_compatibility_headers)
add_subdirectory(sample_common)
add_subdirectory(sample_decode)
add_subdirectory(sample_vpp)
add_subdirectory(sample_encode)
add_subdirectory(sample_multi_transcode)
add_subdirectory(sample_misc/wayland)
add_subdirectory(metrics_monitor)
