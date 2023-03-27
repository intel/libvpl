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
import xml.dom.minidom as minidom  # nosec

from lib.cmdline import run_command, md


def pretty_print_xml(root):
    """Format xml for legibility"""
    serial_xml = ET.tostring(root)
    parsed_xml = minidom.parseString(serial_xml)  # nosec
    return parsed_xml.toprettyxml()


# pylint: disable=too-many-branches
def summarize_report(report):
    """summarize a report"""
    suite = []
    if "issues" in report and report["issues"]:
        for issue in report["issues"]:
            test_info = {}
            suite.append(test_info)
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
            if "checkerProperties" in issue and issue["checkerProperties"]:
                checker_properties = issue["checkerProperties"]
                test_info["desc"] = checker_properties[
                    "subcategoryLongDescription"]
                test_info["effect"] = checker_properties[
                    "subcategoryLocalEffect"]
            else:
                test_info["desc"] = issue["checkerName"]
                test_info["effect"] = issue["type"]
            test_info['type'] = issue["checkerName"]
            test_info['file'] = issue["strippedMainEventFilePathname"]
            test_info['line'] = str(issue["mainEventLineNumber"])
            test_info['col'] = str(issue["mainEventColumnNumber"])
            test_info["name"] = ":".join(
                [test_info["file"], test_info["line"], test_info["col"]])
            test_info["mergeKey"] = issue["mergeKey"]
            summary = []
            if test_info.get("desc"):
                summary.append(test_info["desc"])
            if test_info.get("desc"):
                summary.append(test_info["effect"])
            if test_info.get("mergeKey"):
                summary.append(f"Merge Key: {test_info['mergeKey']}")
            if test_info.get("cid"):
                summary.append(f"CID: {test_info['cid']}")
            if test_info.get("external_ref"):
                summary.append(f"see: {test_info['external_ref']}")
            test_info["summary"] = '\n\n'.join(summary)
            if test_info.get("external_ref"):
                test_info["message"] = test_info.get("external_ref")
            else:
                test_info["message"] = f"CID: {test_info['cid']}"
    return suite


