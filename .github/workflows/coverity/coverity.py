#!/usr/bin/env python3
# PYTHON_ARGCOMPLETE_OK
############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
############################################################################
"""
Run Coverity scan.

This tool requires that Coverity be available in the PATH for
full functionality.
"""
# import modules used
import argparse
from argparse import RawTextHelpFormatter
import os
import sys
import json
import xml.etree.ElementTree as ET  # nosec
from xml.dom import minidom  # nosec

from cmdline import run_command, md, capture_command


def pretty_print_xml(root):
    """Format xml for legibility"""
    serial_xml = ET.tostring(root)
    parsed_xml = minidom.parseString(serial_xml)  # nosec
    return parsed_xml.toprettyxml()


def summarize_server_state(issue, test_info):
    """Summarize state on server for an issue"""
    test_info["status"] = ""
    if "stateOnServer" in issue and issue["stateOnServer"]:
        state_on_server = issue["stateOnServer"]
        test_info["cid"] = state_on_server["cid"]
        test_info["stream"] = state_on_server["stream"]
        if "triage" in state_on_server:
            triage = state_on_server["triage"]
            test_info["status"] = triage["action"].lower()
        test_info["external_ref"] = triage["externalReference"]
    else:
        test_info["cid"] = ""
        test_info["stream"] = ""
        test_info["external_ref"] = ""


def summarize_checker(issue, test_info):
    """Summarize checker properties for an issue"""
    if "checkerProperties" in issue and issue["checkerProperties"]:
        checker_properties = issue["checkerProperties"]
        test_info["desc"] = checker_properties["subcategoryLongDescription"]
        test_info["effect"] = checker_properties["subcategoryLocalEffect"]
    else:
        test_info["desc"] = issue["checkerName"]
        test_info["effect"] = issue["type"]


def build_summary(test_info):
    """Generate a summary string based on test info"""
    summary = []
    if test_info.get("desc"):
        summary.append(test_info["desc"])
    if test_info.get("effect"):
        summary.append(test_info["effect"])
    if test_info.get("mergeKey"):
        summary.append(f"Merge Key: {test_info['mergeKey']}")
    if test_info.get("cid"):
        summary.append(f"CID: {test_info['cid']}")
    if test_info.get("external_ref"):
        summary.append(f"see: {test_info['external_ref']}")
    return '\n\n'.join(summary)


def build_message(test_info):
    """Generate a short message based on test info"""
    if test_info.get("external_ref"):
        return test_info.get("external_ref")
    return f"CID: {test_info['cid']}"


class ReportSummary:
    """Summary of coverity report"""
    def __init__(self, report, problem_statuses):
        self.problems = 0
        self.ignored = 0
        self.problem_statuses = problem_statuses
        self.suite = []
        self.summarize_report(report)

    def summarize_issue(self, issue):
        """Summarize a single issue"""
        test_info = {}
        self.suite.append(test_info)
        summarize_server_state(issue, test_info)
        test_info["ignore"] = test_info["status"] not in self.problem_statuses
        if test_info["ignore"]:
            self.ignored += 1
        else:
            self.problems += 1
        summarize_checker(issue, test_info)
        test_info['type'] = issue["checkerName"]
        test_info['file'] = issue["strippedMainEventFilePathname"]
        test_info['line'] = str(issue["mainEventLineNumber"])
        test_info['col'] = str(issue["mainEventColumnNumber"])
        test_info["name"] = ":".join(
            [test_info["file"], test_info["line"], test_info["col"]])
        test_info["mergeKey"] = issue["mergeKey"]
        test_info["summary"] = build_summary(test_info)
        test_info["message"] = build_message(test_info)

    def summarize_report(self, report):
        """summarize a report"""
        if "issues" in report and report["issues"]:
            for issue in report["issues"]:
                self.summarize_issue(issue)


