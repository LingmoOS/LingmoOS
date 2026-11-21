// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <gtest/gtest.h>
#include <utility>
#include <vector>

#include "compositor/compositor.h"
#include "compositor/shm_pool.h"
#include "compositor/surface.h"
#include "decoration/decoration.h"
#include "decoration/decoration_theme.h"
#include "decoration/shadow.h"
#include "decoration/shadow_node.h"
#include "render/egl_renderer.h"
#include "scene/scene.h"
#include "scene/window_animation.h"
#include "shm_test_util.h"
#include "ui/bitmap.h"
#include "ui/path.h"
#include "wm/shadow_controller.h"
#include "wm/shadow_types.h"
#include "wm/shell_surface.h"
#include "wm/window.h"
#include "wm/window_manager.h"
#include "wm/wm_shadow_controller_delegate.h"

#include "egl_test_util.h"

using namespace Lingmo;
using namespace Lingmo::Decoration;

namespace {

class ShadowPixelTest : public testing::Test
{
protected:
    static constexpr int width = 900;
    static constexpr int height = 650;
    std::unique_ptr<render::EglRenderer> renderer;
    std::unique_ptr<render::EglRenderTarget> target;

    virtual int alphaSize() const { return 8; }

    void SetUp() override
    {
        renderer = render::EglRendererTestAccess::create(alphaSize());
        ASSERT_TRUE(renderer);
        const EGLint attributes[] = {EGL_WIDTH, width, EGL_HEIGHT, height, EGL_NONE};
        const EGLSurface surface =
                eglCreatePbufferSurface(renderer->eglDisplay(), renderer->eglConfig(), attributes);
        ASSERT_NE(surface, EGL_NO_SURFACE);
        target = std::make_unique<render::EglRenderTarget>(renderer->eglDisplay(), surface, nullptr,
                                                           base::Size(width, height));
        ASSERT_TRUE(renderer->beginFrame(target.get()));
    }

    void TearDown() override
    {
        if (renderer) {
            renderer->clearTextureCache();
            renderer->releaseTarget(target.get());
        }
        target.reset();
        renderer.reset();
    }

    std::vector<uint8_t> pixels()
    {
        std::vector<uint8_t> result(width * height * 4);
        glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, result.data());
        EXPECT_EQ(glGetError(), GLenum(GL_NO_ERROR));
        return result;
    }

    int channel(const std::vector<uint8_t> &image, int x, int y, int c = 0)
    {
        return image[((height - 1 - y) * width + x) * 4 + c];
    }

    // `texture` read back top-down at `size`, one texel to a pixel.
    ui::Bitmap readBack(render::Texture *texture, const base::Size &size)
    {
        return renderer->renderToImage(size, [&] {
            renderer->drawTexture(texture, base::Rect(size), 1);
        });
    }

    // The nine-patch source `geometry` renders.
    ui::Bitmap shadowImage(const ShadowGeometry &geometry)
    {
        const auto texture = geometry.render(*renderer);
        return texture ? readBack(texture.get(), geometry.rasterSize()) : ui::Bitmap();
    }
};

// A tall popup with an arrow under its bottom edge.
ui::Path arrowedPopup(const base::Size &size)
{
    const double body = size.height() - 9;
    ui::Path path;
    path.addRoundedRect(ui::RectF {0, 0, double(size.width()), body}, 10, 10);
    ui::Path arrow;
    arrow.moveTo(size.width() / 2.0 - 10, body - 1);
    arrow.lineTo(size.width() / 2.0 + 10, body - 1);
    arrow.lineTo(size.width() / 2.0, size.height());
    arrow.close();
    return path.united(arrow);
}

TEST_F(ShadowPixelTest, SlicesSampleTheRenderedSourceAtBothScalesAndTinySizes)
{
    Metrics metrics;
    for (int scale : {1, 2}) {
        for (const base::Size size :
             {base::Size(1, 1), base::Size(2, 31), base::Size(41, 3), base::Size(73, 91),
              base::Size(600, 400)}) {
            SCOPED_TRACE(::testing::Message()
                         << size.width() << 'x' << size.height() << '@' << scale);
            ShadowGeometry geometry(size, WindowVisualStyle::radiusFor(size),
                                    metrics.shadow(true), scale);
            const ui::Bitmap image = shadowImage(geometry);
            ASSERT_EQ(image.size(), geometry.rasterSize());
            renderer->clear({0, 0, 0, 0});
            const base::Point origin(100, 100);
            paintShadow(*renderer, geometry, origin);
            const auto actual = pixels();
            // Explicit pixel comparison includes every seam, empty middle and
            // each edge/corner. Integer downsampling averages physical texels;
            // past a stretched cut the neighbouring slice matches to a level.
            for (int i = 0; i < 9; ++i) {
                const base::Rect destination = geometry.targets()[i];
                const base::Rect source = geometry.sources()[i];
                for (int y = 0; y < destination.height(); ++y) {
                    for (int x = 0; x < destination.width(); ++x) {
                        int expected = 0;
                        if (i != 4 && !source.isEmpty()) {
                            const double u = source.x()
                                    + (x + 0.5) * source.width() / destination.width() - 0.5;
                            const double v = source.y()
                                    + (y + 0.5) * source.height() / destination.height() - 0.5;
                            const int x0 = std::clamp(int(std::floor(u)), source.left(),
                                                      source.right() - 1);
                            const int x1 = std::clamp(int(std::floor(u)) + 1, source.left(),
                                                      source.right() - 1);
                            const int y0 = std::clamp(int(std::floor(v)), source.top(),
                                                      source.bottom() - 1);
                            const int y1 = std::clamp(int(std::floor(v)) + 1, source.top(),
                                                      source.bottom() - 1);
                            const auto alpha = [&](int px, int py) {
                                return image.alpha(px, py);
                            };
                            const double fx = u - std::floor(u), fy = v - std::floor(v);
                            expected = std::lround(
                                    (1 - fy) * ((1 - fx) * alpha(x0, y0) + fx * alpha(x1, y0))
                                    + fy * ((1 - fx) * alpha(x0, y1) + fx * alpha(x1, y1)));
                        }
                        const int px = origin.x() + destination.x() + x;
                        const int py = origin.y() + destination.y() + y;
                        ASSERT_NEAR(channel(actual, px, py, 3), expected, 1) << px << ',' << py;
                        ASSERT_EQ(channel(actual, px, py), 0);
                    }
                }
            }
        }
    }
}

