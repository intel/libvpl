#!/bin/sh
# shellcheck shell=sh
###############################################################################
# Copyright (C) 2020 Intel Corporation
#
# SPDX-License-Identifier: MIT
###############################################################################
# Configure environment variables

# ############################################################################

# Get absolute path to script, when sourced from bash, zsh and ksh shells.
# Uses `readlink` to remove links and `pwd -P` to turn into an absolute path.
# Converted into a POSIX-compliant function.

# Usage:
#   script_dir=$(get_script_path "$script_rel_path")
#
# Inputs:
#   script/relative/pathname/scriptname
#
# Outputs:
#   /script/absolute/pathname

# executing function in a *subshell* to localize vars and effects on `cd`
get_script_path() (
  script="$1"
  while [ -L "$script" ] ; do
    # combining next two lines fails in zsh shell
    script_dir=$(command dirname -- "$script")
    script_dir=$(cd "$script_dir" && command pwd -P)
    script="$(readlink "$script")"
    case $script in
      (/*) ;;
      (*) script="$script_dir/$script" ;;
    esac
  done
  # combining next two lines fails in zsh shell
  script_dir=$(command dirname -- "$script")
  script_dir=$(cd "$script_dir" && command pwd -P)
  echo "$script_dir"
)


# ############################################################################

# Even though this script is designed to be POSIX compatible, there are lines
# in the code block below that are _not_ POSIX compatible. This works within a
# POSIX compatible shell because they are single-pass interpreters. Each "if
# test" that checks for a non-POSIX shell (zsh, bash, etc.) will return a
# "false" condition in a POSIX shell and, thus, will skip the non-POSIX lines.
# This requires that the "if test" constructs _are_ POSIX compatible.

usage() {
  printf "%s\n"   "ERROR: This script must be sourced."
  printf "%s\n"   "Usage: source $1"
  return 2 2>/dev/null || exit 2
}

if [ -n "$ZSH_VERSION" ] ; then
  # shellcheck disable=2039,2015  # following only executed in zsh
  [[ $ZSH_EVAL_CONTEXT =~ :file$ ]] && vars_script_name="${(%):-%x}" \
      || usage "${(%):-%x}"
elif [ -n "$KSH_VERSION" ] ; then
  # shellcheck disable=2039,2015  # following only executed in ksh
  [[ $(cd "$(dirname -- "$0")" && \
         printf '%s' "${PWD%/}/")$(basename -- "$0") != "${.sh.file}" ]] \
    && vars_script_name="${.sh.file}" || usage "$0"
elif [ -n "$BASH_VERSION" ] ; then
  # shellcheck disable=2039,2015  # following only executed in bash
  (return 0 2>/dev/null) && vars_script_name="${BASH_SOURCE[0]}" || \
      usage "${BASH_SOURCE[0]}"
else
  case ${0##*/} in (sh|dash) vars_script_name="" ;; esac
fi

if [ "" = "$vars_script_name" ] ; then
  >&2 echo ":: ERROR: no support for sourcing from '[dash|sh]' shell." ;
  >&2 echo "   Can be caused by sourcing from a \"shebang-less\" script." ;
  return 1
fi


# ############################################################################

# Append to a env search path and correctly handle delimiters if value is null
append_path ()
{
  path_="$1"
  value_="$2"
  echo "$path_${value_:+:${value_}}" # if value is null, nothing is substituted
}

VPL_ROOT=$(dirname -- "$(get_script_path "$vars_script_name")")
export VPL_ROOT
VPL_INCLUDE="$VPL_ROOT/include"
export VPL_INCLUDE
VPL_LIB="$VPL_ROOT/lib"
export VPL_LIB
VPL_BIN="$VPL_ROOT/bin"
export VPL_BIN

CPATH=$(append_path "$VPL_INCLUDE" "$CPATH")
export CPATH
LIBRARY_PATH=$(append_path "$VPL_LIB" "$LIBRARY_PATH")
export LIBRARY_PATH
PATH=$(append_path "$VPL_BIN" "$PATH")
export PATH
LD_LIBRARY_PATH=$(append_path "$VPL_LIB" "$LD_LIBRARY_PATH")
export LD_LIBRARY_PATH
