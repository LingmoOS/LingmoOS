// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LINGMO_RECORDER_CAPTURE_CLIENT_H
#define LINGMO_RECORDER_CAPTURE_CLIENT_H

#include "base/geometry.h"

#include <cstdint>
#include <string>

#include <memory>
#include <vector>

struct wl_display;
struct wl_output;
struct wl_registry;
struct wl_shm;
struct zwp_linux_dmabuf_v1;
struct zxdg_output_manager_v1;
struct zxdg_output_v1;
struct ext_output_image_capture_source_manager_v1;
struct ext_foreign_toplevel_image_capture_source_manager_v1;
struct ext_image_copy_capture_manager_v1;
struct ext_image_capture_source_v1;
struct ext_foreign_toplevel_list_v1;
struct ext_foreign_toplevel_handle_v1;

// The compositor's capture globals on the recorder's connection.
class CaptureClient
{
public:
    struct Output {
        wl_output *output = nullptr;
        zxdg_output_v1 *xdgOutput = nullptr;
        uint32_t globalName = 0;
        //! The wl_output name, e.g. eDP-1.
        std::string name;
        //! Where the output is in the compositor's global logical space.
        Lingmo::Base::Rect logicalGeometry;
    };

    explicit CaptureClient(wl_display *display);
    ~CaptureClient();

    CaptureClient(const CaptureClient &) = delete;
    CaptureClient &operator=(const CaptureClient &) = delete;

    //! Binds the globals and learns the outputs and windows; blocks for the
    //! round trips that takes. False when this client may not capture.
    bool initialize();

    bool canCaptureOutputs() const;
    bool canCaptureWindows() const;

    wl_display *display() const { return m_display; }
    wl_shm *shm() const { return m_shm; }
    ext_image_copy_capture_manager_v1 *copyManager() const { return m_copyManager; }
    //! Null when frames can only go into shared memory.
    zwp_linux_dmabuf_v1 *dmabuf() const { return m_dmabuf; }

    const Output *outputNamed(const std::string &name) const;

    //! New sources, owned by the caller; null when there is nothing to capture.
    ext_image_capture_source_v1 *createOutputSource(const std::string &outputName);
    ext_image_capture_source_v1 *createWindowSource(const std::string &identifier);

    void flush();

private:
    friend struct CaptureClientListeners;

    struct Toplevel {
        ext_foreign_toplevel_handle_v1 *handle = nullptr;
        std::string identifier;
    };

    void roundtrip();
    void dropClosedToplevels();

    wl_display *m_display = nullptr;
    wl_registry *m_registry = nullptr;
    wl_shm *m_shm = nullptr;
    zxdg_output_manager_v1 *m_xdgOutputManager = nullptr;
    ext_output_image_capture_source_manager_v1 *m_outputSources = nullptr;
    ext_foreign_toplevel_image_capture_source_manager_v1 *m_toplevelSources = nullptr;
    ext_image_copy_capture_manager_v1 *m_copyManager = nullptr;
    zwp_linux_dmabuf_v1 *m_dmabuf = nullptr;
    ext_foreign_toplevel_list_v1 *m_toplevelList = nullptr;
    std::vector<std::unique_ptr<Output>> m_outputs;
    std::vector<std::unique_ptr<Toplevel>> m_toplevels;
};

#endif // LINGMO_RECORDER_CAPTURE_CLIENT_H
