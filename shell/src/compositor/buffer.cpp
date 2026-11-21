// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/buffer.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: integrates the upstream behavior with native
// compositor types and ownership.

#include "compositor/buffer.h"

#include <utility>

#include "compositor/shm_pool.h"
#include "core/logging.h"

namespace Lingmo::Compositor {

Buffer::Buffer() = default;

Buffer::~Buffer()
{
    // A buffer destroyed mid-frame still owes its releases; the client would
    // otherwise wait on a point nobody can signal any more.
    signalReleases(render::SyncFence());
}

void Buffer::setReleaseCallback(std::function<void()> callback)
{
    m_releaseCallback = std::move(callback);
}

void Buffer::setFaultCallback(std::function<void()> callback)
{
    m_faultCallback = std::move(callback);
}

void Buffer::reportFault()
{
    // Once is enough: the client is being disconnected.
    if (auto callback = std::move(m_faultCallback))
        callback();
}

void Buffer::onAttach(bool cached)
{
    ++m_attachCount;
    if (cached)
        ++m_cachedAttachCount;
    m_contentsInUse = true;
    m_released = false;
}

void Buffer::onDetach(bool cached)
{
    if (m_attachCount == 0) {
        LM_WARNING(lingmoAurora) << "Buffer detached more often than attached";
        return;
    }
    --m_attachCount;
    if (cached)
        --m_cachedAttachCount;
    // Nothing will consume contents that are no longer on any surface.
    if (m_attachCount == 0)
        m_contentsInUse = false;
    maybeRelease();
}

void Buffer::onContentsConsumed(render::SyncFence fence)
{
    m_contentsInUse = false;
    addConsumedFence(std::move(fence));
    maybeRelease();
}

void Buffer::addRelease(std::shared_ptr<BufferRelease> release)
{
    if (!release)
        return;
    // A client re-attaching the buffer it already shows sets a new release
    // point each time; the ones before it on the same timeline would otherwise
    // pile up until the buffer is finally released.
    if (const void* timeline = release->timeline()) {
        std::vector<std::shared_ptr<BufferRelease>> superseded;
        for (auto it = m_releases.begin(); it != m_releases.end();) {
            if ((*it)->timeline() == timeline
                && (*it)->timelinePoint() <= release->timelinePoint()) {
                superseded.push_back(std::move(*it));
                it = m_releases.erase(it);
            } else {
                ++it;
            }
        }
        for (const auto& old : superseded)
            old->signal(render::SyncFence());
    }
    m_releases.push_back(std::move(release));
}

void Buffer::onScanoutAcquired()
{
    ++m_scanoutCount;
}

void Buffer::onScanoutReleased(render::SyncFence fence)
{
    if (m_scanoutCount == 0) {
        LM_WARNING(lingmoAurora) << "Buffer left scanout more often than it entered it";
        return;
    }
    --m_scanoutCount;
    addConsumedFence(std::move(fence));
    if (m_scanoutCount == 0)
        maybeRelease();
}

void Buffer::signalReleases(const render::SyncFence& fence)
{
    // Move first: signalling may destroy the client objects behind these and
    // re-enter here through the buffer's own teardown.
    auto releases = std::move(m_releases);
    m_releases.clear();
    for (const auto& release : releases)
        release->signal(fence.duplicate());
}

void Buffer::addConsumedFence(render::SyncFence fence)
{
    // A buffer that stays on a plane is handed an out-fence on every flip, so
    // keep one fd: a signalled fence adds nothing, pending ones are merged.
    if (fence.isSignaled())
        return;
    std::vector<render::SyncFence> fences;
    if (!m_consumedFence.isSignaled())
        fences.push_back(std::move(m_consumedFence));
    fences.push_back(std::move(fence));
    m_consumedFence = render::SyncFence::merge(std::move(fences));
}

void Buffer::maybeRelease()
{
    if (m_released || m_contentsInUse || m_cachedAttachCount || m_scanoutCount)
        return;
    if (m_attachCount > 0 && retainsClientStorage())
        return;

    m_released = true;
    // Explicit sync first: a client using it may never look at wl_buffer.release.
    render::SyncFence completion = std::exchange(m_consumedFence, render::SyncFence());
    signalReleases(completion);
    m_acquireFence.reset();
    if (m_releaseCallback)
        m_releaseCallback();
}

ShmBuffer::ShmBuffer(std::shared_ptr<ShmPool> pool, const base::Size& size, int32_t offset,
                     int32_t stride, render::PixelFormat format)
    : m_pool(std::move(pool))
    , m_size(size)
    , m_offset(offset)
    , m_stride(stride)
    , m_format(format)
{
}

ShmBuffer::~ShmBuffer() = default;

const uint8_t* ShmBuffer::pixels() const
{
    const uint8_t* base = m_pool->data();
    return base ? base + m_offset : nullptr;
}

void ShmBuffer::beginPixelAccess()
{
    m_pool->beginAccess();
}

bool ShmBuffer::endPixelAccess()
{
    if (m_pool->endAccess())
        return true;
    reportFault();
    return false;
}

} // namespace Lingmo::Compositor
