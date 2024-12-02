#!/usr/bin/env python3
############################################################################
# Copyright (C) 2020 Intel Corporation
#
# SPDX-License-Identifier: MIT
############################################################################
"""
Summarize validation results.

The current version of this script expects to be able to
use standard Operating System ways of traversing a Release
artifact tree. This can be accomplished by mounting the
sharepoint folder in OneDrive, or by downloading a copy
of the folder.

Supported test formats (auto-detected):
XML (*.xml):
    Robot Framework XML Format
    jUnit XML Format (xUnit)

CSV (*.csv):
    Teamcity Test Result CSV
    Grid Report CSV
    General CSV
        Delimiter may be:
            ',' (comma), ';' (semi-colon), or '\\t' (tab)
        (optional) Test Suite Column may be:
            'suite', 'suite name', or 'test suite'
        Test Name Column may be:
            'test', 'test name', 'test case', or 'case'
        Result Column may be:
            'status', or 'result'

        Test name is: "<Test Suite>.<Test Name>" if Test Suite
        is set, otherwise "<Test Name>"

        Passing Results are (case insensitive):
            'pass', 'passed', or 'ok'

        Failing Results are (case insensitive):
            'fail', 'failed', or 'error'

        Skipped Results are (case insensitive):
            'skip', 'skipped', 'ignore', or 'ignored'

The defect database file is formatted as json in the form:
    {
        "Package Name" : {
            "Jira ID" : [
                "Test Name",
            ]
        },
    }

    for example:

    {
        "msdk_compatibility" : {
            "VPL7-1762" : [
                "hevce_10b_420_i010_reset.30",
                "hevce_8b_420_iyuv_reset.2",
                "hevce_8b_420_iyuv_reset.30"
            ]
        },
        "operating systems" : {
            "VPL7-3005" : [
                "oneVPL-cpu.Pipeline.1toN.Fused.Decvpp.H264.to I420 & BGRA system memory"
            ],
            "VPL7-3072" : [
                "oneVPL-gen.Decode.H264.Handles corrupted data",
                "oneVPL-gen.Decode.H264.I420"
            ]
        }
    }

"""
# pylint: disable=too-many-lines
import argparse
from argparse import RawTextHelpFormatter
from contextlib import contextmanager
from glob import glob
import os
import sys
import csv
import json
from xml.etree.ElementTree import SubElement  # nosec
from xml.etree.ElementTree import Element  # nosec
import xlsxwriter
from defusedxml import minidom
import defusedxml.ElementTree as DET

SCRIPT_PATH = os.path.realpath(
    os.path.join(os.getcwd(), os.path.dirname(__file__)))


def pretty_print_xml(root):
    """Format xml for legibility"""
    serial_xml = DET.tostring(root)
    parsed_xml = minidom.parseString(serial_xml)
    return parsed_xml.toprettyxml()


@contextmanager
def pushd(dest):
    """change working directory"""
    cur_dir = os.getcwd()
    os.chdir(dest)
    try:
        yield
    finally:
        os.chdir(cur_dir)


INDENT_LEVEL = 0
INDENT = ""


# pylint: disable=global-statement
@contextmanager
def log_scope():
    """Add a scope level to the logging stack"""
    global INDENT_LEVEL
    global INDENT
    INDENT_LEVEL += 1
    INDENT = "\t" * INDENT_LEVEL
    yield
    INDENT_LEVEL -= 1
    INDENT = "\t" * INDENT_LEVEL


def log(msg: str):
    """Log a message, indenting all lines based on the log scope"""
    for line in msg.splitlines():
        print(f"{INDENT}{line}")


def log_summary(name, summary):
    """Log the summary of a test suite"""
    unique = [
        summary.unique_attempt_count, summary.unique_run_count,
        summary.unique_pass_count, summary.unique_fail_count,
        summary.unique_skip_count
    ]
    unique = [str(value) for value in unique]
    total = [
        summary.total_attempt_count, summary.total_run_count,
        summary.total_pass_count, summary.total_fail_count,
        summary.total_skip_count
    ]
    total = [str(value) for value in total]
    if name is not None:
        log(name)
    log("       Attempt\tRun\tPass\tFail\tSkip")
    log(" Total:\t" + ("\t".join(total)))
    log("Unique:\t" + ("\t".join(unique)))


