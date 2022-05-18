# ##############################################################################
# Copyright (C) 2005 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################

add_library(sample_common STATIC)

target_sources(
  sample_common
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
  sample_common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
                       ${CMAKE_CURRENT_SOURCE_DIR}/include/vm)

if(MSVC)
  target_compile_definitions(sample_common PUBLIC _CRT_SECURE_NO_WARNINGS)
endif()

target_compile_definitions(sample_common PUBLIC MFX_DEPRECATED_OFF)

find_package(VPL REQUIRED)
target_link_libraries(sample_common PUBLIC VPL::dispatcher
                                           media_sdk_compatibility_headers)

if(BUILD_TOOLS_ONEVPL_EXPERIMENTAL)
  target_compile_definitions(sample_common PUBLIC -DONEVPL_EXPERIMENTAL)
endif()

if(CMAKE_SYSTEM_NAME MATCHES Linux)
  target_compile_definitions(sample_common PUBLIC LINUX64)

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
    target_compile_definitions(sample_common PUBLIC LIBVA_SUPPORT)
    target_include_directories(sample_common PUBLIC ${PKG_LIBVA_INCLUDE_DIRS})

    if(PKG_LIBDRM_FOUND)
      target_compile_definitions(sample_common PUBLIC LIBVA_DRM_SUPPORT)
      target_include_directories(sample_common
                                 PUBLIC ${PKG_LIBDRM_INCLUDE_DIRS})

      if(PKG_WAYLAND_CLIENT_FOUND)
        target_compile_definitions(sample_common PUBLIC LIBVA_WAYLAND_SUPPORT)
        target_include_directories(
          sample_common
          PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../sample_misc/wayland/include)

        find_program(WAYLAND_SCANNER_BIN "wayland-scanner")
        pkg_check_modules(PKG_WAYLAND_PROTCOLS "wayland-protocols>=1.15")

        if(WAYLAND_SCANNER_BIN AND PKG_WAYLAND_PROTCOLS_FOUND)
          pkg_get_variable(WAYLAND_PROTOCOLS_PATH wayland-protocols pkgdatadir)
          if(WAYLAND_PROTOCOLS_PATH)
            find_file(
              WAYLAND_LINUX_DMABUF_XML_PATH linux-dmabuf-unstable-v1.xml
              PATHS ${WAYLAND_PROTOCOLS_PATH}/unstable/linux-dmabuf
              NO_DEFAULT_PATH)
          endif()
        endif()

        if(WAYLAND_LINUX_DMABUF_XML_PATH)
          target_compile_definitions(sample_common
                                     PUBLIC WAYLAND_LINUX_DMABUF_SUPPORT)
          target_include_directories(
            sample_common
            PUBLIC ${CMAKE_BINARY_DIR}/tools/legacy/sample_misc/wayland)
        endif()

      else()
        message(
          WARNING
            "Wayland modules not found: building sample-common without Wayland support"
        )
      endif()
    else()
      message(
        WARNING
          "libva-drm not found: building sample-common without libVA DRM support"
      )
    endif()

    if(PKG_X11_FOUND)
      target_compile_definitions(sample_common PUBLIC LIBVA_X11_SUPPORT)
      target_link_libraries(sample_common PUBLIC PkgConfig::PKG_X11)

      if(PKG_XCB_FOUND)
        if(PKG_LIBDRM_FOUND)
          target_compile_definitions(sample_common PUBLIC X11_DRI3_SUPPORT)
          target_link_libraries(sample_common PUBLIC PkgConfig::PKG_XCB)
        else()
          message(
            WARNING
              "libdrm not found: building sample-common without DRI3 support")
        endif()
      else()
        message(
          WARNING "xcb not found: building sample-common without DRI3 support")
      endif()
    else()
      message(
        WARNING
          "x11 modules not found: building sample-common without libVA X11 support"
      )
    endif()
  else()
    message(
      WARNING "libva not found: building sample* tools without hardware support"
    )
  endif()

  target_link_libraries(sample_common PUBLIC ${CMAKE_DL_LIBS})

  set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
  set(THREADS_PREFER_PTHREAD_FLAG TRUE)
  find_package(Threads REQUIRED)
  target_link_libraries(sample_common PUBLIC Threads::Threads)
else()
  target_compile_definitions(sample_common PUBLIC MFX_D3D11_SUPPORT NOMINMAX)
  target_link_libraries(sample_common PUBLIC DXGI D3D11 D3D9 DXVA2)
endif()
