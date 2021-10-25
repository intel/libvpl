# Introduction

This folder contains scripts that represent common tasks a developer (or CI)
needs to perform when working on the project. In particular this folder runs
tasks across multiple repos.

These scripts assume that all repos are clones under a single root folder, and
are in folders with the repo name.

The scripts are designed around the model described by
[Github](https://github.blog/2015-06-30-scripts-to-rule-them-all/)


# Scripts

- `clean` - Remove project build files. Will not remove installed files or
bootstrap files placed in `VPL_BUILD_DEPENDENCIES`.

- `bootstrap` - Build the project dependencies and place in the location
  specified by the `VPL_BUILD_DEPENDENCIES` environment variable. If environment
  variable is undefined, define it as and place output in `_deps`.

- `build` - Build and install the project. If the `VPL_INSTALL_DIR` environment
  variable is set, the package will be installed there.