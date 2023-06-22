#!/bin/bash
###############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
###############################################################################
## start of boilerplate to switch to project root ------------------------------
script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
proj_dir="$( dirname "${script_dir}" )"
cd ${script_dir}

HOST_OS='linux'
UNAME=$( command -v uname)

case $( "${UNAME}" | tr '[:upper:]' '[:lower:]') in
  msys*|cygwin*|mingw*)
    # MinGW/MSys type Bash on windows
    HOST_OS='windows'
    ;;
esac
## start of commands -----------------------------------------------------------
RETURNCODE=0

export TOOL="valgrind"

INDEX=0
RETURNCODE=0
SUPPRESSIONS=${script_dir}/default_suppressions.txt

TOOL_OPTS="--tool=memcheck --leak-check=yes --show-leak-kinds=all --num-callers=50 --undef-value-errors=no --errors-for-leak-kinds=definite,indirect,possible --error-exitcode=97 --suppressions=${SUPPRESSIONS}"
while IFS='' read -r COMMAND || [[ -n "${COMMAND}" ]]; do
    if [[ ${COMMAND} =~ ^#.* ]]; then
        # skip comments
        continue
    fi
    TEST_NAME="${COMMAND}"
    echo "Started ${TEST_NAME}"

    echo ""
    echo "Test# $INDEX:"
    echo "${COMMAND}"
    ${COMMAND}
    ERR_CODE=$?
    if [[ "$ERR_CODE" -ne 0 ]]; then
        echo "${TEST_NAME} reported error ${ERR_CODE}"
        echo "Finished ${TEST_NAME}"
        continue
    fi

    echo ""
    echo "${TOOL} <opts> ${COMMAND}"
    ${TOOL} ${TOOL_OPTS} ${COMMAND}
    ERR_CODE=$?
    if [[ "$ERR_CODE" -eq 1 ]]; then
        echo "Dynamic analysis of ${TEST_NAME} reported error ${ERR_CODE}"
        echo "Fatal dynamic analysis error"
        echo "Exiting..."
        RETURNCODE=1
        break
    fi
    if [[ "$ERR_CODE" -eq 97 ]]; then
        echo "Dynamic analysis of ${TEST_NAME} reported error ${ERR_CODE}"
        echo "Result: FAIL"
        RETURNCODE=1
    else
        echo "Result: PASS"
    fi

    ((INDEX++))
done < ${script_dir}/tests.txt

exit $RETURNCODE
