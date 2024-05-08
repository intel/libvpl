#!/usr/bin/env python
"""Check to see if results from scorecard exceed minimum required values."""

import argparse
import json
import sys
import yaml


def get_options():
    """Parse command line."""
    description = __doc__.split('\n', maxsplit=1)[0]
    parser = argparse.ArgumentParser(description=description)

    parser.add_argument('scorecard',
                        metavar='FILE',
                        help='JSON file containing scorecard')

    parser.add_argument('--config',
                        metavar='FILE',
                        help='YAML file with required minimum scores')

    return parser.parse_args()


def get_scores(scorecard):
    """Extract checks from scorecard as key value pairs"""
    result = {}
    for check in scorecard['checks']:
        result[check['name']] = check['score']
    return result


def main(scorecard, config):
    """Compare scorecard scores with expectations from config.

    Return number of checks that fail to meet expectations.

    """
    num_fails = 0
    checks = config.get('Checks')
    scores = get_scores(scorecard)
    for check, minval in checks.items():
        score = scores.get(check, -1)
        if score < minval:
            num_fails += 1
        print(f"{check:23}: {score:2}/{minval:2}"
              f"{' ':6} {'Passed' if score >= minval  else 'Failed'}")
    return num_fails


if __name__ == "__main__":
    options = get_options()

    with open(options.scorecard, 'r', encoding="utf-8") as json_file:
        score_dict = json.load(json_file)

    config_dict = {'Checks': {}}
    if options.config:
        with open(options.config, 'r', encoding="utf-8") as yaml_file:
            config_dict = yaml.safe_load(yaml_file)

    sys.exit(main(score_dict, config_dict))
