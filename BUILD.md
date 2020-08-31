# Configuring the Build Environment

This section describes the steps to configure the environment. It extends the
high level build information in the README.

The basic sequence is

1. Configure the environment

2. Build dependencies using script/bootstrap

3. Build the code using the script/build 

After you have configured the environment you can continue with the basic build
steps in the README. Note that if dependencies are not modified bootstrapping
only needs to be done once. The build script will look in the location specified
by the `VPL_BUILD_DEPENDENCIES` environment variable.


## Configuring the Windows Environment

Install common developer tools

        - [Microsoft Visual Studio](https://visualstudio.microsoft.com/) Visual Studio 2017 or newer

        - [CMake](https://cmake.org/) 

        - [Python3](https://www.python.org/)

        - [git](https://git-scm.com/)


### Install MSYS2 based tools

MSYS2 setup can be done many ways but this guide is based on 
[chocolatey](http://chocolatey.org). Some of the paths to mingw may be
different if another method to install MSYS2 is used.

From a cmd or powershell with admin permissions

```bash
       choco install msys2
```

Start a mingw64 shell from cmd or powershell with regular permissions.  

```bash
       c:\tools\msys64\msys2.exe
```

Please note: this path is from choco install.  If another install method is used
the path to the shell may be different.

Get packages required by build

```bash
       pacman -Syu
       pacman --needed -Sy mingw-w64-x86_64-toolchain base-devel yasm nasm mingw-w64-x86_64-cmake git python-pip mingw-w64-x86_64-meson mingw-w64-x86_64-ninja
```

## Configuring the Ubuntu Environment

From a shell as root:

```bash
       apt update
       apt-get update && apt-get install -y --no-install-recommends \
       build-essential git pkg-config yasm nasm cmake python3 \
       python3-setuptools python3-pip
       pip3 install -U wheel --user 
       pip3 install meson ninja
```


## Docker

See docker/README.md for more information.