class TestResult:
    """Results of running a test (possibly more than once)"""
    def __init__(self, name):
        self.classname = None
        self.name = name
        self.runs = 0
        self.passes = 0
        self.fails = 0
        self.skips = 0
        self._dirty = True

    def clean(self):
        """Clear "dirty" flag for summary caching"""
        self._dirty = False

    @property
    def is_dirty(self):
        """Check "dirty" flag for summary caching"""
        return self._dirty

    def squash(self):
        """Rewrite result to look like a single test execution"""
        if self.fails:
            self.runs = 1
            self.passes = 0
            self.fails = 1
            self.skips = 0
        elif self.passes:
            self.runs = 1
            self.passes = 1
            self.fails = 0
            self.skips = 0
        elif self.skips:
            self.runs = 0
            self.passes = 0
            self.fails = 0
            self.skips = 1
        self._dirty = True

    def fill_xunit(self, suite_xml, test_class=None):
        """Add test result to an XML element in xUnit format"""
        for _ in range(self.passes):
            testcase = SubElement(suite_xml, "testcase")
            testcase.attrib["name"] = str(self.name)
            testcase.attrib["status"] = "Pass"
            if test_class is not None:
                testcase.attrib["classname"] = str(test_class)

        for _ in range(self.fails):
            testcase = SubElement(suite_xml, "testcase")
            testcase.attrib["name"] = str(self.name)
            if test_class is not None:
                testcase.attrib["classname"] = str(test_class)
            testcase.attrib["status"] = "Fail"
            failure = SubElement(testcase, "failure")
            failure.attrib["type"] = "Test Failed"

        for _ in range(self.skips):
            testcase = SubElement(suite_xml, "testcase")
            if test_class is not None:
                testcase.attrib["classname"] = str(test_class)
            testcase.attrib["status"] = "Skip"
            testcase.attrib["name"] = str(self.name)
            skipped = SubElement(testcase, "skipped")
            skipped.text = "Skipped"

    @property
    def pass_count(self):
        """Number of passes recorded"""
        return self.passes

    @property
    def fail_count(self):
        """Number of fails recorded"""
        return self.fails

    @property
    def skip_count(self):
        """Number of skips recorded"""
        return self.skips

    @property
    def run_count(self):
        """Number of runs recorded"""
        return self.runs

    @property
    def attempt_count(self):
        """Number of attempts recorded"""
        return self.runs + self.skips

    def update(self, other):
        """Add results from another test result"""
        self.runs += other.runs
        self.passes += other.passes
        self.fails += other.fails
        self.skips += other.skips
        self._dirty = True

    def add_pass(self):
        """Record a pass"""
        self.passes += 1
        self.runs += 1
        self._dirty = True
        return self

    def add_skip(self):
        """Record a skip"""
        self.skips += 1
        self._dirty = True
        return self

    def add_fail(self):
        """Record a fail"""
        self.fails += 1
        self.runs += 1
        self._dirty = True
        return self

    @property
    def unique_pass_count(self):
        """Report if all recorded results should be rolled up as a pass
        Reported as 1 or 0 to simplify accumulator logic higher in the stack
        """
        if self.runs > 0 and self.fails == 0:
            return 1
        return 0

    @property
    def unique_fail_count(self):
        """Report if all recorded results should be rolled up as a fail
        Reported as 1 or 0 to simplify accumulator logic higher in the stack
        """
        if self.fails > 0:
            return 1
        return 0

    @property
    def unique_skip_count(self):
        """Report if all recorded results should be rolled up as a skip
        Reported as 1 or 0 to simplify accumulator logic higher in the stack
        """
        if self.skips > 0 and self.runs == 0:
            return 1
        return 0

    @property
    def unique_run_count(self):
        """Report if all recorded results should be rolled up as a run
        Reported as 1 or 0 to simplify accumulator logic higher in the stack
        """
        if self.runs > 0:
            return 1
        return 0

    @property
    def unique_attempt_count(self):
        """Report if all recorded results should be rolled up as an attempt
        Reported as 1 or 0 to simplify accumulator logic higher in the stack
        """
        if self.skips > 0 or self.runs > 0:
            return 1
        return 0


