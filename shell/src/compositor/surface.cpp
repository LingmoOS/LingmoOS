// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/surface.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: integrates the upstream behavior with native
// compositor types and ownership.

#include "compositor/surface.h"

#include <poll.h>
#include <sys/epoll.h>

#include <algorithm>
#include <cmath>
#include <utility>

#include "base/event_loop.h"
#include "compositor/compositor.h"
#include "compositor/sub_surface.h"
#include "compositor/surface_observer.h"
#include "core/logging.h"
#include "core/main_loop.h"
#include "core/scoped_fd.h"
#include "render/drm_syncobj.h"
#include "render/renderer.h"
#include "scene/surface_node.h"
namespace scene = Lingmo::Scene;

namespace Lingmo::Compositor {

namespace {

bool transformSwapsAxes(Transform transform)
{
    switch (transform) {
    case Transform::Rotate90:
    case Transform::Rotate270:
    case Transform::FlippedRotate90:
    case Transform::FlippedRotate270:
        return true;
    default:
        return false;
    }
}

// SkRegion, and base::Region after it, cannot represent a rect whose extent
// reaches INT32_MAX; clients send exactly that to mean "all of it".
base::Rect clampDamage(const base::Rect &rect)
{
    constexpr int kMax = 0x7FFFFFFE;
    base::Rect result = rect;
    if (result.width() == 0x7FFFFFFF)
        result.setWidth(kMax);
    if (result.height() == 0x7FFFFFFF)
        result.setHeight(kMax);
    return result.intersected(base::Rect(-kMax / 2, -kMax / 2, kMax, kMax));
}

// The size of a surface without a buffer.
constexpr base::Size kNoSize(-1, -1);

// Held commits across all surfaces, so that the common case never walks a tree.
size_t s_heldCommits = 0;
uint64_t s_commitSequence = 0;

// A readability watch on the main loop, removed with its owner. The watch may
// remove itself from its own callback, so the id is shared with it.
class FenceWatch
{
public:
    FenceWatch() = default;
    FenceWatch(FenceWatch &&) = default;
    FenceWatch &operator=(FenceWatch &&) = default;
    ~FenceWatch() { stop(); }

    void start(int fd, std::function<void()> ready)
    {
        stop();
        base::EventLoop *loop = core::mainLoop();
        m_id = std::make_shared<int>(0);
        *m_id = loop->watch(fd, EPOLLIN, [loop, id = m_id, ready = std::move(ready)](uint32_t) {
            // Level-triggered: a signalled fence behind an older commit would
            // fire on every iteration.
            loop->unwatch(std::exchange(*id, 0));
            ready();
        });
    }

    void stop()
    {
        if (m_id && *m_id)
            core::mainLoop()->unwatch(std::exchange(*m_id, 0));
    }

private:
    std::shared_ptr<int> m_id;
};

// Edges inclusive, and never true when either rectangle has no width or no
// height.
bool rectFContains(const base::Rect &outer, const ui::RectF &inner)
{
    if (outer.width() == 0 || outer.height() == 0 || inner.width == 0 || inner.height == 0)
        return false;
    return inner.x >= outer.x() && inner.right() <= outer.right() && inner.y >= outer.y()
            && inner.bottom() <= outer.bottom();
}

} // namespace

// Everything one wl_surface.commit carries, taken out of pending state when the
// client sends it, so later requests cannot leak into a commit that waits.
struct Surface::CommitRequest {
    struct Fence {
        // Readable once signalled: a sync_file or a syncobj eventfd.
        ScopedFd fd;
        FenceWatch watch;
    };

    CommitRequest() = default;
    CommitRequest(const CommitRequest &) = delete;
    CommitRequest &operator=(const CommitRequest &) = delete;

    // What a dropped commit still owes the client.
    ~CommitRequest()
    {
        for (const FrameCallback &callback : state.frameCallbacks)
            callback(std::nullopt);
        if (state.release)
            state.release->signal(render::SyncFence());
        if (buffer)
            buffer->onDetach(true);
    }

    void addFence(ScopedFd fd)
    {
        if (fd.isValid())
            fences.push_back(Fence {std::move(fd), {}});
    }

    bool isReady()
    {
        std::erase_if(fences, [](const Fence &fence) {
            // An error or a hangup never turns into a signal; waiting on would
            // hold the surface for good.
            pollfd readable = {fence.fd.get(), POLLIN, 0};
            return ::poll(&readable, 1, 0) > 0;
        });
        return fences.empty();
    }

