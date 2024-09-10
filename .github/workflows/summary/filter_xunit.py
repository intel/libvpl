#!/usr/bin/env python3
############################################################################
# Copyright (C) 2020 Intel Corporation
#
# SPDX-License-Identifier: MIT
############################################################################
"""
Post-process xUnit tests.

"""
import argparse
import os
import csv
import defusedxml.ElementTree as DET

## Alternate get testcase ... name +if skipped failure or error


def read_command_line(cmd_line):
    """Read command line arguments"""
    parser = argparse.ArgumentParser(
        description=globals()['__doc__'],
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)

    parser.add_argument('xunit_path',
                        metavar="FILE",
                        help="xUnit file to process",
                        type=os.path.abspath,
                        action='store')

    parser.add_argument('-i',
                        dest='include_list',
                        nargs='+',
                        metavar="substring",
                        help="list of test name substrings to include",
                        default=["Double", "Null", "Unsupported", "Invalid"])

    parser.add_argument('-o',
                        dest='csv_path',
                        metavar="csvfile",
                        help="path to the generated output file",
                        default="out.csv")

    args = parser.parse_args(args=cmd_line)

    return args


def read_xunit(xunit_path):
    """Read data from xunit file into dictionary"""
    print("filename:", xunit_path)
    tree = DET.parse(xunit_path)
    root = tree.getroot()
    data = {}
    for test_case in root.findall("testcase"):
        name = ""
        if "classname" in test_case.attrib:
            classname = test_case.attrib["classname"].split(".")
            if classname:
                name += ".".join(classname) + "."
        name += test_case.attrib['name']

        failure = test_case.find("failure")
        if failure is not None:
            data[name] = "failure"
            continue

        error = test_case.find("error")
        if error is not None:
            data[name] = "error"
            continue

        skip = test_case.find("skipped")
        if skip is not None:
            data[name] = "skipped"
            continue

        data[name] = "pass"
    return data


def filter_xunit(data, include_list):
    """Filter xunit data"""
    filtered_data = {
        k: v
        for k, v in data.items()
        if any(substring in k for substring in include_list)
    }
    return filtered_data


def write_csv(data, csv_file):
    """Write data to CSV file"""
    with open(csv_file, 'w', newline='', encoding="utf-8") as csvfile:
        csvwriter = csv.writer(csvfile,
                               delimiter=',',
                               quotechar='|',
                               quoting=csv.QUOTE_MINIMAL)
        for k, result in data.items():
            csvwriter.writerow([k, result])


def run(args):
    """main entry point"""
    raw_data = read_xunit(args.xunit_path)
    filtered_data = filter_xunit(raw_data, args.include_list)
    write_csv(filtered_data, args.csv_path)


if __name__ == '__main__':
    run(read_command_line(None))
