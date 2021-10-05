if(NOT PKG_WAYLAND_CLIENT_FOUND)
  return()
endif()

set(TARGET vpl_wayland)

find_package(PkgConfig REQUIRED)
pkg_check_modules(PKG_WAYLAND_CLIENT wayland-client)
pkg_check_modules(PKG_LIBVA libva libva-drm)
pkg_check_modules(PKG_LIBDRM libdrm)

if(NOT PKG_LIBDRM_FOUND)
  message(WARNING "No libdrm, cannot build vpl_wayland")
  return()
endif()

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../../sample_common/include
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../../api/vpl
  ${PKG_LIBDRM_INCLUDE_DIRS}
  ${PKG_WAYLAND_CLIENT_INCLUDE_DIRS})

add_definitions(-DLIBVA_WAYLAND_SUPPORT)
add_library(${TARGET} SHARED "")
target_sources(
  ${TARGET}
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/class_wayland.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/src/listener_wayland.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/src/wayland-drm-protocol.c
          ${CMAKE_CURRENT_SOURCE_DIR}/src/linux-dmabuf-unstable-v1.c)

target_link_libraries(${TARGET} sample_common wayland-client va drm drm_intel)

install(TARGETS ${TARGET} LIBRARY DESTINATION ${_TOOLS_LIB_PATH})