def xunit_from_report(xunit, summary):
    """write an xunit report for the results"""
    testsuites = ET.Element('testsuites')
    testsuite = ET.SubElement(testsuites, "testsuite")
    testsuite.attrib["name"] = "coverity"
    suite_fail = 0
    for test in summary.suite:
        testcase = ET.SubElement(testsuite, "testcase")
        testcase.attrib["classname"] = test['type']
        testcase.attrib["name"] = test["name"]
        if test["ignore"]:
            testcase.attrib["status"] = "Skip"
            skipped = ET.SubElement(testcase, "skipped")
            skipped.text = test["summary"]
            skipped.attrib["message"] = test["message"]
        else:
            suite_fail += 1
            testcase.attrib["status"] = "Fail"
            failure = ET.SubElement(testcase, "failure")
            failure.attrib["type"] = test['type']
            failure.text = test["summary"]
            failure.attrib["message"] = test["message"]
    testsuite.attrib["tests"] = str(len(summary.suite))
    testsuite.attrib["failures"] = str(summary.problems)
    testsuites.attrib["tests"] = str(len(summary.suite))

    pretty_xunit_report = pretty_print_xml(testsuites)
    with open(xunit, "w", encoding="utf-8") as xml_file:
        xml_file.write(pretty_xunit_report)


def read_command_line(cmd_line):
    """
    Read command line arguments
    """

    # Now read full arguments
    parser = argparse.ArgumentParser(description=globals()['__doc__'],
                                     formatter_class=RawTextHelpFormatter)
    parser.add_argument('--version',
                        action='version',
                        version='%(prog)s 0.2.0')
    parser.add_argument('--url',
                        action="store",
                        required=True,
                        help='Coverity Connect Server URL')
    parser.add_argument('--auth-key-file',
                        action="store",
                        default=None,
                        help='Authentication key file.')
    parser.add_argument('--user',
                        action="store",
                        default=None,
                        help='User name.')
    parser.add_argument('--email',
                        action="store",
                        required=True,
                        help='User email.')
    parser.add_argument('--password', default=None, help='User password.')
    parser.add_argument('--stream',
                        action="store",
                        required=True,
                        help='Coverity target stream')
    parser.add_argument('--project',
                        action="store",
                        required=True,
                        help='Coverity target project')
    parser.add_argument('--project-name',
                        action="store",
                        required=True,
                        help='Project Name')
    parser.add_argument('--bu',
                        action="store",
                        default="",
                        help='Business unit (for reports)')
    parser.add_argument(
        '--report',
        action="store_true",
        help='Force reporting, even if no new issues were found')
    #build options
    parser.add_argument('--dir',
                        dest='intermediate_dir',
                        action='store',
                        required=True,
                        help="Intermediate directory")

    parser.add_argument('--strip-path',
                        action='store',
                        required=True,
                        help="Path to remove from hits")
    parser.add_argument('--description',
                        action='store',
                        default="",
                        help="Snapshot description")
    parser.add_argument('--report-dir',
                        action='store',
                        required=True,
                        help="Report Directory")
    parser.add_argument('--code-version',
                        action='store',
                        required=True,
                        help="Code version")
    parser.add_argument('command',
                        help="Build command",
                        nargs=argparse.REMAINDER,
                        action='store')

    # parse arguments (will exit here on invalid args or help)
    args = parser.parse_args(args=cmd_line)

    # Resolve settings form arguments
    if args.auth_key_file:
        args.auth_key_file = os.path.abspath(args.auth_key_file)
    args.intermediate_dir = os.path.abspath(args.intermediate_dir)
    args.report_dir = os.path.abspath(args.report_dir)
    args.strip_path = os.path.abspath(args.strip_path)
    return args


def configure_coverity():
    """Configure Coverity"""
    if os.name == "nt":
        run_command("cov-configure", "--msvc")
    run_command("cov-configure", "--gcc")
    run_command("cov-configure", "--template", "--compiler", "cc",
                "--comptype", "gcc")
    run_command("cov-configure", "--template", "--compiler", "c++",
                "--comptype", "g++")