class TestContainer:
    """A collection of test results"""
    def __init__(self):
        self.tests = {}
        self._dirty = True

    def clean(self):
        """Clear "dirty" flag for summary caching"""
        self._dirty = False
        for test in self.tests.values():
            test.clean()

    @property
    def is_dirty(self):
        """Check "dirty" flag for summary caching"""
        if self._dirty:
            return True
        for test in self.tests.values():
            if test.is_dirty:
                return True
        return False

    def _get_test(self, name):
        """find or create a test by name"""
        if name not in self.tests:
            self._dirty = True
            test = TestResult(name)
            self.tests[name] = test
        return self.tests[name]

    @property
    def unique_pass_count(self):
        """Report number of cases where for all tests with
        the same name all tests are passing"""
        count = 0
        for test in self.tests.values():
            count += test.unique_pass_count
        return count

    @property
    def unique_fail_count(self):
        """Report number of cases where for all tests with
        the same name at least one test is failing"""
        count = 0
        for test in self.tests.values():
            count += test.unique_fail_count
        return count

    @property
    def unique_skip_count(self):
        """Report number of cases where for all tests with
        the same name all tests are skipped"""
        count = 0
        for test in self.tests.values():
            count += test.unique_skip_count
        return count

    @property
    def unique_run_count(self):
        """Report number of cases where for all tests with
        the same name all tests not tests are skipped"""
        count = 0
        for test in self.tests.values():
            count += test.unique_run_count
        return count

    @property
    def unique_attempt_count(self):
        """Report number of cases where for all tests with
        the same name all tests have been run or skipped."""
        count = 0
        for test in self.tests.values():
            count += test.unique_attempt_count
        return count

    @property
    def total_pass_count(self):
        """Report the total count of passes recorded for all tests"""
        count = 0
        for test in self.tests.values():
            count += test.pass_count
        return count

    @property
    def total_fail_count(self):
        """Report the total count of fails recorded for all tests"""
        count = 0
        for test in self.tests.values():
            count += test.fail_count
        return count

    @property
    def total_skip_count(self):
        """Report the total count of skips recorded for all tests"""
        count = 0
        for test in self.tests.values():
            count += test.skip_count
        return count

    @property
    def total_run_count(self):
        """Report the total count of runs recorded for all tests"""
        count = 0
        for test in self.tests.values():
            count += test.run_count
        return count

    @property
    def total_attempt_count(self):
        """Report the total count of attempts recorded for all tests"""
        count = 0
        for test in self.tests.values():
            count += test.attempt_count
        return count

    @property
    def test_count(self):
        """Report the total count tests recorded"""
        count = len(self.tests)
        return count

    def add_pass(self, name):
        """record a test passing"""
        return self._get_test(name).add_pass()

    def add_skip(self, name):
        """record a test skipped"""
        return self._get_test(name).add_skip()

    def add_fail(self, name):
        """record a test failing"""
        return self._get_test(name).add_fail()

    def update(self, other):
        """Add results from another test collection"""
        for name, test_result in other.tests.items():
            result = self._get_test(name)
            result.update(test_result)


class CIDictReader(csv.DictReader):
    """A Case insensitive DictReader. All names will be casefolded (aggressively lowercased)"""
    @property
    def fieldnames(self):
        """Access field names"""
        return [
            field.strip().casefold()
            for field in csv.DictReader.fieldnames.fget(self)
        ]

    # pylint: disable=no-member
    def next(self):
        """Iterate"""
        return CIDictReader(csv.DictReader.next(self))


