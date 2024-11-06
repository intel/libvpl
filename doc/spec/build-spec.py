#!/usr/bin/env python3
############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
############################################################################
"""
Build specification.
"""
# pylint: disable=invalid-name,consider-using-f-string

# Main script to build the specification. Derived from:
# github.com/oneapi-src/oneAPI-spec/blob/main/scripts/oneapi.py

import argparse
import glob
import os
import os.path
import sys
import shutil
import subprocess  # nosec B404
from functools import wraps
from os.path import join

sphinx_build = 'sphinx-build'
source_dir = 'source'
build_dir = 'build'
doxygen_dir = 'doxygen'
doxygen_xml = join(doxygen_dir, 'xml', 'index.xml')

indent = 0
cl_args = []


def action(func):
    """Execute and log action"""
    @wraps(func)
    def wrapped(*args, **kwargs):
        global indent  # pylint: disable=W0603
        log('%s: %s' % (
            args[1] if len(args) > 1 and args[1] else wrapped.__name__,
            args[0],
        ))
        indent += 2
        x = func(*args, **kwargs)
        indent -= 2
        return x

    return wrapped


class cd:
    """Context manager for changing the current working directory"""
    def __init__(self, newPath):
        self.savedPath = ""
        self.newPath = os.path.expanduser(newPath)

    def __enter__(self):
        self.savedPath = os.getcwd()
        log('cd ' + self.newPath)
        os.chdir(self.newPath)

    def __exit__(self, etype, value, traceback):
        os.chdir(self.savedPath)


def log(*args, **kwargs):
    """Log message"""
    print(indent * ' ' + ' '.join(map(str, args)), flush=True, **kwargs)


def shell(c):
    """Execute shell command"""
    log(c)
    if cl_args.dry_run:
        return
    subprocess.check_call(c, shell=True)  # nosec B602


def copy(src, dst):
    """Execute copy command"""
    log('cp', src, dst)
    if cl_args.dry_run:
        return
    shutil.copy(src, dst)


def makedirs(path):
    """Execute mkdir command"""
    log('mkdir -p', path)
    if cl_args.dry_run:
        return
    os.makedirs(path)


def sphinx(root, target):
    """Execute sphinx build"""
    if not cl_args.verbose:
        os.environ['LATEXMKOPTS'] = '--silent'
        os.environ['LATEXOPTS'] = '-interaction=nonstopmode -halt-on-error'
    sphinx_args = '-N'
    if not cl_args.verbose:
        sphinx_args += ' -q'
    if not cl_args.serial:
        sphinx_args += ' -j auto'
    if cl_args.a:
        sphinx_args += ' -a'
    if cl_args.n:
        sphinx_args += ' -n'
    if cl_args.W:
        sphinx_args += ' -W'

    shell('%s -M %s %s %s %s' % (
        sphinx_build,
        target,
        join(root, source_dir),
        join(root, build_dir),
        sphinx_args,
    ))


def get_env(var):
    """Get environment variable"""
    return os.environ[var] if var in os.environ else ''


def root_only(root):
    """Check if in root dir"""
    if root != '.':
        sys.exit('Error: Only works from root')


@action
def dockerbuild(root, _target=None):
    """Build image"""
    cd(root)
    copy(join(root, 'requirements.txt'), join(root, 'docker'))
    copy(join(root, 'ubuntu-packages.txt'), join(root, 'docker'))
    copy(join(root, 'install.sh'), join(root, 'docker'))
    shell('docker build'
          ' -f ./doc/spec/docker/Dockerfile.build'
          ' --build-arg http_proxy=%s'
          ' --build-arg https_proxy=%s'
          ' --build-arg no_proxy=%s'
          ' --tag vpl-spec %s' %
          (get_env('http_proxy'), get_env('https_proxy'), get_env('no_proxy'),
           join(root, 'docker')))


@action
def dockerrun(_root, _target=None):
    """Run image"""
    shell('docker run --rm -it'
          ' -e http_proxy=%s'
          ' -e https_proxy=%s'
          ' -e no_proxy=%s'
          ' --user %s:%s'
          ' --volume=%s:/build'
          ' --workdir=/build'
          ' vpl-spec' % (
              get_env('http_proxy'),
              get_env('https_proxy'),
              get_env('no_proxy'),
              os.getuid(),  # pylint: disable=no-member
              os.getgid(),  # pylint: disable=no-member
              os.getcwd(),
          ))


@action
def clean(root, _target=None):
    """Execute sphinx clean"""
    apply_dirs(root, 'clean')
    sphinx(root, 'clean')


def command(root, target):
    """Execute command list"""
    commands[target](root, target)


def apply_dirs(root, target):
    """Execute command list"""
    elements = join(root, 'doc', 'spec')
    if os.path.exists(elements):
        for curr_dir in dirs:
            command(join(elements, curr_dir), target)


def up_to_date(target, deps):
    """Check if up to date"""
    if not os.path.exists(target):
        return False
    for dep in deps:
        if os.path.getmtime(target) < os.path.getmtime(dep):
            return False
    return True


def doxygen_files(root):
    """Get doxyfile"""
    return [join(root, 'Doxyfile')] + glob.glob(join('api', 'vpl', '**'),
                                                recursive=True)


def doxygen(root, _target=None):
    """Execute doxygen build"""
    with cd(root):
        doxyfile = 'Doxyfile'
        if not os.path.exists(doxyfile) or up_to_date(join(root, doxygen_xml),
                                                      doxygen_files(root)):
            return
        shell('doxygen %s' % doxyfile)


@action
def prep(root='.', _target=None):
    """Prepare doxygen build"""
    apply_dirs(root, 'prep')
    doxygen(root)


@action
def build(root, target):
    """Build sphinx"""
    prep(root)
    sphinx(root, target)


def remove_elements(li, elements):
    """Remove elements"""
    for e in elements:
        if e in li:
            li.remove(e)
    return li


@action
def sort_words(_root, _target=None):
    """Sort words in spell check"""
    with open(join('source', 'spelling_wordlist.txt'),
              encoding="utf-8") as fin:
        lines = fin.readlines()
    with open(join('source', 'spelling_wordlist.txt'), 'w',
              encoding="utf-8") as fout:
        for li in sorted(list(set(lines))):
            fout.write(li)


commands = {
    'clean': clean,
    'dockerbuild': dockerbuild,
    'dockerrun': dockerrun,
    'html': build,
    'latexpdf': build,
    'spelling': build,
    'singlehtml': build,
    'prep': prep,
    'sort-words': sort_words,
}

dirs = [
    'vpl',
]


def main():
    """Main specification build"""
    global cl_args  # pylint: disable=W0603
    parser = argparse.ArgumentParser(
        description='Build Intel® Video Processing Library spec.')
    parser.add_argument('action',
                        choices=commands.keys(),
                        default='html',
                        nargs='?')
    parser.add_argument('root', nargs='?', default=join('doc', 'spec'))
    parser.add_argument('--branch')
    parser.add_argument('--verbose', action='store_true')
    parser.add_argument('--dry-run', action='store_true')
    parser.add_argument('--serial', action='store_true')
    parser.add_argument('-W', action='store_true')
    parser.add_argument('-a',
                        action='store_true',
                        help='sphinx -a (build all files)')
    parser.add_argument('-n',
                        action='store_true',
                        help='sphinx -n (nitpicky mode)')
    cl_args = parser.parse_args()

    commands[cl_args.action](cl_args.root, cl_args.action)


main()
