/*############################################################################
  # Copyright (C) 2005 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef LISTENER_WAYLAND_H
#define LISTENER_WAYLAND_H

#include <stdint.h>
#include "class_wayland.h"

/* drm listeners */
void drm_handle_device(void* data, struct wl_drm* drm, const char* device);

void drm_handle_format(void* data, struct wl_drm* drm, uint32_t format);

void drm_handle_authenticated(void* data, struct wl_drm* drm);

void drm_handle_capabilities(void* data, struct wl_drm* drm, uint32_t value);

/* registry listeners */
void registry_handle_global(void* data,
                            struct wl_registry* registry,
                            uint32_t name,
                            const char* interface,
                            uint32_t version);

void remove_registry_global(void* data, struct wl_registry* regsitry, uint32_t name);

/* surface listener */
void shell_surface_ping(void* data, struct wl_shell_surface* shell_surface, uint32_t serial);

void shell_surface_configure(void* data,
                             struct wl_shell_surface* shell_surface,
                             uint32_t edges,
                             int32_t width,
                             int32_t height);

void handle_done(void* data, struct wl_callback* callback, uint32_t time);

void buffer_release(void* data, struct wl_buffer* buffer);

#if defined(WAYLAND_LINUX_XDG_SHELL_SUPPORT)
/* xdg shell */
void xdg_wm_base_ping(void* data, struct xdg_wm_base* xdg_wm_base, uint32_t serial);

void xdg_surface_configure(void* data, struct xdg_surface* xdg_surface, uint32_t serial);

void xdg_toplevel_configure(void* data,
                            struct xdg_toplevel* xdg_toplevel,
                            int32_t width,
                            int32_t height,
                            struct wl_array* states);

void xdg_toplevel_close(void* data, struct xdg_toplevel* xdg_toplevel);
#endif

#endif /* LISTENER_WAYLAND_H */
