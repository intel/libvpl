/*
 * Copyright © 2008-2011 Kristian Høgsberg
 * Copyright © 2010-2011 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdint.h>
#include <stdlib.h>
#include "wayland-util.h"

extern const struct wl_interface wl_buffer_interface;

static const struct wl_interface* types[] = {
    NULL,
    &wl_buffer_interface,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    &wl_buffer_interface,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    &wl_buffer_interface,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
};

static const struct wl_message wl_drm_requests[] = {
    { "authenticate", "u", types + 0 },
    { "create_buffer", "nuiiuu", types + 1 },
    { "create_planar_buffer", "nuiiuiiiiii", types + 7 },
    { "create_prime_buffer", "2nhiiuiiiiii", types + 18 },
};

static const struct wl_message wl_drm_events[] = {
    { "device", "s", types + 0 },
    { "format", "u", types + 0 },
    { "authenticated", "", types + 0 },
    { "capabilities", "u", types + 0 },
};

WL_EXPORT const struct wl_interface wl_drm_interface = {
    "wl_drm", 2, 4, wl_drm_requests, 4, wl_drm_events,
};