class TestSuite(TestContainer):
    """A collection of test results grouped together to represent a
    higher order result
    """
    def __init__(self, name, location):
        super().__init__()
        self.name = name
        self.location = location

    def composite_result(self):
        """merge all suites under this object into one suite
        (degenerate case)"""
        return self

    def squash(self, location):
        """Rewrite to represent a single execution of each test"""
        self.location = location
        for test in self.tests.values():
            test.squash()
        self._dirty = True

    def print_log(self):
        """Print summary to the log"""
        log_summary(self.name, self)

    def _read_robot_test(self, element, parent_name=None):
        """Read in results from a Robot Framework Test"""
        tests_found = 0
        test_name = element.attrib["name"]
        if parent_name:
            suite_name = parent_name + "." + test_name
        else:
            suite_name = test_name
        for status in element.findall("./status"):
            status_name = status.attrib["status"].lower()
            if status_name == "pass":
                tests_found += 1
                self.add_pass(suite_name)
            elif status_name == "skip":
                tests_found += 1
                self.add_skip(suite_name)
            else:
                tests_found += 1
                self.add_fail(suite_name)
        return tests_found

    def _read_robot_suite(self, element, parent_name=None):
        """Read in results from a Robot Framework Suite"""
        tests_found = 0
        suite_name = element.attrib["name"]
        if parent_name:
            suite_name = parent_name + "." + suite_name
        for test in element.findall("./test"):
            tests_found += self._read_robot_test(test, suite_name)
        for suite in element.findall("./suite"):
            tests_found += self._read_robot_suite(suite, suite_name)
        return tests_found

    def _read_robot_result(self, root):
        """Read in results from a Robot Framework xml file root element"""
        tests_found = 0
        for test in root.findall("./test"):
            tests_found += self._read_robot_test(test)
        for suite in root.findall("./suite"):
            tests_found += self._read_robot_suite(suite)
        return tests_found

    # pylint: disable=too-many-branches
    def _read_cdash_test(self, element):
        """Read in results from a CDash test report"""
        tests_found = 1
        test_status = element.attrib["Status"].lower()
        test_name = None
        if not test_name:
            name_el = element.find("Name")
            if name_el is not None:
                test_name = name_el.text
        if not test_name:
            name_el = element.find("FullName")
            if name_el is not None:
                test_name = name_el.text
        if not test_name:
            name_el = element.find("FullCommandLine")
            if name_el is not None and name_el.text:
                test_name = name_el.text
        if not test_name:
            test_name = "unnamed"
        if test_status == "passed":
            self.add_pass(test_name)
        elif test_status == "failed":
            self.add_fail(test_name)
        elif test_status == "Timeout":
            self.add_fail(test_name)
        elif test_status == "OTHER_FAULT":
            self.add_fail(test_name)
        elif test_status == "notrun":
            self.add_skip(test_name)
        elif test_status == "Disabled":
            self.add_skip(test_name)
        else:
            self.add_fail(test_name)
        return tests_found

    def _read_cdash_result(self, root):
        """Read in results from a CDash test report root element"""
        tests_found = 0
        for test in root.findall("./Testing/Test"):
            tests_found += self._read_cdash_test(test)
        return tests_found

    def _read_xunit_result(self, root):
        """Read in results from a JUnit xml file"""
        tests_found = 0
        for test_case in root.findall("testcase"):
            name = ""
            if "classname" in test_case.attrib:
                classname = test_case.attrib["classname"].split(".")
                if classname:
                    name += ".".join(classname) + "."
            name += test_case.attrib['name']

            failure = test_case.find("failure")
            if failure is not None:
                tests_found += 1
                self.add_fail(name)
                continue

            error = test_case.find("error")
            if error is not None:
                tests_found += 1
                self.add_fail(name)
                continue

            skip = test_case.find("skipped")
            if skip is not None:
                tests_found += 1
                self.add_skip(name)
                continue

            tests_found += 1
            self.add_pass(name)
        return tests_found

    def _read_csv_result(self, result: str):
        """Read in results from a TeamCity test format csv file"""
        tests_found = 0
        with open(result, newline='', encoding="utf8") as csv_file:
            try:
                dialect = csv.Sniffer().sniff(csv_file.readline(),
                                              delimiters=';,\t')
            except:
                print(f"Error reading {result}", file=sys.stderr)
                raise
            csv_file.seek(0)
            reader = CIDictReader(csv_file, dialect=dialect)
            for row in reader:

                name = ""
                for suite_col in ['suite', 'suite name', 'test suite']:
                    if suite_col in row:
                        name += row[suite_col]
                        break

                name += "." if name else ""

                for test_col in ['test', 'test name', 'test case', 'case']:
                    if test_col in row:
                        name += row[test_col]
                        break

                status = None
                for status_col in ['status', 'result']:
                    if status_col in row:
                        status = row[status_col].casefold()
                        break

                if name and status:
                    if status in ['pass', 'passed', 'ok']:
                        tests_found += 1
                        self.add_pass(name)
                    elif status in ['fail', 'failed', 'error']:
                        tests_found += 1
                        self.add_fail(name)
                    elif status in ['skip', 'skipped', 'ignore', 'ignored']:
                        tests_found += 1
                        self.add_skip(name)
        return tests_found

    def read_result(self, result: str, result_format: str):
        """Read in results from a file"""
        tests_found = 0
        if result_format == "xml":
            tree = DET.parse(result)
            root = tree.getroot()
            if root.tag == "robot":
                tests_found = self._read_robot_result(root)
            elif root.tag in ["testsuite", "testsuites", "testcase"]:
                tests_found = self._read_xunit_result(root)
            elif root.tag in ["Site", "NewDataSet"]:
                tests_found = self._read_cdash_result(root)
        elif result_format == "csv":
            tests_found = self._read_csv_result(result)
        return tests_found

    def fill_xunit(self, suites_xml, package=None):
        """Write out suite as JUnit format XML"""
        testsuite = SubElement(suites_xml, "testsuite")
        if package is not None:
            testsuite.attrib["package"] = str(package)
        if self.name is not None:
            testsuite.attrib["name"] = str(self.name)
            if package is None:
                testsuite.attrib["name"] = str(self.name)
            else:
                testsuite.attrib["name"] = str(package) + "/" + self.name
        elif package is not None:
            testsuite.attrib["name"] = str(package)
        summary = self
        testsuite.attrib["tests"] = str(summary.total_run_count)
        testsuite.attrib["failures"] = str(summary.total_fail_count)
        testsuite.attrib["skipped"] = str(summary.total_skip_count)
        for test in self.tests.values():
            test.fill_xunit(testsuite, test_class=package)


