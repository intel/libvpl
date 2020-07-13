#!/bin/bash
###############################################################################
# Copyright (C) 2020 Intel Corporation
#
# SPDX-License-Identifier: MIT
###############################################################################
# Configure environment variables

# Append to a env search path and correctly handle delimiters if value is null
append_path ()
{
  local path="$1"
  local value="$2"
  echo "$path${value:+:${value}}" # if value is null, nothing is substituted
}

ENV_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
export VPL_ROOT="$(dirname "$ENV_DIR" )"
export VPL_INCLUDE="$VPL_ROOT/include"
export VPL_LIB="$VPL_ROOT/lib"
export VPL_BIN="$VPL_ROOT/bin"

export CPATH=$(append_path $VPL_INCLUDE $CPATH)
export LIBRARY_PATH=$(append_path "$VPL_LIB" $LIBRARY_PATH)
export PATH=$(append_path "$VPL_BIN" "$PATH")
export LD_LIBRARY_PATH=$(append_path "$VPL_LIB" $LD_LIBRARY_PATH)
