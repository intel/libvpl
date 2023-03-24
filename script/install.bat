@rem ------------------------------------------------------------------------------
@rem Copyright (C) Intel Corporation
@rem
@rem SPDX-License-Identifier: MIT
@rem ------------------------------------------------------------------------------
@rem Install the product.
@rem
@rem If argument is provided then it will be interpreted as the install prefix. If
@rem called without arguments then installation will be to `VPL_INSTALL_DIR` if
@rem defined or system default location otherwise.


@echo off
setlocal ENABLEEXTENSIONS ENABLEDELAYEDEXPANSION

for %%Q in ("%~dp0\.") DO set "script_dir=%%~fQ"
pushd %script_dir%\..
  set "source_dir=%cd%"
popd
set "build_dir=%source_dir%\_build"

if [%1]==[] goto no_args
  ::  use first argument as install prefix
  cmake --install "%build_dir%" --config Release --strip --prefix "%1"
  goto done
:no_args
if DEFINED VPL_INSTALL_DIR (
  :: no argument but env variable was provided, install to VPL_INSTALL_DIR
  cmake --install "%build_dir%" --config Release --strip --prefix "%VPL_INSTALL_DIR%"
) else (
  :: no argument or env variable defined, use system default install location
  cmake --install "%build_dir%" --config Release --strip
)
:done

endlocal
