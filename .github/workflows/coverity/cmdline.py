############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
############################################################################
"""commandline like tools"""

import shutil
import os
import sys
from contextlib import contextmanager
import time
import subprocess  # nosec

VERBOSE = False
if 'VERBOSE' in os.environ:
    if os.environ['VERBOSE'] not in ['']:
        VERBOSE = True


def _resolve_path(path):
    """resolve a path name from either a string, or a list of sub-paths"""
    if isinstance(path, list):
        return os.path.join(*path)
    return path


def _escape_cmd_arg(arg):
    """quote/escape and argument for a command line call so that it can
    be safely used even if it has special charaters"""
    if ' ' in arg or '"' in arg:
        return '"' + arg.replace('"', '""') + '"'
    return arg


def log(message):
    """Log activity"""
    if VERBOSE:
        print(f">> {message}", file=sys.stderr)
        sys.stderr.flush()


@contextmanager
def pushd(dst):
    """change working directory"""
    cur_dir = os.getcwd()
    dest = os.path.join(cur_dir, _resolve_path(dst))
    os.chdir(dest)
    log(f'pushd {dest}')
    try:
        yield
    finally:
        log(f'popd -> {cur_dir}')
        os.chdir(cur_dir)


#pylint: disable=invalid-name
def rm(target):
    """delete a file or folder"""
    target = _resolve_path(target)
    if os.path.exists(target):
        # Delete sometimes fails if done immediately, timeout
        # is not great, but allows filesystem settings to stabilize.
        timeout = time.time() + 10
        while time.time() < timeout:
            try:
                if os.path.isfile(target):
                    log(f'rm {target}')
                    os.remove(target)
                    break
                if os.path.isdir(target):
                    log(f'rm -rf {target}')
                    shutil.rmtree(target)
                    break
            except PermissionError:
                time.sleep(1)


#pylint: disable=invalid-name
def md(target):
    """make a folder"""
    target = _resolve_path(target)
    if target and not os.path.exists(target):
        log(f'mkdir -p {target}')
        os.makedirs(target)


#pylint: disable=invalid-name
def cp(src, dest):
    """copy a file or folder"""
    src = _resolve_path(src)
    dest = _resolve_path(dest)
    if os.path.exists(src):
        rm(dest)
        md(os.path.dirname(src))
        if os.path.isfile(src):
            log(f'cp {src} {dest}')
            shutil.copyfile(src, dest)
        elif os.path.isdir(src):
            log(f'cp {src} {dest}')
            shutil.copytree(src, dest)
        else:
            raise RuntimeError("Cannot copy unknown file type")


def join_command(command):
    """Join a series or parameters into a command, escaping if needed"""
    return ' '.join([_escape_cmd_arg(argument) for argument in command])


def run_command(*args, no_throw=False, env=None):
    """Run a command"""
    if len(args) == 1:
        cmd = args[0]
    else:
        cmd = join_command(args)
    log(f'{cmd}')
    if os.name != 'nt':
        cmd = "exec bash -c '" + cmd + "'"
    with subprocess.Popen(cmd, shell=True, env=env) as proc:  # nosec
        proc.communicate()
        if not no_throw and proc.returncode != 0:
            raise RuntimeError("Error running command: " + cmd)
        return proc.returncode


def run_commands(*args, no_throw=False, env=None):
    """Run several commands"""
    commands = []
    for arg in args:
        if isinstance(arg, (str)):
            commands.append(arg)
        else:
            commands.append(join_command(arg))
    if os.name == 'nt':
        script_file = "temp.bat"
    else:
        script_file = "temp.sh"
    with open(script_file, "w", encoding="utf-8") as script:
        log('echo "')
        for cmd in commands:
            log(f'{cmd}')
            script.write(cmd + "\n")
        log(f'" > {script_file}')
    log('{script_file}')
    if os.name == 'nt':
        cmd = script_file
    else:
        cmd = "exec bash -c 'source " + script_file + "'"
    with subprocess.Popen(cmd, shell=True, env=env) as proc:  # nosec
        proc.communicate()
        rm(script_file)
        if not no_throw and proc.returncode != 0:
            raise RuntimeError("Error running: \n" + "\n".join(commands))
        return proc.returncode


def capture_command(*args, env=None):
    """Run a command and capture the output"""
    if len(args) == 1:
        cmd = args[0]
    else:
        cmd = join_command(args)
    log(f'{cmd}')
    if os.name != 'nt':
        cmd = "exec bash -c '" + cmd + "'"
    with subprocess.Popen(  # nosec
            cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            universal_newlines=True,
            shell=True,
            env=env) as proc:
        result = proc.communicate()
        return (result[0], result[1], proc.returncode)


def capture_commands(*args, env=None):
    """Run several commands and capture the output"""
    commands = []
    for arg in args:
        if not arg:
            continue
        if isinstance(arg, (str)):
            commands.append(arg)
        else:
            commands.append(join_command(arg))
    script_file = None
    if os.name == 'nt':
        script_file = "temp.bat"
    else:
        script_file = "temp.sh"
    with open(script_file, "w", encoding="utf-8") as script:
        log('echo "')
        for cmd in commands:
            log(f'{cmd}')
            script.write(cmd + "\n")
        log(f'" > {script_file}')
    log('{script_file}')
    if os.name == 'nt':
        cmd = script_file
    else:
        cmd = "exec bash -c 'source " + script_file + "'"
    with subprocess.Popen(  # nosec
            cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            universal_newlines=True,
            shell=True,
            env=env) as proc:
        result = proc.communicate()
        rm(script_file)
        return (result[0], result[1], proc.returncode)
