#!/usr/bin/env python
############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
############################################################################
"""Black Duck Binary Analysis Tool

This tool allows uploading scan results to BDBA and downloading results needed
for SDL.

It returns 0 on success.
"""

# import modules used here -- sys is a very standard one
import argparse
import logging
import sys
import os
from contextlib import contextmanager
import json
import time
import requests

from requests.adapters import HTTPAdapter
from urllib3.util import Retry

PROGNAME = "bdba"
SERVER = "https://bdba001.icloud.intel.com"
POLLING_INTERVAL = 10


def fail(*args, **kwargs):
    """Exit with error message."""
    print(
        sys.argv[0] + ": ",
        end='',
        file=sys.stderr,
    )
    print(*args, file=sys.stderr, flush=True, **kwargs)
    sys.exit(-1)


@contextmanager
def restrict_output():
    """Define context manager to restrict output."""
    with open(os.devnull, "w", encoding='utf-8') as devnull:
        standard_error = sys.stderr
        sys.stderr = devnull
        try:
            yield devnull
        finally:
            sys.stderr = standard_error


def requests_with_retry(retries=3,
                        backoff_factor=0.3,
                        status_forcelist=(500, 502, 504),
                        session=None):
    """Return request session with retry."""
    session = session or requests.Session()
    retry = Retry(total=retries,
                  read=retries,
                  connect=retries,
                  backoff_factor=backoff_factor,
                  status_forcelist=status_forcelist)
    adapter = HTTPAdapter(max_retries=retry)
    session.mount('http://', adapter)
    session.mount('https://', adapter)
    return session


def format_attributes(**attributes):
    """Return a string of comma-separated key-value pairs."""
    return ", ".join(f"{param}: {value}"
                     for param, value in attributes.items())


def upload(filename, credentials, server_url=SERVER, **kwargs):
    """Upload a new application and return product_id.
    """
    logging.debug("Uploading application to server")
    if not os.path.isfile(filename):
        fail(f"cannot access `{filename}`: No such file")

    archive_name = os.path.basename(filename)
    headers = {}
    group = '<default>'
    if 'group' in kwargs:
        group = kwargs['group']
        headers['Group'] = group
    if 'version' in kwargs:
        version = kwargs['version']
        headers['Version'] = version
    try:
        with restrict_output():
            with open(filename, 'rb') as binary:
                response_upload = requests_with_retry().put(
                    f'{server_url}/api/upload/{archive_name}',
                    data=binary,
                    auth=(credentials['user'], credentials['password']),
                    verify=False,
                    headers=headers)
        response_upload.raise_for_status()

        logging.info("%s has been uploaded to %s %s", archive_name, server_url,
                     headers)

        logging.debug("response: %s", response_upload.text)
    except requests.exceptions.RequestException as err:
        fail(err)

    product_id = report_url = product_name = None
    try:
        parsed_string = json.loads(response_upload.text)
        product_id = parsed_string['results']['product_id']
        report_url = parsed_string['results']['report_url']
        product_name = os.path.splitext(
            os.path.basename(parsed_string['results']['filename']))[0]
    except json.JSONDecodeError:
        fail(f"failed to parse response: "
             f"Product ID: '{product_id}' "
             f"Product URL: '{report_url} "
             f"Product name: '{product_name}'")
    else:
        logging.info(
            "response: Product ID: '%s' Product URL: '%s Product name: '%s'",
            product_id, report_url, product_name)

    return product_id


def wait_for_results(product_id, credentials, server_url=SERVER):
    """Waits for results and returns parsed json string.
    """
    print("Waiting for analysis results to be ready",
          file=sys.stderr,
          end='',
          flush=True)
    status = "B"
    while status == "B":
        try:
            with restrict_output():
                response_info = requests_with_retry().get(
                    f'{server_url}/api/product/{product_id}/',
                    auth=(credentials['user'], credentials['password']),
                    verify=False)
            response_info.raise_for_status()
        except requests.exceptions.RequestException as err:
            fail(err)
        logging.debug("response: %s", response_info.text)
        try:
            parsed_string = json.loads(response_info.text)
            status = parsed_string['results']['status']
        except json.JSONDecodeError:
            fail("failed to parse response: status {status}")
        if status == "B":
            print(".", file=sys.stderr, end='', flush=True)
            time.sleep(POLLING_INTERVAL)
    print("", file=sys.stderr, flush=True)
    if status == 'F':
        fail("Analysis failed on server")
    return parsed_string


