# ##############################################################################
# Copyright (C) 2005 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################

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

  find_package(PkgConfig REQUIRED)
  # note: pkg-config version for libva is *API* version
  pkg_check_modules(PKG_LIBVA QUIET libva>=1.2 IMPORTED_TARGET GLOBAL)
  pkg_check_modules(PKG_LIBVA_DRM QUIET libva-drm>=1.2 IMPORTED_TARGET GLOBAL)
  pkg_check_modules(PKG_LIBDRM QUIET libdrm>=2.4.91 IMPORTED_TARGET GLOBAL)
  pkg_check_modules(PKG_X11 QUIET x11 IMPORTED_TARGET GLOBAL)
  pkg_check_modules(PKG_LIBVA_X11 QUIET libva-x11>=1.10.0 IMPORTED_TARGET
                    GLOBAL)
  pkg_check_modules(
    PKG_XCB
    QUIET
    xcb
    xcb-dri3
    x11-xcb
    xcb-present
    IMPORTED_TARGET
    GLOBAL)
  pkg_check_modules(PKG_WAYLAND_CLIENT QUIET wayland-client IMPORTED_TARGET
                    GLOBAL)
  pkg_check_modules(PKG_LIBWAYLAND QUIET libwayland IMPORTED_TARGET GLOBAL)

  if(PKG_LIBVA_FOUND AND PKG_LIBVA_DRM_FOUND)
    target_compile_definitions(${TARGET} PUBLIC LIBVA_SUPPORT)
    target_include_directories(${TARGET} PUBLIC ${PKG_LIBVA_INCLUDE_DIRS})

    if(PKG_LIBDRM_FOUND)
      target_compile_definitions(${TARGET} PUBLIC LIBVA_DRM_SUPPORT)
      target_include_directories(${TARGET} PUBLIC ${PKG_LIBDRM_INCLUDE_DIRS})
      if(ENABLE_WAYLAND)
        if(PKG_WAYLAND_CLIENT_FOUND)
          target_compile_definitions(${TARGET} PUBLIC LIBVA_WAYLAND_SUPPORT)
          target_include_directories(
            ${TARGET}
            PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../sample_misc/wayland/include)

          find_program(WAYLAND_SCANNER_BIN "wayland-scanner")
          pkg_check_modules(PKG_WAYLAND_PROTCOLS "wayland-protocols>=1.15")

          if(WAYLAND_SCANNER_BIN AND PKG_WAYLAND_PROTCOLS_FOUND)
            pkg_get_variable(WAYLAND_PROTOCOLS_PATH wayland-protocols
                             pkgdatadir)
            if(WAYLAND_PROTOCOLS_PATH)
              find_file(
                WAYLAND_LINUX_DMABUF_XML_PATH linux-dmabuf-unstable-v1.xml
                PATHS ${WAYLAND_PROTOCOLS_PATH}/unstable/linux-dmabuf
                NO_DEFAULT_PATH)
            endif()
          else()
            message(
              SEND_ERROR
                "wayland-protocols not found: set ENABLE_WAYLAND=OFF to build tools with wayland disabled"
            )
          endif()

          if(WAYLAND_LINUX_DMABUF_XML_PATH)
            target_compile_definitions(${TARGET}
                                       PUBLIC WAYLAND_LINUX_DMABUF_SUPPORT)
            target_include_directories(
              ${TARGET}
              PUBLIC ${CMAKE_BINARY_DIR}/tools/legacy/sample_misc/wayland)
          endif()

        else()
          message(
            SEND_ERROR
              "Wayland modules not found: set ENABLE_WAYLAND=OFF to build tools with wayland disabled"
          )
        endif()
      else()
        message(STATUS "Building ${TARGET} without Wayland support")
      endif()
    else()
      message(
        SEND_ERROR
          "libdrm not found: set BUILD_TOOLS=OFF to skip building tools")
    endif()
    if(ENABLE_X11)
      if(PKG_X11_FOUND)
        target_compile_definitions(${TARGET} PUBLIC LIBVA_X11_SUPPORT)
        target_link_libraries(${TARGET} PUBLIC PkgConfig::PKG_X11)

        if(PKG_XCB_FOUND)
          if(PKG_LIBDRM_FOUND)
            target_compile_definitions(${TARGET} PUBLIC X11_DRI3_SUPPORT)
            target_link_libraries(${TARGET} PUBLIC PkgConfig::PKG_XCB)
          else()
            message(
              SEND_ERROR
                "libdrm not found: set BUILD_TOOLS=OFF to skip building tools")
          endif()
        else()
          message(
            SEND_ERROR
              "xcb not found: set ENABLE_X11=OFF to to build tools with X11 disabled"
          )
        endif()
      else()
        message(
          SEND_ERROR
            "X11 modules not found: set ENABLE_X11=OFF to to build tools with X11 disabled"
        )
      endif()
    else()
      message(STATUS "Building ${TARGET} without X11 support")
    endif()
  else()
    message(
      SEND_ERROR "libva not found: set BUILD_TOOLS=OFF to skip building tools")
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