TEST_F(ShadowPixelTest, AShadowStretchedFromANearbySizeStaysCloseToTheExactOne)
{
    Metrics metrics;
    const base::Point origin(100, 100);
    for (int scale : {1, 2}) {
        for (const auto &[size, near] : {std::pair {base::Size(120, 135), base::Size(120, 150)},
                                         std::pair {base::Size(90, 60), base::Size(100, 66)}}) {
            SCOPED_TRACE(::testing::Message()
                         << size.width() << 'x' << size.height() << '@' << scale);
            const ShadowGeometry exact(size, 10, metrics.shadow(true), scale);
            renderer->clear({0, 0, 0, 0});
            paintShadow(*renderer, exact, origin);
            const auto expected = pixels();
            // Over its own size it is the plain draw.
            renderer->clear({0, 0, 0, 0});
            paintStretchedShadow(*renderer, exact, base::Rect(origin, size));
            EXPECT_EQ(pixels(), expected);

            renderer->clear({0, 0, 0, 0});
            const ShadowGeometry nearby(near, 10, metrics.shadow(true), scale);
            paintStretchedShadow(*renderer, nearby, base::Rect(origin, size));
            const auto actual = pixels();
            // The window covers what lies under it.
            int worst = 0;
            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    if (!base::Rect(origin, size).contains(base::Point(x, y)))
                        worst = std::max(worst, std::abs(channel(actual, x, y, 3)
                                                         - channel(expected, x, y, 3)));
                }
            }
            EXPECT_LE(worst, 5);
        }
    }
}

TEST_F(ShadowPixelTest, ActualWindowSceneClearsOldCoverageAndPreservesTranslucentInterior)
{
    scene::Scene scene;
    scene.setSize({width, height});
    scene.setBackground({1, 1, 1, 1});
    compositor::Compositor compositor(scene.root());
    decoration::Theme theme;
    decoration::DecorationProvider provider(&theme);
    wm::WindowManager manager(&compositor, nullptr);
    manager.setFrameProvider(&provider);
    manager.setOutputBounds({0, 0, width, height});
    decoration::ShadowProvider shadowProvider(&theme);
    wm::WmShadowControllerDelegate shadowDelegate;
    wm::ShadowController shadowController(&manager, &shadowProvider, &shadowDelegate);
    auto surface = compositor.createSurface();
    auto *window =
            manager.createWindow(surface.get(), {.requestedSize = {300, 200}, .decorated = true});
    window->setBounds({100, 100, 302, 231});
    window->setVisible(true);
    window->activate();
    auto tint = std::make_unique<scene::ColorNode>(render::Color {1, 0, 0, 0.5f});
    tint->setGeometry({1, 30, 300, 200});
    window->node()->addChild(std::move(tint));
    for (auto *node : compositor.surfaceContainer()->children()) {
        if (auto *animation = dynamic_cast<scene::WindowAnimation *>(node))
            animation->finish();
    }
    scene.render(*renderer);
    const auto active = pixels();
    EXPECT_LT(channel(active, 250, 341), 250);
    EXPECT_EQ(channel(active, 250, 200), 255);
    EXPECT_NEAR(channel(active, 250, 200, 1), 127, 1); // No black under client.
    window->setBounds(window->bounds().translated(400, 0));
    for (auto *node : compositor.surfaceContainer()->children()) {
        if (auto *animation = dynamic_cast<scene::WindowAnimation *>(node))
            animation->finish();
    }
    scene.render(*renderer);
    auto moved = pixels();
    EXPECT_EQ(channel(moved, 250, 341), 255);
    EXPECT_LT(channel(moved, 650, 341), 250);
    window->minimize();
    for (auto *node : compositor.surfaceContainer()->children()) {
        if (auto *animation = dynamic_cast<scene::WindowAnimation *>(node))
            animation->finish();
    }
    scene.render(*renderer);
    auto hidden = pixels();
    EXPECT_EQ(channel(hidden, 650, 341), 255);
    window->restore();
    window->activate();
    for (auto *node : compositor.surfaceContainer()->children()) {
        if (auto *animation = dynamic_cast<scene::WindowAnimation *>(node))
            animation->finish();
    }
    scene.render(*renderer);
    EXPECT_LT(channel(pixels(), 650, 341), 250);
    manager.destroyWindow(window);
    for (auto *node : compositor.surfaceContainer()->children()) {
        if (auto *animation = dynamic_cast<scene::WindowAnimation *>(node))
            animation->finish();
    }
    scene.render(*renderer);
    auto removed = pixels();
    EXPECT_EQ(channel(removed, 650, 341), 255);
    theme.clearTextures();
}

} // namespace

TEST_F(ShadowPixelTest, RendererRetiresGpuTexturesWithCacheAndOutputTarget)
{
    const ShadowStyle style {{{1, {}, 0.2f}, {0, {}, 0.1f}}};
    ShadowGeometry geometry({300, 200}, 0, style);
    paintShadow(*renderer, geometry, {100, 100});
    std::vector<GLuint> names;
    for (const auto &texture : renderer->cachedTextureSet(geometry.key(), [] {
             return render::Renderer::TextureSet {};
         })) {
        if (texture) {
            const GLuint name = static_cast<render::GlTexture *>(texture.get())->name();
            names.push_back(name);
            EXPECT_TRUE(glIsTexture(name));
        }
    }
    ASSERT_EQ(names.size(), 1u);
    for (int i = 1; i <= 16; ++i) {
        auto variant = style;
        variant[0].offset = {i, 0};
        paintShadow(*renderer, ShadowGeometry({300, 200}, 0, variant), {100, 100});
    }
    EXPECT_EQ(renderer->textureCacheSize(), render::Renderer::textureCacheLimit);
    for (GLuint name : names)
        EXPECT_FALSE(glIsTexture(name));
    renderer->releaseTarget(target.get());
    EXPECT_EQ(renderer->textureCacheSize(), 0u);
    ASSERT_TRUE(renderer->beginFrame(target.get()));
    const auto misses = renderer->textureCacheMisses();
    paintShadow(*renderer, geometry, {100, 100});
    EXPECT_EQ(renderer->textureCacheMisses(), misses + 1);
    EXPECT_EQ(glGetError(), GLenum(GL_NO_ERROR));
}