def get_components(product_id, credentials, server_url=SERVER, filename=None):
    """Retrieve components as CSV.
    """
    if not filename:
        return
    logging.debug("Retrieving components as CSV")
    try:
        with restrict_output():
            response = requests_with_retry().get(
                f'{server_url}/api/product/{product_id}/csv-libs',
                auth=(credentials['user'], credentials['password']),
                verify=False)
            response.raise_for_status()
    except requests.exceptions.RequestException as err:
        fail(err)
    logging.debug("response: %s", response.text)
    try:
        with open(filename, 'wb') as components_csv:
            components_csv.write(response.content)
    except IOError as err:
        fail(f"cannot write `{filename}`: {os.strerror(err.errno)}")


def get_vulns(product_id, credentials, server_url=SERVER, filename=None):
    """Retrieve known vulnerabilities as CSV.
    """
    if not filename:
        return
    logging.debug("Retrieving known vulnerabilities as CSV")
    try:
        with restrict_output():
            response = requests_with_retry().get(
                f'{server_url}/api/product/{product_id}/csv-vulns?cvss_version=3',
                auth=(credentials['user'], credentials['password']),
                verify=False)
            response.raise_for_status()
    except requests.exceptions.RequestException as err:
        fail(err)
    logging.debug("response: %s", response.text)
    try:
        with open(filename, 'wb') as vulns_csv:
            vulns_csv.write(response.content)
    except IOError as err:
        fail(f"cannot write `{filename}`: {os.strerror(err.errno)}")


def get_summary(product_id, credentials, server_url=SERVER, filename=None):
    """Retrieve analysis summary as PDF.
    """
    if filename:
        logging.debug("Retrieving analysis summary as PDF")
    try:
        with restrict_output():
            response = requests_with_retry().get(
                f'{server_url}/api/product/{product_id}/pdf-report?cvss_version=3',
                auth=(credentials['user'], credentials['password']),
                verify=False)
            response.raise_for_status()
    except requests.exceptions.RequestException as err:
        fail(err)
    logging.debug("response: %s", response.text)
    try:
        with open(filename, 'wb') as report_csv:
            report_csv.write(response.content)
    except IOError as err:
        fail(f"cannot write `{filename}`: {os.strerror(err.errno)}")


def main():
    """Gather our code in a main() function"""
    parser = argparse.ArgumentParser(
        prog=PROGNAME,
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('BINARY', type=str)
    parser.add_argument('-u', '--user', type=str, required=True)
    parser.add_argument('-p', '--password', type=str, required=True)
    parser.add_argument('-g', '--group', type=str, required=True)
    parser.add_argument('-V', '--version', type=str)
    parser.add_argument('-r', '--resultfile', type=str)
    parser.add_argument('-c', '--componentfile', type=str)
    parser.add_argument('-v', '--vulnfile', type=str)
    parser.add_argument('--debug', action='store_true')

    args = parser.parse_args()
    if args.debug:
        logging.basicConfig(format='%(levelname)s:%(message)s',
                            stream=sys.stderr,
                            level=logging.DEBUG)
    else:
        logging.basicConfig(format='%(levelname)s:%(message)s',
                            stream=sys.stderr,
                            level=logging.INFO)

    credentials = {'user': args.user, 'password': args.password}

    product_id = upload(args.BINARY,
                        credentials,
                        group=args.group,
                        version=args.version)
    #product_id = 2720487
    results = wait_for_results(product_id, credentials)
    print(results)
    verdict = results['results']['summary']['verdict'][
        'short'] + '\n' + results['results']['summary']['verdict']['detailed']

    print(f"verdict: {verdict}", file=sys.stderr)
    get_vulns(product_id, credentials, filename=args.vulnfile)
    get_components(product_id, credentials, filename=args.componentfile)
    get_summary(product_id, credentials, filename=args.resultfile)
    if not verdict.startswith("Pass"):
        sys.exit(1)
    sys.exit(0)


# Standard boilerplate to call the main() function to begin
# the program.
if __name__ == '__main__':
    main()
