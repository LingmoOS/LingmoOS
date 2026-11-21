// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cstdlib>
#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <gtest/gtest.h>

#include <drm_fourcc.h>
#include <fcntl.h>
#include <gbm.h>
#include <unistd.h>

#include <algorithm>
#include <functional>
#include <memory>
#include <string>

#include "render/egl_render_target.h"
#include "render/egl_renderer.h"
#include "base/geometry.h"

using namespace Lingmo;

namespace {

class CaptureDmaBuf : public testing::Test
{
protected:
    void SetUp() override
    {
        const char *devicePath = std::getenv("LINGMO_TEST_DRM_DEVICE");
        if (!devicePath || !*devicePath)
            GTEST_SKIP() << "Set LINGMO_TEST_DRM_DEVICE to an accessible DRM device";
        m_fd = open(devicePath, O_RDWR | O_CLOEXEC);
        ASSERT_GE(m_fd, 0);
        m_device.reset(gbm_create_device(m_fd));
        ASSERT_TRUE(m_device);
        m_renderer = render::EglRenderer::create(m_device.get(), DRM_FORMAT_XRGB8888);
        ASSERT_TRUE(m_renderer);
        const auto &formats = m_renderer->captureDmaBufFormats();
        if (formats.empty())
            GTEST_SKIP() << "This GPU cannot render captures into DMA-BUFs";
        EXPECT_NE(m_renderer->captureDmaBufDevice(), 0u);
        ASSERT_TRUE(formats.contains(DRM_FORMAT_ARGB8888));

        m_surface.reset(gbm_surface_create(m_device.get(), 64, 48, DRM_FORMAT_XRGB8888,
                                           GBM_BO_USE_RENDERING | GBM_BO_USE_SCANOUT));
        ASSERT_TRUE(m_surface);
        m_target = m_renderer->createTarget(m_surface.get(), {64, 48});
        ASSERT_TRUE(m_target);

        // The client's buffer, allocated the way the portal does.
        m_bo.reset(gbm_bo_create(m_device.get(), 64, 48, DRM_FORMAT_ARGB8888,
                                 GBM_BO_USE_RENDERING | GBM_BO_USE_LINEAR));
        ASSERT_TRUE(m_bo);
        m_attributes.size = base::Size(64, 48);
        m_attributes.format = DRM_FORMAT_ARGB8888;
        m_attributes.modifier = gbm_bo_get_modifier(m_bo.get());
        render::DmaBufPlane plane;
        plane.fd = ScopedFd(gbm_bo_get_fd(m_bo.get()));
        plane.stride = gbm_bo_get_stride(m_bo.get());
        ASSERT_GE(plane.fd.get(), 0);
        m_attributes.planes.push_back(std::move(plane));
    }

    void TearDown() override
    {
        m_target.reset();
        m_renderer.reset();
        m_bo.reset();
        m_surface.reset();
        m_device.reset();
        if (m_fd >= 0)
            close(m_fd);
    }

    bool capture(const std::function<void()> &paint)
    {
        const render::OutputRenderContext context {"test", base::Rect(0, 0, 64, 48), {64, 48}, 1, 0,
                                                   1, base::Rect(0, 0, 64, 48)};
        m_renderer->setOutputContext(context);
        if (!m_renderer->beginFrame(m_target.get()))
            return false;
        const bool rendered =
                m_renderer->captureToDmaBuf(context, base::Size(64, 48), m_attributes, paint);
        // The compositor hands a frame over only after its presentation, by
        // which time the GPU has finished; here nothing waits for it but this.
        glFinish();
        m_renderer->cancelFrame();
        return rendered;
    }

    // Reads the buffer back the way a consumer does: through a fresh import of
    // the same DMA-BUF. A CPU map is no oracle on virgl, whose guest copy is
    // not refreshed for another context's rendering; a consumer samples the
    // host copy directly. Texture row 0 is the first row in memory, so
    // glReadPixels y counts memory rows from the top of the image.
    std::function<std::string(int, int)> readback()
    {
        auto createImage = reinterpret_cast<PFNEGLCREATEIMAGEKHRPROC>(
                eglGetProcAddress("eglCreateImageKHR"));
        auto imageTargetTexture = reinterpret_cast<PFNGLEGLIMAGETARGETTEXTURE2DOESPROC>(
                eglGetProcAddress("glEGLImageTargetTexture2DOES"));
        EXPECT_TRUE(createImage && imageTargetTexture);
        const EGLint attribs[] = {EGL_WIDTH,
                                  64,
                                  EGL_HEIGHT,
                                  48,
                                  EGL_LINUX_DRM_FOURCC_EXT,
                                  EGLint(DRM_FORMAT_ARGB8888),
                                  EGL_DMA_BUF_PLANE0_FD_EXT,
                                  m_attributes.planes[0].fd.get(),
                                  EGL_DMA_BUF_PLANE0_OFFSET_EXT,
                                  0,
                                  EGL_DMA_BUF_PLANE0_PITCH_EXT,
                                  EGLint(m_attributes.planes[0].stride),
                                  EGL_NONE};
        m_image = createImage(m_renderer->eglDisplay(), EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT,
                              nullptr, attribs);
        EXPECT_NE(m_image, EGL_NO_IMAGE_KHR);
        glGenTextures(1, &m_texture);
        glBindTexture(GL_TEXTURE_2D, m_texture);
        imageTargetTexture(GL_TEXTURE_2D, m_image);
        glGenFramebuffers(1, &m_framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture,
                               0);
        EXPECT_EQ(glCheckFramebufferStatus(GL_FRAMEBUFFER), GL_FRAMEBUFFER_COMPLETE);
        return [](int x, int row) {
            uint8_t rgba[4] = {0, 0, 0, 0};
            glReadPixels(x, row, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
            return std::string(reinterpret_cast<const char *>(rgba), 4);
        };
    }

    void releaseReadback()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDeleteFramebuffers(1, &m_framebuffer);
        glDeleteTextures(1, &m_texture);
        reinterpret_cast<PFNEGLDESTROYIMAGEKHRPROC>(eglGetProcAddress("eglDestroyImageKHR"))(
                m_renderer->eglDisplay(), m_image);
    }

