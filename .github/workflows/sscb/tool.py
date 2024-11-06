"""
This script provides functionality for processing issue tracking data
extracted from JSON and YAML files. It includes the ability to count
issues by severity, filter issues based on configuration, and determine
an overall pass/fail status based on the issues present.
"""
import json
import argparse
import yaml


# Function to load data from a JSON file
def load_json(file_name):
    """
    Load data from a JSON file.

    Args:
        file_name (str): The name of the file to load.

    Returns:
        dict: The data loaded from the JSON file.
    """
    with open(file_name, 'r', encoding="utf-8") as file:
        return json.load(file)


# Function to load data from a YAML file
def load_yaml(file_name):
    """
    Load data from a YAML file.

    Args:
        file_name (str): The name of the file to load.

    Returns:
        dict: The data loaded from the YAML file.
    """
    with open(file_name, 'r', encoding="utf-8") as file:
        return yaml.safe_load(file)


# Function to count the number of high severity issues
def high_count(issues):
    """
    Count the number of high severity issues.

    Args:
        issues (list of dict): A list of issues.

    Returns:
        int: The count of high severity issues.
    """
    return sum(1 for issue in issues if issue['severity'] == 'High')


# Function to count the number of medium severity issues
def medium_count(issues):
    """
    Count the number of medium severity issues.

    Args:
        issues (list of dict): A list of issues.

    Returns:
        int: The count of medium severity issues.
    """
    return sum(1 for issue in issues if issue['severity'] == 'Medium')


# Function to count the number of low severity issues
def low_count(issues):
    """
    Count the number of low severity issues.

    Args:
        issues (list of dict): A list of issues.

    Returns:
        int: The count of low severity issues.
    """
    return sum(1 for issue in issues if issue['severity'] == 'Low')


# Function to count the number of informational severity issues
def info_count(issues):
    """
    Count the number of informational severity issues.

    Args:
        issues (list of dict): A list of issues.

    Returns:
        int: The count of informational severity issues.
    """
    return sum(1 for issue in issues if issue['severity'] == 'Info')


# Retrieve ignored issues from the YAML configuration
def get_ignored_issues(config):
    """
    Retrieve ignored issues from the YAML configuration.

    Args:
        config (dict): A dictionary containing configuration, including ignored issues.

    Returns:
        tuple: A tuple containing lists of ignored issues, ignored severity, and ignored file names.
    """
    ignored_issues = config.get('ignored_issues', [])
    ignored_severity = config.get('ignored_severity', [])
    ignored_file = config.get('ignored_file', [])
    return ignored_issues, ignored_severity, ignored_file


# Function to extract the file name from a given path
def extract_file_name(full_file_name):
    """
    Extracts the third component from a file path.

    Args:
        full_file_name (str): The full file path.

    Returns:
        str: The extracted file name or 'Unknown' if not available.
    """
    parts = full_file_name.split('|')
    return parts[2] if len(parts) >= 3 else 'Unknown'


# Function to determine if an issue should be included based on various criteria
def issues_included(issue_key, severity, ignored_issues, ignored_severities,
                    ignored_file, file_name):
    """
    Determines if an issue should be included based on various criteria.

    Args:
        issue_key (str): The issue key.
        severity (str): The severity of the issue.
        ignored_issues (list): A list of issue keys to ignore.
        ignored_severities (list): A list of severities to ignore.
        ignored_file (list): A list of file names to ignore.
        file_name (str): The file name of the issue.

    Returns:
        bool: True if the issue should be included, False otherwise.
    """
    return (issue_key not in ignored_issues
            and severity not in ignored_severities
            and file_name not in ignored_file)


# Main function to extract issues from JSON data
def get_issues(data, ignored_issues, ignored_severities, ignored_file):
    """
    Extract issues from the parsed JSON data, excluding specific criteria.

    Args:
        data (dict): The parsed data from JSON.
        ignored_issues (list): A list of issue keys to ignore.
        ignored_severities (list): A list of severities to ignore.
        ignored_file (list): A list of file names to ignore.

    Returns:
        list of dict: A list of issue dictionaries.
    """
    issues = []
    for file_key, issue_details in data.items():
        if not file_key.startswith('File'):
            continue

        full_file_name = issue_details.get('File Name', 'Unknown')
        file_name = extract_file_name(full_file_name)

        for key, value in issue_details.items():
            if not (isinstance(value, str) and '[' in value and ']' in value):
                continue

            start = value.find('[') + 1
            end = value.find(']')
            severity_with_info = value[start:end].strip().split(' ')[0]

            if issues_included(key, severity_with_info, ignored_issues,
                               ignored_severities, ignored_file, file_name):
                issues.append({
                    "issue_name": key,
                    "severity": severity_with_info,
                    "file": file_name
                })
    return issues


def pass_or_fail(issues):
    """
    Determine the overall pass/fail status based on the list of issues.

    Args:
        issues (list of dict): A list of issues.

    Returns:
        str: "Pass" if there are no issues, otherwise "Fail".
    """
    # Determine if the result is pass or fail based on remaining issues
    return "Pass" if not issues else "Fail"


def main():
    """
    Main part of the script.
    Parses command-line arguments to obtain JSON and YAML file names,
    loads data from these files, processes it to filter out ignored issues,
    and prints a summary of the analysis including the overall status
    and details of each issue.
    """
    # Create the parser
    parser = argparse.ArgumentParser()

    # Add arguments for the JSON and YAML file names
    parser.add_argument('json_file')
    parser.add_argument('yaml_file')

    # Parse the command-line arguments
    args = parser.parse_args()

    # Use the parsed arguments to get the file names
    json_data = load_json(args.json_file)
    ignore_config = load_yaml(args.yaml_file)

    # Retrieve ignored issues from YAML configuration
    ignored_issues_list, ignored_severities_list, ignored_file_list = get_ignored_issues(
        ignore_config)

    # Get the list of issues while excluding the ignored ones
    issues_list = get_issues(json_data, ignored_issues_list,
                             ignored_severities_list, ignored_file_list)

    # Determine the overall pass or fail status
    result = pass_or_fail(issues_list)
    print("Status:", result)

    # Output the overall status and counts of each severity level
    print("High severity count:", high_count(issues_list))
    print("Medium severity count:", medium_count(issues_list))
    print("Low severity count:", low_count(issues_list))
    print("Info severity count:", info_count(issues_list))

    # Output the details of each issue not ignored
    for issue in issues_list:
        print(f"Issue Name: {issue['issue_name']}; "
              f"Severity: {issue['severity']}; "
              f"File: {issue['file']}")


if __name__ == "__main__":
    main()