def build_under_coverity(intermediate_dir, command, strip_path):
    """Build project under coverity"""
    run_command("cov-build", "--dir", intermediate_dir, *command)
    # Analyze
    run_command("cov-analyze", "--dir", intermediate_dir, "--strip-path",
                strip_path, "--enable-constraint-fpp", "--ticker-mode", "none",
                "--disable-default", "--concurrency", "--security", "--rule",
                "--enable-fnptr", "--enable-virtual", "--enable",
                "SECURE_CODING")


# pylint: disable=too-many-arguments,too-many-positional-arguments
# pylint: disable=too-many-locals
def get_preview_report(intermediate_dir,
                       url,
                       stream,
                       code_version,
                       strip_path,
                       preview_report_v2_path,
                       full_report_v9_path,
                       active_report_v9_path,
                       full_html_report_path,
                       active_html_report_path,
                       text_output=None,
                       user=None,
                       password=None,
                       auth_key_file=None):
    """Generate preview reports (Pull from Coverity Connect, but don't push)"""
    env = os.environ.copy()
    if password is not None:
        env["COVERITY_PASSPHRASE"] = password
    if user is not None:
        env["COV_USER"] = user
    args = [
        "--dir",
        intermediate_dir,
        "--url",
        url,
        "--stream",
        stream,
        "--ticker-mode",
        "none",
        "--version",
        code_version,
        "--strip-path",
        strip_path,
        "--preview-report-v2",
        preview_report_v2_path,
    ]
    if auth_key_file:
        args.extend([
            "--auth-key-file",
            auth_key_file,
        ])
    run_command("cov-commit-defects", *args, env=env)

    run_command("cov-format-errors", "--dir", intermediate_dir,
                "--no-default-triage-filters", "--preview-report-v2",
                preview_report_v2_path, "--json-output-v9",
                full_report_v9_path)

    run_command("cov-format-errors", "--dir", intermediate_dir,
                "--preview-report-v2", preview_report_v2_path,
                "--json-output-v9", active_report_v9_path)

    md(full_html_report_path)
    run_command("cov-format-errors", "--dir", intermediate_dir,
                "--no-default-triage-filters", "--preview-report-v2",
                preview_report_v2_path, "--html-output", full_html_report_path)

    md(active_html_report_path)
    run_command("cov-format-errors", "--dir", intermediate_dir,
                "--preview-report-v2", preview_report_v2_path, "--html-output",
                active_html_report_path)

    if text_output:
        with open(text_output, 'w', encoding="utf-8") as dest:
            dest.write(
                capture_command("cov-format-errors", "--dir", intermediate_dir,
                                "--preview-report-v2", preview_report_v2_path,
                                "--triage-attribute-regex", "action",
                                "Undecided", "--text-output-style",
                                "multiline")[0])


# pylint: disable=too-many-arguments,too-many-positional-arguments
def publish_to_coverity_connect(intermediate_dir,
                                url,
                                stream,
                                code_version,
                                strip_path,
                                description,
                                snapshot_id_path,
                                user=None,
                                password=None,
                                auth_key_file=None):
    """Publish to Coverity Connect"""
    env = os.environ.copy()
    if password is not None:
        env["COVERITY_PASSPHRASE"] = password
    if user is not None:
        env["COV_USER"] = user
    args = [
        "--dir",
        intermediate_dir,
        "--url",
        url,
        "--stream",
        stream,
        "--ticker-mode",
        "none",
        "--version",
        code_version,
        "--strip-path",
        strip_path,
        "--description",
        description,
        "--snapshot-id-file",
        snapshot_id_path,
    ]
    if auth_key_file:
        args.extend([
            "--auth-key-file",
            auth_key_file,
        ])
    run_command("cov-commit-defects", *args, env=env)


