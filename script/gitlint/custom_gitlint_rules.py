# pylint: disable=too-few-public-methods
###############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
###############################################################################
"""
Custom rules for gitlint
"""

import os
from gitlint.rules import LineRule, RuleViolation, CommitMessageTitle


class TitleCapitalized(LineRule):
    """This rule will enforce that the first word of the commit message title is
capitalized"""

    name = "title-is-capitalized"
    id = "UL3"
    target = CommitMessageTitle

    def validate(self, line, _commit):
        """Validate that the title is capitalized"""
        violations = []
        if line:
            if not line[0].isupper():
                violation = RuleViolation(
                    self.id, "Title does not start with a capital letter",
                    line)
                violations.append(violation)

        return violations


__location__ = os.path.realpath(
    os.path.join(os.getcwd(), os.path.dirname(__file__)))


class TitleImperative(LineRule):
    """This rule will enforce that the commit message title uses the imperative
mood.

For now rather than using an advanced linguistic database, it just rejects
common bad patterns, such as words ending in ed or ing."""

    name = "title-is-imperative"
    id = "UL5"
    target = CommitMessageTitle

    def validate(self, line, _commit):
        """Validate that the title is imperative"""
        violations = []
        bad_suffixes = ('ed', 'ing')
        with open(os.path.join(__location__, 'verbs.txt'),
                  encoding="utf-8") as verbfile:
            verbs_ending_with_s = [line.rstrip() for line in verbfile]
        for word in line.split():
            if word.endswith(bad_suffixes) or (word.endswith('s')
                                               and word.lower()
                                               not in verbs_ending_with_s):
                violation = RuleViolation(self.id, "Title is not imperative",
                                          line)
                violations.append(violation)
            break

        return violations
