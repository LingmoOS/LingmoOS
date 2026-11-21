// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/buffer.h
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: the gpu::/viz:: texture production machinery is gone.
// What is kept is the attach/detach/release lifecycle.

#ifndef LINGMO_COMPOSITOR_BUFFER_H_
#define LINGMO_COMPOSITOR_BUFFER_H_

#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

#include "base/geometry.h"

#include "render/scanout.h"
#include "render/sync_fence.h"
#include "render/texture.h"

namespace Lingmo::Compositor {

class ShmPool;

// The explicit-synchronization counterpart of wl_buffer.release: the compositor
// signals it once it has finished reading the buffer for one commit.
//
// Signalling must happen exactly once and must not be skipped, or the client
// waits forever. Every path that drops a release - the surface going away, a
// failed commit, a discarded frame - signals it with whatever fence it has,
// including none, which means "already finished".
class BufferRelease
{
public:
    virtual ~BufferRelease() = default;

    // `fence` becomes signalled when the read actually completes. An invalid
    // fence means the read is already done.
    virtual void signal(render::SyncFence fence) = 0;

    // The timeline this release is a point on, and the point, for a release
    // that is one; null for the others. A client waits on the newest point it
    // set for a buffer, so an older point on the same timeline is done the
    // moment a newer one takes its place.
    virtual const void* timeline() const { return nullptr; }
    virtual uint64_t timelinePoint() const { return 0; }
};

// The compositor-side view of a wl_buffer.
//
// Release means the compositor no longer needs the client's storage, never
// merely that a surface committed. Two things have to be true before
// wl_buffer.release is sent:
//
//   * the contents have been consumed (copied, or scanned out for the last
//     time), and
//   * the buffer is no longer the committed content of any surface, unless
//     the compositor does not retain client storage at all.
//
// The second condition is what retainsClientStorage() relaxes: shm contents are
// copied into a GL texture, so the client may reuse its storage as soon as that
// copy has been issued, even while the buffer is still on screen.
class Buffer
{
public:
    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;
    virtual ~Buffer();

    virtual base::Size size() const = 0;
    virtual render::PixelFormat format() const = 0;

    // Read-only mapping of the buffer contents, or nullptr for buffers the
    // renderer samples directly. Valid only until the buffer is released.
    virtual const uint8_t* pixels() const = 0;
    virtual int stride() const = 0;

    // Brackets every read of pixels(). endPixelAccess() returns false when the
    // client's storage vanished mid-read: what was read is zeros, and the
    // fault callback has already told the protocol layer.
    virtual void beginPixelAccess() {}
    virtual bool endPixelAccess() { return true; }

    // A texture the renderer can sample the buffer through without an upload,
    // or nullptr for buffers whose pixels() have to be copied. Imported once
    // per wl_buffer, so later commits of the same buffer cost nothing.
    virtual std::shared_ptr<render::Texture> texture() const { return nullptr; }

    // Whether the compositor still needs the client's storage once the contents
    // have been consumed.
    virtual bool retainsClientStorage() const { return true; }

    // Non-null for buffers a display controller may be able to scan out
    // directly. Shm buffers never are: their pixels live in client memory the
    // compositor has to copy.
    virtual render::ScanoutBuffer* scanoutBuffer() { return nullptr; }

    // Whether a GPU fence can say when the contents are ready and when the
    // compositor is done with them. Shm contents are copied out on the CPU.
    virtual bool supportsExplicitSync() const { return false; }

    // The fences of client GPU work still writing the contents, as the kernel
    // tracks them on the storage. Empty when nothing is pending or the kernel
    // cannot tell; a commit is held until these signal.
    virtual std::vector<render::SyncFence> implicitFences() const { return {}; }

    // Sends wl_buffer.release. Set by the protocol layer, which owns the buffer
    // through the wl_buffer resource.
    void setReleaseCallback(std::function<void()> callback);

    // Set like the release callback. Invoked when the client's storage turned
    // out not to exist while being read, which is a protocol violation the
    // protocol layer answers by disconnecting the client.
    void setFaultCallback(std::function<void()> callback);

    // Called when the buffer becomes, and stops being, the committed content of
    // a surface.
    // Cached attachments cannot be released by another surface's upload.
    void onAttach(bool cached = false);
    void onDetach(bool cached = false);

    // Called once the compositor has taken everything it needs out of the
    // client's storage for the current attachment. `fence`, when valid, is
    // when the GPU or the display controller actually finishes the read.
    void onContentsConsumed(render::SyncFence fence = render::SyncFence());

    // --- Explicit synchronization. ---

    // The fence the compositor must wait for before reading this buffer, set
    // at commit from the surface's acquire point. Invalid means the buffer is
    // ready now, which is also the implicit-sync case.
    const render::SyncFence& acquireFence() const { return m_acquireFence; }
    void setAcquireFence(render::SyncFence fence) { m_acquireFence = std::move(fence); }

    // Adds a release the compositor owes for the current attachment. The same
    // buffer may be committed to several surfaces, so these accumulate.
    void addRelease(std::shared_ptr<BufferRelease> release);

    size_t pendingReleaseCount() const { return m_releases.size(); }

    // Held by the display controller while the buffer is committed to a
    // hardware plane. Release waits for this in addition to detach, or a client
    // that reattached would get its storage back while KMS still displays it.
    void onScanoutAcquired();
    void onScanoutReleased(render::SyncFence fence);

    uint32_t attachCount() const { return m_attachCount; }

    bool contentsInUse() const { return m_contentsInUse; }

    // False between onAttach() and the matching release.
    bool isReleased() const { return m_released; }

protected:
    Buffer();

    void reportFault();

private:
    void maybeRelease();
    // Signals and drops every owed release. Safe to call more than once.
    void signalReleases(const render::SyncFence& fence);
    void addConsumedFence(render::SyncFence fence);

    std::function<void()> m_releaseCallback;
    std::function<void()> m_faultCallback;
    render::SyncFence m_acquireFence;
    std::vector<std::shared_ptr<BufferRelease>> m_releases;
    // The fences handed to onContentsConsumed() and onScanoutReleased(), kept
    // until the buffer is actually released so a late detach still reports the
    // right completion. Signalled ones are dropped and pending ones merged.
    render::SyncFence m_consumedFence;
    uint32_t m_scanoutCount = 0;
    uint32_t m_attachCount = 0;
    uint32_t m_cachedAttachCount = 0;
    bool m_contentsInUse = false;
    bool m_released = true;
};

// A buffer backed by a range of a wl_shm_pool.
class ShmBuffer : public Buffer
{
public:
    ShmBuffer(std::shared_ptr<ShmPool> pool, const base::Size& size, int32_t offset, int32_t stride,
              render::PixelFormat format);
    ~ShmBuffer() override;

    base::Size size() const override { return m_size; }

    render::PixelFormat format() const override { return m_format; }

    const uint8_t* pixels() const override;

    int stride() const override { return m_stride; }

    void beginPixelAccess() override;
    bool endPixelAccess() override;

    // The pixels are copied into a compositor-owned texture, so the client can
    // have its storage back as soon as the copy has been issued.
    bool retainsClientStorage() const override { return false; }

    int32_t offset() const { return m_offset; }

    const ShmPool* pool() const { return m_pool.get(); }

private:
    std::shared_ptr<ShmPool> m_pool;
    base::Size m_size;
    int32_t m_offset = 0;
    int32_t m_stride = 0;
    render::PixelFormat m_format = render::PixelFormat::Bgrx8888;
};

} // namespace Lingmo::Compositor

#endif // LINGMO_COMPOSITOR_BUFFER_H_
