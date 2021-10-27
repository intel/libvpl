/*############################################################################
  # Copyright (C) 2005 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "listener_wayland.h"
#include <iostream>
#include "class_wayland.h"

/* drm listener */
void drm_handle_device(void* data, struct wl_drm* drm, const char* device) {
    Wayland* wayland = static_cast<Wayland*>(data);
    wayland->DrmHandleDevice(device);
}

void drm_handle_format(void* data, struct wl_drm* drm, uint32_t format) {
    /* NOT IMPLEMENTED */
}

void drm_handle_capabilities(void* data, struct wl_drm* drm, uint32_t value) {
    /* NOT IMPLEMENTED */
}

void drm_handle_authenticated(void* data, struct wl_drm* drm) {
    Wayland* wayland = static_cast<Wayland*>(data);
    wayland->DrmHandleAuthenticated();
}

/* registry listener */
void registry_handle_global(void* data,
                            struct wl_registry* registry,
                            uint32_t name,
                            const char* interface,
                            uint32_t version) {
    Wayland* wayland = static_cast<Wayland*>(data);
    wayland->RegistryGlobal(registry, name, interface, version);
}

void remove_registry_global(void* data, struct wl_registry* registy, uint32_t name) {
    /* NOT IMPLEMENTED */
}

/* surface listener */
void shell_surface_ping(void* data, struct wl_shell_surface* shell_surface, uint32_t serial) {
    wl_shell_surface_pong(shell_surface, serial);
}

void shell_surface_configure(void* data,
                             struct wl_shell_surface* shell_surface,
                             uint32_t edges,
                             int32_t width,
                             int32_t height) {
    /* NOT IMPLEMENTED */
}

void handle_done(void* data, struct wl_callback* callback, uint32_t time) {
    Wayland* wayland = static_cast<Wayland*>(data);
    wayland->DestroyCallback();
}

void buffer_release(void* data, struct wl_buffer* buffer) {
    Wayland* wayland = static_cast<Wayland*>(data);
    wayland->RemoveBufferFromList(buffer);
    wl_buffer_destroy(buffer);
    buffer = NULL;
}
