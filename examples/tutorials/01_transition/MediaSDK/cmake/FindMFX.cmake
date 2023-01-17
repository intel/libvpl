include(FindPackageHandleStandardArgs)

# INTELMEDIASDKROOT is the Media SDK install location it can be set either by
# environment variable (default for Media SDK install) or with
# -DINTELMEDIASDKROOT

if(DEFINED ENV{INTELMEDIASDKROOT})
  set(INTELMEDIASDKROOT $ENV{INTELMEDIASDKROOT})
endif()

find_path(MFX_INCLUDE_DIR mfxvideo.h PATHS ${INTELMEDIASDKROOT}/include)
find_library(MFX_LIBRARY libmfx PATHS ${INTELMEDIASDKROOT}/lib/x64)

if(NOT MFX_LIBRARY)
  message(
    FATAL_ERROR
      "libmfx not found.  Set INTELMEDIASDKROOT to root of Media SDK install directory"
  )
endif()

find_package_handle_standard_args(MFX DEFAULT_MSG MFX_INCLUDE_DIR MFX_LIBRARY)

mark_as_advanced(MFX_LIBRARY MFX_INCLUDE_DIR)

if(MFX_FOUND)
  set(MFX_LIBRARIES ${MFX_LIBRARY})
  set(MFX_INCLUDE_DIRS ${MFX_INCLUDE_DIR})
endif()
