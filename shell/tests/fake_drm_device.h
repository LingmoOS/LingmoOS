// Copyright 2016 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/ozone/platform/drm/gpu/fake_drm_device.h
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: a scripted DrmDeviceApi rather than a fake of the whole
// DRM ioctl surface. It records commits, can be told to fail a specific one,
// and hands out synthetic handles, which is what the plane planning, the atomic
// TEST_ONLY path and buffer lifetime need to be testable without a GPU.

#ifndef LINGMO_TESTS_FAKE_DRM_DEVICE_H_
#define LINGMO_TESTS_FAKE_DRM_DEVICE_H_

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <map>
#include <vector>

#include "backend/drm/drm_device_api.h"

namespace Lingmo::Testing {

class FakeDrmDevice final : public backend::DrmDeviceApi
{
public:
    struct Commit {
        backend::AtomicRequest request;
        uint32_t flags = 0;
        bool hadUserData = false;
        void *userData = nullptr;
    };

    int fd() const override { return -1; }

    gbm_device *gbmDevice() const override { return gbm; }

    // --- Framebuffers. ---

    bool addFramebuffer(const backend::FramebufferParams &params, uint32_t *framebufferId) override
    {
        addFramebufferCalls.push_back(params);
        if (addFramebufferFails)
            return false;
        *framebufferId = m_nextFramebufferId++;
        liveFramebuffers.push_back(*framebufferId);
        return true;
    }

    bool removeFramebuffer(uint32_t framebufferId) override
    {
        liveFramebuffers.erase(
                std::remove(liveFramebuffers.begin(), liveFramebuffers.end(), framebufferId),
                liveFramebuffers.end());
        return true;
    }

    bool exportCrtcFramebuffer(uint32_t crtcId, render::DmaBufAttributes *attributes) override
    {
        exportedCrtcs.push_back(crtcId);
        return crtcFramebuffer && crtcFramebuffer(crtcId, attributes);
    }

    // What the CRTC shows before this device's first commit, as another DRM
    // master left it. Unset, the CRTC shows nothing.
    std::function<bool(uint32_t, render::DmaBufAttributes *)> crtcFramebuffer;
    std::vector<uint32_t> exportedCrtcs;

    bool primeFdToHandle(int fd, uint32_t *handle) override
    {
        if (primeFails)
            return false;
        // The kernel hands out one handle per underlying buffer; keying on the
        // fd reproduces that, which is what the handle refcount has to survive.
        auto entry = m_handles.find(fd);
        if (entry == m_handles.end())
            entry = m_handles.emplace(fd, m_nextHandle++).first;
        *handle = entry->second;
        openHandles.push_back(entry->second);
        return true;
    }

    bool closeHandle(uint32_t handle) override
    {
        const auto it = std::find(openHandles.begin(), openHandles.end(), handle);
        if (it == openHandles.end())
            return false;
        openHandles.erase(it);
        closedHandles.push_back(handle);
        return true;
    }

    bool createPropertyBlob(const void *data, size_t size, uint32_t *id) override
    {
        if (blobFails)
            return false;
        *id = ++lastBlob;
        const auto *bytes = static_cast<const uint8_t *>(data);
        blobs[*id] = std::vector<uint8_t>(bytes, bytes + size);
        return true;
    }

    void destroyPropertyBlob(uint32_t id) override { destroyedBlobs.push_back(id); }

    // The blob's contents viewed as an array of T.
    template<typename T>
    std::vector<T> blobAs(uint32_t id) const
    {
        const auto blob = blobs.find(id);
        if (blob == blobs.end())
            return {};
        std::vector<T> values(blob->second.size() / sizeof(T));
        std::memcpy(values.data(), blob->second.data(), values.size() * sizeof(T));
        return values;
    }

    bool blobFails = false;
    uint32_t lastBlob = 0;
    std::map<uint32_t, std::vector<uint8_t>> blobs;
    std::vector<uint32_t> destroyedBlobs;

    // --- Atomic commits. ---

    bool atomicCommit(const backend::AtomicRequest &request, uint32_t flags,
                      void *userData) override
    {
        commits.push_back(Commit {request, flags, userData != nullptr, userData});
        if (commitFilter)
            return commitFilter(commits.back());
        return true;
    }

    // --- Cursor. ---

    bool setCursor(uint32_t crtcId, uint32_t handle, const base::Size &size,
                   const base::Point &hotspot) override
    {
        if (setCursorFails)
            return false;
        cursorCrtc = crtcId;
        cursorHandle = handle;
        cursorSize = size;
        cursorHotspot = hotspot;
        ++setCursorCalls;
        return true;
    }

    bool moveCursor(uint32_t, const base::Point &position) override
    {
        cursorPosition = position;
        ++moveCursorCalls;
        return true;
    }

    base::Size maxCursorSize() const override { return maxCursor; }

    bool createDumbBuffer(const base::Size &size, backend::DumbBufferInfo *info) override
    {
        if (dumbBufferFails)
            return false;
        info->handle = m_nextHandle++;
        info->stride = static_cast<uint32_t>(size.width() * 4);
        info->size = static_cast<uint64_t>(info->stride) * size.height();
        ++liveDumbBuffers;
        return true;
    }

    bool destroyDumbBuffer(uint32_t) override
    {
        --liveDumbBuffers;
        return true;
    }

    void *mapDumbBuffer(uint32_t handle, uint64_t size) override
    {
        void *data = std::calloc(1, size);
        dumbMappings[handle] = data;
        ++liveMappings;
        return data;
    }

    void unmapDumbBuffer(void *data, uint64_t) override
    {
        if (!data)
            return;
        if (beforeUnmap)
            beforeUnmap(data);
        std::erase_if(dumbMappings, [data](const auto &entry) {
            return entry.second == data;
        });
        std::free(data);
        --liveMappings;
    }

    // --- Scripting. ---

    // Returns false to fail a commit. Called for every commit, test and flip.
    std::function<bool(const Commit &)> commitFilter;
    bool addFramebufferFails = false;
    std::function<void(const void *)> beforeUnmap;
    bool primeFails = false;
    bool setCursorFails = false;
    bool dumbBufferFails = false;
    base::Size maxCursor {64, 64};
    // A real GBM device standing in for the renderer that shares this file.
    gbm_device *gbm = nullptr;

    // --- Recorded state. ---

    std::vector<Commit> commits;
    std::vector<backend::FramebufferParams> addFramebufferCalls;
    std::vector<uint32_t> liveFramebuffers;
    std::vector<uint32_t> openHandles;
    std::vector<uint32_t> closedHandles;
    uint32_t cursorCrtc = 0;
    uint32_t cursorHandle = 0;
    base::Size cursorSize;
    base::Point cursorHotspot;
    base::Point cursorPosition;
    int setCursorCalls = 0;
    int moveCursorCalls = 0;
    int liveDumbBuffers = 0;
    int liveMappings = 0;
    // What each mapped dumb buffer holds, by handle.
    std::map<uint32_t, void *> dumbMappings;

    // Commits that were validation only, and the ones that were real.
    size_t testCommitCount() const { return countCommits(true); }

    size_t realCommitCount() const { return countCommits(false); }

private:
    size_t countCommits(bool testOnly) const;

    std::map<int, uint32_t> m_handles;
    uint32_t m_nextFramebufferId = 100;
    uint32_t m_nextHandle = 1;
};

} // namespace Lingmo::Testing

#endif // LINGMO_TESTS_FAKE_DRM_DEVICE_H_
