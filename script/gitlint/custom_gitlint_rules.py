# pylint: disable=too-few-public-methods
###############################################################################
# Copyright (C) 2020 Intel Corporation
#
# SPDX-License-Identifier: MIT
###############################################################################
"""
Custom rules for gitlint
"""

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
        for word in line.split():
            if word.endswith(bad_suffixes):
                violation = RuleViolation(self.id, "Title is not imperative",
                                          line)
                violations.append(violation)
            break

        return violations
