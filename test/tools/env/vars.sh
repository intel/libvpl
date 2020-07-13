#!/usr/bin/env bash
############################################################################
# Copyright (C) 2019 Intel Corporation
#
# SPDX-License-Identifier: MIT
############################################################################

HOST_OS='linux'
UNAME=$( command -v uname)

case $( "${UNAME}" | tr '[:upper:]' '[:lower:]') in
  msys*|cygwin*|mingw*)
    # MinGW/MSys type Bash on windows
    HOST_OS='windows'
    ;;
esac

# Append to a env search path and correctly handle delimiters if value is null
append_path ()
{
  local path="$1"
  local value="$2"
  echo "$path${value:+:${value}}" # if value is null, nothing is substituted
}

# Configure enviroment variables
ENV_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
export VPL_TOOLS_DIR="$(dirname "$ENV_DIR" )"
export VPL_TOOLS_PYTHONPATH=$VPL_TOOLS_DIR/python
export PYTHONPATH=$(append_path "$VPL_TOOLS_PYTHONPATH" $PYTHONPATH)