TEST_F(ShadowPixelTest, ActiveInactiveAndTransientOverlapUseWindowStacking)
{
    scene::Scene scene;
    scene.setSize({width, height});
    scene.setBackground({0.88f, 0.90f, 0.94f, 1});
    compositor::Compositor compositor(scene.root());
    decoration::Theme theme;
    decoration::DecorationProvider provider(&theme);
    wm::WindowManager manager(&compositor, nullptr);
    manager.setFrameProvider(&provider);
    manager.setOutputBounds({0, 0, width, height});
    decoration::ShadowProvider shadowProvider(&theme);
    wm::WmShadowControllerDelegate shadowDelegate;
    wm::ShadowController shadowController(&manager, &shadowProvider, &shadowDelegate);
    auto lowerSurface = compositor.createSurface();
    auto upperSurface = compositor.createSurface();
    auto *lower = manager.createWindow(
            lowerSurface.get(),
            {.requestedSize = {600, 440}, .title = "Inactive window", .decorated = true});
    auto *upper = manager.createWindow(
            upperSurface.get(),
            {.requestedSize = {300, 200}, .title = "Active window", .decorated = true});
    lower->setBounds({80, 80, 602, 471});
    upper->setBounds({240, 180, 302, 231});
    const auto addContent = [](wm::Window *window, const render::Color &color) {
        auto node = std::make_unique<scene::ColorNode>(color);
        node->setGeometry(window->contentBounds().translated(-window->bounds().topLeft()));
        window->node()->addChild(std::move(node));
    };
    addContent(lower, {1, 0.94f, 0.88f, 1});
    addContent(upper, {0.92f, 0.96f, 1, 1});
    ASSERT_TRUE(upper->setTransientParent(lower));
    upper->activate();
    for (auto *node : compositor.surfaceContainer()->children()) {
        if (auto *animation = dynamic_cast<scene::WindowAnimation *>(node))
            animation->finish();
    }
    scene.render(*renderer);
    auto image = pixels();
    EXPECT_LT(channel(image, 390, 421), 250); // Upper shadow darkens lower content.
    EXPECT_EQ(channel(image, 390, 280, 2), 255); // Upper interior has no shadow fill.
    EXPECT_NEAR(channel(image, 150, 280), 255, 1); // Lower interior away from upper shadow.
    lower->activate();
    for (auto *node : compositor.surfaceContainer()->children()) {
        if (auto *animation = dynamic_cast<scene::WindowAnimation *>(node))
            animation->finish();
    }
    scene.render(*renderer);
    const auto parentRaised = pixels();
    // Transient windows and their shadows stay above an activated parent.
    EXPECT_EQ(manager.windows().back(), upper);
    EXPECT_EQ(channel(parentRaised, 390, 280, 2), 255);
    EXPECT_LT(channel(parentRaised, 390, 421), 250);
    EXPECT_GT(channel(parentRaised, 390, 421), channel(image, 390, 421));

    ASSERT_TRUE(upper->setTransientParent(nullptr));
    manager.raise(lower);
    scene.render(*renderer);
    const auto unrelatedParentRaised = pixels();
    EXPECT_EQ(manager.windows().back(), lower);
    EXPECT_NEAR(channel(unrelatedParentRaised, 390, 280, 2), 224, 1);
    EXPECT_EQ(channel(unrelatedParentRaised, 390, 421), 255);
    manager.raise(upper); // Change order without changing the activation state.
    for (auto *node : compositor.surfaceContainer()->children()) {
        if (auto *animation = dynamic_cast<scene::WindowAnimation *>(node))
            animation->finish();
    }
    scene.render(*renderer);
    auto inactive = pixels();
    EXPECT_EQ(channel(inactive, 390, 280, 2), 255);
    EXPECT_GT(channel(inactive, 390, 421), channel(image, 390, 421));
    upper->activate();
    for (auto *node : compositor.surfaceContainer()->children()) {
        if (auto *animation = dynamic_cast<scene::WindowAnimation *>(node))
            animation->finish();
    }
    scene.render(*renderer);
    image = pixels();
    const char *artifact = std::getenv("LINGMO_SHADOW_TEST_IMAGE");
    if (artifact && *artifact) {
        const ui::Bitmap bottomUp = ui::Bitmap::copyOf(image.data(), width, height, width * 4,
                                                       ui::Bitmap::Format::Rgba8888Premultiplied);
        EXPECT_TRUE(bottomUp.mirrored(false, true).savePng(artifact));
    }
    manager.destroyWindow(upper);
    manager.destroyWindow(lower);
    theme.clearTextures();
}

TEST_F(ShadowPixelTest, OffscreenSnapshotPreservesOrientationAlphaAndFramebuffer)
{
    std::unique_ptr<render::Texture> snapshot;
    const base::Rect bounds(100, 80, 40, 40);
    ASSERT_TRUE(renderer->renderToTexture(snapshot, bounds, [&] {
        renderer->fillRect({100, 80, 40, 20}, {1, 0, 0, .5f});
        renderer->fillRect({100, 100, 40, 20}, {0, 0, 1, 1});
    }));
    const auto name = static_cast<render::GlTexture *>(snapshot.get())->name();
    renderer->clear({0, 0, 0, 0});
    renderer->drawTexture(snapshot.get(), bounds, 1);
    const auto image = pixels();
    EXPECT_NEAR(channel(image, 110, 90, 0), 128, 1);
    EXPECT_NEAR(channel(image, 110, 90, 3), 128, 1);
    EXPECT_EQ(channel(image, 110, 110, 2), 255);
    EXPECT_EQ(channel(image, 110, 110, 3), 255);
    EXPECT_EQ(channel(image, 10, 10, 3), 0);
    ASSERT_TRUE(renderer->renderToTexture(snapshot, bounds, [&] {
        renderer->fillRect(bounds, {0, 1, 0, 1});
    }));
    EXPECT_EQ(static_cast<render::GlTexture *>(snapshot.get())->name(), name);
    renderer->drawTexture(snapshot.get(), bounds, .5f);
    EXPECT_EQ(glGetError(), GLenum(GL_NO_ERROR));
    snapshot.reset();
    EXPECT_FALSE(glIsTexture(name));
}

