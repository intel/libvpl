# Introduction

This folder contains scripts that represent common tasks a developer (or CI)
needs to perform when working on the project.

The scripts are designed around the model described by
[Github](https://github.blog/2015-06-30-scripts-to-rule-them-all/)


# Scripts

- `lint` - Analyze project source for potential issues, including compliance
  with coding style. Requires Python modules in `requirements-dev.txt`.

- `bootstrap` - Build the project dependencies and place in the location
  specified by the `VPL_BUILD_DEPENDENCIES` environment variable. If environment
  variable is undefined, define it as and place output in `_deps`.

- `build` - Build the project and place output in `_build`. If the
  `VPL_INSTALL_DIR` environment variable is set, the package will be installed
  there at install time.

- `install` - Install the project built in the build stage.
