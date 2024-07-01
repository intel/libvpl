#!/bin/bash
###############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
###############################################################################
# Utility functions for artifact handling

function copy_artifact() {
    source_dir="$source_root/$1"
    source_file="$2"
    dest_dir="$dest_root/$3"
    dest_file="${4:-$source_file}"
    if [ -f "$source_dir/$source_file" ]; then
        if [ ! -d "$dest_dir" ]; then
        mkdir -p "$dest_dir"
        fi
        cp "$source_dir/$source_file" "$dest_dir/$dest_file"
    fi
}

function copy_all_artifacts() {
    source_dir="$source_root/$1"
    dest_dir="$dest_root/$2"
    if [ -d "$source_dir" ]; then
        if [ ! -d "$dest_dir" ]; then
        mkdir -p "$dest_dir"
        fi
        cp -r "$source_dir/." "$dest_dir/"
    fi
}

function copy_glob_artifact() {
    local prev_shopt=$(shopt -p nullglob)
    shopt -s nullglob
    source_glob="$source_root/$1"
    dest_dir="$2"
    dest_file="$3"
    saved_IFS="$IFS"
    IFS=
    for f in $source_glob
    do
        f_rel="$(realpath -s --relative-to="$source_root" "$f")"
        f_dir="$(dirname "$f_rel")"
        f_name="$(basename "$f_rel")"
        copy_artifact "$f_rel" "$f_name" "$dest_dir" "$dest_file"
    done
    IFS="$saved_IFS"
    ${prev_shopt}
}