class TestPackage:
    """Intermediate level Container for test information.
    Collects Packages, that hold Suites, that hold Results"""
    def __init__(self, name):
        self.name = name
        self.suites = []
        self._composite = None
        self._dirty = True

    def clean(self):
        """Clear "dirty" flag for summary caching"""
        self._dirty = False
        for suite in self.suites:
            suite.clean()

    @property
    def is_dirty(self):
        """Check "dirty" flag for summary caching"""
        if self._dirty:
            return True
        for suite in self.suites:
            if suite.is_dirty:
                return True
        return False

    def composite_result(self):
        """merge all suites under this object into one suite"""
        if self.is_dirty:
            self._composite = TestSuite(self.name, None)
            for suite in self.suites:
                self._composite.update(suite)
        return self._composite

    def add_suite(self, suite: TestSuite):
        """Add a suite under this package"""
        self._dirty = True
        self.suites.append(suite)
        return suite

    def print_log(self):
        """Print summary to the log"""
        log_summary(self.name, self.composite_result())
        with log_scope():
            for suite in self.suites:
                suite.print_log()

    def scan_test_results(self,
                          source: str,
                          suite_name: str,
                          suite_per_file=False):
        """Read through all files in a folder to find test results"""
        test_result_files = []
        with pushd(source):
            test_result_files.extend([('xml', os.path.abspath(match))
                                      for match in glob("*.xml")])
            test_result_files.extend([('csv', os.path.abspath(match))
                                      for match in glob("*.csv")])
        count = 0
        suite = TestSuite(suite_name, source)

        for result_format, result_file in test_result_files:
            tests_found = None
            if suite_per_file:
                suite = TestSuite(
                    suite_name + "/" + os.path.basename(result_file),
                    result_file)
            tests_found = suite.read_result(result_file, result_format)
            if tests_found:
                count += 1
                if suite_per_file:
                    suite.squash(suite.location)
                    self.add_suite(suite)
        if not suite_per_file:
            if count > 0:
                suite.squash(source)
                self.add_suite(suite)
        return count

    def scan_tests(self,
                   folder_path: str,
                   suite_name: str = None,
                   suite_per_file=False):
        """Scan folder for test results."""
        with log_scope():
            self.scan_test_results(folder_path,
                                   suite_name,
                                   suite_per_file=suite_per_file)
            for folder in os.scandir(folder_path):
                if folder.is_dir():
                    if suite_name:
                        sub_suite = f'{suite_name}/{folder.name}'
                    else:
                        sub_suite = folder.name
                    self.scan_tests(folder.path,
                                    sub_suite,
                                    suite_per_file=suite_per_file)

    def fill_xunit(self, suites_xml):
        """Write out package as JUnit format XML"""
        for suite in self.suites:
            suite.fill_xunit(suites_xml, package=self.name)


