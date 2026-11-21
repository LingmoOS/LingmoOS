// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/wayland/test/integration/buffer_checker_test.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: allocates each advertised format and modifier with GBM
// and imports it through the renderer the compositor samples client buffers with.

#include <GLES3/gl3.h>
#include <drm_fourcc.h>
#include <fcntl.h>
#include <gbm.h>
#include <gtest/gtest.h>
#include <cstdlib>
#include <unistd.h>

#include <cstdio>
#include <memory>

#include "render/egl_renderer.h"
#include "base/geometry.h"

using namespace Lingmo;

namespace {

struct BoDeleter {
    void operator()(gbm_bo *bo) const { gbm_bo_destroy(bo); }
};

using Bo = std::unique_ptr<gbm_bo, BoDeleter>;

Bo allocate(gbm_device *device, uint32_t format, uint64_t modifier)
{
    constexpr uint32_t kWidth = 64;
    constexpr uint32_t kHeight = 48;
    if (modifier == DRM_FORMAT_MOD_INVALID)
        return Bo(gbm_bo_create(device, kWidth, kHeight, format, GBM_BO_USE_RENDERING));
    return Bo(gbm_bo_create_with_modifiers2(device, kWidth, kHeight, format, &modifier, 1,
                                            GBM_BO_USE_RENDERING));
}

// Every plane, as a client would send them in zwp_linux_buffer_params_v1.
render::DmaBufAttributes attributesFor(gbm_bo *bo, uint32_t format, uint64_t modifier)
{
    render::DmaBufAttributes attributes;
    attributes.size = base::Size(int(gbm_bo_get_width(bo)), int(gbm_bo_get_height(bo)));
    attributes.format = format;
    attributes.modifier = modifier == DRM_FORMAT_MOD_INVALID ? modifier : gbm_bo_get_modifier(bo);
    for (int plane = 0; plane < gbm_bo_get_plane_count(bo); ++plane) {
        render::DmaBufPlane dmaBufPlane;
        dmaBufPlane.fd = ScopedFd(gbm_bo_get_fd_for_plane(bo, plane));
        dmaBufPlane.offset = gbm_bo_get_offset(bo, plane);
        dmaBufPlane.stride = gbm_bo_get_stride_for_plane(bo, plane);
        attributes.planes.push_back(std::move(dmaBufPlane));
    }
    return attributes;
}

} // namespace

// Exo's CanUseAnyReportedBufferFormats and CanUseAnyReportedBufferModifiers:
// whatever the compositor advertises and the GPU can allocate must import and
// draw, or a client following the advertisement gets params.failed.
TEST(DmaBufFormatGbm, EveryAdvertisedFormatThatAllocatesImportsAndDraws)
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
            gbm_surface_create(device.get(), 128, 128, DRM_FORMAT_XRGB8888,
                               GBM_BO_USE_RENDERING | GBM_BO_USE_SCANOUT),
            gbm_surface_destroy);
    ASSERT_TRUE(surface);
    auto target = renderer->createTarget(surface.get(), {128, 128});
    ASSERT_TRUE(target);

    const render::DmaBufFormats &formats = renderer->dmaBufFormats();
    ASSERT_FALSE(formats.empty());
    int allocated = 0, advertised = 0;
    for (const auto &[format, modifiers] : formats) {
        for (uint64_t modifier : modifiers) {
            ++advertised;
            const Bo bo = allocate(device.get(), format, modifier);
            // Formats GBM cannot allocate, such as most YUV layouts, are the
            // client's own problem, as exo counts them as unsupported usages.
            if (!bo)
                continue;
            ++allocated;
            SCOPED_TRACE(testing::Message() << "format " << std::hex << format << " modifier "
                                            << modifier);
            const render::DmaBufAttributes attributes = attributesFor(bo.get(), format, modifier);
            auto texture = renderer->importDmaBuf(attributes);
            ASSERT_TRUE(texture);
            EXPECT_EQ(base::Size(64, 48), texture->size());

            renderer->setOutputContext({"gbm", base::Rect(0, 0, 128, 128), {128, 128}, 1, 0});
            ASSERT_TRUE(renderer->beginFrame(target.get()));
            renderer->clear(render::Color {0, 0, 0, 1});
            static_cast<render::Renderer &>(*renderer).drawTexture(texture.get(),
                                                                   base::Rect(10, 10, 64, 48));
            EXPECT_EQ(GLenum(GL_NO_ERROR), glGetError());
            ASSERT_TRUE(renderer->endFrame());
            if (auto *front = gbm_surface_lock_front_buffer(surface.get()))
                gbm_surface_release_buffer(surface.get(), front);
            texture.reset();
        }
    }
    std::printf("renderer=%s advertised=%d allocated=%d\n", renderer->glesRenderer().c_str(),
                advertised, allocated);
    EXPECT_GT(allocated, 0);

    glFinish();
    renderer->releaseTarget(target.get());
    target.reset();
    renderer.reset();
    eglReleaseThread();
}
