#!/bin/bash
# ==============================================================================
# Copyright (C) 2018-2020 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==============================================================================
# oneAPI Video Processing Library (oneVPL) docker sample execution script
docker run -it --rm -v ${PWD}/../test/content:/content vpl:18.04 /oneVPL/_build/hello_decode h265 /content/cars_128x96.h265 out_128x96.yuv
docker run -it --rm -v ${PWD}/../test/content:/content vpl:20.04 /oneVPL/_build/hello_decode h265 /content/cars_128x96.h265 out_128x96.yuv