class TestReport:
    """Top level container for test information.
    Collects Packages, that hold Suites, that hold Results"""
    def __init__(self):
        self.packages = []
        self._composite = None
        self._dirty = True

    def clean(self):
        """Clear "dirty" flag for summary caching"""
        self._dirty = False
        for package in self.packages:
            package.clean()

    @property
    def is_dirty(self):
        """Check "dirty" flag for summary caching"""
        if self._dirty:
            return True
        for package in self.packages:
            if package.is_dirty:
                return True
        return False

    def composite_result(self):
        """merge all suites under this object into one suite"""
        if self.is_dirty:
            self._composite = TestSuite("Report", None)
            for package in self.packages:
                self._composite.update(package.composite_result())
        return self._composite

    def add_package(self, name):
        """Add a package to the report"""
        result = TestPackage(name)
        self.packages.append(result)
        return result

    def print_log(self):
        """Print summary to the log"""
        log_summary(None, self.composite_result())
        with log_scope():
            for package in self.packages:
                package.print_log()

    def scan_tests(self, folder_path: str, suite_per_file=False):
        """Scan a folder for test results"""
        for folder in os.scandir(folder_path):
            if not folder.is_dir():
                continue
            test_suite = self.add_package(folder.name)
            test_suite.scan_tests(folder.path, suite_per_file=suite_per_file)

    def as_xunit(self):
        """Return the report represented as JUnit format XML"""
        suites_xml = Element('testsuites')
        summary = self.composite_result()
        suites_xml.attrib["tests"] = str(summary.total_run_count)
        suites_xml.attrib["failures"] = str(summary.total_fail_count)
        for package in self.packages:
            package.fill_xunit(suites_xml)
        return suites_xml


def get_suite_defects(summary, package_defects):
    """Get the set of defects that apply to a given suite"""
    result = set()
    for test_name in summary.tests:
        if summary.tests[test_name].fail_count > 0:
            found = False
            for defect, tests in package_defects.items():
                if test_name in tests:
                    found = True
                    result.add(defect)
            if not found:
                result.add("unknown")
    return result


# pylint: disable=too-many-locals,too-many-nested-blocks,too-many-branches
def write_excel_package_page(workbook: xlsxwriter.Workbook,
                             package: TestPackage,
                             report_all_tests=False,
                             defects=None):
    """Write an Excel tab with package details"""
    if defects is None:
        defects = {}
    suite_header_fmt = workbook.add_format()
    suite_header_fmt.set_rotation(90)
    suite_header_fmt.set_bold(True)
    header_fmt = workbook.add_format()
    header_fmt.set_bold(True)

    summary = package.composite_result()
    sheet = workbook.add_worksheet()
    sheet.name = package.name
    sheet.set_column("A:A", 9)
    sheet.write_string("A1", "Jira", cell_format=header_fmt)
    sheet.set_column("B:B", 70)
    sheet.write_string("B1", "Test", cell_format=header_fmt)
    f_row = 0
    f_col = 2
    for suite in package.suites:
        suite_name = suite.name
        if suite_name is None:
            suite_name = "<empty>"
        sheet.set_column(f_col, f_col, 2.8)
        sheet.write_string(f_row,
                           f_col,
                           suite_name,
                           cell_format=suite_header_fmt)
        f_col += 1
    width = f_col
    f_row += 1
    for test_name in summary.tests:
        if report_all_tests or summary.tests[test_name].fail_count > 0:
            f_col = 0
            jiras = set()
            for jira, tests in defects.items():
                if test_name in tests:
                    jiras.add(jira)
            sheet.write_string(f_row, f_col, " ".join(sorted(jiras)))
            f_col += 1
            sheet.write(f_row, f_col, test_name)
            f_col += 1
            for suite in package.suites:
                if test_name in suite.tests:
                    if report_all_tests:
                        if suite.tests[test_name].unique_fail_count:
                            sheet.write(f_row, f_col, "Fail")
                        elif suite.tests[test_name].unique_skip_count:
                            sheet.write(f_row, f_col, "Skip")
                        elif suite.tests[test_name].unique_pass_count:
                            sheet.write(f_row, f_col, "Pass")
                    elif test_name in suite.tests and suite.tests[
                            test_name].fail_count > 0:
                        sheet.write(f_row, f_col, "X")
                f_col += 1
            f_row += 1
    sheet.autofilter(0, 0, f_row - 1, width - 1)


