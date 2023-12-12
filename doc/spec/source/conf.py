# SPDX-FileCopyrightText: 2019-2020 Intel Corporation
#
# SPDX-License-Identifier: MIT
"""
Main configuration file for specification build.
"""
# pylint: disable=invalid-name,exec-used

# For Intel® Video Processing Library (Intel® VPL)

# -*- coding: utf-8 -*-
#
# Configuration file for the Sphinx documentation builder.
#
# This file does only contain a selection of the most common options. For a
# full list see the documentation:
# http://www.sphinx-doc.org/en/master/config

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
import sys
from os.path import join

project = 'Intel\u00AE VPL'

repo_root = '..'
sys.path.append(repo_root)
exec(open(join(repo_root, 'common_conf.py'), encoding="utf-8").read())  # nosec # pylint: disable=R1732
exec(open(join(repo_root, 'element_conf.py'), encoding="utf-8").read())  # nosec # pylint: disable=R1732

cpp_id_attributes = ['MFX_CDECL', 'MFX_DEPRECATED']

c_id_attributes = ['MFX_CDECL', 'MFX_DEPRECATED']

spelling_word_list_filename = ['../spelling_wordlist.txt']