    int m_fd = -1;
    std::unique_ptr<gbm_device, decltype(&gbm_device_destroy)> m_device {nullptr,
                                                                         gbm_device_destroy};
    std::unique_ptr<render::EglRenderer> m_renderer;
    std::unique_ptr<gbm_surface, decltype(&gbm_surface_destroy)> m_surface {nullptr,
                                                                            gbm_surface_destroy};
    std::unique_ptr<render::EglRenderTarget> m_target;
    std::unique_ptr<gbm_bo, decltype(&gbm_bo_destroy)> m_bo {nullptr, gbm_bo_destroy};
    render::DmaBufAttributes m_attributes;
    EGLImageKHR m_image = EGL_NO_IMAGE_KHR;
    GLuint m_texture = 0;
    GLuint m_framebuffer = 0;
};

} // namespace

// Renders a capture straight into a client-style DMA-BUF and reads the buffer
// back through GBM: the rows must come out top-down, as a PipeWire consumer
// expects, without any readback on the compositor side.
TEST_F(CaptureDmaBuf, RendersTopDownIntoALinearBuffer)
{
    ASSERT_TRUE(capture([&] {
        m_renderer->clear({0, 0, 1, 1});
        // Red at the top-left corner, green at the bottom-right corner.
        m_renderer->fillRect(base::Rect(0, 0, 8, 8), {1, 0, 0, 1});
        m_renderer->fillRect(base::Rect(56, 40, 8, 8), {0, 1, 0, 1});
    }));

    const auto at = readback();
    EXPECT_EQ(at(2, 2), std::string("\xff\x00\x00\xff", 4));
    EXPECT_EQ(at(61, 45), std::string("\x00\xff\x00\xff", 4));
    EXPECT_EQ(at(32, 24), std::string("\x00\x00\xff\xff", 4));
    releaseReadback();
}

// A consumer reuses its buffers, so the copy must replace every pixel: what
// the buffer held before may not show through transparent or translucent
// parts of the capture.
TEST_F(CaptureDmaBuf, TransparentPixelsReplaceTheOldContents)
{
    // Leave the previous frame in the buffer: opaque magenta everywhere.
    ASSERT_TRUE(capture([&] { m_renderer->clear({1, 0, 1, 1}); }));

    ASSERT_TRUE(capture([&] {
        m_renderer->clear({0, 0, 0, 0});
        m_renderer->fillRect(base::Rect(0, 0, 8, 8), {1, 0, 0, 1});
        m_renderer->fillRect(base::Rect(56, 40, 8, 8), {0, 1, 0, 0.5f});
    }));

    const auto at = readback();
    EXPECT_EQ(at(2, 2), std::string("\xff\x00\x00\xff", 4));
    EXPECT_EQ(at(32, 24), std::string("\x00\x00\x00\x00", 4));
    // Premultiplied half green, with no magenta underneath.
    const std::string translucent = at(61, 45);
    EXPECT_EQ(uint8_t(translucent[0]), 0);
    EXPECT_NEAR(uint8_t(translucent[1]), 128, 1);
    EXPECT_EQ(uint8_t(translucent[2]), 0);
    EXPECT_NEAR(uint8_t(translucent[3]), 128, 1);
    releaseReadback();
}

// Consumers that negotiate modifiers (Sunshine, OBS) never offer INVALID; they
// meet the compositor on an explicit LINEAR and allocate with it.
TEST_F(CaptureDmaBuf, AcceptsAnExplicitLinearModifier)
{
    const auto &formats = m_renderer->captureDmaBufFormats();
    const auto &modifiers = formats.at(DRM_FORMAT_ARGB8888);
    ASSERT_NE(std::find(modifiers.begin(), modifiers.end(), DRM_FORMAT_MOD_LINEAR),
              modifiers.end());

    const uint64_t linear = DRM_FORMAT_MOD_LINEAR;
    m_bo.reset(gbm_bo_create_with_modifiers2(m_device.get(), 64, 48, DRM_FORMAT_ARGB8888,
                                             &linear, 1, GBM_BO_USE_RENDERING));
    ASSERT_TRUE(m_bo);
    ASSERT_EQ(gbm_bo_get_modifier(m_bo.get()), DRM_FORMAT_MOD_LINEAR);
    m_attributes.modifier = DRM_FORMAT_MOD_LINEAR;
    m_attributes.planes[0].fd = ScopedFd(gbm_bo_get_fd(m_bo.get()));
    m_attributes.planes[0].stride = gbm_bo_get_stride(m_bo.get());

    ASSERT_TRUE(capture([&] {
        m_renderer->clear({0, 0, 1, 1});
        m_renderer->fillRect(base::Rect(0, 0, 8, 8), {1, 0, 0, 1});
    }));

    const auto at = readback();
    EXPECT_EQ(at(2, 2), std::string("\xff\x00\x00\xff", 4));
    EXPECT_EQ(at(32, 24), std::string("\x00\x00\xff\xff", 4));
    releaseReadback();
}