TEST_F(ShadowPixelTest, MinimizeLandingMatchesTheExportedThumbnailPixels)
{
    for (const double scale : {1.0, 1.25, 2.0}) {
        SCOPED_TRACE(scale);
        auto context = renderer->outputContext();
        context.scale = scale;
        context.logicalViewport = base::Rect(0, 0, base::roundToInt(width / scale),
                                                     base::roundToInt(height / scale));
        context.nativeContentRect = base::Rect(0, 0, width, height);
        renderer->setOutputContext(context);

        int64_t now = 0;
        scene::WindowAnimation animation([&] {
            return now;
        });
        const base::Rect bounds(20, 20, 300, 200), slot(101, 231, 53, 35);
        animation.setGeometry(bounds);
        auto shadow = std::make_unique<scene::ColorNode>(render::Color {0, 0, 0, .4f});
        shadow->setGeometry(base::Rect(base::Point(), bounds.size()).adjusted(-8, -8, 8, 8));
        animation.addChild(std::move(shadow));

        auto visual = std::make_unique<scene::WindowVisualNode>();
        scene::WindowVisualState state;
        state.shape = {base::RectF(base::SizeF(bounds.size())), 22};
        state.clipContent = true;
        visual->setState(state);
        for (int x = 0; x < bounds.width(); ++x) {
            const render::Color color = x % 2 ? render::Color {1, .2f, 0, .6f}
                                              : render::Color {0, .3f, 1, .4f};
            auto stripe = std::make_unique<scene::ColorNode>(color);
            stripe->setGeometry({x, 0, 1, bounds.height()});
            visual->addChild(std::move(stripe));
        }
        animation.addChild(std::move(visual));
        animation.finish();
        animation.render(*renderer);
        animation.setVisible(false);
        animation.setHoldAtEnd(true);
        animation.start(scene::WindowAnimation::Type::Minimize, bounds,
                        slot);
        ui::Bitmap image;
        animation.requestSnapshot(
                [&](const ui::Bitmap &snapshot) {
                    image = snapshot;
                },
                384);
        animation.render(*renderer);
        now = scene::WindowAnimation::duration(scene::WindowAnimation::Type::Minimize,
                                               bounds, slot);
        renderer->clear({0, 0, 0, 0});
        animation.render(*renderer);
        const auto landed = pixels();
        ASSERT_EQ(image.size(), base::Size(int(std::ceil(slot.width() * scale)),
                                      int(std::ceil(slot.height() * scale))));
        ASSERT_TRUE(animation.isHolding());

        // Model the dock's unmodified image upload and linear sampling, including alpha edges.
        const ui::Bitmap rgba = image.converted(ui::Bitmap::Format::Rgba8888Premultiplied);
        auto thumbnail = renderer->createTexture(rgba.size(), rgba.constBits(), int(rgba.stride()),
                                                 render::PixelFormat::Rgba8888);
        renderer->clear({0, 0, 0, 0});
        renderer->drawTextureF(thumbnail.get(), base::RectF(slot), 1);
        EXPECT_EQ(landed, pixels());
        const int centerAlpha = channel(landed, base::roundToInt(slot.center().x() * scale),
                                         base::roundToInt(slot.center().y() * scale), 3);
        EXPECT_GT(centerAlpha, 0);
        EXPECT_LT(centerAlpha, 255);
        now += 100;
        renderer->clear({0, 0, 0, 0});
        animation.render(*renderer);
        EXPECT_EQ(landed, pixels());
    }
}

TEST_F(ShadowPixelTest, RestoreStateOnlyCommitDoesNotPaintWhiteIntoDockArea)
{
    scene::Scene scene;
    scene.setBackground({0, 0, 1, 1});
    compositor::Compositor compositor(scene.root());
    wm::WindowManager manager(&compositor, nullptr);
    manager.setOutputBounds({0, 0, width, height - 100});
    auto surface = compositor.createSurface();
    wm::ShellSurface shell(&manager, surface.get());
    uint32_t serial = 0;
    shell.setConfigureCallback([&](const wm::ShellSurface::ConfigureState &) {
        return ++serial;
    });
    const int poolSize = 8 << 20;
    const int fd = testsupport::makeMemfd(poolSize);
    ASSERT_GE(fd, 0);
    const std::vector<uint8_t> white(poolSize, 255);
    EXPECT_EQ(pwrite(fd, white.data(), white.size(), 0), poolSize);
    auto pool = compositor::ShmPool::create(fd, poolSize);
    ASSERT_TRUE(pool);
    const auto commitBuffer = [&](base::Size size) {
        auto buffer = pool->createBuffer(size, 0, size.width() * 4,
                                         render::PixelFormat::Bgrx8888,
                                         nullptr);
        surface->attach(buffer, {});
        surface->damage(base::Rect(base::Point(), size));
        surface->commit();
        compositor.updateTextures(*renderer);
    };
    shell.setToplevel();
    surface->commit();
    ASSERT_TRUE(shell.acknowledgeConfigure(serial));
    commitBuffer({400, 300});
    shell.window()->setBounds({200, 150, 400, 300});
    auto *animation = dynamic_cast<scene::WindowAnimation *>(shell.window()->node());
    ASSERT_NE(animation, nullptr);
    animation->finish();
    scene.render(*renderer);
    shell.maximize();
    ASSERT_TRUE(shell.acknowledgeConfigure(serial));
    commitBuffer({width, height - 100});
    animation->finish();
    scene.render(*renderer);
    const auto maximized = pixels();
    ASSERT_EQ(channel(maximized, width / 2, height - 120, 0), 255);
    ASSERT_EQ(channel(maximized, width / 2, height - 20, 0), 0);

    shell.unmaximize();
    ASSERT_TRUE(shell.acknowledgeConfigure(serial));
    surface->commit();
    compositor.updateTextures(*renderer);
    scene.render(*renderer);
    const auto waiting = pixels();
    for (int y = height - 90; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            ASSERT_EQ(channel(waiting, x, y, 0), 0) << x << ',' << y;
            ASSERT_EQ(channel(waiting, x, y, 2), 255) << x << ',' << y;
        }
    }
    EXPECT_EQ(shell.window()->bounds(), base::Rect(200, 150, 400, 300));
    shell.setGeometry({0, 0, 400, 300});
    commitBuffer({width, height - 100}); // Render thread's old-size frame after ack.
    scene.render(*renderer);
    const auto stale = pixels();
    EXPECT_EQ(channel(stale, width / 2, height - 20, 0), 0);
    EXPECT_EQ(channel(stale, width / 2, height - 20, 2), 255);
    EXPECT_TRUE(animation->isWaitingForBoundsContent());
    commitBuffer({400, 300});
    scene.render(*renderer);
    const auto resumed = pixels();
    EXPECT_EQ(channel(resumed, width / 2, height - 20, 0), 0);
    EXPECT_EQ(channel(resumed, width / 2, height - 20, 2), 255);
    scene.root()->releaseRenderResources();
}

