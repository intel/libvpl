# Introduction

This folder contains scripts that represent common tasks a developer (or CI)
needs to perform when getting started with the project.

The scripts are loosely based on the model described by
[Github](https://github.blog/2015-06-30-scripts-to-rule-them-all/)


# Scripts

- `bootstrap` - Build the project dependencies.

- `build` - Build the project and place output in `_build`. Also install to
  staging area `_install`.

- `test` - Run basic smoke testing on built project. Scope can be limited by
  providing subset of tests as argument from among: lint, unit

- `install` - Install the project built in the build stage. If argument is
  provided then it will be interpreted as the install prefix. If called without
  arguments then installation will be to `VPL_INSTALL_DIR` if defined or system
  default location otherwise.


**LEGAL NOTICE: By downloading and using any scripts (the “Software Package”)
and the included software or software made available for download, you agree to
the terms and conditions of the software license agreements for the Software
Package, which may also include notices, disclaimers, or license terms for third
party software (together, the “Agreements”) included in this README file.  If
the Software Package is installed through a silent install, your download and
use of the Software Package indicates your acceptance of the Agreements.**
