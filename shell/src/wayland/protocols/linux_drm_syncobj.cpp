// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/buffer.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: maps Exo's release dependency lifecycle onto explicit
// Wayland DRM syncobj timeline points.

#include "wayland/protocols/linux_drm_syncobj.h"

#include <linux-drm-syncobj-v1-server-protocol.h>

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "compositor/buffer.h"
#include "compositor/surface.h"
#include "compositor/surface_observer.h"
#include "core/logging.h"
#include "core/scoped_fd.h"
#include "render/drm_syncobj.h"
#include "wayland/server_util.h"

namespace Lingmo::Wayland {

namespace {

////////////////////////////////////////////////////////////////////////////////
// wp_linux_drm_syncobj_timeline_v1:

struct Timeline {
    std::shared_ptr<render::DrmSyncobj> syncobj;
};

void timeline_destroy(wl_client*, wl_resource* resource)
{
    wl_resource_destroy(resource);
}

const struct wp_linux_drm_syncobj_timeline_v1_interface timeline_implementation = {
    timeline_destroy,
};

////////////////////////////////////////////////////////////////////////////////
// wp_linux_drm_syncobj_surface_v1:

// One release point the compositor owes a client.
//
// Upstream: chromium/src/components/exo/buffer.cc, whose release callback plays
// the same role for gpu::SyncToken.
class TimelineRelease final : public compositor::BufferRelease
{
public:
    TimelineRelease(std::shared_ptr<render::DrmSyncobj> syncobj, uint64_t point)
        : m_syncobj(std::move(syncobj))
        , m_point(point)
    {
    }

    ~TimelineRelease() override
    {
        // Dropping a release without signalling it hangs the client, so the
        // destructor is the last line of defence.
        signal(render::SyncFence());
    }

    void signal(render::SyncFence fence) override
    {
        if (m_signaled)
            return;
        m_signaled = true;
        if (m_syncobj && !m_syncobj->signal(m_point, fence)) {
            LM_WARNING(lingmoWayland) << "Failed to signal a DRM syncobj release point";
        }
    }

    const void* timeline() const override { return m_syncobj.get(); }

    uint64_t timelinePoint() const override { return m_point; }

private:
    std::shared_ptr<render::DrmSyncobj> m_syncobj;
    uint64_t m_point = 0;
    bool m_signaled = false;
};

// The per-surface half of the protocol. It observes the surface so that a
// destroyed wl_surface turns later requests into the no_surface error rather
// than a use-after-free, exactly as the protocol requires.
class SyncobjSurface final : public compositor::SurfaceObserver
{
public:
    SyncobjSurface(wl_resource* resource, compositor::Surface* surface);
    ~SyncobjSurface() override;

    void onSurfaceDestroying(compositor::Surface* surface) override
    {
        if (surface != m_surface)
            return;
        surface->removeSurfaceObserver(this);
        m_surface = nullptr;
        // Or a new surface allocated at the same address would read as one
        // that already has a synchronization object.
        unregister();
    }

    // Checked when the client commits, before the commit waits for its point.
    std::function<void()> onSurfaceCommitRequested(compositor::Surface* surface) override
    {
        if (surface == m_surface)
            validateCommit();
        return {};
    }

    compositor::Surface* surface() const { return m_surface; }

    // Leaves the side table that answers surface_exists.
    void unregister();

    void setAcquirePoint(const std::shared_ptr<render::DrmSyncobj>& syncobj, uint64_t point)
    {
        m_acquireSyncobj = syncobj;
        m_acquirePoint = point;
        m_hasAcquire = true;
        if (m_surface)
            m_surface->setAcquirePoint(syncobj, point);
    }

