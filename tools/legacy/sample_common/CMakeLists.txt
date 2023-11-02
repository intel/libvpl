# ##############################################################################
# Copyright (C) 2005 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
set(TOOLS_DEFAULT ON)
if(CMAKE_SYSTEM_NAME MATCHES Linux)
  # Check dependencies up-front so we can set option defaults
  find_package(PkgConfig REQUIRED)
  # note: pkg-config version for libva is *API* version
  pkg_check_modules(PKG_LIBVA libva>=1.2 IMPORTED_TARGET GLOBAL)
  pkg_check_modules(PKG_LIBVA_DRM libva-drm>=1.2 IMPORTED_TARGET GLOBAL)
  pkg_check_modules(PKG_LIBDRM libdrm>=2.4.91 IMPORTED_TARGET GLOBAL)
  pkg_check_modules(PKG_X11 x11 IMPORTED_TARGET GLOBAL)
  pkg_check_modules(PKG_LIBVA_X11 libva-x11>=1.10.0 IMPORTED_TARGET GLOBAL)
  pkg_check_modules(
    PKG_XCB
    xcb
    xcb-dri3
    x11-xcb
    xcb-present
    IMPORTED_TARGET
    GLOBAL)
  pkg_check_modules(PKG_WAYLAND_CLIENT wayland-client IMPORTED_TARGET GLOBAL)
  find_program(WAYLAND_SCANNER_BIN "wayland-scanner")
  pkg_check_modules(PKG_WAYLAND_PROTOCOLS "wayland-protocols>=1.15")

  # Set defaults for ENABLE_VA
  if(PKG_LIBVA_FOUND AND PKG_LIBVA_DRM_FOUND)
    set(VA_DEFAULT ON)
  else()
    set(VA_DEFAULT OFF)
  endif()
  set(ENABLE_VA
      ${VA_DEFAULT}
      CACHE BOOL "Enable VA support in tools.")

  # Set default for ENABLE_DRM
  if(VA_DEFAULT AND PKG_LIBDRM_FOUND)
    set(DRM_DEFAULT ON)
  else()
    set(DRM_DEFAULT OFF)
  endif()
  set(ENABLE_DRM
      ${DRM_DEFAULT}
      CACHE BOOL "Enable VA/DRM in tools.")

  # Set default for ENABLE_WAYLAND
  if(VA_DEFAULT
     AND DRM_DEFAULT
     AND PKG_WAYLAND_CLIENT_FOUND
     AND WAYLAND_SCANNER_BIN
     AND PKG_WAYLAND_PROTOCOLS_FOUND)
    set(WAYLAND_DEFAULT ON)
  else()
    set(WAYLAND_DEFAULT OFF)
  endif()
  set(ENABLE_WAYLAND
      ${WAYLAND_DEFAULT}
      CACHE BOOL "Enable VA/Wayland in tools.")

  # Set default for ENABLE_X11
  if(VA_DEFAULT AND PKG_X11_FOUND)
    set(X11_DEFAULT ON)
  else()
    set(X11_DEFAULT OFF)
  endif()
  set(ENABLE_X11
      ${X11_DEFAULT}
      CACHE BOOL "Enable VA/X11 in tools.")
  # set default for ENABLE_DRI3
  if(X11_DEFAULT
     AND PKG_XCB_FOUND
     AND PKG_LIBDRM_FOUND)
    set(DRI3_DEFAULT ON)
  else()
    set(DRI3_DEFAULT OFF)
  endif()
  set(ENABLE_DRI3
      ${DRI3_DEFAULT}
      CACHE BOOL "Enable X11/DRI3 in tools.")
endif()

# Error if options conflict
if(NOT ENABLE_VA
   AND (ENABLE_DRM
        OR ENABLE_WAYLAND
        OR ENABLE_X11
        OR ENABLE_DRI3
       ))
  message(
    SEND_ERROR "conflicting options: VA backend requested but ENABLE_VA=OFF")
endif()

if(ENABLE_WAYLAND AND NOT ENABLE_DRM)
  message(SEND_ERROR "conflicting options: Wayland requires ENABLE_DRM=ON")
endif()

if(ENABLE_DRI3 AND (NOT ENABLE_X11 OR NOT ENABLE_DRM))
  message(
    SEND_ERROR
      "conflicting options: DRI3 requires ENABLE_X11=ON and ENABLE_DRM=ON")
endif()

set(TARGET sample_common)
add_library(${TARGET} STATIC)

