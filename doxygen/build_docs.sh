#!/bin/bash
###############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
###############################################################################
# Build Doxygen documentation
SCRIPT_DIR=$(dirname $0)

doxygen $SCRIPT_DIR/Doxyfile
cp -r assets/ html/