    void setReleasePoint(const std::shared_ptr<render::DrmSyncobj>& syncobj, uint64_t point)
    {
        m_releaseSyncobj = syncobj;
        m_releasePoint = point;
        m_hasRelease = true;
        if (m_surface)
            m_surface->setBufferRelease(std::make_shared<TimelineRelease>(syncobj, point));
    }

private:
    void validateCommit()
    {
        SyncobjCommit commit;
        commit.buffer = m_surface && m_surface->lastCommitAttachedBuffer();
        commit.explicitSyncBuffer = commit.buffer && m_surface->lastCommitBufferSupportsExplicitSync();
        commit.acquire = m_hasAcquire;
        commit.release = m_hasRelease;
        commit.conflicting = m_hasAcquire && m_hasRelease && m_acquireSyncobj == m_releaseSyncobj
                && m_acquirePoint >= m_releasePoint;
        switch (syncobjCommitError(commit)) {
        case WP_LINUX_DRM_SYNCOBJ_SURFACE_V1_ERROR_NO_BUFFER:
            wl_resource_post_error(m_resource, WP_LINUX_DRM_SYNCOBJ_SURFACE_V1_ERROR_NO_BUFFER,
                                   "a timeline point was set without a buffer");
            break;
        case WP_LINUX_DRM_SYNCOBJ_SURFACE_V1_ERROR_UNSUPPORTED_BUFFER:
            wl_resource_post_error(m_resource,
                                   WP_LINUX_DRM_SYNCOBJ_SURFACE_V1_ERROR_UNSUPPORTED_BUFFER,
                                   "the buffer cannot be synchronized explicitly");
            break;
        case WP_LINUX_DRM_SYNCOBJ_SURFACE_V1_ERROR_NO_ACQUIRE_POINT:
            wl_resource_post_error(m_resource,
                                   WP_LINUX_DRM_SYNCOBJ_SURFACE_V1_ERROR_NO_ACQUIRE_POINT,
                                   "a buffer was committed without an acquire point");
            break;
        case WP_LINUX_DRM_SYNCOBJ_SURFACE_V1_ERROR_NO_RELEASE_POINT:
            wl_resource_post_error(m_resource,
                                   WP_LINUX_DRM_SYNCOBJ_SURFACE_V1_ERROR_NO_RELEASE_POINT,
                                   "a buffer was committed without a release point");
            break;
        case WP_LINUX_DRM_SYNCOBJ_SURFACE_V1_ERROR_CONFLICTING_POINTS:
            wl_resource_post_error(m_resource,
                                   WP_LINUX_DRM_SYNCOBJ_SURFACE_V1_ERROR_CONFLICTING_POINTS,
                                   "the release point does not follow the acquire point on the "
                                   "same timeline");
            break;
        }

        m_hasAcquire = false;
        m_hasRelease = false;
        m_acquireSyncobj.reset();
        m_releaseSyncobj.reset();
    }