    uint64_t sequence = 0;
    ExtendedState state;
    // Holds the cached attachment's claim on client storage, and keeps the
    // contents a client destroys its wl_buffer under while the commit waits.
    std::shared_ptr<Buffer> buffer;
    bool attachedBuffer = false;
    bool explicitSyncBuffer = false;
    std::shared_ptr<render::PresentationContent> presentation;
    SubSurfaceList subSurfaces;
    std::vector<std::pair<SurfaceObserver *, std::function<void()>>> roleState;
    std::vector<Fence> fences;
    bool held = false;
    // wl_subsurface.set_desync, which applies a cache rather than commits one.
    bool desynchronize = false;
};

uint32_t Surface::preferredScale() const
{
    return m_compositor ? m_compositor->preferredScaleFor(this) : 120;
}

void Surface::notifyOutputScaleChanged()
{
    const auto observers = m_observers;
    const auto alive = lifetimeToken();
    for (auto *observer : observers) {
        if (!*alive)
            return;
        if (hasSurfaceObserver(observer))
            observer->onScaleFactorChanged(this);
    }
}

Surface::Surface(Compositor *compositor)
    : m_compositor(compositor)
{
    m_pendingSubSurfaces.push_back({this, {}});
    m_cachedSubSurfaces = m_subSurfaces = m_pendingSubSurfaces;
    if (m_compositor)
        m_node = m_compositor->createSurfaceNode(this);
}

Surface::~Surface()
{
    *m_lifetime = nullptr;
    // Out of the compositor's list before anything is notified: an observer may
    // enumerate surfaces, and must not be handed one that is being destroyed.
    if (m_compositor)
        m_compositor->surfaceDestroyed(this);

    // Observers next: they hold raw pointers to this surface and must drop
    // them before any of its state is torn down. One observer's handler may
    // destroy another observer, so re-check membership on every step.
    for (SurfaceObserver *observer : std::vector<SurfaceObserver *>(m_observers)) {
        if (hasSurfaceObserver(observer))
            observer->onSurfaceDestroying(this);
    }

    if (m_subSurface)
        m_subSurface->onSurfaceDestroying(this);
    // Children own their protocol objects and survive destruction of the parent.
    while (m_pendingSubSurfaces.size() > 1) {
        auto it = std::find_if(m_pendingSubSurfaces.begin(), m_pendingSubSurfaces.end(),
                               [this](const auto &e) {
                                   return e.surface != this;
                               });
        it->surface->m_subSurface->onSurfaceDestroying(this);
    }
    s_heldCommits -= m_heldCommits.size();
    m_heldCommits.clear();
    if (m_cached.buffer) {
        if (auto buffer = m_cached.buffer->buffer.lock())
            buffer->onDetach(true);
    }
    auto presentations = std::move(m_presentations);
    for (auto &weak : presentations) {
        if (auto content = weak.lock())
            content->complete({});
    }
    // Cancel every callback the client is still waiting on.
    for (auto *list : {&m_pending.frameCallbacks, &m_cached.frameCallbacks, &m_state.frameCallbacks,
                       &m_inFlightFrameCallbacks}) {
        for (const FrameCallback &callback : *list)
            callback(std::nullopt);
        list->clear();
    }

    if (m_state.buffer) {
        if (std::shared_ptr<Buffer> buffer = m_state.buffer->buffer.lock())
            buffer->onDetach();
    }

    // Releases that never reached a buffer are signalled here rather than left
    // for a client that would wait on them forever.
    for (auto *state : {&m_pending, &m_cached, &m_state}) {
        if (state->release)
            state->release->signal(render::SyncFence());
        state->release.reset();
    }

    if (m_node && m_node->parent())
        m_node->parent()->removeChild(m_node);
}

void Surface::attach(const std::shared_ptr<Buffer> &buffer, const base::Point &offset)
{
    m_pending.buffer = BufferAttachment {buffer, buffer ? buffer->size() : kNoSize};
    m_pending.basicState.offset = offset;
}

bool Surface::hasPendingAttachedBuffer() const
{
    return m_pending.buffer.has_value() && !m_pending.buffer->buffer.expired();
}

void Surface::setAcquireFence(render::SyncFence fence)
{
    m_pending.acquireFence = std::move(fence);
}

void Surface::setAcquirePoint(std::shared_ptr<render::DrmSyncobj> timeline, uint64_t point)
{
    m_pending.acquireTimeline = std::move(timeline);
    m_pending.acquirePoint = point;
}

void Surface::setBufferRelease(std::shared_ptr<BufferRelease> release)
{
    // A release replaced before it was ever committed is still owed: the client
    // is waiting on that timeline point and nothing else will signal it.
    if (m_pending.release && m_pending.release != release)
        m_pending.release->signal(render::SyncFence());
    m_pending.release = std::move(release);
}

void Surface::damage(const base::Rect &rect)
{
    m_pending.damage += clampDamage(rect);
}

void Surface::damageBuffer(const base::Rect &rect)
{
    m_pending.bufferDamage += clampDamage(rect);
}

void Surface::requestFrameCallback(FrameCallback callback)
{
    m_pending.frameCallbacks.push_back(std::move(callback));
}

void Surface::setBackgroundBlur(const base::Rect &rect, int radius)
{
    m_pending.basicState.blurRect = rect;
    m_pending.basicState.blurRadius = radius;
}

ui::Path Surface::backgroundBlurShape() const
{
    const base::Rect rect = backgroundBlurRect();
    const ui::Path &shape = m_state.basicState.blurShape;
    if (rect.isEmpty() || shape.isEmpty())
        return {};
    // Glass keeps its lens on the rectangle, so the shape may reach past it.
    if (glass().enabled)
        return shape;
    if (rectFContains(rect, shape.controlPointRect()))
        return shape;
    ui::Path bounds;
    bounds.addRect(ui::RectF {double(rect.x()), double(rect.y()), double(rect.width()),
                              double(rect.height())});
    return shape.intersected(bounds);
}

void Surface::setOpaqueRegion(const base::Region &region)
{
    m_pending.basicState.opaqueRegion = region;
}

void Surface::setInputRegion(const base::Region &region)
{
    m_pending.basicState.inputRegion = region;
}

void Surface::resetInputRegion()
{
    m_pending.basicState.inputRegion.reset();
}

void Surface::setBufferScale(int scale)
{
    m_pending.basicState.bufferScale = scale;
}

void Surface::setClientScale(double scale)
{
    m_pending.basicState.clientScale = scale > 0 ? scale : 1;
}

void Surface::setBufferTransform(Transform transform)
{
    m_pending.basicState.bufferTransform = transform;
}

void Surface::requestPresentationCallback(render::PresentationContent::Callback callback)
{
    if (!m_pendingPresentation) {
        m_pendingPresentation = std::make_shared<render::PresentationContent>();
        std::erase_if(m_presentations, [](const auto &item) {
            return item.expired();
        });
        m_presentations.push_back(m_pendingPresentation);
    }
    m_pendingPresentation->add(std::move(callback));
}

void Surface::abandonUnsubmittedPresentation(const render::PresentationSubmission &submission)
{
    if (m_node && !submission.contains(m_node->presentationContent()))
        m_node->setPresentationContent(nullptr);
}

void Surface::commit()
{
    const auto alive = m_lifetime;
    auto request = std::make_unique<CommitRequest>();
    request->sequence = ++s_commitSequence;
    request->state = std::move(m_pending);
    m_pending = ExtendedState();
    m_pending.basicState = request->state.basicState;
    m_pending.basicState.offset = {};
    request->presentation = std::move(m_pendingPresentation);
    request->subSurfaces = m_pendingSubSurfaces;

    ExtendedState &state = request->state;
    request->buffer = state.buffer ? state.buffer->buffer.lock() : nullptr;
    // Cached attachments also hold client storage until applied or replaced.
    if (request->buffer)
        request->buffer->onAttach(true);
    request->attachedBuffer = request->buffer != nullptr;
    request->explicitSyncBuffer = request->buffer && request->buffer->supportsExplicitSync();
    m_lastCommitAttachedBuffer = request->attachedBuffer;
    m_lastCommitBufferSupportsExplicitSync = request->explicitSyncBuffer;

    if (request->buffer) {
        if (state.acquireTimeline) {
            ScopedFd signalled = state.acquireTimeline->signalEventFd(state.acquirePoint);
            // The kernel cannot wait for a point it has no fence for yet; take
            // whatever is there now.
            if (!signalled.isValid())
                state.acquireFence = state.acquireTimeline->exportFence(state.acquirePoint);
            request->addFence(std::move(signalled));
        }
        if (state.acquireFence.isValid()) {
            request->addFence(ScopedFd(state.acquireFence.duplicate().release()));
        } else if (!state.acquireTimeline) {
            for (render::SyncFence &fence : request->buffer->implicitFences())
                request->addFence(ScopedFd(fence.release()));
        }
    }
    state.acquireTimeline.reset();

    for (SurfaceObserver *observer : std::vector<SurfaceObserver *>(m_observers)) {
        if (!hasSurfaceObserver(observer))
            continue;
        if (auto install = observer->onSurfaceCommitRequested(this))
            request->roleState.emplace_back(observer, std::move(install));
        if (!*alive)
            return;
    }

    if ((s_heldCommits == 0 || !rootSurface()->oldestHeldCommit()) && request->isReady()) {
        applyCommit(*request);
        return;
    }
    holdCommit(std::move(request));
}

void Surface::applyCommit(CommitRequest &request)
{
    if (request.desynchronize) {
        if (!isSynchronized())
            commitDesynchronized();
        return;
    }

    const auto alive = m_lifetime;
    ExtendedState &incoming = request.state;
    auto replacedApplied = m_node ? m_node->presentationContent() : nullptr;
    auto replacedPresentation =
            std::exchange(m_cachedPresentation, std::move(request.presentation));
    m_needsCommitSurface = true;
    // Cached buffer damage cannot be reinterpreted through a later mapping.
    m_cached.fullDamage |= m_cached.basicState.crop != incoming.basicState.crop
            || m_cached.basicState.viewport != incoming.basicState.viewport
            || m_cached.basicState.bufferScale != incoming.basicState.bufferScale
            || m_cached.basicState.clientScale != incoming.basicState.clientScale
            || m_cached.basicState.bufferTransform != incoming.basicState.bufferTransform
            || (m_cached.buffer && incoming.buffer
                && m_cached.buffer->size != incoming.buffer->size);
    // Synchronized commits accumulate offsets before the parent applies them.
    const base::Point offset = m_cached.basicState.offset + incoming.basicState.offset;
    m_cached.basicState = incoming.basicState;
    m_cached.basicState.offset = offset;
    m_lastCommitAttachedBuffer = request.attachedBuffer;
    m_lastCommitBufferSupportsExplicitSync = request.explicitSyncBuffer;
    if (incoming.buffer) {
        // The incoming buffer took its cached claim when the commit was sent.
        if (auto outgoing = m_cached.buffer ? m_cached.buffer->buffer.lock() : nullptr)
            outgoing->onDetach(true);
        request.buffer.reset();
        m_cached.buffer = std::move(incoming.buffer);
        if (m_cached.release && m_cached.release != incoming.release)
            m_cached.release->signal(render::SyncFence());
        m_cached.acquireFence = std::move(incoming.acquireFence);
        m_cached.release = std::move(incoming.release);
    }
    incoming.acquireFence.reset();
    // A release committed without a buffer applies to nothing, and the protocol
    // layer has already raised no_buffer; signal it rather than strand it.
    if (incoming.release) {
        incoming.release->signal(render::SyncFence());
        incoming.release.reset();
    }
    m_cached.damage += incoming.damage;
    m_cached.bufferDamage += incoming.bufferDamage;
    m_cached.frameCallbacks.insert(m_cached.frameCallbacks.end(),
                                   std::make_move_iterator(incoming.frameCallbacks.begin()),
                                   std::make_move_iterator(incoming.frameCallbacks.end()));
    incoming.frameCallbacks.clear();
    if (request.held) {
        // Children added or removed while the commit waited are already in the
        // live lists; the commit places the others.
        const auto isIn = [](const SubSurfaceList &list, Surface *surface) {
            return std::any_of(list.begin(), list.end(), [surface](const auto &e) {
                return e.surface == surface;
            });
        };
        request.subSurfaces.remove_if([&](const auto &e) {
            return !isIn(m_pendingSubSurfaces, e.surface);
        });
        for (const auto &entry : m_pendingSubSurfaces) {
            if (!isIn(request.subSurfaces, entry.surface))
                request.subSurfaces.push_back(entry);
        }
    }
    m_cachedSubSurfaces = std::move(request.subSurfaces);

    // Exo SubSurface::OnSurfaceCommit: early out when synchronized with parent.
    if (!isSynchronized())
        commitDesynchronized();

    // Exo runs this through SurfaceDelegate::OnSurfaceCommit, which owns the
    // hierarchy commit above. Lingmo notifies afterwards instead, so a role
    // owner sees committed state. Handlers may destroy other observers.
    for (SurfaceObserver *observer : std::vector<SurfaceObserver *>(m_observers)) {
        if (!*alive)
            return;
        if (!hasSurfaceObserver(observer))
            continue;
        for (auto &[owner, install] : request.roleState) {
            if (owner == observer && install)
                std::exchange(install, nullptr)();
        }
        observer->onSurfaceCommit(this);
    }
}

void Surface::holdCommit(std::unique_ptr<CommitRequest> request)
{
    request->held = true;
    for (CommitRequest::Fence &fence : request->fences) {
        fence.watch.start(fence.fd.get(), [alive = m_lifetime] {
            if (*alive)
                (*alive)->applyReadyCommits();
        });
    }
    m_heldCommits.push_back(std::move(request));
    ++s_heldCommits;
}

void Surface::applyReadyCommits()
{
    const auto alive = m_lifetime;
    while (*alive) {
        Surface *surface = rootSurface()->oldestHeldCommit();
        if (!surface || !surface->m_heldCommits.front()->isReady())
            return;
        std::unique_ptr<CommitRequest> request = std::move(surface->m_heldCommits.front());
        surface->m_heldCommits.pop_front();
        --s_heldCommits;
        surface->applyCommit(*request);
    }
}

void Surface::requestDesynchronizedCommit()
{
    if (s_heldCommits == 0 || !rootSurface()->oldestHeldCommit()) {
        commitDesynchronized();
        return;
    }
    auto request = std::make_unique<CommitRequest>();
    request->sequence = ++s_commitSequence;
    request->desynchronize = true;
    holdCommit(std::move(request));
}

Surface *Surface::rootSurface()
{
    Surface *root = this;
    while (Surface *parent = root->parentSurface())
        root = parent;
    return root;
}

Surface *Surface::oldestHeldCommit()
{
    Surface *oldest = m_heldCommits.empty() ? nullptr : this;
    for (const auto &entry : m_pendingSubSurfaces) {
        if (entry.surface == this)
            continue;
        Surface *candidate = entry.surface->oldestHeldCommit();
        if (candidate
            && (!oldest
                || candidate->m_heldCommits.front()->sequence
                        < oldest->m_heldCommits.front()->sequence))
            oldest = candidate;
    }
    return oldest;
}

void Surface::applyCachedState()
{
    auto replaced = m_node ? m_node->presentationContent() : nullptr;
    if (m_node)
        m_node->setPresentationContent(std::move(m_cachedPresentation));
    else
        m_cachedPresentation.reset();
    m_state.damage = damageRegion();
    m_state.bufferDamage = {};
    const base::Size oldSize = contentSize();
    const bool mappingChanged = m_state.basicState.crop != m_cached.basicState.crop
            || m_state.basicState.viewport != m_cached.basicState.viewport
            || m_state.basicState.bufferScale != m_cached.basicState.bufferScale
            || m_state.basicState.clientScale != m_cached.basicState.clientScale
            || m_state.basicState.bufferTransform != m_cached.basicState.bufferTransform;
    const base::Point offset = m_state.basicState.offset + m_cached.basicState.offset;
    m_state.basicState = m_cached.basicState;
    m_state.basicState.offset = offset;
    m_cached.basicState.offset = {};
    if (m_cached.buffer) {
        auto incoming = m_cached.buffer->buffer.lock();
        // Attach before detach, including when cached/current share a buffer.
        // A prior upload by another surface must not release our cached pixels.
        if (incoming)
            incoming->onAttach();
        if (auto outgoing = m_state.buffer ? m_state.buffer->buffer.lock() : nullptr)
            outgoing->onDetach();
        if (incoming)
            incoming->onDetach(true);
        if (incoming) {
            incoming->setAcquireFence(std::move(m_cached.acquireFence));
            incoming->addRelease(std::move(m_cached.release));
        } else if (m_cached.release) {
            m_cached.release->signal(render::SyncFence());
        }
        m_cached.acquireFence.reset();
        m_cached.release.reset();
        // Transfer the cached attachment's reference without releasing it.
        m_state.buffer = std::move(m_cached.buffer);
        m_cached.buffer.reset();
        m_uploadBuffer = std::move(incoming);
        m_textureDirty = true;
    }
    // Damage invalidates the committed contents, not released client storage.
    // A new attachment is required before uploading that storage again.
    if (mappingChanged || m_cached.fullDamage) {
        m_uploadAll = true;
    } else {
        m_uploadDamage += m_cached.bufferDamage;
        for (const base::Rect &rect : m_cached.damage)
            m_uploadDamage += surfaceRectToBuffer(rect);
    }
    m_state.damage += m_cached.damage;
    m_state.bufferDamage += m_cached.bufferDamage;
    m_cached.damage = {};
    m_cached.bufferDamage = {};
    m_state.frameCallbacks.insert(m_state.frameCallbacks.end(),
                                  std::make_move_iterator(m_cached.frameCallbacks.begin()),
                                  std::make_move_iterator(m_cached.frameCallbacks.end()));
    m_cached.frameCallbacks.clear();
    if ((mappingChanged || m_cached.fullDamage)
        && (!oldSize.isEmpty() || m_state.basicState.crop || m_state.basicState.viewport)) {
        m_state.damage += base::Rect(base::Point(), oldSize);
        m_state.damage += base::Rect(base::Point(), contentSize());
    }
    m_cached.fullDamage = false;
    m_state.damage = damageRegion();
    m_state.bufferDamage = {};
    if (m_node)
        m_node->setTextureMapping(textureMapping());
    m_subSurfaces = m_cachedSubSurfaces;
    updateSubSurfaceStacking();
    if (m_node) {
        m_node->setGeometry(base::Rect(m_position, contentSize()));
        // The opaque region is committed state and can change without the
        // buffer changing, so it is applied here rather than at upload.
        m_node->setOpaque(isFullyOpaque());
        m_node->setBackgroundBlur(backgroundBlurRect(), backgroundBlurRadius(), glass(),
                                  backgroundBlurShape());
    }
    if (m_node)
        m_node->notifyDamage(damageRegion());
    if (m_compositor && m_node && m_node->isEffectivelyVisible())
        m_compositor->scheduleRepaint();
}

void Surface::addSurfaceObserver(SurfaceObserver *observer)
{
    if (!hasSurfaceObserver(observer))
        m_observers.push_back(observer);
}

void Surface::removeSurfaceObserver(SurfaceObserver *observer)
{
    m_observers.erase(std::remove(m_observers.begin(), m_observers.end(), observer),
                      m_observers.end());
    for (const auto &request : m_heldCommits) {
        std::erase_if(request->roleState, [observer](const auto &entry) {
            return entry.first == observer;
        });
    }
}

bool Surface::hasSurfaceObserver(const SurfaceObserver *observer) const
{
    return std::find(m_observers.begin(), m_observers.end(), observer) != m_observers.end();
}

// Port of the hit_test_region_ half of Exo Surface::CommitSurfaceHierarchy.
// The local region is this surface's own input region clipped to its content;
// the full region unions in every subsurface, which is what a parent's hit test
// needs before descending.
void Surface::setExternalInputRegion(std::optional<base::Region> region)
{
    m_externalInputRegion = std::move(region);
    updateHitTestRegion();
}

void Surface::updateHitTestRegion()
{
    m_subSurfaceStack.clear();
    m_subSurfaceStack.reserve(m_subSurfaces.size());
    for (const auto &entry : m_subSurfaces)
        m_subSurfaceStack.emplace_back(entry.surface, entry.position);

    const base::Rect content(base::Point(0, 0), contentSize());
    const std::optional<base::Region> &inputRegion = m_state.basicState.inputRegion;
    m_localHitTestRegion =
            inputRegion ? (*inputRegion & base::Region(content)) : base::Region(content);

    if (m_externalInputRegion)
        m_localHitTestRegion &= *m_externalInputRegion;
    m_hitTestRegion = m_localHitTestRegion;
    for (const auto &entry : m_subSurfaces) {
        if (entry.surface == this)
            continue;
        m_hitTestRegion += entry.surface->m_hitTestRegion.translated(entry.position);
    }
}

void Surface::commitSurfaceHierarchy(bool synchronized)
{
    const auto alive = m_lifetime;
    // Port of Exo Surface::CommitSurfaceHierarchy, without Viz resource assembly.
    if (m_needsCommitSurface && (synchronized || !isSynchronized())) {
        for (SurfaceObserver *observer : std::vector<SurfaceObserver *>(m_observers)) {
            if (hasSurfaceObserver(observer) && !observer->onSurfaceStateApplying(this))
                return;
        }
        m_needsCommitSurface = false;
        synchronized = true;
        applyCachedState();
        if (!*alive)
            return;
    }
    // Synchronously commit all cached state of the sub-surface and descendants.
    std::vector<std::shared_ptr<Surface *>> children;
    for (const auto &entry : m_subSurfaces)
        if (entry.surface != this)
            children.push_back(entry.surface->lifetimeToken());
    for (const auto &child : children) {
        if (*child)
            (*child)->commitSurfaceHierarchy(synchronized);
        if (!*alive)
            return;
    }
    updateHitTestRegion();
    updateMapping();
}

void Surface::commitDesynchronized()
{
    rootSurface()->commitSurfaceHierarchy(false);
}

bool Surface::isSynchronized() const
{
    return m_subSurface && m_subSurface->isSurfaceSynchronized();
}

Surface *Surface::parentSurface() const
{
    return m_subSurface ? m_subSurface->parent() : nullptr;
}

bool Surface::isMapped() const
{
    if (!hasContents())
        return false;
    if (!m_hadSubSurfaceRole)
        return true;
    return parentSurface() && parentSurface()->isMapped();
}

void Surface::updateMapping()
{
    const auto alive = m_lifetime;
    auto replaced = m_node ? m_node->presentationContent() : nullptr;
    if (m_node) {
        m_node->setVisible(isMapped() && !m_hidden);
        m_node->setHiddenWithWindow(isMapped() && m_hidden);
        if (!m_node->isEffectivelyVisible())
            m_node->setPresentationContent(nullptr);
    }
    std::vector<std::shared_ptr<Surface *>> children;
    for (const auto &entry : m_pendingSubSurfaces)
        if (entry.surface != this)
            children.push_back(entry.surface->lifetimeToken());
    for (const auto &child : children) {
        if (*child)
            (*child)->updateMapping();
        if (!*alive)
            return;
    }
}

void Surface::addSubSurface(Surface *child)
{
    child->m_hadSubSurfaceRole = true;
    child->setPosition({});
    if (m_node && child->m_node) {
        auto node = child->m_node->parent() ? child->m_node->parent()->removeChild(child->m_node)
                                            : std::move(child->m_detachedNode);
        m_node->addChild(std::move(node));
    }
    for (auto *list : {&m_pendingSubSurfaces, &m_cachedSubSurfaces, &m_subSurfaces})
        list->push_back({child, {}});
    child->updateMapping();
}

void Surface::removeSubSurface(Surface *child)
{
    for (auto *list : {&m_pendingSubSurfaces, &m_cachedSubSurfaces, &m_subSurfaces})
        list->remove_if([child](const auto &e) {
            return e.surface == child;
        });
    if (child->m_node && child->m_node->parent()) {
        child->m_detachedNode = child->m_node->parent()->removeChild(child->m_node);
        child->m_node->setVisible(false);
    }
    updateSubSurfaceStacking();
    if (m_compositor)
        m_compositor->scheduleRepaint();
    // Either side may have waited only behind the other's commits. Not from
    // here: this runs inside protocol requests and surface destruction.
    if (s_heldCommits) {
        for (Surface *surface : {this, child}) {
            core::mainLoop()->post([alive = surface->lifetimeToken()] {
                if (*alive)
                    (*alive)->applyReadyCommits();
            });
        }
    }
}

void Surface::setSubSurfacePosition(Surface *child, const base::Point &position)
{
    for (auto &entry : m_pendingSubSurfaces)
        if (entry.surface == child)
            entry.position = position;
}

bool Surface::placeSubSurface(Surface *child, Surface *reference, bool above)
{
    // Exo DoPlaceAboveOrBelow's list splice, with an explicit parent sentinel.
    // Unlike upstream's above-only parent special case this also supports below.
    if (child == reference)
        return false;
    auto find = [&](Surface *surface) {
        return std::find_if(m_pendingSubSurfaces.begin(), m_pendingSubSurfaces.end(),
                            [surface](const auto &e) {
                                return e.surface == surface;
                            });
    };
    auto position = find(reference);
    auto it = find(child);
    if (position == m_pendingSubSurfaces.end() || it == m_pendingSubSurfaces.end())
        return false;
    if (above)
        ++position;
    m_pendingSubSurfaces.splice(position, m_pendingSubSurfaces, it);
    return true;
}

void Surface::updateSubSurfaceStacking()
{
    size_t below = 0;
    bool foundParent = false;
    std::vector<scene::Node *> order;
    for (const auto &entry : m_subSurfaces)
        if (entry.surface != this && entry.surface->m_node)
            order.push_back(entry.surface->m_node);
    const bool reorder = m_node && m_node->children() != order;
    for (const auto &entry : m_subSurfaces) {
        if (entry.surface == this) {
            foundParent = true;
            continue;
        }
        entry.surface->setPosition(entry.position);
        if (reorder && entry.surface->m_node)
            m_node->raise(entry.surface->m_node);
        if (!foundParent)
            ++below;
    }
    if (m_node)
        m_node->setChildrenBelowParent(below);
}

std::shared_ptr<Buffer> Surface::buffer() const
{
    return m_state.buffer ? m_state.buffer->buffer.lock() : nullptr;
}

base::Size Surface::bufferSize() const
{
    return m_state.buffer ? m_state.buffer->size : kNoSize;
}

base::Size Surface::contentSize() const
{
    return contentSizeOf(m_state);
}

base::Size Surface::cachedContentSize() const
{
    return contentSizeOf(m_cached);
}

base::Size Surface::contentSizeOf(const ExtendedState &state)
{
    const base::Size size = state.buffer ? state.buffer->size : kNoSize;
    if (size.isEmpty())
        return kNoSize;

    if (state.basicState.viewport)
        return *state.basicState.viewport;
    base::Size content;
    if (state.basicState.crop) {
        content = state.basicState.crop->size().toSize();
    } else {
        const int scale = std::max(1, state.basicState.bufferScale);
        const base::Size scaled(size.width() / scale, size.height() / scale);
        content = transformSwapsAxes(state.basicState.bufferTransform)
                ? base::Size(scaled.height(), scaled.width())
                : scaled;
    }
    // Acts as an implicit viewport destination; a client viewport arrives in
    // logical units already.
    const double clientScale = state.basicState.clientScale;
    if (clientScale != 1)
        content = base::Size(std::max(1, base::roundToInt(content.width() / clientScale)),
                             std::max(1, base::roundToInt(content.height() / clientScale)));
    return content;
}

Surface::ViewportError Surface::cachedViewportError() const
{
    const auto &state = m_cached.basicState;
    if (!state.crop)
        return ViewportError::None;
    if (!state.viewport
        && (std::floor(state.crop->width()) != state.crop->width()
            || std::floor(state.crop->height()) != state.crop->height()))
        return ViewportError::BadSize;
    const auto &attachment = m_cached.buffer ? m_cached.buffer : m_state.buffer;
    if (attachment && !attachment->size.isEmpty()) {
        base::SizeF size(attachment->size);
        if (transformSwapsAxes(state.bufferTransform))
            size = size.transposed();
        size = size / std::max(1, state.bufferScale);
        if (!base::RectF(size).contains(*state.crop))
            return ViewportError::OutOfBuffer;
    }
    return ViewportError::None;
}

base::AffineTransform Surface::surfaceToBuffer() const
{
    const base::Size size = bufferSize();
    const base::Size content = contentSize();
    if (size.isEmpty() || content.isEmpty())
        return {};

    using base::AffineTransform;
    // Exo ToBufferTransformMatrix: normalized raw buffer to transformed buffer.
    AffineTransform transform;
    switch (bufferTransform()) {
    case Transform::Normal:
        break;
    case Transform::Rotate90:
        transform = AffineTransform(0, -1, 1, 0, 0, 1);
        break;
    case Transform::Rotate180:
        transform = AffineTransform(-1, 0, 0, -1, 1, 1);
        break;
    case Transform::Rotate270:
        transform = AffineTransform(0, 1, -1, 0, 1, 0);
        break;
    case Transform::Flipped:
        transform = AffineTransform(-1, 0, 0, 1, 1, 0);
        break;
    case Transform::FlippedRotate90:
        transform = AffineTransform(0, 1, 1, 0, 0, 0);
        break;
    case Transform::FlippedRotate180:
        transform = AffineTransform(1, 0, 0, -1, 0, 1);
        break;
    case Transform::FlippedRotate270:
        transform = AffineTransform(0, -1, -1, 0, 1, 1);
        break;
    }
    base::SizeF transformed(size);
    if (transformSwapsAxes(bufferTransform()))
        transformed = transformed.transposed();
    transformed = transformed / std::max(1, bufferScale());
    const base::RectF source = crop().value_or(base::RectF(transformed));
    // Exo's crop normalization and inverse buffer transform, retaining corner orientation.
    return AffineTransform::fromScale(source.width() / content.width(),
                                      source.height() / content.height())
            * AffineTransform::fromTranslate(source.x(), source.y())
            * AffineTransform::fromScale(1 / transformed.width(), 1 / transformed.height())
            * transform.inverted() * AffineTransform::fromScale(size.width(), size.height());
}

base::AffineTransform Surface::textureMapping() const
{
    if (bufferSize().isEmpty() || contentSize().isEmpty())
        return {};
    return base::AffineTransform::fromScale(contentSize().width(), contentSize().height())
            * surfaceToBuffer()
            * base::AffineTransform::fromScale(1.0 / bufferSize().width(),
                                               1.0 / bufferSize().height());
}

base::Rect Surface::bufferRectToSurface(const base::Rect &rect) const
{
    if (contentSize().isEmpty())
        return {};
    const base::AffineTransform mapping = surfaceToBuffer();
    const double determinant = mapping.determinant();
    // Valid extreme scaling can fall below AffineTransform::inverted's singularity tolerance.
    const base::AffineTransform inverse(
            mapping.m22() / determinant, -mapping.m12() / determinant, -mapping.m21() / determinant,
            mapping.m11() / determinant,
            (mapping.m21() * mapping.dy() - mapping.m22() * mapping.dx()) / determinant,
            (mapping.m12() * mapping.dx() - mapping.m11() * mapping.dy()) / determinant);
    const base::RectF mapped = inverse.mapRect(base::RectF(rect));
    return mapped.intersected(base::RectF(base::SizeF(contentSize()))).toEnclosingRect();
}

base::Rect Surface::surfaceRectToBuffer(const base::Rect &rect) const
{
    if (contentSize().isEmpty())
        return {};
    // Linear sampling blends neighbours, but a texel changes only where the
    // buffer did; outward rounding keeps the partly covered ones.
    return surfaceToBuffer()
            .mapRect(base::RectF(rect))
            .toEnclosingRect()
            .intersected(base::Rect(bufferSize()));
}

base::Region Surface::damageRegion() const
{
    base::Region result = m_state.damage;
    for (const base::Rect &rect : m_state.bufferDamage)
        result += bufferRectToSurface(rect);
    return result;
}

void Surface::reparentNode(scene::Node *newParent)
{
    if (!m_node || (newParent && m_node->parent() == newParent))
        return;

    std::unique_ptr<scene::Node> owned =
            m_node->parent() ? m_node->parent()->removeChild(m_node) : std::move(m_detachedNode);
    if (!owned)
        return;

    if (newParent)
        newParent->addChild(std::move(owned));
    else
        m_detachedNode = std::move(owned);
}

void Surface::setHidden(bool hidden)
{
    const auto alive = m_lifetime;
    auto replaced = m_node ? m_node->presentationContent() : nullptr;
    if (hidden == m_hidden)
        return;
    m_hidden = hidden;
    updateMapping();
    if (!*alive)
        return;
    if (m_compositor)
        m_compositor->scheduleRepaint();
}

void Surface::setPosition(const base::Point &position)
{
    m_position = position;
    if (m_node) {
        m_node->setGeometry(base::Rect(m_position, contentSize()));
        // The opaque region is committed state and can change without the
        // buffer changing, so it is applied here rather than at upload.
        m_node->setOpaque(isFullyOpaque());
        m_node->setBackgroundBlur(backgroundBlurRect(), backgroundBlurRadius(), glass(),
                                  backgroundBlurShape());
    }
}

bool Surface::updateTexture(render::Renderer &renderer)
{
    if (!m_textureDirty || !m_node)
        return false;

    m_textureDirty = false;
    m_state.damage = base::Region();
    m_state.bufferDamage = base::Region();

    // Texture uploads run after protocol dispatch, which may destroy wl_buffer.
    auto uploadBuffer = std::move(m_uploadBuffer);
    std::shared_ptr<Buffer> buffer = uploadBuffer ? uploadBuffer : this->buffer();
    // Chromium retires resize buffers before the next repaint. Resource
    // destruction is not a null attachment; keep the last uploaded contents.
    if (!buffer && hasContents())
        return false;

    if (!buffer || buffer->size().isEmpty()) {
        m_node->setTexture(nullptr);
        m_node->setScanoutBuffer(nullptr);
        m_node->setAcquireFence(render::SyncFence());
        return false;
    }

    m_node->setAcquireFence(buffer->acquireFence().duplicate());

    // A DMA-BUF is sampled where the client left it; there is nothing to upload
    // and nothing to consume, so the release waits for detach instead. The same
    // storage is what the display controller may scan out.
    if (std::shared_ptr<render::Texture> imported = buffer->texture()) {
        m_node->setTexture(std::move(imported));
        // Aliased so that holding the scanout view keeps the whole wl_buffer
        // alive while KMS is still displaying it.
        if (render::ScanoutBuffer *scanout = buffer->scanoutBuffer())
            m_node->setScanoutBuffer(std::shared_ptr<render::ScanoutBuffer>(buffer, scanout));
        else
            m_node->setScanoutBuffer(nullptr);
        return true;
    }

    m_node->setScanoutBuffer(nullptr);
    if (!buffer->pixels()) {
        m_node->setTexture(nullptr);
        return false;
    }

    render::Texture *existing = m_node->texture();
    bool uploaded = false;
    // Outside its damage the buffer holds what the texture already has, as
    // wlroots and mutter take it; exo copies shm whole into shared images.
    const base::Region damage = std::exchange(m_uploadAll, false)
            ? base::Region(base::Rect(base::Point(), buffer->size()))
            : m_uploadDamage;
    m_uploadDamage = {};
    buffer->beginPixelAccess();
    if (existing && existing->size() == buffer->size() && existing->format() == buffer->format()) {
        uploaded = renderer.updateTexture(existing, buffer->pixels(), buffer->stride(),
                                          buffer->format(), damage);
    }
    if (!uploaded) {
        std::unique_ptr<render::Texture> texture = renderer.createTexture(
                buffer->size(), buffer->pixels(), buffer->stride(), buffer->format());
        uploaded = texture != nullptr;
        m_node->setTexture(std::move(texture));
    }
    // The client pulled its storage away mid-copy; the texture holds zeros
    // rather than content, so show nothing.
    if (!buffer->endPixelAccess()) {
        m_node->setTexture(nullptr);
        uploaded = false;
    }

    // The pixels now live in a compositor-owned texture, so the client may
    // reuse its storage.
    buffer->onContentsConsumed();
    return uploaded;
}

// The content covers its whole geometry with no transparency, which is what
// lets a display controller put it on a plane without blending.
bool Surface::isFullyOpaque() const
{
    const base::Size size = contentSize();
    if (size.isEmpty())
        return false;
    std::shared_ptr<Buffer> buffer = this->buffer();
    if (buffer && !render::pixelFormatHasAlpha(buffer->format()))
        return true;
    return opaqueRegion().contains(base::Rect(base::Point(0, 0), size));
}

void Surface::frameSubmitted(uint64_t generation)
{
    if (m_hadSubSurfaceRole && !isMapped())
        return;
    if (generation && (!isMapped() || !m_node || !m_node->isEffectivelyVisible()))
        return;
    if (m_state.frameCallbacks.empty())
        return;
    m_frameGeneration = generation;

    m_inFlightFrameCallbacks.insert(m_inFlightFrameCallbacks.end(),
                                    std::make_move_iterator(m_state.frameCallbacks.begin()),
                                    std::make_move_iterator(m_state.frameCallbacks.end()));
    m_state.frameCallbacks.clear();
}

bool Surface::awaitsFrame() const
{
    if (!isMapped() || !m_node || !m_node->isEffectivelyVisible())
        return false;
    const auto presentation = m_node->presentationContent();
    return !m_state.frameCallbacks.empty() || (presentation && presentation->isPending());
}

bool Surface::updatePlacement()
{
    Placement current;
    if (m_node) {
        current.bounds = m_node->absoluteGeometry();
        current.shown = m_node->isEffectivelyVisible() && !contentSize().isEmpty();
    }
    return std::exchange(m_placement, current) != current;
}

void Surface::frameDiscarded(uint64_t generation)
{
    if (generation != m_frameGeneration)
        return;
    m_state.frameCallbacks.insert(m_state.frameCallbacks.begin(),
                                  std::make_move_iterator(m_inFlightFrameCallbacks.begin()),
                                  std::make_move_iterator(m_inFlightFrameCallbacks.end()));
    m_inFlightFrameCallbacks.clear();
}

void Surface::frameDrawn(uint32_t timeMs, uint64_t generation)
{
    if (generation != m_frameGeneration)
        return;
    std::vector<FrameCallback> callbacks;
    callbacks.swap(m_inFlightFrameCallbacks);
    const auto alive = m_lifetime;
    for (const FrameCallback &callback : callbacks)
        callback(*alive ? std::optional<uint32_t>(timeMs) : std::nullopt);
}

void Surface::setSkipTaskbar(bool skip)
{
    if (m_skipTaskbar == skip)
        return;
    m_skipTaskbar = skip;
    notifyWindowHintsChanged();
}

void Surface::setAlwaysOnTop(bool alwaysOnTop)
{
    if (m_alwaysOnTop == alwaysOnTop)
        return;
    m_alwaysOnTop = alwaysOnTop;
    notifyWindowHintsChanged();
}

void Surface::setBodyShape(const ui::Path &shape)
{
    if (shape == m_bodyShape)
        return;
    m_bodyShape = shape;
    notifyWindowHintsChanged();
}

void Surface::setCastsShadow(bool castsShadow)
{
    if (m_castsShadow == castsShadow)
        return;
    m_castsShadow = castsShadow;
    notifyWindowHintsChanged();
}

void Surface::setOpacity(float opacity)
{
    if (m_opacity == opacity)
        return;
    m_opacity = opacity;
    notifyWindowHintsChanged();
}

void Surface::notifyWindowHintsChanged()
{
    for (auto *observer : std::vector<SurfaceObserver *>(m_observers)) {
        if (hasSurfaceObserver(observer))
            observer->onSurfaceWindowHintsChanged(this);
    }
}

uint64_t Surface::addMenuSource()
{
    const uint64_t id = ++m_nextMenuSource;
    m_menuSources.emplace(id, MenuAddress {});
    return id;
}

void Surface::setMenuAddress(uint64_t source, const MenuAddress &address)
{
    if (auto it = m_menuSources.find(source); it != m_menuSources.end()) {
        it->second = address;
        updateMenuAddress();
    }
}

void Surface::removeMenuSource(uint64_t source)
{
    m_menuSources.erase(source);
    updateMenuAddress();
}

void Surface::updateMenuAddress()
{
    MenuAddress address;
    for (auto it = m_menuSources.rbegin(); it != m_menuSources.rend(); ++it) {
        const MenuAddress &source = it->second;
        if (address.service.empty() && !source.service.empty() && !source.path.empty()) {
            address.service = source.service;
            address.path = source.path;
        }
        if (address.gtkBusName.empty() && !source.gtkBusName.empty() && !source.gtkMenuPath.empty()) {
            address.gtkBusName = source.gtkBusName;
            address.gtkMenuPath = source.gtkMenuPath;
            address.gtkWindowPath = source.gtkWindowPath;
            address.gtkApplicationPath = source.gtkApplicationPath;
        }
    }
    if (address == m_menuAddress)
        return;
    m_menuAddress = address;
    for (auto *observer : std::vector<SurfaceObserver *>(m_observers)) {
        if (hasSurfaceObserver(observer))
            observer->onSurfaceMenuChanged(this);
    }
}

} // namespace Lingmo::Compositor
