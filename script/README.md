# Introduction

This folder contains scripts that represent common tasks a developer (or CI)
needs to perform when working on the project.

The scripts are designed around the model described by
[Github](https://github.blog/2015-06-30-scripts-to-rule-them-all/)


# Scripts

- `lint` - Analyze project source for potential issues, including compliance
  with coding style. Requires Python modules in `requirements-dev.txt`.

- `clean` - Remove project build files. 

- `bootstrap` - Build the project dependencies.

- `build` - Build the project and place output in `_build`. If the
  `VPL_INSTALL_DIR` environment variable is set, the package will be installed
  there at install time.

- `install` - Install the project built in the build stage.

- `test` or `test32` - Run basic smoke testing on built project.

- `stress` - Run stress testing on built project.

# Environment Variables

While no environment variables are strictly required to use these scripts 
there are environment variables that, if set, allow control of how and
where the scripts build and place files.

## Common

These settings are shared with the CPU Runtime.

- `VPL_INSTALL_DIR` - The root folder in which to install after building.
By default files will be installed in a system location such as `/` or `~/`
however developers will often want to segregate code being developed.
If this is set VPL build scripts will place files under that folder and
use files found under that folder to find cross dependencies.

## Dispatcher Specific

- `VPL_DISP_BUILD_DIR` - The folder to be used by CMake for building. 
(Defaults to `<script-dir>/../_build/`)


**LEGAL NOTICE:  By downloading and using any scripts (the “Software Package”) and the included software or software made available for download, you agree to the terms and conditions of the software license agreements for the Software Package, which may also include notices, disclaimers, or license terms for third party software (together, the “Agreements”) included in this README file.
If the Software Package is installed through a silent install, your download and use of the Software Package indicates your acceptance of the Agreements.**
