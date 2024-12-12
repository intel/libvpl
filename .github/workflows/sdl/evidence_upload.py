"""
Module Description: This script provides a function to upload files for SDL evidence.

Usage:
    python evidence_upload.py --api_key <API_KEY>
    --user_id <USER_ID> --project_id <PROJECT_ID>
    --task_id <TASK_ID> --file_paths <FILE_PATHS> [--label <LABELS>]
"""
import os
import argparse
from urllib.parse import quote, quote_plus
import sys
import requests
import urllib3

urllib3.disable_warnings()


def upload_file(api_key, user_id, project_id, task_id, file_path, labels,
                output_prefix):
    # pylint: disable=too-many-locals
    # pylint: disable=too-many-arguments
    # pylint: disable=too-many-positional-arguments
    """
    Upload a file for a specific task.

    Parameters:
        api_key (str): API key for authentication.
        user_id (str): User ID.
        task_id (str): Task ID.
        project_id (int): Project ID.
        file_path (str): Path to the file to be uploaded.
        labels (list): List of labels or tags for the file upload.
        output_prefix (str): Prefix for output messages.

    """
    # URL encode user-provided values
    user_id_encoded = quote(user_id)
    project_id_encoded = quote_plus(str(project_id))
    task_id_encoded = quote_plus(task_id)

    # Build URL
    base_url = "https://sdl-e.app.intel.com/uploader/v1/evidence/uploads/documents/creators"
    upload_url = (f"{base_url}/{user_id_encoded}"
                  f"?projectId={project_id_encoded}&taskId={task_id_encoded}")
    if labels:
        for label in labels:
            if label:
                upload_url = f"{upload_url}&label={quote_plus(label)}"
    # Set headers
    headers = {
        "apikey": api_key,
    }

    # Prepare files for upload in multipart/form-data format
    try:
        file_name_with_prefix = f"{output_prefix}{os.path.basename(file_path)}"
        with open(file_path, 'rb') as file:
            files = {'file': (file_name_with_prefix, file)}

            # Make the request
            response = requests.post(upload_url,
                                     headers=headers,
                                     files=files,
                                     verify=False)
            print(f"Response Content: {response.text}")

            # Check for success
            if response.status_code == 200:
                response_text = response.text
                if 'Documents were uploaded correctly.' in response_text:
                    print(f"File for Task {task_id}  uploaded successfully!")
                else:
                    print(
                        f"Failed to upload file  for Task {task_id}. Response: {response_text}"
                    )
                    sys.exit(response.status_code)
            else:
                print(
                    f"Failed to upload file  for Task {task_id}. "
                    f"Status Code: {response.status_code}, Response: {response.text}"
                )
                sys.exit(response.status_code)

    except requests.RequestException as exception:
        print(f"Request failed for Task {task_id}: {exception}")
        sys.exit(1)
    except OSError as exception:
        print(f"Failed to open file {file_path}: {exception}")
        sys.exit(1)


def main():
    """
    Main function for uploading files.

    Command-line Arguments:
        --api_key (str): API key for authentication.
        --user_id (str): User ID.
        --project_id (int): Project ID.
        --task_id (str): Task ID.
        --file_paths (list): List of file paths.
        --label (list, optional): Label or tags for this file upload.
        --output_prefix (str): Prefix for output messages.
    """
    # Create argument parser
    parser = argparse.ArgumentParser(description="Upload files.")

    # Add arguments
    parser.add_argument("--api_key",
                        required=True,
                        help="API key for authentication")
    parser.add_argument("--user_id", required=True, help="User ID")
    parser.add_argument("--project_id",
                        required=True,
                        type=int,
                        help="Project ID")
    parser.add_argument("--task_id", required=True, help="Task ID")
    parser.add_argument("--file_paths",
                        required=True,
                        nargs="+",
                        help="List of file paths")
    parser.add_argument("--label",
                        required=False,
                        nargs="*",
                        help="Label or tags for this file upload")
    parser.add_argument("--output_prefix",
                        required=True,
                        help="Prefix for output messages")

    # Parse command-line arguments
    args = parser.parse_args()

    # Upload files
    for file_path in args.file_paths:
        upload_file(args.api_key, args.user_id, args.project_id, args.task_id,
                    file_path, args.label, args.output_prefix)
    sys.exit(0)


if __name__ == "__main__":
    main()
