############################################################################
# Copyright (C) 2020 Intel Corporation
#
# SPDX-License-Identifier: MIT
############################################################################
"""
Utility functions
"""

import sys
import subprocess
import queue
import datetime
import threading


def _capture_output(pipe, handlers):
    """
    read lines from a file like object and
    call zero or more handlers for each line
    """
    for line in iter(pipe.readline, ''):
        for handler in handlers:
            handler(line)
    pipe.close()


def _write_output(source, dest, prefix=''):
    """
    read lines from a queue like source and
    write them to a file like destination
    """
    for line in iter(source, None):
        dest.write(prefix + line)
        dest.flush()


# pylint: disable=too-few-public-methods
class Runner:
    """Process execution wrapper"""
    def __init__(self):
        # Set up thread i/o storage. (Note: Queue is synchronized)
        self.stderr_q = queue.Queue()
        self.stdout_q = queue.Queue()
        self.stdout = []
        self.stderr = []
        self.capture_threads = []
        self.write_threads = []

    def run(self, cmd, timeout=None):
        """run a command"""
        timeout_exceeded = False
        proc = subprocess.Popen(cmd,
                                stdout=subprocess.PIPE,
                                stderr=subprocess.PIPE,
                                universal_newlines=True,
                                close_fds=(sys.platform != 'win32'),
                                bufsize=1)
        self._start_threads(proc)
        # Wait for process to complete
        busy = True
        start_time = datetime.datetime.now()
        while busy:
            busy = False
            try:
                proc.wait(5)
            except subprocess.TimeoutExpired:
                end_time = datetime.datetime.now()
                if timeout is None or (end_time -
                                       start_time).total_seconds() < timeout:
                    busy = True
                else:
                    print("TIMEOUT!")
                    timeout_exceeded = True
                    proc.kill()
        self._end_threads()
        if timeout_exceeded:
            raise RuntimeError(
                "Timeout exceeded: process killed after {} seconds".format(
                    timeout))
        return (proc.returncode, ''.join(self.stdout), ''.join(self.stderr))

    def _start_threads(self, proc):
        """start i/o capture threads"""
        try:
            # Set up reader threads
            self.capture_threads = [
                threading.Thread(target=_capture_output,
                                 args=(proc.stdout,
                                       [self.stdout_q.put,
                                        self.stdout.append])),
                threading.Thread(target=_capture_output,
                                 args=(proc.stderr,
                                       [self.stderr_q.put,
                                        self.stderr.append]))
            ]

            # Set up writer threads
            self.write_threads = [
                threading.Thread(target=_write_output,
                                 args=(self.stderr_q.get, sys.stderr)),
                threading.Thread(target=_write_output,
                                 args=(self.stdout_q.get, sys.stdout))
            ]

            # Start threads
            for thread in self.capture_threads:
                thread.daemon = True
                thread.start()
            for thread in self.write_threads:
                thread.daemon = True
                thread.start()
        except OSError as err:
            self.capture_threads = []
            print("Problem capturing process output")
            print(str(err))
        except:
            self.capture_threads = []
            print("Unexpected error")
            raise

    def _end_threads(self):
        """end i/o capture threads"""
        # Wait for reader threads to complete
        for thread in self.capture_threads:
            thread.join()

        # Signal writer threads to finish
        self.stdout_q.put(None)
        self.stderr_q.put(None)
        sys.stderr.flush()
        sys.stdout.flush()


def run_cmd(cmd, timeout=None):
    """run a command line operation"""
    return Runner().run(cmd, timeout)