def write_excel_dashboard_structure(workbook, dashboard):
    """Write summary dashboard high level page structure"""
    col_break_fmt = workbook.add_format()
    col_break_fmt.bg_color = '#F2F2F2'
    col_break_fmt.set_left(1)
    col_break_fmt.set_right(1)
    header_break_fmt = workbook.add_format()
    header_break_fmt.set_bold(True)
    header_break_fmt.set_bottom(1)
    header_col_break_fmt = workbook.add_format()
    header_col_break_fmt.bg_color = '#F2F2F2'
    header_col_break_fmt.set_left(1)
    header_col_break_fmt.set_right(1)
    header_col_break_fmt.set_bottom(1)
    shared_header_fmt = workbook.add_format()
    shared_header_fmt.set_align('center')
    shared_header_fmt.set_bold(True)
    bold_fmt = workbook.add_format()
    bold_fmt.set_bold(True)

    dashboard.set_column("A:A", 6, cell_format=bold_fmt)
    dashboard.set_column("B:C", 6.5)
    dashboard.set_column("D:D", 1.9, cell_format=col_break_fmt)
    dashboard.set_column("E:E", 20.15)
    dashboard.set_column("F:M", 4.45)
    dashboard.set_column("N:N", 1.9, cell_format=col_break_fmt)
    dashboard.set_column("O:O", 48.25)
    dashboard.set_column("P:S", 4.45)
    dashboard.set_row(1, None, cell_format=header_break_fmt)
    dashboard.write_blank("D2", None, cell_format=header_col_break_fmt)
    dashboard.write_blank("N2", None, cell_format=header_col_break_fmt)
    for cells, header in [
        ("A1:C1", "Summary"),
        ("F1:I1", "Total"),
        ("J1:M1", "Unique"),
    ]:
        dashboard.merge_range(cells, header, cell_format=shared_header_fmt)

    for col, header in [
        ("B", "Count"),
        ("C", "Rate"),
        ("E", "Category"),
        ("F", "Tests"),
        ("G", "Pass"),
        ("H", "Fail"),
        ("I", "Skip"),
        ("J", "Tests"),
        ("K", "Pass"),
        ("L", "Fail"),
        ("M", "Skip"),
        ("O", "Suite"),
        ("P", "Tests"),
        ("Q", "Pass"),
        ("R", "Fail"),
        ("S", "Skip"),
        ("T", "Jiras"),
    ]:
        dashboard.write_string(f"{col}2", header)


def write_excel_dashboard_summary(workbook, dashboard):
    """Write top level summary portion of dashboard"""
    footnote_fmt = workbook.add_format()
    footnote_fmt.set_font_size(8)
    footnote_fmt.set_align('left')
    footnote_fmt.set_align('top')
    rate_field_fmt = workbook.add_format()
    rate_field_fmt.set_num_format("0.00%")

    dashboard.write("A3", "Total")
    dashboard.write("A4", "Tests")
    dashboard.write_formula("B4", "=SUM(F:F)")
    dashboard.write("A5", "Pass*")
    dashboard.write_formula("B5", "=SUM(G:G)")
    dashboard.write_formula("C5", "=B5/SUM(B5:B6)", cell_format=rate_field_fmt)
    dashboard.write("A6", "Fail*")
    dashboard.write_formula("B6", "=SUM(H:H)")
    dashboard.write_formula("C6", "=B6/SUM(B5:B6)", cell_format=rate_field_fmt)
    dashboard.write("A7", "Skip")
    dashboard.write_formula("B7", "=SUM(I:I)")
    dashboard.write_formula("C7", "=B7/B4", cell_format=rate_field_fmt)
    dashboard.merge_range("A8:C8",
                          "*Rate excludes skipped",
                          cell_format=footnote_fmt)
    dashboard.write("A10", "Unique")
    dashboard.write("A11", "Tests")
    dashboard.write_formula("B11", "=SUM(J:J)")
    dashboard.write("A12", "Pass")
    dashboard.write_formula("B12", "=SUM(K:K)")
    dashboard.write_formula("C12",
                            "=B12/SUM(B12:B13)",
                            cell_format=rate_field_fmt)
    dashboard.write("A13", "Fail")
    dashboard.write_formula("B13", "=SUM(L:L)")
    dashboard.write_formula("C13",
                            "=B13/SUM(B12:B13)",
                            cell_format=rate_field_fmt)
    dashboard.write("A14", "Skip")
    dashboard.write_formula("B14", "=SUM(M:M)")
    dashboard.write_formula("C14", "=B14/B11", cell_format=rate_field_fmt)


