How to set up MSDK samples build environment.

Generally, we want to separate MSDK samples build/debug process from MSDK dispatcher and library build process.
To do it, we've created bat script that copies all the required libraries to a MSDK_Install directory. All the sample projects should refer to that directory and NOT to any directory related to dispatcher or library directly.
The sturcture of this directory is exactly the same as MSDK installation directory on end user PC.

To compile samples you'll need:
1. Microsoft Visual Studio 2010 or later.
2. Dispatcher (msdk_api) compiled with the same version of Visual Studio you going to use 
   with samples (2010 by default).
   For easier debugging and building, compile all 4 versions of library 
   (all combinations of win32/x64 debug/release).
3. MSDK Library DLLs compiled (any version, you can use precompiled binaries)

To create MSDK_Install directory:
1. Run createMSDKInstalled.bat providing 2 parameters:
   - Path to target directory (including its name)
   - Path to mdp_msdk-lib/api directory.
   - Path to mdp_msdk-samples directory.

   Note: createMSDKInstalled.bat assumes that both dispatcher libaries and library dll 
   files are stored at mdp_msdk-lib/api\..\Build directory. That path is taken from current 
   settings of that project. However, if it will be changed in future, those changes should 
   be reflected in .bat file.

   Example: createMSDKInstalled.bat c:\MSDK_Inst_Dir C:\VC\mdp_msdk-lib/api C:\VC\mdp_msdk-samples

   After running bat file make sure that target directory has the same structure as shown below:

   MSDK_Install
     bin
       Win32 (libmfx*.dll should be here)
       x64   (libmfx*.dll should be here)
     include
     lib
       Win32 (libmfx*.lib should be here)
       x64   (libmfx*.lib should be here)

   ,also INTELMEDIASDKROOT environment variable should be set to the TargeDir 
   (createMSDKInstalled.bat takes care of it)

2. Set INTELMEDIASDK_WINSDK_PATH to Windows SDK location manually 
   (something like C:\Program Files (x86)\Windows Kits\8.1)
3. After that you may remove all the library and dispatcher stuff 
   (except files stored in MSDK_Install directory), samples source code should use only the 
   files stored in MSDK_Install directory.
   You may compile samples source code using Visual Studio 2010 or later.



External dependencies:
*** OpenCL ***
You'll need Intel OpenCL code builder (part of INDE) installed to compile plugin_opencl sample

*** FFMPEG ***
You'll need SPECIFIC build of FFMPEG, otherwise it will crash with access violation.
Binaries for win64 can be found here: http://ffmpeg.zeranoe.com/builds/win64/shared/ffmpeg-2.0.2-win64-shared.7z
Binaries for win32 can be found here: http://ffmpeg.zeranoe.com/builds/win32/shared/ffmpeg-2.0.2-win32-shared.7z
Headers can be found here:
http://ffmpeg.zeranoe.com/builds/win64/dev/ffmpeg-2.0.2-win64-dev.7z

You have to set INTELMEDIASDK_FFMPEG_ROOT environment variable to the FFMPEG installation directory.
Also, check that you have this structure of FFMPEG directory and move files if needed:
FFMPEG
  doc
  include (subdirectories with include files are here)
  lib
    Win32 (.lib files for Win32 are here)
    x64  (.lib files for x64 are here)
  licenses

Also, you have to copy all the .dll files for corresponding platform to the directory where sample executable resides (or add .dll directory to the PATH environment variable)