TEST_F(ShadowPixelTest, BackgroundBlurMatchesGaussianAndLeavesOutsideUntouched)
{
    renderer->clear({0, 0, 0, 1});
    renderer->fillRect({450, 0, 450, height}, {1, 1, 1, 1});
    renderer->drawBackgroundBlur({300, 100, 300, 400}, 24, {300, 100, 300, 400});
    const auto actual = pixels();
    EXPECT_EQ(channel(actual, 299, 300), 0);
    EXPECT_EQ(channel(actual, 601, 300), 255);
    EXPECT_NEAR(channel(actual, 450, 300), 128, 5);
    EXPECT_NEAR(channel(actual, 420, 300), 40, 5);
    EXPECT_NEAR(channel(actual, 480, 300), 215, 5);
    EXPECT_EQ(channel(actual, 450, 99), 255);
}

// Bare optics: no softening, no tint, no lighting, so the only thing under test
// is where the lens reads the backdrop from.
render::GlassParams bareGlass()
{
    render::GlassParams glass;
    glass.enabled = true;
    glass.blurSigma = 0;
    glass.profile = 1;
    glass.chroma = 0;
    glass.specularIntensity = 0;
    glass.sheen = 0;
    glass.ambientOcclusion = 0;
    glass.tintAlpha = 0;
    return glass;
}

TEST_F(ShadowPixelTest, GlassSofteningMatchesDiscreteGaussian)
{
    const base::Rect bounds(300, 100, 300, 400);
    auto glass = bareGlass();
    glass.displacement = 0;
    for (float sigma : {0.0f, 1.2f, 2.4f, 10.0f}) {
        glass.blurSigma = sigma;
        const float effectiveSigma = std::max(sigma, 0.05f);
        double weights[61];
        double total = 0;
        for (int i = -30; i <= 30; ++i) {
            weights[i + 30] = std::exp(-double(i * i) / (2 * effectiveSigma * effectiveSigma));
            total += weights[i + 30];
        }
        for (bool horizontal : {false, true}) {
            renderer->clear({0, 0, 0, 1});
            renderer->fillRect(horizontal ? base::Rect(450, 0, 450, height)
                                          : base::Rect(0, 300, width, height - 300),
                               {1, 1, 1, 1});
            renderer->drawBackgroundBlur(bounds, 24, bounds, glass);
            const auto actual = pixels();
            for (int offset = -32; offset <= 32; ++offset) {
                double expected = 0;
                for (int i = -30; i <= 30; ++i) {
                    // The glass shader also averages a subpixel footprint in its flat center.
                    const int position = offset + i;
                    const double coverage = position < -1 ? 0
                            : position == -1              ? 0.09375
                            : position == 0               ? 0.90625
                                                          : 1;
                    expected += weights[i + 30] * coverage;
                }
                // Past a sigma of 4 the blur runs at a reduced resolution,
                // whose bilinear resampling widens it slightly.
                EXPECT_NEAR(channel(actual, horizontal ? 450 + offset : 450,
                                    horizontal ? 300 : 300 + offset),
                            255 * expected / total, sigma > 4 ? 3 : 2)
                        << "sigma=" << sigma << " horizontal=" << horizontal
                        << " offset=" << offset;
            }
        }
    }
}

TEST_F(ShadowPixelTest, GlassRefractionReadsTheBackdropFromFurtherInside)
{
    const base::Rect bounds(300, 100, 300, 400);
    // A radius of half the width makes the whole lens a roll-off band.
    const int radius = 150;
    const auto backdrop = [&] {
        renderer->clear({0, 0, 0, 1});
        renderer->fillRect({450, 0, 450, height}, {1, 1, 1, 1});
    };
    render::GlassParams glass = bareGlass();
    // Zero falls back to the corner radius, making the whole lens a roll-off
    // band.
    glass.bevel = 0;
    glass.depth = 200;
    glass.displacement = 0;
    backdrop();
    renderer->drawBackgroundBlur(bounds, radius, bounds, glass);
    const auto straight = pixels();
    EXPECT_EQ(channel(straight, 445, 300), 0);
    EXPECT_EQ(channel(straight, 455, 300), 255);

    glass.displacement = 80;
    backdrop();
    renderer->drawBackgroundBlur(bounds, radius, bounds, glass);
    const auto refracted = pixels();
    // Each half of the lens bends towards the centre, so both sides sample
    // across the edge of the white half and swap brightness.
    EXPECT_EQ(channel(refracted, 445, 300), 255);
    EXPECT_EQ(channel(refracted, 455, 300), 0);
    EXPECT_EQ(channel(refracted, 299, 300), 0);

    // A surface barely raised off its back face is flat: no bend, whatever the
    // ray length.
    glass.depth = 0.05;
    backdrop();
    renderer->drawBackgroundBlur(bounds, radius, bounds, glass);
    const auto flat = pixels();
    EXPECT_EQ(channel(flat, 445, 300), 0);
    EXPECT_EQ(channel(flat, 455, 300), 255);
}

