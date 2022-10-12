/*############################################################################
  # Copyright (C) 2005 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef WAYLAND_CLASS_H
#define WAYLAND_CLASS_H

#if defined(LIBVA_WAYLAND_SUPPORT)

extern "C" {
    #include <intel_bufmgr.h>
}
    #include <poll.h>
    #include <wayland-client.h>
    #include <list>
    #include "mfx_buffering.h"
    #include "sample_defs.h"
    #include "vpl/mfxstructures.h"
    #include "wayland-drm-client-protocol.h"
    #if defined(WAYLAND_LINUX_DMABUF_SUPPORT)
        #include "linux-dmabuf-unstable-v1.h"
    #endif
    #if defined(WAYLAND_LINUX_XDG_SHELL_SUPPORT)
        #include "xdg-shell-client-protocol.h"
    #endif

typedef struct buffer wld_buffer;

/* ShmPool Struct */
struct ShmPool {
    int fd;
    uint32_t* memory;
    unsigned capacity;
    unsigned size;
};

class Wayland : public CBuffering {
public:
    Wayland();
    virtual ~Wayland();
    virtual bool InitDisplay();
    virtual bool CreateSurface();
    virtual void FreeSurface();
    virtual void SetRenderWinPos(int x, int y);
    virtual void RenderBuffer(struct wl_buffer* buffer, mfxFrameSurface1* surface);
    virtual void RenderBufferWinPosSize(struct wl_buffer* buffer,
                                        int x,
                                        int y,
                                        int32_t width,
                                        int32_t height);
    bool CreateShmPool(int fd, int32_t size, int prot);
    void FreeShmPool();
    /* Create wl_buffer from shm_pool */
    struct wl_buffer* CreateShmBuffer(unsigned width,
                                      unsigned height,
                                      unsigned stride,
                                      uint32_t PIXEL_FORMAT_ID);
    void FreeShmBuffer(struct wl_buffer* buffer);
    int Dispatch();
    struct wl_buffer* CreatePlanarBuffer(uint32_t name,
                                         int32_t width,
                                         int32_t height,
                                         uint32_t format,
                                         int32_t offsets[3],
                                         int32_t pitches[3]);
    virtual struct wl_buffer* CreatePrimeBuffer(uint32_t name,
                                                int32_t width,
                                                int32_t height,
                                                uint32_t format,
                                                int32_t offsets[3],
                                                int32_t pitches[3]);
    struct wl_display* GetDisplay() {
        return m_display;
    }
    struct wl_registry* GetRegistry() {
        return m_registry;
    }
    struct wl_compositor* GetCompositor() {
        return m_compositor;
    }
    struct wl_shell* GetShell() {
        return m_shell;
    }
    struct wl_drm* GetDrm() {
        return m_drm;
    }
    #if defined(WAYLAND_LINUX_DMABUF_SUPPORT)
    struct zwp_linux_dmabuf_v1* GetDMABuf() {
        return m_dmabuf;
    }
    #endif
    struct wl_shm* GetShm() {
        return m_shm;
    };
    struct wl_shm_pool* GetShmPool() {
        return m_pool;
    }
    struct wl_surface* GetSurface() {
        return m_surface;
    }
    struct wl_shell_surface* GetShellSurface() {
        return m_shell_surface;
    }
    struct wl_callback* GetCallback() {
        return m_callback;
    }
    void SetCompositor(struct wl_compositor* compositor) {
        m_compositor = compositor;
    }
    void SetShell(struct wl_shell* shell) {
        m_shell = shell;
    }
    void SetShm(struct wl_shm* shm) {
        m_shm = shm;
    }
    void SetDrm(struct wl_drm* drm) {
        m_drm = drm;
    }
    #if defined(WAYLAND_LINUX_DMABUF_SUPPORT)
    void SetDMABuf(struct zwp_linux_dmabuf_v1* dmabuf) {
        m_dmabuf = dmabuf;
    }
    #endif
    void DrmHandleDevice(const char* device);
    void DrmHandleAuthenticated();
    void RegistryGlobal(struct wl_registry* registry,
                        uint32_t name,
                        const char* interface,
                        uint32_t version);
    void DisplayFlush() {
        wl_display_flush(m_display);
    }
    virtual int DisplayRoundtrip();
    void DestroyCallback();
    virtual void Sync();
    virtual void SetPerfMode(bool perf_mode);
    void AddBufferToList(wld_buffer* buffer);
    void RemoveBufferFromList(struct wl_buffer* buffer);
    void DestroyBufferList();

private:
    //no copies allowed
    Wayland(const Wayland&);
    void operator=(const Wayland&);

    struct wl_display* m_display;
    struct wl_registry* m_registry;
    struct wl_compositor* m_compositor;
    struct wl_shell* m_shell;
    struct wl_drm* m_drm;
    #if defined(WAYLAND_LINUX_DMABUF_SUPPORT)
    struct zwp_linux_dmabuf_v1* m_dmabuf;
    #endif
    struct wl_shm* m_shm;
    struct wl_shm_pool* m_pool;
    struct wl_surface* m_surface;
    struct wl_shell_surface* m_shell_surface;
    struct wl_callback* m_callback;
    struct wl_event_queue* m_event_queue;
    volatile int m_pending_frame;
    struct ShmPool* m_shm_pool;
    #if defined(WAYLAND_LINUX_XDG_SHELL_SUPPORT)
    struct xdg_wm_base* m_xdg_wm_base;
    struct xdg_surface* m_xdg_surface;
    struct xdg_toplevel* m_xdg_toplevel;
    #endif
    int m_display_fd;
    int m_fd;
    struct pollfd m_poll;
    drm_intel_bufmgr* m_bufmgr;
    char* m_device_name;
    int m_x, m_y;
    bool m_perf_mode;

protected:
    std::list<wld_buffer*> m_buffers_list;
};

extern "C" Wayland* WaylandCreate();
extern "C" void WaylandDestroy(Wayland* pWld);

#endif //LIBVA_WAYLAND_SUPPORT

#endif /* WAYLAND_CLASS_H */