    wl_resource* m_resource = nullptr;
    compositor::Surface* m_surface = nullptr;
    // The surface this was registered under. m_surface is cleared when the
    // wl_surface dies, so unregistering needs the original pointer.
    compositor::Surface* m_registered = nullptr;
    std::shared_ptr<render::DrmSyncobj> m_acquireSyncobj;
    std::shared_ptr<render::DrmSyncobj> m_releaseSyncobj;
    uint64_t m_acquirePoint = 0;
    uint64_t m_releasePoint = 0;
    bool m_hasAcquire = false;
    bool m_hasRelease = false;
};

uint64_t combinePoint(uint32_t hi, uint32_t lo)
{
    return (static_cast<uint64_t>(hi) << 32) | lo;
}

void syncobj_surface_destroy(wl_client*, wl_resource* resource)
{
    wl_resource_destroy(resource);
}

// Returns false and posts no_surface when the wl_surface is already gone.
bool checkSurfaceAlive(wl_resource* resource, SyncobjSurface* syncobjSurface)
{
    if (syncobjSurface->surface())
        return true;
    wl_resource_post_error(resource, WP_LINUX_DRM_SYNCOBJ_SURFACE_V1_ERROR_NO_SURFACE,
                           "the associated wl_surface was destroyed");
    return false;
}

void syncobj_surface_set_acquire_point(wl_client*, wl_resource* resource, wl_resource* timeline,
                                       uint32_t pointHi, uint32_t pointLo)
{
    auto* syncobjSurface = getUserDataAs<SyncobjSurface>(resource);
    if (!checkSurfaceAlive(resource, syncobjSurface))
        return;
    syncobjSurface->setAcquirePoint(getUserDataAs<Timeline>(timeline)->syncobj,
                                    combinePoint(pointHi, pointLo));
}

void syncobj_surface_set_release_point(wl_client*, wl_resource* resource, wl_resource* timeline,
                                       uint32_t pointHi, uint32_t pointLo)
{
    auto* syncobjSurface = getUserDataAs<SyncobjSurface>(resource);
    if (!checkSurfaceAlive(resource, syncobjSurface))
        return;
    syncobjSurface->setReleasePoint(getUserDataAs<Timeline>(timeline)->syncobj,
                                    combinePoint(pointHi, pointLo));
}

const struct wp_linux_drm_syncobj_surface_v1_interface syncobj_surface_implementation = {
    syncobj_surface_destroy,
    syncobj_surface_set_acquire_point,
    syncobj_surface_set_release_point,
};

// Surfaces that already carry a syncobj extension, so a second get_surface can
// raise surface_exists rather than silently replacing the first. A side table
// for the same reason wl_compositor keeps one: the association belongs to the
// protocol layer, not to the compositor model.
std::vector<compositor::Surface*>& syncobjSurfaces()
{
    static std::vector<compositor::Surface*> surfaces;
    return surfaces;
}

SyncobjSurface::SyncobjSurface(wl_resource* resource, compositor::Surface* surface)
    : m_resource(resource)
    , m_surface(surface)
    , m_registered(surface)
{
    m_surface->addSurfaceObserver(this);
    syncobjSurfaces().push_back(surface);
}

SyncobjSurface::~SyncobjSurface()
{
    if (m_surface) {
        m_surface->removeSurfaceObserver(this);
        m_surface = nullptr;
    }
    unregister();
}

void SyncobjSurface::unregister()
{
    if (!m_registered)
        return;
    auto& surfaces = syncobjSurfaces();
    surfaces.erase(std::remove(surfaces.begin(), surfaces.end(), m_registered), surfaces.end());
    m_registered = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
// wp_linux_drm_syncobj_manager_v1:

void syncobj_manager_destroy(wl_client*, wl_resource* resource)
{
    wl_resource_destroy(resource);
}

void syncobj_manager_get_surface(wl_client* client, wl_resource* resource, uint32_t id,
                                 wl_resource* surfaceResource)
{
    compositor::Surface* surface = surfaceFromResource(surfaceResource);
    if (!surface)
        return;

    auto& existing = syncobjSurfaces();
    if (std::find(existing.begin(), existing.end(), surface) != existing.end()) {
        wl_resource_post_error(resource, WP_LINUX_DRM_SYNCOBJ_MANAGER_V1_ERROR_SURFACE_EXISTS,
                               "the surface already has a synchronization object");
        return;
    }

    wl_resource* syncobjResource =
        wl_resource_create(client, &wp_linux_drm_syncobj_surface_v1_interface,
                           wl_resource_get_version(resource), id);
    if (!syncobjResource) {
        wl_client_post_no_memory(client);
        return;
    }

    setImplementation(syncobjResource, &syncobj_surface_implementation,
                      std::make_unique<SyncobjSurface>(syncobjResource, surface));
}

void syncobj_manager_import_timeline(wl_client* client, wl_resource* resource, uint32_t id,
                                     int32_t fd)
{
    const int drmFd = static_cast<int>(reinterpret_cast<intptr_t>(
        wl_resource_get_user_data(resource)));
    std::shared_ptr<render::DrmSyncobj> syncobj =
        render::DrmSyncobj::import(drmFd, ScopedFd(fd));
    if (!syncobj) {
        wl_resource_post_error(resource, WP_LINUX_DRM_SYNCOBJ_MANAGER_V1_ERROR_INVALID_TIMELINE,
                               "the timeline could not be imported");
        return;
    }

    wl_resource* timelineResource =
        wl_resource_create(client, &wp_linux_drm_syncobj_timeline_v1_interface,
                           wl_resource_get_version(resource), id);
    if (!timelineResource) {
        wl_client_post_no_memory(client);
        return;
    }

    setImplementation(timelineResource, &timeline_implementation,
                      std::make_unique<Timeline>(Timeline {std::move(syncobj)}));
}

const struct wp_linux_drm_syncobj_manager_v1_interface syncobj_manager_implementation = {
    syncobj_manager_destroy,
    syncobj_manager_get_surface,
    syncobj_manager_import_timeline,
};

void bind_syncobj_manager(wl_client* client, void* data, uint32_t version, uint32_t id)
{
    wl_resource* resource =
        wl_resource_create(client, &wp_linux_drm_syncobj_manager_v1_interface,
                           static_cast<int>(version), id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &syncobj_manager_implementation, data, nullptr);
}

} // namespace

int syncobjCommitError(const SyncobjCommit& commit)
{
    if (!commit.buffer)
        return commit.acquire || commit.release ? WP_LINUX_DRM_SYNCOBJ_SURFACE_V1_ERROR_NO_BUFFER
                                                : -1;
    if (!commit.explicitSyncBuffer)
        return WP_LINUX_DRM_SYNCOBJ_SURFACE_V1_ERROR_UNSUPPORTED_BUFFER;
    if (!commit.acquire)
        return WP_LINUX_DRM_SYNCOBJ_SURFACE_V1_ERROR_NO_ACQUIRE_POINT;
    if (!commit.release)
        return WP_LINUX_DRM_SYNCOBJ_SURFACE_V1_ERROR_NO_RELEASE_POINT;
    if (commit.conflicting)
        return WP_LINUX_DRM_SYNCOBJ_SURFACE_V1_ERROR_CONFLICTING_POINTS;
    return -1;
}

wl_global* createLinuxDrmSyncobjGlobal(wl_display* display, int drmFd)
{
    if (!display || drmFd < 0)
        return nullptr;

    // A device that cannot make a timeline cannot serve this protocol at all.
    if (!render::DrmSyncobj::create(drmFd)) {
        LM_INFO(lingmoWayland) << "DRM syncobj timelines are unavailable; no explicit sync";
        return nullptr;
    }

    return wl_global_create(display, &wp_linux_drm_syncobj_manager_v1_interface,
                            kLinuxDrmSyncobjVersion,
                            reinterpret_cast<void*>(static_cast<intptr_t>(drmFd)),
                            bind_syncobj_manager);
}

} // namespace Lingmo::Wayland
