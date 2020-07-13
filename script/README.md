# Introduction

This folder contains scripts that represent common tasks a developer (or CI)
needs to perform when working on the project.

These are designed around the model described by
[Github](https://github.blog/2015-06-30-scripts-to-rule-them-all/)


# Scripts

- `lint` - Analyze project source for potential issues. Requires Python modules
  in `requirements-dev.txt`.

- `bootstrap` - Build the project dependencies and place in the location
  specified by `VPL_BUILD_DEPENDENCIES` in the environment. If environment
  variable is undefined, define it as and place output in `_deps`.

- `build` - Build the project and place output in `_build`.

- `package` - Create zip packages and place in `_build`.

  -  `*-all.zip` contains the files in the complete product installation

  -  `*-dev.zip` contains the files for the developer package

  -  `*-runtime.zip` contains the files for the runtime package

- `test` - Run smoke tests.

- `cibuild` - Run CI steps. This is the script CI calls when it is triggered on
  a merge request.