TEST_F(ShadowPixelTest, GlassTintsTheBodyAndLeavesTheEdgeToTheOutline)
{
    const base::Rect bounds(300, 100, 300, 400);
    renderer->clear({0.5, 0.5, 0.5, 1});
    render::GlassParams glass = bareGlass();
    glass.tintRed = 0;
    glass.tintGreen = 0;
    glass.tintBlue = 0;
    glass.tintAlpha = 0.5;
    renderer->drawBackgroundBlur(bounds, 24, bounds, glass);
    const auto tinted = pixels();
    // Half a black tint over a mid grey backdrop, and no edge treatment of its
    // own: the hairline around the body is the window outline's job.
    EXPECT_NEAR(channel(tinted, 450, 300), 64, 2);
    EXPECT_EQ(channel(tinted, 450, 101), channel(tinted, 450, 300));
    EXPECT_EQ(channel(tinted, 450, 99), 128);

    // Ambient occlusion is the one inside-the-edge effect, and it is opt-in.
    glass.ambientOcclusion = 0.5;
    glass.ambientOcclusionRadius = 6;
    renderer->clear({0.5, 0.5, 0.5, 1});
    renderer->drawBackgroundBlur(bounds, 24, bounds, glass);
    const auto occluded = pixels();
    EXPECT_LT(channel(occluded, 450, 101), channel(occluded, 450, 300) - 20);
    EXPECT_EQ(channel(occluded, 450, 110), channel(occluded, 450, 300));
}

TEST_F(ShadowPixelTest, GlassRimWrapsTheEdgeInADarkLineAndLightsTheTopInsideIt)
{
    const base::Rect bounds(300, 100, 300, 400);
    render::GlassParams glass = bareGlass();
    glass.rim = 1;
    renderer->clear({0.5, 0.5, 0.5, 1});
    renderer->drawBackgroundBlur(bounds, 24, bounds, glass);
    const auto rimmed = pixels();
    const int body = channel(rimmed, 450, 300);
    EXPECT_NEAR(body, 128, 1);
    // The dark hairline runs all the way round, lightest along the top and
    // bottom; on the sides it is a gain and a lift below the backdrop.
    EXPECT_LT(channel(rimmed, 450, 100), body - 5);
    EXPECT_LT(channel(rimmed, 450, 499), body - 5);
    EXPECT_NEAR(channel(rimmed, 300, 300), 102, 3);
    EXPECT_NEAR(channel(rimmed, 599, 300), 102, 3);
    // Just inside it, lit from above, so the bottom catches about a third as much.
    EXPECT_GT(channel(rimmed, 450, 101), 185);
    EXPECT_NEAR(channel(rimmed, 450, 498), 153, 4);
    // A short glow inside the lit edges only.
    EXPECT_GT(channel(rimmed, 450, 102), body);
    EXPECT_LT(channel(rimmed, 450, 102), 160);
    EXPECT_EQ(channel(rimmed, 450, 120), body);
    EXPECT_EQ(channel(rimmed, 301, 300), body);
    EXPECT_EQ(channel(rimmed, 450, 99), 128);
}

TEST_F(ShadowPixelTest, GlassRimCornerPutsTheLitHairlineInsideTheDarkOne)
{
    const base::Rect bounds(300, 100, 300, 400);
    render::GlassParams glass = bareGlass();
    glass.rim = 1;
    renderer->clear({0.5, 0.5, 0.5, 1});
    renderer->drawBackgroundBlur(bounds, 24, bounds, glass);
    const auto rimmed = pixels();
    const int body = channel(rimmed, 450, 300);
    // On the diagonal of the top left corner, the outermost pixel is darkened
    // and the next one in is lit.
    EXPECT_LT(channel(rimmed, 307, 107), body - 3);
    EXPECT_GT(channel(rimmed, 308, 108), body + 15);
}

TEST_F(ShadowPixelTest, GlassRimFadesOverANearlyWhiteBody)
{
    const base::Rect bounds(300, 100, 300, 400);
    render::GlassParams glass = bareGlass();
    glass.rim = 1;
    glass.tintRed = glass.tintGreen = glass.tintBlue = 0;
    glass.tintAlpha = 0.04f;
    renderer->clear({1, 1, 1, 1});
    renderer->drawBackgroundBlur(bounds, 24, bounds, glass);
    const auto rimmed = pixels();
    const int body = channel(rimmed, 450, 300);
    EXPECT_NEAR(body, 245, 1);
    // The glow inside the hairline would clip at white; it steps down to the
    // body right away.
    EXPECT_LE(channel(rimmed, 450, 102), body + 5);
    EXPECT_LE(channel(rimmed, 450, 104), body + 2);
}

TEST_F(ShadowPixelTest, GlassSaturationScalesTheBackdropAroundItsLuma)
{
    const base::Rect bounds(300, 100, 300, 400);
    render::GlassParams glass = bareGlass();
    glass.saturation = 0;
    renderer->clear({0.25, 0.5, 0.75, 1});
    renderer->drawBackgroundBlur(bounds, 24, bounds, glass);
    const auto grey = pixels();
    EXPECT_NEAR(channel(grey, 450, 300, 0), 119, 2);
    EXPECT_NEAR(channel(grey, 450, 300, 1), 119, 2);
    EXPECT_NEAR(channel(grey, 450, 300, 2), 119, 2);

    glass.saturation = 1.5;
    renderer->clear({0.25, 0.5, 0.75, 1});
    renderer->drawBackgroundBlur(bounds, 24, bounds, glass);
    const auto vivid = pixels();
    EXPECT_LT(channel(vivid, 450, 300, 0), 40);
    EXPECT_GT(channel(vivid, 450, 300, 2), 220);
    EXPECT_NEAR(channel(vivid, 450, 99, 0), 64, 1);
}

TEST_F(ShadowPixelTest, BlurInsideAnimationSnapshotReadsTheParentBackdrop)
{
    const base::Rect bounds(250, 150, 400, 350);
    const auto background = [&] {
        renderer->clear({0, 0, 0, 1});
        renderer->fillRect({450, 0, 450, height}, {1, 0, 0, 1});
        renderer->fillRect({0, 325, width, 325}, {0, 1, 0, 1});
    };
    background();
    renderer->drawBackgroundBlur(bounds, 20, bounds);
    const auto direct = pixels();
    background();
    std::unique_ptr<render::Texture> snapshot;
    ASSERT_TRUE(renderer->renderToTexture(snapshot, bounds.adjusted(-64, -64, 64, 64), [&] {
        renderer->drawBackgroundBlur(bounds, 20, bounds);
    }));
    renderer->drawTexture(snapshot.get(), bounds.adjusted(-64, -64, 64, 64), 1);
    const auto offscreen = pixels();
    for (const base::Point point :
         {base::Point(450, 250), base::Point(350, 325), base::Point(450, 325), base::Point(500, 450)}) {
        for (int c = 0; c < 4; ++c)
            EXPECT_NEAR(channel(direct, point.x(), point.y(), c),
                        channel(offscreen, point.x(), point.y(), c), 3);
    }
    EXPECT_GT(channel(offscreen, 450, 250, 0), 100);
    EXPECT_GT(channel(offscreen, 350, 325, 1), 100);
}