def write_reports(args, snapshot_id):
    """Write PDF reports"""
    cfg = f"""# This is an example configuration file for Coverity report generators. It
# tells report generators how to generate reports. You can make and modify a
# copy of it for use in configuring a report generator.
#
# Its initial sections apply to all report generators. Later sections,
# marked with "#######" comments, are intended for specific report
# generators.
#
# This file is In YAML format, version 1.2. See
# https://yaml.org/spec/1.2/spec.html for documentation on the format.
# If you simply modify a copy of this file you probably do not need to
# know details about YAML, however, be aware that structure is
# determined by indentation, i.e., the number of spaces at the beginning
# of the line. Multi-line text should be placed inside double quotes ("").
#
# The YAML constructs used here are "mappings" or key: value pairs. Some
# mappings' values are file pathnames. Pathnames may use a slash or
# backslash, whichever is appropriate for the platform, as a separator.
# (Note that according to YAML's quoting rules, backslashes are not
# special unless they are in a character sequence enclosed in double
# quotes.) A relative pathname here is interpreted as relative to the
# directory containing this configuration file. If the configuration did
# not come from a file (e.g., was read on standard input), then the
# pathname would be relative to the report generator process's working
# directory.
#

################## Sections that apply to all reports #############
# Describes information needed in interpreting the rest of the config.
version:   
    # The version of this file's schema.
    # Mandatory field
    schema-version: 6

# Describes settings needed for connecting to Coverity Connect.
connection:   
    # The URL of the Coverity Connect instance.
    # Mandatory field
    url: {args.url}

    # Coverity Connect username. Password or other authentication key
    # is passed in to the application from the command line.
    # Mandatory field
    username: {args.user}

    # Path to an optional file containing additional CA certificates for
    # use in establishing a secure HTTPS connection. These must be in PEM
    # format.
    ssl-ca-certs: 

# The name of the Coverity Connect project.
# Mandatory field
project: "{args.project}"

# Describes the fields in the title page of the report.
title-page:   
    # Name of your company
    # Mandatory field
    company-name: "Intel"

    # Name of the software development project. May be distinct from the
    # Coverity Connect project name.
    # Mandatory field
    project-name: "{args.project_name}"

    # e.g. project-version: v.1.2.3
    # Mandatory field
    project-version: {args.code_version}

    # Optional path to a logo file for your company. Valid image types
    # are bmp, gif, jpg, and png. The maximum allowed image size is 210
    # pixels wide by 70 pixels high
    # Note that backslash characters in a path must be doubled
    # e.g. logo: C:\\logo\\ourlogo.jpg
    # e.g. logo: /var/logo/ourlogo.png
    logo: 

    # Name of your division, group, team or other organizational unit.
    # Mandatory field
    organizational-unit-name: "IPAS"

    # Organizational unit term (e.g., division, group, team).
    # Mandatory field
    organizational-unit-term: "{args.bu}"

    # Name of the entity for which the report was prepared.
    # This is also known as project-contact-name for
    # CIR, CVSS, PCIDSS, MobileOwasp, and Owasp2017
    # Mandatory field
    prepared-for: "{args.user}"

    # Project contact email address. It is used for
    # CIR, CVSS, PCIDSS, MobileOwasp, and Owasp2017
    # Mandatory field
    project-contact-email: "{args.email}"

    # Name of the entity that prepared the report.
    # Mandatory field
    prepared-by: "{args.user}"

# Locale of the report. Valid values are
#     en_US (English)
#     ja_JP (Japanese)
#     ko_KR (Korean)
#     zh_CN (Simplified Chinese)
# Default is en_US
locale: en_US

# Some reports display information about individual issues. These reports
# bound the number of issues displayed in order to control the size of the
# report. This bound is called the issue cutoff count. It is used for CVSS,
# Security, PCIDSS, MobileOwasp, Owasp2017 and DISA-STIG reports.
# Default is 200. Maximum is 10000 for report.
issue-cutoff-count: 4000

# Used for retrieving the defects of specific snapshot id, instead of using the latest snapshot id of
# all the streams associated with project(default behavior).
# It is not supported for CIR report.
snapshot-id: {snapshot_id}

# The most recent snapshot of each stream in the project whose date is less than or equal to the given date,
# will be used to collect the defects instead of latest snapshot
# Snapshot-id will get the highest priority if both snapshot-id and snapshot-date is present.
# Snapshot date should be in MM/DD/YYYY format
# It is not supported for CIR report.
#snapshot-date: 03/29/2022

# An optional comma-separated list of Coverity Connect issue kinds
# if issue kinds are listed here, the report will include issues only for
# the listed issue kinds.
# Possible values fore Issue Kind :
# Quality
# Security
# example: Quality
# example: Quality, Security
# Default: Include issues for both Quality and Security
issue-kind: SECURITY

# Components specification
# An optional comma-separated list of Coverity Connect component names,
# including component map name.
# If components are listed here, the report will include data only for
# the listed components.
# example: Default.lib,Default.src
components: 

################## CERT report #############
cert-report:   
    # Used for CERT config for target level. Valid values are
    # F ==> Fully Compliant
    # L2 ==> L2 Compliant
    # L1 ==> L1 Compliant
    # Default is F
    target-level: L1

################## Synopsys Software Integrity Report #############
ssir-report:   
    # analysis date should be in MM/DD/YYYY format
    # Mandatory field
    analysis-date: 

    # Optional legal text to display in the report
    # e.g:
    #     "This is the first line of multiline legal text
    #     and this is the second line."
    #
    legal-text: ""

################## Coverity Integrity Report #############
cir-report:   
    # Project-related fields
    # The project-description defaults to its description in Coverity
    # Connect, if any.
    # e.g. project-details: Some details for this project
    #      project-description: A short description of the project
    project-description: ""
    project-details: ""

    # Target integrity level.
    # 1 ==> < 1 defect per thousand lines of code
    # 2 ==> < .1 defects per thousand lines of code
    # 3 ==> < .01 defects per thousand lines of code, and other requirements
    # Default is 1
    target-integrity-level: 

    # Name of the highest severity value.
    # Default is "Major".
    high-severity-name: 

    # Name of the unspecified severity value.
    # Default is "Unspecified".
    unspecified-severity-name: 

    # Trial flag. Use "true" if page 3 of the report should not be generated.
    # Page 3 contains severity data which is not relevant for projects that
    # do not use severity.
    # Default is false
    trial: 

    # LOC multiplier for the number of lines of code that have been
    # inspected.
    # Default is 1
    loc-multiplier:

    # Whether to include Low impact defects for calculating the defect density
    # Possible values can be true, false
    # Default is false
    include-low-impact:

################## Coverity Security Report #############
security-report:   
    # There are four Assurance Levels, representing Security Scores of greater
    # than or equal to 60, 70, 80, and 90. When choosing the Assurance Level,
    # consider the potential for damage to life, property, or reputation.
    # An application with high damage potential should have a
    # high Assurance Level.
    # Default is 90 without plugin
    # if plugin yaml is available in the environment, values will come from plugin
    # and the highest score will be the default
    assurance-level-score: 90

    # A level indicating the minimum acceptable score for the report to
    # be considered passing.
    # Possible values can be (AL1, AL2, AL3, AL4) without plugin
    # and default will be AL1, if plugin yaml is available in the environment, values will come from plugin
    # and the highest score value will be the default
    assurance-level: AL1

    # The name of the set of severity mappings used to determine the score
    # of each issue. See the documentation for a description of the severity
    # mapping. The first three mappings are built-in. "Custom" indicates
    # that the mapping identified by "custom-severity-mapping" and
    # "severity-mapping-description" should be used.
    # Valid values:
    #    Carrier Grade
    #    Web application
    #    Desktop application
    #    Custom
    # Default : Carrier Grade, if plugin yaml is available in the environment, values will come from plugin
    # and the first severity-mapping will be the default
    severity-mapping: Carrier grade

    # Optional descriptive text for custom severity mapping
    severity-mapping-description: ""

    # If severity-mapping is Custom then set the below severity map
    custom-severity-mapping:   
        # Possible values are
        #    very high
        #    high
        #    medium
        #    low
        #    very low
        #    informational
        # Default is very high
        modify-data: Very High
        read-data: Very High
        dos-unreliable-execution: Very High
        dos-resource-consumption: Very High
        execute-unauthorized-code: Very High
        gain-privileges: Very High
        bypass-protection-mechanism: Very High
        hide-activities: Very High

disa-stig:   
    # DISA-STIG version
    # default version: V4

    version: V5
"""
    with open("_covreport.yml", "w", encoding="utf-8") as file:
        file.write(cfg)

    # Special environment with Coverity user information
    env = os.environ.copy()
    if args.password:
        env["COVERITY_PASSPHRASE"] = args.password
    if args.user:
        env["COV_USER"] = args.user

    report_path = os.path.join(args.report_dir, "cvss_report.pdf")
    arguments = [
        "_covreport.yml",
        "--output",
        report_path,
        "--report",
    ]
    if args.password:
        arguments.extend([
            "--password",
            "env:COVERITY_PASSPHRASE",
        ])
    run_command("cov-generate-cvss-report", *arguments, env=env)
    report_path = os.path.join(args.report_dir, "security_report.pdf")
    arguments = [
        "_covreport.yml",
        "--output",
        report_path,
    ]
    if args.password:
        arguments.extend([
            "--password",
            "env:COVERITY_PASSPHRASE",
        ])
    run_command("cov-generate-security-report", *arguments, env=env)


