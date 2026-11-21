// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "render/egl_fence.h"

#include <EGL/eglext.h>
#include <GLES3/gl3.h>

#include "core/logging.h"

namespace Lingmo::Render {

namespace {

class ScopedEglSync
{
public:
    ScopedEglSync(EGLDisplay display, EGLSyncKHR sync, void *destroy)
        : m_display(display)
        , m_sync(sync)
        , m_destroy(reinterpret_cast<PFNEGLDESTROYSYNCKHRPROC>(destroy))
    {
    }

    ~ScopedEglSync()
    {
        if (m_sync != EGL_NO_SYNC_KHR)
            m_destroy(m_display, m_sync);
    }

    ScopedEglSync(const ScopedEglSync &) = delete;
    ScopedEglSync &operator=(const ScopedEglSync &) = delete;

    EGLSyncKHR get() const { return m_sync; }

private:
    EGLDisplay m_display;
    EGLSyncKHR m_sync;
    PFNEGLDESTROYSYNCKHRPROC m_destroy;
};

} // namespace

EglFence::EglFence(EglContext &context)
    : m_context(context)
{
    if (!m_context.supports("EGL_KHR_fence_sync")
        || !m_context.supports("EGL_ANDROID_native_fence_sync")) {
        LM_INFO(lingmoRender) << "EGL has no native fence sync; explicit sync falls back to CPU waits";
        return;
    }

    m_createSync = reinterpret_cast<void *>(eglGetProcAddress("eglCreateSyncKHR"));
    m_destroySync = reinterpret_cast<void *>(eglGetProcAddress("eglDestroySyncKHR"));
    m_dupNativeFence = reinterpret_cast<void *>(eglGetProcAddress("eglDupNativeFenceFDANDROID"));
    // The server-side wait is what keeps the compositor thread out of the
    // acquire path; without it every wait has to block on the CPU.
    m_waitSyncServer = reinterpret_cast<void *>(eglGetProcAddress("eglWaitSyncKHR"));
    if (!m_createSync || !m_destroySync || !m_dupNativeFence || !m_waitSyncServer) {
        m_createSync = m_destroySync = m_dupNativeFence = m_waitSyncServer = nullptr;
        LM_INFO(lingmoRender) << "EGL fence entry points are missing; explicit sync uses CPU waits";
    }
}

void EglFence::wait(const SyncFence &fence, bool inFrame, int timeoutMs)
{
    if (!fence.isValid())
        return;

    if (m_createSync && m_waitSyncServer && inFrame) {
        // eglCreateSyncKHR takes ownership of the fd on success, so hand it a
        // duplicate and keep the caller's fence intact.
        SyncFence duplicate = fence.duplicate();
        if (duplicate.isValid()) {
            EGLint attribs[] = {EGL_SYNC_NATIVE_FENCE_FD_ANDROID, duplicate.get(), EGL_NONE};
            auto createSync = reinterpret_cast<PFNEGLCREATESYNCKHRPROC>(m_createSync);
            ScopedEglSync sync(
                    m_context.display(),
                    createSync(m_context.display(), EGL_SYNC_NATIVE_FENCE_ANDROID, attribs),
                    m_destroySync);
            if (sync.get() != EGL_NO_SYNC_KHR) {
                duplicate.release();
                auto waitSync = reinterpret_cast<PFNEGLWAITSYNCKHRPROC>(m_waitSyncServer);
                waitSync(m_context.display(), sync.get(), 0);
                return;
            }
        }
    }

    fence.wait(timeoutMs);
}

SyncFence EglFence::release(bool inFrame)
{
    if (!m_createSync || !m_dupNativeFence || !inFrame)
        return SyncFence();

    static const EGLint attribs[] = {EGL_NONE};
    auto createSync = reinterpret_cast<PFNEGLCREATESYNCKHRPROC>(m_createSync);
    ScopedEglSync sync(m_context.display(),
                       createSync(m_context.display(), EGL_SYNC_NATIVE_FENCE_ANDROID, attribs),
                       m_destroySync);
    if (sync.get() == EGL_NO_SYNC_KHR)
        return SyncFence();

    // The fence fd only becomes available once the commands are in the driver.
    glFlush();
    auto dupFence = reinterpret_cast<PFNEGLDUPNATIVEFENCEFDANDROIDPROC>(m_dupNativeFence);
    const int fd = dupFence(m_context.display(), sync.get());

    if (fd == EGL_NO_NATIVE_FENCE_FD_ANDROID)
        return SyncFence();
    return SyncFence(ScopedFd(fd));
}

} // namespace Lingmo::Render
