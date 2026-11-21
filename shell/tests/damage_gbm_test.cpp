// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <GLES3/gl3.h>
#include <drm_fourcc.h>
#include <fcntl.h>
#include <gbm.h>
#include <gtest/gtest.h>
#include <cstdlib>
#include <unistd.h>

#include "render/egl_renderer.h"
#include "scene/scene.h"
#include "base/geometry.h"
#include "ui/bitmap.h"

using namespace Lingmo;

TEST(DamageGbm, RealBufferAgeMatchesCompleteFrames)
{
    const char *devicePath = std::getenv("LINGMO_TEST_DRM_DEVICE");
    if (!devicePath || !*devicePath)
        GTEST_SKIP() << "Set LINGMO_TEST_DRM_DEVICE to an accessible DRM device";
    const int fd = open(devicePath, O_RDWR | O_CLOEXEC);
    ASSERT_GE(fd, 0);

    struct Close {
        int fd;

        ~Close() { close(fd); }
    } closeFd {fd};

    std::unique_ptr<gbm_device, decltype(&gbm_device_destroy)> device(gbm_create_device(fd),
                                                                      gbm_device_destroy);
    ASSERT_TRUE(device);
    auto renderer = render::EglRenderer::create(device.get(), DRM_FORMAT_XRGB8888);
    ASSERT_TRUE(renderer);
    std::unique_ptr<gbm_surface, decltype(&gbm_surface_destroy)> surface(
            gbm_surface_create(device.get(), 256, 256, DRM_FORMAT_XRGB8888,
                               GBM_BO_USE_RENDERING | GBM_BO_USE_SCANOUT),
            gbm_surface_destroy);
    ASSERT_TRUE(surface);
    auto target = renderer->createTarget(surface.get(), {256, 256});
    ASSERT_TRUE(target);
    scene::Scene scene;
    scene.setSize({256, 256});
    auto *node =
            scene.root()->addChild(std::make_unique<scene::ColorNode>(render::Color {1, 0, 0, 1}));
    auto *overlay = scene.root()->addChild(
            std::make_unique<scene::ColorNode>(render::Color {0, 1, 0, 0.5}));
    overlay->setGeometry({-25, 25, 80, 80});

    struct FrontBuffer {
        gbm_surface *surface;
        gbm_bo *buffer = nullptr;

        ~FrontBuffer()
        {
            if (buffer)
                gbm_surface_release_buffer(surface, buffer);
        }
    } held {surface.get()};

    int partialFrames = 0;
    for (double scale : {1.0, 1.25, 1.5, 1.75}) {
        for (uint32_t rotation = 0; rotation < 4; ++rotation) {
            for (int frame = 0; frame < 8; ++frame) {
                node->setGeometry({-30 + frame * 3, 20 + frame, 20 + frame, 25});
                renderer->setOutputContext(
                        {"gbm", base::Rect(-40, 10, 256, 256), {256, 256}, scale, rotation});
                ASSERT_TRUE(renderer->beginFrame(target.get()));
                scene.render(*renderer);
                GLint scissor[4];
                glGetIntegerv(GL_SCISSOR_BOX, scissor);
                partialFrames += scissor[2] * scissor[3] < 256 * 256;
                ui::Bitmap partial(256, 256, ui::Bitmap::Format::Rgba8888Premultiplied);
                glReadPixels(0, 0, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, partial.bits());
                partial = partial.mirrored(false, true).converted(ui::Bitmap::Format::Argb32Premultiplied);
                const auto complete = renderer->captureFrame([&] {
                    scene.render(*renderer);
                });
                // Virgl resolves half-pixel edge ties differently on GBM and texture FBOs.
                glDisable(GL_SCISSOR_TEST);
                renderer->clear(scene.background());
                scene.root()->render(*renderer);
                ui::Bitmap full(256, 256, ui::Bitmap::Format::Rgba8888Premultiplied);
                glReadPixels(0, 0, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, full.bits());
                full = full.mirrored(false, true).converted(ui::Bitmap::Format::Argb32Premultiplied);
                EXPECT_EQ(partial, full)
                        << "scale=" << scale << " rotation=" << rotation << " frame=" << frame;
                EXPECT_EQ(complete, renderer->captureFrame([&] {
                    scene.render(*renderer);
                }));
                ASSERT_TRUE(renderer->endFrame());
                auto *front = gbm_surface_lock_front_buffer(surface.get());
                ASSERT_NE(front, nullptr);
                if (held.buffer)
                    gbm_surface_release_buffer(surface.get(), held.buffer);
                held.buffer = front;
            }
        }
    }
    const char *list = eglQueryString(renderer->eglDisplay(), EGL_EXTENSIONS);
    const std::string extensions = list ? list : "";
    if (render::hasExtension(extensions, "EGL_EXT_buffer_age"))
        EXPECT_GT(partialFrames, 0);
    std::printf("GBM renderer=%s partial=%d/128 extensions=%s\n",
                renderer->glesRenderer().c_str(), partialFrames, extensions.c_str());
    glFinish();
    if (held.buffer) {
        gbm_surface_release_buffer(surface.get(), held.buffer);
        held.buffer = nullptr;
    }
    renderer->releaseTarget(target.get());
    target.reset();
    renderer.reset();
    eglReleaseThread();
}