TEST_F(ShadowPixelTest, BlurClampsAtOutputEdgesAndRestoresScissorAndFramebuffer)
{
    renderer->clear({1, 0, 0, 1});
    glEnable(GL_SCISSOR_TEST);
    glScissor(0, height - 100, 100, 100);
    GLint before = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &before);
    renderer->drawBackgroundBlur({-20, -20, 150, 150}, 0, {-20, -20, 150, 150});
    GLint after = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &after);
    EXPECT_EQ(before, after);
    EXPECT_TRUE(glIsEnabled(GL_SCISSOR_TEST));
    GLint scissor[4];
    glGetIntegerv(GL_SCISSOR_BOX, scissor);
    EXPECT_EQ(scissor[2], 100);
    glDisable(GL_SCISSOR_TEST);
    const auto actual = pixels();
    EXPECT_EQ(channel(actual, 0, 0), 255);
    EXPECT_EQ(channel(actual, 99, 99), 255);
    renderer->fillRect({700, 500, 10, 10}, {0, 1, 0, 1});
    EXPECT_EQ(channel(pixels(), 705, 505, 1), 255);
}

// A frameless Lingmo window is one rounded rectangle, so its shadow has to be
// cut around a rounded bottom. Cutting a square one out instead leaves the
// bottom corners with a hard, completely unshadowed wedge between the client's
// arc and the cut-out.
TEST_F(ShadowPixelTest, BottomCornersAreShadowedOnlyWhenTheSilhouetteIsRounded)
{
    Metrics metrics;
    constexpr base::Size kWindow(400, 300);
    const int radius = WindowVisualStyle::radiusFor(kWindow);
    ASSERT_GT(radius, 2);

    const auto bottomLeftWedge = [&](const Corners &corners) {
        ShadowGeometry geometry(kWindow, radius, metrics.shadow(true), 1, corners);
        const ui::Bitmap image = shadowImage(geometry);
        const base::Point box(geometry.margins().left(),
                         geometry.margins().top() + geometry.canonicalSize().height() - 1);
        // Inside the corner's bounding box but outside its arc, so a rounded
        // silhouette leaves it to the shadow and a square one claims it.
        int total = 0;
        for (int i = 1; i <= 2; ++i)
            total += image.alpha(box.x() + i, box.y() - i);
        return total;
    };

    // FishUI's window corner radius. The compositor cannot ask a client for
    // its radius, so the two have to be kept the same number here.
    EXPECT_EQ(metrics.cornerRadius, 14);
    EXPECT_EQ(WindowVisualStyle::radiusFor(kWindow), metrics.cornerRadius);

    EXPECT_GT(bottomLeftWedge(Corners::all()), 0);
    // A deliberately square silhouette remains available to callers.
    EXPECT_EQ(bottomLeftWedge(Corners::top()), 0);
    // Two silhouettes of the same window are two different cached images.
    EXPECT_NE(ShadowGeometry(kWindow, radius, metrics.shadow(true), 1, Corners::all()).key(),
              ShadowGeometry(kWindow, radius, metrics.shadow(true), 1, Corners::top()).key());
}

// The cut-out is the window clip's own coverage, so wherever the window covers a
// pixel whole none of its shadow shows through a translucent client.
TEST_F(ShadowPixelTest, AllCornerShapeAndTransparentInterior)
{
    Metrics metrics;
    EXPECT_EQ(WindowVisualStyle::radiusFor({10, 40}), 5);
    EXPECT_EQ(WindowVisualStyle::radiusFor({100, 4}), 2);
    const int radius = WindowVisualStyle::radiusFor({200, 160});
    ShadowGeometry geometry({200, 160}, radius, metrics.shadow(true));
    const ui::Bitmap image = shadowImage(geometry);
    ASSERT_EQ(image.size(), geometry.rasterSize());
    const base::Rect box(base::Point(geometry.margins().left(), geometry.margins().top()),
                    geometry.canonicalSize());
    const ui::Bitmap shape = renderer->renderToImage(image.size(), [&] {
        renderer->pushRoundedClip({base::RectF(box), double(radius)});
        renderer->fillRect(box, {1, 1, 1, 1});
        renderer->popRoundedClip();
    });
    EXPECT_GT(image.alpha(box.x(), box.y()), 0); // Outside rounded top.
    EXPECT_GT(image.alpha(box.x(), box.bottom() - 1), 0); // Outside rounded bottom.
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            const uint32_t pixel = image.pixel(x, y);
            if (shape.alpha(x, y) == 255)
                ASSERT_EQ(pixel >> 24, 0u) << x << ',' << y;
            // Black, premultiplied.
            ASSERT_EQ(pixel & 0xffffff, 0u) << x << ',' << y;
        }
    }
}

TEST_F(ShadowPixelTest, ApertureCutsAreConstantForLinearFiltering)
{
    Metrics metrics;
    for (int scale : {1, 2}) {
        ShadowGeometry geometry({700, 500}, 11, metrics.shadow(true), scale);
        const ui::Bitmap image = shadowImage(geometry);
        ASSERT_EQ(image.size(), geometry.rasterSize());
        const base::Rect middle = geometry.sources()[4];
        const auto alpha = [&](int x, int y) {
            return image.alpha(x, y);
        };
        for (int y = 0; y < image.height(); ++y) {
            EXPECT_EQ(alpha(middle.left() - 1, y), alpha(middle.left(), y)) << y;
            EXPECT_EQ(alpha(middle.right() - 1, y), alpha(middle.right(), y)) << y;
        }
        for (int x = 0; x < image.width(); ++x) {
            EXPECT_EQ(alpha(x, middle.top() - 1), alpha(x, middle.top())) << x;
            EXPECT_EQ(alpha(x, middle.bottom() - 1), alpha(x, middle.bottom())) << x;
        }
    }
}