def run(args):
    """main entry point
    """
    # Set up analysis environment
    configure_coverity()
    md(args.intermediate_dir)
    build_under_coverity(args.intermediate_dir, args.command, args.strip_path)

    # Gather data
    md(os.path.join(args.report_dir, "json"))
    preview_report_v2_path = os.path.join(args.report_dir, "json",
                                          "preview_report_v2.json")
    json_report_path = os.path.join(args.report_dir, "json",
                                    "errors_v9_full.json")
    get_preview_report(args.intermediate_dir,
                       args.url,
                       args.stream,
                       args.code_version,
                       args.strip_path,
                       preview_report_v2_path,
                       json_report_path,
                       os.path.join(args.report_dir, "json",
                                    "errors_v9_active.json"),
                       os.path.join(args.report_dir, "html_full"),
                       os.path.join(args.report_dir, "html_active"),
                       os.path.join(args.report_dir, "text_report.txt"),
                       user=args.user,
                       password=args.password,
                       auth_key_file=args.auth_key_file)

    error_count = 0
    with open(json_report_path, "r", encoding="utf-8") as json_file:
        report = json.load(json_file)
        summary = ReportSummary(report, ['undecided'])
        error_count = summary.problems
        xunit_from_report(os.path.join(args.report_dir, "xunit.xml"), summary)
    snapshot_id = None
    if args.report or error_count > 0:
        publish_to_coverity_connect(args.intermediate_dir,
                                    args.url,
                                    args.stream,
                                    args.code_version,
                                    args.strip_path,
                                    args.description,
                                    "_snapshot_id.txt",
                                    user=args.user,
                                    password=args.password,
                                    auth_key_file=args.auth_key_file)

        with open("_snapshot_id.txt", "r",
                  encoding="utf-8") as snapshot_id_file:
            snapshot_id = snapshot_id_file.read().strip()
    snapshot_info = {}
    if snapshot_id is not None:
        snapshot_info["id"] = snapshot_id
    snapshot_info["stream"] = args.stream
    snapshot_info["version"] = args.code_version
    with open(os.path.join(args.report_dir, "json", "info.json"),
              "w",
              encoding="utf-8") as info_file:
        json.dump(snapshot_info, info_file, indent=2)

    if snapshot_id is not None:
        write_reports(args, snapshot_id)
    return error_count


# pylint: disable=bare-except
if __name__ == '__main__':
    sys.exit(run(read_command_line(None)))
