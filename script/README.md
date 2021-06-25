# Introduction

This folder contains scripts that represent common tasks a developer (or CI)
needs to perform when working on the project.

The scripts are designed around the model described by
[Github](https://github.blog/2015-06-30-scripts-to-rule-them-all/)


# Scripts

- `lint` - Analyze project source for potential issues, including compliance
  with coding style. Requires Python modules in `requirements-dev.txt`.

- `clean` - Remove project build files. Will not remove installed files or
bootstrap files placed in `VPL_BUILD_DEPENDENCIES`.


- `bootstrap` - Build the project dependencies and place in the location
  specified by the `VPL_BUILD_DEPENDENCIES` environment variable. If environment
  variable is undefined, define it as and place output in `_deps`.

- `build` - Build the project and place output in `_build`. If the
  `VPL_INSTALL_DIR` environment variable is set, the package will be installed
  there at install time.

- `install` - Install the project built in the build stage.

- `test` or `test32` - Run basic smoke testing on built project.

- `stress` - Run stress testing on built project.


**LEGAL NOTICE:  By downloading and using any scripts (the “Software Package”) and the included software or software made available for download, you agree to the terms and conditions of the software license agreements for the Software Package, which may also include notices, disclaimers, or license terms for third party software (together, the “Agreements”) included in this README file.
If the Software Package is installed through a silent install, your download and use of the Software Package indicates your acceptance of the Agreements.**