// Wide lobes are blurred on a halved copy; across a straight edge they have to
// stay within a few levels of the real Gaussian, and come out symmetric but for
// the rounding of the 8-bit passes, at most a level apiece.
TEST_F(ShadowPixelTest, WideLobesFollowTheGaussianAcrossAStraightEdge)
{
    for (const auto &[sigma, scale] : {std::pair {24.0f, 1}, {9.0f, 2}, {3.2f, 1}, {1.5f, 2}}) {
        SCOPED_TRACE(::testing::Message() << sigma << '@' << scale);
        const ShadowStyle style {{{sigma, {}, 1.0f}}};
        ShadowGeometry geometry({400, 300}, 0, style, scale);
        const ui::Bitmap image = shadowImage(geometry);
        ASSERT_EQ(image.size(), geometry.rasterSize());
        const int edge = geometry.margins().left() * scale;
        const int middle = image.height() / 2;
        const float deviceSigma = sigma * scale;
        for (int x = std::max(0, edge - int(3 * deviceSigma)); x < edge; ++x) {
            const double expected =
                    255 * 0.5 * std::erfc((edge - x - 0.5) / (deviceSigma * std::sqrt(2.0)));
            EXPECT_NEAR(image.alpha(x, middle), expected, 3) << x;
        }
        for (int y = 0; y < image.height(); ++y) {
            for (int x = 0; x < image.width() / 2; ++x)
                ASSERT_NEAR(image.alpha(x, y), image.alpha(image.width() - 1 - x, y),
                            2)
                        << x << ',' << y;
        }
    }
}

TEST_F(ShadowPixelTest, SilhouetteKeepsTheArrowAndItsStretchedRunsConstant)
{
    Metrics metrics;
    const base::Size size(160, 700);
    const auto style = metrics.shadowFor(wm::ShadowElevation::MenuOrTooltip);
    for (int scale : {1, 2}) {
        SCOPED_TRACE(scale);
        ShadowGeometry geometry(size, 0, style, scale, Corners::all(),
                                arrowedPopup(size));
        const ui::Bitmap image = shadowImage(geometry);
        ASSERT_EQ(image.size(), geometry.rasterSize());
        const auto alpha = [&](int x, int y) {
            return image.alpha(x, y);
        };
        const base::Rect middle = geometry.sources()[4];
        for (int x = 0; x < image.width(); ++x) {
            EXPECT_EQ(alpha(x, middle.top() - 1), alpha(x, middle.top())) << x;
            EXPECT_EQ(alpha(x, middle.bottom() - 1), alpha(x, middle.bottom())) << x;
        }
        const base::Point box(geometry.margins().left() * scale, geometry.margins().top() * scale);
        const int bottom = box.y() + geometry.canonicalSize().height() * scale;
        // Inside the arrow nothing; just past its tip more shadow than at the
        // same depth beside it, where the body edge is further away.
        EXPECT_EQ(alpha(box.x() + 80 * scale, bottom - 4 * scale), 0);
        EXPECT_GT(alpha(box.x() + 80 * scale, bottom + 2 * scale),
                  alpha(box.x() + 40 * scale, bottom + 2 * scale));
    }
}

// The caster is drawn with no clip and at the image's own scale, whatever the
// node being painted is under; both are the caller's again afterwards.
TEST_F(ShadowPixelTest, RenderingIgnoresAndKeepsTheCallersClipAndScale)
{
    Metrics metrics;
    ShadowGeometry geometry({300, 200}, 11, metrics.shadow(true), 2);
    const ui::Bitmap plain = shadowImage(geometry);
    ASSERT_FALSE(plain.isNull());

    renderer->clear({0, 0, 0, 0});
    renderer->setRasterScale(1.5);
    renderer->pushRoundedClip({base::RectF(0, 0, 10, 10), 5});
    const auto texture = geometry.render(*renderer);
    EXPECT_EQ(renderer->rasterScale(), 1.5);
    renderer->fillRect({0, 0, 40, 40}, {1, 0, 0, 1});
    renderer->popRoundedClip();
    renderer->setRasterScale(1);
    ASSERT_TRUE(texture);
    EXPECT_EQ(readBack(texture.get(), geometry.rasterSize()), plain);
    // Still clipped: the fill stayed inside the 10px circle at 1.5.
    const auto actual = pixels();
    EXPECT_EQ(channel(actual, 7, 7), 255);
    EXPECT_EQ(channel(actual, 20, 20), 0);
    EXPECT_EQ(glGetError(), GLenum(GL_NO_ERROR));
}

namespace {

class OpaqueBlurPixelTest : public ShadowPixelTest
{
    int alphaSize() const override { return 0; }
};

} // namespace

TEST_F(OpaqueBlurPixelTest, BackgroundBlurReadsAnXrgbOutput)
{
    renderer->clear({0, 0, 0, 1});
    renderer->fillRect({450, 0, 450, height}, {1, 1, 1, 1});
    renderer->drawBackgroundBlur({300, 100, 300, 400}, 24, {300, 100, 300, 400});
    const auto actual = pixels();
    EXPECT_NEAR(channel(actual, 450, 300), 128, 5);
    EXPECT_NEAR(channel(actual, 420, 300), 40, 5);
    EXPECT_NEAR(channel(actual, 480, 300), 215, 5);
}

TEST_F(OpaqueBlurPixelTest, WindowSnapshotCombinesOpaqueOutputAndTransparentLayers)
{
    renderer->clear({0, 0, 0, 1});
    renderer->fillRect({450, 0, 450, height}, {1, 1, 1, 1});
    const base::Rect bounds(300, 100, 300, 400);
    std::unique_ptr<render::Texture> snapshot;
    ASSERT_TRUE(renderer->renderToTexture(snapshot, bounds.adjusted(-64, -64, 64, 64), [&] {
        renderer->drawBackgroundBlur(bounds, 24, bounds);
    }));
    renderer->drawTexture(snapshot.get(), bounds.adjusted(-64, -64, 64, 64), 1);
    const auto actual = pixels();
    EXPECT_EQ(channel(actual, 299, 300), 0);
    EXPECT_EQ(channel(actual, 601, 300), 255);
    EXPECT_NEAR(channel(actual, 450, 300), 128, 5);
    EXPECT_NEAR(channel(actual, 420, 300), 40, 5);
    EXPECT_NEAR(channel(actual, 480, 300), 215, 5);
}