def write_excel_package_summary(workbook, dashboard, package, row):
    """Write summary for one package on dashboard"""
    # workbook is passed to allow access to book wide information like formats
    del workbook
    summary = package.composite_result()
    dashboard.write(f"E{row}", package.name)
    dashboard.write(f"F{row}", summary.total_attempt_count)
    dashboard.write(f"G{row}", summary.total_pass_count)
    dashboard.write(f"H{row}", summary.total_fail_count)
    dashboard.write(f"I{row}", summary.total_skip_count)
    dashboard.write(f"J{row}", summary.unique_attempt_count)
    dashboard.write(f"K{row}", summary.unique_pass_count)
    dashboard.write(f"L{row}", summary.unique_fail_count)
    dashboard.write(f"M{row}", summary.unique_skip_count)


# pylint: disable=too-many-arguments,too-many-positional-arguments
def write_excel_suite_summary(workbook, dashboard, suite, row, target_folder,
                              package_defects):
    """Write summary for one suite on dashboard"""
    # workbook is passed to allow access to book wide information like formats
    del workbook
    summary = suite.composite_result()
    if suite.location is not None:
        location = os.path.relpath(suite.location, target_folder)
        location.replace("\\", "/")
        dashboard.write_url(f"O{row}",
                            f"external:{location}",
                            string=suite.name)
    else:
        dashboard.write(f"O{row}", suite.name)
    dashboard.write(f"P{row}", summary.total_attempt_count)
    dashboard.write(f"Q{row}", summary.total_pass_count)
    dashboard.write(f"R{row}", summary.total_fail_count)
    dashboard.write(f"S{row}", summary.total_skip_count)
    dashboard.write(f"T{row}",
                    ",".join(get_suite_defects(summary, package_defects)))


def write_excel(report: TestReport,
                target,
                report_all_tests=False,
                defects=None):
    """Write report as an Excel file"""
    if defects is None:
        defects = {}
    target = os.path.abspath(target)
    target_folder = os.path.dirname(target)
    workbook = xlsxwriter.Workbook(target)
    dash = workbook.add_worksheet()
    dash.name = "Dashboard"

    write_excel_dashboard_structure(workbook, dash)
    write_excel_dashboard_summary(workbook, dash)
    summary = report.composite_result()
    row = 3
    for package in report.packages:
        package_defects = defects.get(package.name, {})
        write_excel_package_summary(workbook, dash, package, row)
        row += 1
        for suite in package.suites:
            write_excel_suite_summary(workbook, dash, suite, row,
                                      target_folder, package_defects)
            row += 1
        if summary.total_fail_count > 0:
            write_excel_package_page(workbook,
                                     package,
                                     report_all_tests=report_all_tests,
                                     defects=package_defects)

    workbook.close()


def read_command_line(cmd_line):
    """
    Read command line arguments
    """
    # Now read full arguments
    parser = argparse.ArgumentParser(description=globals()['__doc__'],
                                     formatter_class=RawTextHelpFormatter)
    parser.add_argument('source',
                        help="Validation data dir",
                        type=os.path.abspath,
                        action='store')

    parser.add_argument('--defects',
                        help="Database of known defects",
                        type=os.path.abspath)

    parser.add_argument('--suite-per-file',
                        help="Treat each result file as a separate suite",
                        action='store_true')

    parser.add_argument('--report-all-tests',
                        help="List all tests in summary report",
                        action='store_true')

    # parse arguments (will exit here on invalid args or help)
    args = parser.parse_args(args=cmd_line)
    args.dest = args.source
    return args


def load_defects(db_path):
    """Load defect database"""
    if not db_path or not os.path.isfile(db_path):
        return {}
    with open(db_path, newline='', encoding="utf8") as db_file:
        json_data = json.load(db_file)
        return json_data


def run(args):
    """main entry point"""
    validation_summary_path = os.path.join(args.dest,
                                           "validation-summary.xlsx")
    xunit_path = os.path.join(args.dest, "validation-summary-xunit.xml")
    report = TestReport()
    report.scan_tests(args.source, suite_per_file=args.suite_per_file)
    report.print_log()
    defects = load_defects(args.defects)
    if os.path.isfile(validation_summary_path):
        os.remove(validation_summary_path)
    write_excel(report,
                validation_summary_path,
                report_all_tests=args.report_all_tests,
                defects=defects)
    with open(xunit_path, "w", encoding="utf8") as xml_file:
        xunit_report = report.as_xunit()
        pretty_xunit_report = pretty_print_xml(xunit_report)
        xml_file.write(pretty_xunit_report)


if __name__ == '__main__':
    run(read_command_line(None))