target_sources(
  ${TARGET}
  PRIVATE src/avc_bitstream.cpp
          src/avc_nal_spl.cpp
          src/avc_spl.cpp
          src/base_allocator.cpp
          src/brc_routines.cpp
          src/d3d11_allocator.cpp
          src/d3d11_device.cpp
          src/d3d_allocator.cpp
          src/d3d_device.cpp
          src/decode_render.cpp
          src/general_allocator.cpp
          src/mfx_buffering.cpp
          src/parameters_dumper.cpp
          src/plugin_utils.cpp
          src/preset_manager.cpp
          src/sample_utils.cpp
          src/sysmem_allocator.cpp
          src/v4l2_util.cpp
          src/vaapi_allocator.cpp
          src/vaapi_device.cpp
          src/vaapi_utils_android.cpp
          src/vaapi_utils.cpp
          src/vaapi_utils_drm.cpp
          src/vaapi_utils_x11.cpp
          src/vpl_implementation_loader.cpp
          src/vpp_ex.cpp
          src/vm/atomic.cpp
          src/vm/atomic_linux.cpp
          src/vm/shared_object.cpp
          src/vm/shared_object_linux.cpp
          src/vm/thread_linux.cpp
          src/vm/thread_windows.cpp
          src/vm/time.cpp
          src/vm/time_linux.cpp)