def xunit_from_report(report, xunit, include_actions):
    """write an xunit report for the results"""
    suite = summarize_report(report)
    testsuites = ET.Element('testsuites')
    testsuite = ET.SubElement(testsuites, "testsuite")
    testsuite.attrib["name"] = "coverity"
    suite_fail = 0
    for test in suite:
        ignore = test["status"] not in include_actions
        testcase = ET.SubElement(testsuite, "testcase")
        testcase.attrib["classname"] = test['type']
        testcase.attrib["name"] = test["name"]
        if ignore:
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
    testsuite.attrib["tests"] = str(len(suite))
    testsuite.attrib["failures"] = str(suite_fail)
    testsuites.attrib["tests"] = str(len(suite))
    testsuites.attrib["failures"] = str(suite_fail)

    pretty_xunit_report = pretty_print_xml(testsuites)
    with open(xunit, "w") as xml_file:
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
    parser.add_argument('--user',
                        action="store",
                        required=True,
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
    args.intermediate_dir = os.path.abspath(args.intermediate_dir)
    args.report_dir = os.path.abspath(args.report_dir)
    args.strip_path = os.path.abspath(args.strip_path)
    return args


def run(args):
    """main entry point
    """
    # Set up analysis environment
    md(args.intermediate_dir)

    if os.name == "nt":
        run_command("cov-configure", "--msvc")
    run_command("cov-configure", "--gcc")
    run_command("cov-configure", "--template", "--compiler", "cc",
                "--comptype", "gcc")
    run_command("cov-configure", "--template", "--compiler", "c++",
                "--comptype", "g++")

    run_command("cov-build", "--dir", args.intermediate_dir, *args.command)
    # Analyze
    run_command("cov-analyze", "--dir", args.intermediate_dir, "--strip-path",
                args.strip_path, "--enable-constraint-fpp", "--ticker-mode",
                "none", "--disable-default", "--concurrency", "--security",
                "--rule", "--enable-fnptr", "--enable-virtual", "--enable",
                "SECURE_CODING")

    # Gather data
    md(os.path.join(args.report_dir, "json"))
    preview_report_v2_path = os.path.join(args.report_dir, "json",
                                          "preview_report_v2.json")
    snapshot_id_path = "_snapshot_id.txt"

    # Special environment with Coverity user information
    env = os.environ.copy()
    env["COVERITY_PASSPHRASE"] = args.password
    env["COV_USER"] = args.user
    run_command("cov-commit-defects",
                "--dir",
                args.intermediate_dir,
                "--url",
                args.url,
                "--stream",
                args.stream,
                "--ticker-mode",
                "none",
                "--version",
                args.code_version,
                "--strip-path",
                args.strip_path,
                "--preview-report-v2",
                preview_report_v2_path,
                env=env)

    report_path = os.path.join(args.report_dir, "json", "errors_v9_full.json")
    json_report_path = report_path  # use this report to emit issues
    run_command("cov-format-errors", "--dir", args.intermediate_dir,
                "--no-default-triage-filters", "--preview-report-v2",
                preview_report_v2_path, "--json-output-v9", report_path)

    report_path = os.path.join(args.report_dir, "json",
                               "errors_v9_active.json")
    run_command("cov-format-errors", "--dir", args.intermediate_dir,
                "--preview-report-v2", preview_report_v2_path,
                "--json-output-v9", report_path)

    report_path = os.path.join(args.report_dir, "html_full")
    md(report_path)
    run_command("cov-format-errors", "--dir", args.intermediate_dir,
                "--no-default-triage-filters", "--preview-report-v2",
                preview_report_v2_path, "--html-output", report_path)

    report_path = os.path.join(args.report_dir, "html_active")
    md(report_path)
    run_command("cov-format-errors", "--dir", args.intermediate_dir,
                "--preview-report-v2", preview_report_v2_path, "--html-output",
                report_path)

    run_command("cov-commit-defects",
                "--dir",
                args.intermediate_dir,
                "--url",
                args.url,
                "--stream",
                args.stream,
                "--ticker-mode",
                "none",
                "--version",
                args.code_version,
                "--strip-path",
                args.strip_path,
                "--description",
                args.description,
                "--snapshot-id-file",
                snapshot_id_path,
                env=env)

    snapshot_id = None
    with open(snapshot_id_path, "r") as snapshot_id_file:
        snapshot_id = snapshot_id_file.read().strip()
    snapshot_info = {}
    snapshot_info["id"] = snapshot_id
    snapshot_info["stream"] = args.stream
    snapshot_info["version"] = args.code_version
    with open(os.path.join(args.report_dir, "json", "info.json"),
              "w") as info_file:
        json.dump(snapshot_info, info_file, indent=2)

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
# all the streams associated with project(default behaviour).
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
    with open("_covreport.yml", "w") as file:
        file.write(cfg)

    report_path = os.path.join(args.report_dir, "cvss_report.pdf")
    run_command("cov-generate-cvss-report",
                "_covreport.yml",
                "--output",
                report_path,
                "--report",
                "--password",
                "env:COVERITY_PASSPHRASE",
                env=env)
    report_path = os.path.join(args.report_dir, "security_report.pdf")
    run_command("cov-generate-security-report",
                "_covreport.yml",
                "--output",
                report_path,
                "--password",
                "env:COVERITY_PASSPHRASE",
                env=env)

    with open(json_report_path, "r") as json_file:
        report = json.load(json_file)
        xunit_from_report(report, os.path.join(args.report_dir, "xunit.xml"),
                          ['undecided'])

    return 0


# pylint: disable=bare-except
if __name__ == '__main__':
    sys.exit(run(read_command_line(None)))
