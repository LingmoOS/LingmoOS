// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/wayland/wayland_dmabuf_feedback_manager.h
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: gpu::Capabilities becomes render::DmaBufDevice, the
// format table lives in a sealed memfd instead of a
// base::ReadOnlySharedMemoryRegion, and the scanout tranche comes from a
// primary-plane format provider the shell installs rather than the display
// manager. Overlay priority hints do not exist here; fullscreen is the only
// scanout reason.

#ifndef LINGMO_WAYLAND_PROTOCOLS_DMABUF_FEEDBACK_MANAGER_H_
#define LINGMO_WAYLAND_PROTOCOLS_DMABUF_FEEDBACK_MANAGER_H_

#include <sys/types.h>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <vector>

#include "compositor/surface_observer.h"
#include "core/scoped_fd.h"

struct wl_client;
struct wl_resource;

namespace Lingmo::Compositor {
class Compositor;
class Surface;
}

namespace Lingmo::Render {
class DmaBufDevice;
}

namespace Lingmo::Wayland {

class DmaBufFeedback;
class DmaBufFeedbackTranche;
class DmaBufSurfaceFeedback;
class DmaBufSurfaceFeedbackRef;

// DRM fourcc -> (index into the format table -> modifier). The indices are what
// tranches actually carry over the wire.
using IndexedDrmFormatsAndModifiers = std::map<uint32_t, std::map<size_t, uint64_t>>;

// DRM fourcc -> the modifiers the primary planes accept for it. An empty
// modifier list means the plane only knows the implicit modifier.
using ScanoutFormats = std::map<uint32_t, std::vector<uint64_t>>;

// Owns everything the linux-dmabuf global needs: which formats and modifiers
// are supported, the shared format table, the default feedback and the
// per-surface feedbacks that gain a scanout tranche while their surface is a
// scanout candidate.
class DmaBufFeedbackManager : public compositor::SurfaceObserver
{
public:
    DmaBufFeedbackManager(compositor::Compositor* compositor, render::DmaBufDevice* device);
    DmaBufFeedbackManager(const DmaBufFeedbackManager&) = delete;
    DmaBufFeedbackManager& operator=(const DmaBufFeedbackManager&) = delete;
    ~DmaBufFeedbackManager() override;

    compositor::Compositor* compositor() const { return m_compositor; }

    // The highest zwp_linux_dmabuf_v1 version this platform can honour: 4 when
    // the main device is known and a format table could be built, 3 otherwise.
    uint32_t versionSupportedByPlatform() const { return m_version; }

    bool isFormatSupported(uint32_t format) const;

    // The version < 4 way of advertising capabilities.
    void sendFormatsAndModifiers(wl_resource* resource) const;

    void getDefaultFeedback(wl_client* client, wl_resource* dmaBufResource, uint32_t feedbackId);
    void getSurfaceFeedback(wl_client* client, wl_resource* dmaBufResource, uint32_t feedbackId,
                            wl_resource* surfaceResource);

    const IndexedDrmFormatsAndModifiers& formatsAndModifiers() const
    {
        return m_formatsAndModifiers;
    }

    // What the outputs' primary planes can scan out. Absent, or returning
    // nothing, no scanout tranche is ever sent.
    void setScanoutFormats(std::function<ScanoutFormats()> provider);

    // A candidate's surface feedback carries a scanout tranche restricted to
    // the primary planes' formats, so a client renders straight into a buffer
    // the compositor can flip. The shell marks fullscreen windows' root
    // surfaces. Every change is re-sent to the surface's feedback objects.
    void setScanoutCandidate(compositor::Surface* surface, bool candidate);
    bool isScanoutCandidate(const compositor::Surface* surface) const;

    // Rebuilds the candidates' scanout tranches, after the outputs changed.
    void refreshScanoutTranches();

    // Live per-surface feedback objects, for tests.
    size_t surfaceFeedbackCount() const { return m_surfaceFeedbacks.size(); }

    // compositor::SurfaceObserver, on the candidate surfaces: a dead candidate
    // must not be found again by a surface reusing its address.
    void onSurfaceDestroying(compositor::Surface* surface) override;

private:
    friend class DmaBufSurfaceFeedback;
    friend class DmaBufSurfaceFeedbackRef;

    void sendFeedback(const DmaBufFeedback* feedback, wl_resource* resource) const;
    void sendTranche(const DmaBufFeedbackTranche* tranche, wl_resource* resource) const;
    void sendSurfaceFeedback(DmaBufSurfaceFeedback* surfaceFeedback) const;
    void removeSurfaceFeedback(compositor::Surface* surface);
    // The scanout tranche `surface` would get now, or null when none of the
    // default tranche can be scanned out.
    std::unique_ptr<DmaBufFeedbackTranche> makeScanoutTranche() const;

    compositor::Compositor* const m_compositor;
    uint32_t m_version = 0;
    IndexedDrmFormatsAndModifiers m_formatsAndModifiers;
    ScopedFd m_formatTableFd;
    size_t m_formatTableSize = 0;
    std::unique_ptr<DmaBufFeedback> m_defaultFeedback;
    std::function<ScanoutFormats()> m_scanoutFormats;
    std::set<compositor::Surface*> m_scanoutCandidates;
    std::map<compositor::Surface*, std::unique_ptr<DmaBufSurfaceFeedback>> m_surfaceFeedbacks;
};

} // namespace Lingmo::Wayland

#endif // LINGMO_WAYLAND_PROTOCOLS_DMABUF_FEEDBACK_MANAGER_H_