target_include_directories(
  ${TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
                   ${CMAKE_CURRENT_SOURCE_DIR}/include/vm)

if(MSVC)
  target_compile_definitions(${TARGET} PUBLIC _CRT_SECURE_NO_WARNINGS)
endif()

target_compile_definitions(${TARGET} PUBLIC MFX_DEPRECATED_OFF)

find_package(VPL REQUIRED)
target_link_libraries(${TARGET} PUBLIC VPL::dispatcher
                                       media_sdk_compatibility_headers)

if(BUILD_TOOLS_ONEVPL_EXPERIMENTAL)
  target_compile_definitions(${TARGET} PUBLIC -DONEVPL_EXPERIMENTAL)
endif()

if(CMAKE_SYSTEM_NAME MATCHES Linux)
  target_compile_definitions(${TARGET} PUBLIC LINUX64)

  if(NOT ENABLE_VA)
    message(STATUS "Building ${TARGET} without VA support")
  else()
    if(PKG_LIBVA_FOUND AND PKG_LIBVA_DRM_FOUND)
      target_compile_definitions(${TARGET} PUBLIC LIBVA_SUPPORT)
      target_include_directories(${TARGET} PUBLIC ${PKG_LIBVA_INCLUDE_DIRS})

      if(ENABLE_DRM)
        if(PKG_LIBDRM_FOUND)
          target_compile_definitions(${TARGET} PUBLIC LIBVA_DRM_SUPPORT)
          target_include_directories(${TARGET}
                                     PUBLIC ${PKG_LIBDRM_INCLUDE_DIRS})

          if("${PKG_LIBDRM_VERSION}" VERSION_GREATER 2.4.113)
            target_compile_definitions(${TARGET}
                                       PUBLIC DRM_LINUX_MODIFIER_TILED4_SUPPORT)
          else()
            message(
              STATUS
                "libdrm version ${PKG_LIBDRM_VERSION} does not support I915_FORMAT_MOD_4_TILED"
            )
          endif()

          if("${PKG_LIBDRM_VERSION}" VERSION_GREATER 2.4.107)
            target_compile_definitions(
              ${TARGET} PUBLIC DRM_LINUX_FORMAT_MODIFIER_BLOB_SUPPORT)
          else()
            message(
              STATUS
                "libdrm version ${PKG_LIBDRM_VERSION} does not support drmModeFormatModifierBlobIterNext() API"
            )
          endif()

          if("${PKG_LIBDRM_VERSION}" VERSION_GREATER 2.4.104)
            target_compile_definitions(${TARGET} PUBLIC DRM_LINUX_HDR_SUPPORT)
          else()
            message(
              STATUS
                "libdrm version ${PKG_LIBDRM_VERSION} does not support hdr_output_metadata header"
            )
          endif()

          if("${PKG_LIBDRM_VERSION}" VERSION_GREATER 2.4.97)
            target_compile_definitions(${TARGET} PUBLIC DRM_LINUX_P010_SUPPORT)
          else()
            message(
              STATUS
                "libdrm version ${PKG_LIBDRM_VERSION} does not support DRM_FORMAT_P010"
            )
          endif()

        else()
          message(
            SEND_ERROR
              "libdrm not found: set ENABLE_DRM=OFF to to build tools with DRM disabled"
          )
        endif()
      else()
        message(STATUS "Building ${TARGET} without DRM support")
      endif()

      if(ENABLE_WAYLAND)
        if(PKG_WAYLAND_CLIENT_FOUND)
          target_compile_definitions(${TARGET} PUBLIC LIBVA_WAYLAND_SUPPORT)
          target_include_directories(${TARGET}
                                     PUBLIC ${PKG_WAYLAND_CLIENT_INCLUDE_DIRS})
          target_include_directories(
            ${TARGET}
            PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../sample_misc/wayland/include)
          if(WAYLAND_SCANNER_BIN AND PKG_WAYLAND_PROTOCOLS_FOUND)
            pkg_get_variable(WAYLAND_PROTOCOLS_PATH wayland-protocols
                             pkgdatadir)
            if(WAYLAND_PROTOCOLS_PATH)
              find_file(
                WAYLAND_LINUX_DMABUF_XML_PATH linux-dmabuf-unstable-v1.xml
                PATHS ${WAYLAND_PROTOCOLS_PATH}/unstable/linux-dmabuf
                NO_DEFAULT_PATH)
              find_file(WAYLAND_LINUX_XDG_SHELL_XML_PATH xdg-shell.xml
                        PATHS ${WAYLAND_PROTOCOLS_PATH}/stable/xdg-shell
                              NO_DEFAULT_PATH)
            endif()
          else()
            message(
              SEND_ERROR
                "wayland-protocols not found: set ENABLE_WAYLAND=OFF to build tools with Wayland disabled"
            )
          endif()

          if(WAYLAND_LINUX_DMABUF_XML_PATH)
            target_compile_definitions(${TARGET}
                                       PUBLIC WAYLAND_LINUX_DMABUF_SUPPORT)
            target_include_directories(
              ${TARGET}
              PUBLIC ${CMAKE_BINARY_DIR}/tools/legacy/sample_misc/wayland)
          endif()

          if(WAYLAND_LINUX_XDG_SHELL_XML_PATH)
            target_compile_definitions(${TARGET}
                                       PUBLIC WAYLAND_LINUX_XDG_SHELL_SUPPORT)
            target_include_directories(
              ${TARGET}
              PUBLIC ${CMAKE_BINARY_DIR}/tools/legacy/sample_misc/wayland)
          else()
            message(SEND_ERROR "xdg-shell.xml could not be found")
          endif()

        else()
          message(
            SEND_ERROR
              "wayland-client not found: set ENABLE_WAYLAND=OFF to build tools with Wayland disabled"
          )
        endif()
      else()
        message(STATUS "Building ${TARGET} without Wayland support")
      endif()

      if(ENABLE_X11)
        if(PKG_X11_FOUND)
          target_compile_definitions(${TARGET} PUBLIC LIBVA_X11_SUPPORT)
          target_link_libraries(${TARGET} PUBLIC PkgConfig::PKG_X11)

          if(ENABLE_DRI3)
            if(PKG_XCB_FOUND AND PKG_LIBDRM_FOUND)
              target_compile_definitions(${TARGET} PUBLIC X11_DRI3_SUPPORT)
              target_link_libraries(${TARGET} PUBLIC PkgConfig::PKG_XCB)
            else()
              message(
                SEND_ERROR
                  "DRI3 dependencies not found: set ENABLE_DRI3=OFF to to build tools with DRI3 disabled"
              )
            endif()
          else()
            message(STATUS "Building ${TARGET} without DRI3 support")
          endif()
        else()
          message(
            SEND_ERROR
              "X11 modules not found: set ENABLE_X11=OFF to build tools with X11 disabled"
          )
        endif()
      else()
        message(STATUS "Building ${TARGET} without X11 support")
      endif()
    else()
      message(
        SEND_ERROR
          "libva not found: set ENABLE_VA=OFF to build tools with VA disabled")
    endif()
  endif()

  target_link_libraries(${TARGET} PUBLIC ${CMAKE_DL_LIBS})

  set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
  set(THREADS_PREFER_PTHREAD_FLAG TRUE)
  find_package(Threads REQUIRED)
  target_link_libraries(${TARGET} PUBLIC Threads::Threads)
else()
  target_compile_definitions(${TARGET} PUBLIC MFX_D3D11_SUPPORT NOMINMAX)
  target_link_libraries(${TARGET} PUBLIC DXGI D3D11 D3D9 DXVA2)
endif()
