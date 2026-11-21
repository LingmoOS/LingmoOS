// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <GLES3/gl3.h>
#include <gtest/gtest.h>

#include "base/geometry.h"
#include "compositor/compositor.h"
#include "compositor/sub_surface.h"
#include "compositor/surface.h"
#include "decoration/decoration.h"
#include "decoration/decoration_theme.h"
#include "decoration/shadow_node.h"
#include "egl_test_util.h"
#include "input/hit_test.h"
#include "scene/scene.h"
#include "scene/surface_node.h"
#include "scene/window_animation.h"
#include "shm_test_util.h"
#include "wm/shadow_controller.h"
#include "wm/shell_surface.h"
#include "wm/toplevel_event_handler.h"
#include "wm/window.h"
#include "wm/window_manager.h"
#include "wm/window_visual_controller.h"
#include "wm/wm_shadow_controller_delegate.h"
#include "ui/path.h"

using namespace Lingmo;
using Lingmo::base::roundToInt;
using Mode = scene::WindowVisualMode;

namespace {

struct WindowScene {
    scene::Scene scene;
    compositor::Compositor compositor {scene.root()};
    decoration::Theme theme;
    decoration::DecorationProvider frames {&theme};
    wm::WindowManager manager {&compositor, nullptr};
    decoration::ShadowProvider shadows {&theme};
    wm::WmShadowControllerDelegate delegate;
    wm::ShadowController shadowController {&manager, &shadows, &delegate};
    std::vector<std::shared_ptr<compositor::Buffer>> buffers;
    std::vector<std::unique_ptr<compositor::Surface>> surfaces;
    std::vector<std::unique_ptr<wm::ShellSurface>> shells;

    WindowScene()
    {
        scene.setSize({640, 480});
        manager.setOutputBounds({0, 0, 640, 480});
        manager.setFrameProvider(&frames);
        manager.setVisualStyle(theme.visualStyle());
    }

    compositor::Surface *surface()
    {
        surfaces.push_back(compositor.createSurface());
        return surfaces.back().get();
    }

    void attach(compositor::Surface *surface, base::Size size, render::Color color,
                base::Rect body = {})
    {
        std::vector<uint8_t> bytes(size.width() * size.height() * 4);
        for (size_t i = 0; i < bytes.size(); i += 4) {
            const base::Point position(int(i / 4) % size.width(), int(i / 4) / size.width());
            if (!body.isEmpty() && !body.contains(position))
                continue;
            bytes[i] = roundToInt(color.r * color.a * 255);
            bytes[i + 1] = roundToInt(color.g * color.a * 255);
            bytes[i + 2] = roundToInt(color.b * color.a * 255);
            bytes[i + 3] = roundToInt(color.a * 255);
        }
        const int fd = testsupport::makeMemfd(bytes.size());
        ASSERT_GE(fd, 0);
        ASSERT_EQ(pwrite(fd, bytes.data(), bytes.size(), 0), ssize_t(bytes.size()));
        auto pool = compositor::ShmPool::create(fd, bytes.size());
        buffers.push_back(pool->createBuffer(size, 0, size.width() * 4,
                                             render::PixelFormat::Rgba8888, nullptr));
        surface->attach(buffers.back(), {});
        surface->damage(base::Rect(base::Point(), size));
        surface->commit();
    }

    wm::Window *window(Mode mode, render::Color color, base::Size size = {160, 120},
                       bool exterior = false)
    {
        auto *root = surface();
        auto shell = std::make_unique<wm::ShellSurface>(&manager, root);
        shell->setToplevel();
        shell->setHasDecorationObject(true);
        shell->setRequestedDecorationMode(mode == Mode::ServerFrame
                                                  ? wm::ShellSurface::DecorationMode::ServerSide
                                                  : wm::ShellSurface::DecorationMode::ClientSide);
        if (exterior)
            shell->setGeometry({8, 8, size.width() - 16, size.height() - 16});
        attach(root, size, color);
        auto *window = shell->window();
        EXPECT_NE(window, nullptr);
        if (window) {
            window->setBounds({base::Point(80, 80), window->bounds().size()});
            window->activate();
            static_cast<scene::WindowAnimation *>(window->node())->finish();
        }
        shells.push_back(std::move(shell));
        return window;
    }
};

class WindowVisualPixelTest : public testing::Test
{
protected:
    static constexpr int width = 640, height = 480;
    std::unique_ptr<render::EglRenderer> renderer;
    std::unique_ptr<render::EglRenderTarget> target;

    void SetUp() override
    {
        renderer = render::EglRendererTestAccess::create();
        ASSERT_NE(renderer, nullptr);
        const EGLint attrs[] = {EGL_WIDTH, width, EGL_HEIGHT, height, EGL_NONE};
        const auto surface =
                eglCreatePbufferSurface(renderer->eglDisplay(), renderer->eglConfig(), attrs);
        ASSERT_NE(surface, EGL_NO_SURFACE);
        target = std::make_unique<render::EglRenderTarget>(renderer->eglDisplay(), surface, nullptr,
                                                           base::Size(width, height));
        ASSERT_TRUE(renderer->beginFrame(target.get()));
    }

    void TearDown() override
    {
        renderer->clearTextureCache();
        renderer->releaseTarget(target.get());
        target.reset();
        renderer.reset();
    }

    std::vector<uint8_t> pixels()
    {
        std::vector<uint8_t> image(width * height * 4);
        glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
        EXPECT_EQ(glGetError(), GLenum(GL_NO_ERROR));
        return image;
    }

    int pixel(const std::vector<uint8_t> &image, int x, int y, int channel = 0)
    {
        return image[((height - 1 - y) * width + x) * 4 + channel];
    }

    void paint(WindowScene &model, wm::Window *window, render::Color desktop = {0, 0, 0, 0})
    {
        model.compositor.updateTextures(*renderer);
        EXPECT_NE(window->rootSurface()->node()->texture(), nullptr);
        EXPECT_TRUE(window->rootSurface()->node()->isEffectivelyVisible());
        renderer->clear(desktop);
        window->node()->render(*renderer);
    }
};

TEST_F(WindowVisualPixelTest, ContinuousClipMatchesRasterSilhouette)
{
    for (double scale : {1.0, 1.25, 1.5, 2.0}) {
        SCOPED_TRACE(scale);
        renderer->setRasterScale(scale);
        renderer->clear({0, 0, 0, 0});
        const render::RoundedShape shape {{20, 20, 100, 80}, 20};
        renderer->pushRoundedClip(shape);
        renderer->fillRect(shape.bounds.toEnclosingRect(), {1, 1, 1, 1});
        renderer->popRoundedClip();
        const auto image = pixels();

        decoration::Image reference({width, height});
        decoration::fillContinuousRoundedRect(reference,
                                              {20 * scale, 20 * scale, 100 * scale, 80 * scale},
                                              20 * scale, {1, 1, 1, 1}, decoration::Corners::all());
        for (int y = int(20 * scale); y < int(100 * scale); ++y) {
            for (int x = int(20 * scale); x < int(120 * scale); ++x) {
                const int expected = reference.bits()[(y * width + x) * 4 + 3];
                // Four samples across a near-horizontal edge quantize coverage by 1/4.
                EXPECT_NEAR(pixel(image, x, y, 3), expected, 32)
                        << "pixel=" << x << ',' << y;
            }
        }
    }
}

TEST_F(WindowVisualPixelTest, CircularClipMatchesRasterSilhouette)
{
    struct Restore {
        ~Restore() { render::setContinuousCorners(true); }
    } restore;
    render::setContinuousCorners(false);
    for (double scale : {1.0, 1.5, 2.0}) {
        SCOPED_TRACE(scale);
        renderer->setRasterScale(scale);
        renderer->clear({0, 0, 0, 0});
        const render::RoundedShape shape {{20, 20, 100, 80}, 20};
        renderer->pushRoundedClip(shape);
        renderer->fillRect(shape.bounds.toEnclosingRect(), {1, 1, 1, 1});
        renderer->popRoundedClip();
        const auto image = pixels();

        decoration::Image reference({width, height});
        decoration::fillRoundedRect(reference, {20 * scale, 20 * scale, 100 * scale, 80 * scale},
                                    20 * scale, {1, 1, 1, 1}, decoration::Corners::all());
        for (int y = int(20 * scale); y < int(100 * scale); ++y) {
            for (int x = int(20 * scale); x < int(120 * scale); ++x) {
                const int expected = reference.bits()[(y * width + x) * 4 + 3];
                EXPECT_NEAR(pixel(image, x, y, 3), expected, 32)
                        << "pixel=" << x << ',' << y;
            }
        }
    }
}

TEST_F(WindowVisualPixelTest, OwnershipAndOnePhysicalHairlineAtEveryScale)
{
    for (const auto [mode, exterior] :
         {std::pair {Mode::ServerFrame, false}, std::pair {Mode::ServerExterior, false},
          std::pair {Mode::ServerFrame, true}, std::pair {Mode::ServerExterior, true}}) {
        for (double scale : {1., 1.25, 1.5, 1.75, 2.}) {
            for (float client : {0.08f, 0.95f}) {
                for (float desktop : {0.15f, 0.85f}) {
                    SCOPED_TRACE(testing::Message() << int(mode) << '/' << exterior << '@' << scale
                                                    << '/' << client << '/' << desktop);
                    WindowScene model;
                    auto *window =
                            model.window(mode, {client, client, client, 1}, {160, 120}, exterior);
                    ASSERT_NE(window, nullptr);
                    renderer->setRasterScale(scale);
                    renderer->suppressWindowOutlines = true;
                    paint(model, window, {desktop, desktop, desktop, 1});
                    const auto bare = pixels();
                    renderer->suppressWindowOutlines = false;
                    paint(model, window, {desktop, desktop, desktop, 1});
                    const auto image = pixels();
                    const auto state = wm::WindowVisualController::evaluate(*window);
                    EXPECT_EQ(state.mode, mode);
                    const auto bounds = state.shape.bounds.translated(window->bounds().topLeft());
                    const int l = roundToInt(bounds.x() * scale), t = roundToInt(bounds.y() * scale);
                    const int r = roundToInt((bounds.x() + bounds.width()) * scale);
                    const int b = roundToInt((bounds.y() + bounds.height()) * scale);
                    const int x = (l + r) / 2, y = (t + b) / 2;
                    const auto blend = [](const render::Color &color, float background) {
                        return roundToInt(255 * (color.r * color.a + background * (1 - color.a)));
                    };
                    const auto &highlight = state.highlightColor;
                    // The highlight is the outermost physical pixel of the shape; fractional
                    // boundaries distribute its coverage.
                    EXPECT_NEAR(pixel(image, x, t),
                                blend(highlight, mode == Mode::ServerFrame ? 1 : client), 1);
                    EXPECT_NEAR(pixel(image, l, y), blend(highlight, client), 1);
                    if (std::fmod(bounds.width() * scale, 1.) == 0)
                        EXPECT_NEAR(pixel(image, r - 1, y), blend(highlight, client), 1);
                    if (std::fmod(bounds.height() * scale, 1.) == 0)
                        EXPECT_NEAR(pixel(image, x, b - 1), blend(highlight, client), 1);
                    // Client pixels start right after it; nothing wider is drawn inside.
                    EXPECT_NEAR(pixel(image, l + 1, y), client * 255, 1);
                    EXPECT_NEAR(pixel(image, r - 3, y), client * 255, 1);
                    EXPECT_NEAR(pixel(image, x, b - 3), client * 255, 1);
                    EXPECT_NEAR(pixel(image, x, t + 1),
                                mode == Mode::ServerFrame ? 255 : client * 255, 1);
                    // The dark outline lies just outside, over the desktop and shadow.
                    EXPECT_NEAR(pixel(image, l - 1, y),
                                blend(state.outlineColor, pixel(bare, l - 1, y) / 255.f), 1);
                    EXPECT_NEAR(pixel(image, x, t - 1),
                                blend(state.outlineColor, pixel(bare, x, t - 1) / 255.f), 1);
                    // All four corners expose the desktop plus exterior shadow, never square client
                    // pixels.
                    for (auto corner :
                         {base::Point(l, t), base::Point(r - 1, t), base::Point(l, b - 1),
                          base::Point(r - 1, b - 1)})
                        EXPECT_LT(pixel(image, corner.x(), corner.y()), desktop * 255 + 1);
                    EXPECT_NEAR(pixel(image, x, y), client * 255, 1);
                    if (mode == Mode::ServerExterior) {
                        EXPECT_EQ(window->contentBounds(), window->bounds());
                        EXPECT_TRUE(window->frameInsets().isNull());
                    } else {
                        EXPECT_EQ(window->frameInsets(), model.theme.metrics().insets());
                    }
                }
            }
        }
    }
}

TEST_F(WindowVisualPixelTest, CsdExteriorIsClippedWhileHeaderBarAndResizeInputSurvive)
{
    WindowScene model;
    auto *window = model.window(Mode::ServerExterior, {1, 0, 0, 1}, {160, 120}, true);
    ASSERT_NE(window, nullptr);
    EXPECT_FALSE(window->isDecorated());
    EXPECT_EQ(window->surfaceOrigin(), base::Point(-8, -8));
    EXPECT_EQ(window->clientExteriorInsets(), (wm::Insets {8, 8, 8, 8}));
    EXPECT_EQ(wm::WindowVisualController::evaluate(*window).shape.bounds, base::RectF(0, 0, 144, 104));
    renderer->setRasterScale(1);
    paint(model, window);
    const auto image = pixels();
    EXPECT_EQ(pixel(image, 76, 120), 0);
    EXPECT_GT(pixel(image, 76, 120, 3), 0);
    EXPECT_EQ(pixel(image, 150, 84), 255);
    EXPECT_EQ(pixel(image, 150, 120), 255);
    EXPECT_EQ(pixel(image, 80, 80), 0);

    base::PointF local;
    EXPECT_EQ(input::findTargetSurface(&model.compositor, {76, 120}, &local),
              window->rootSurface());
    EXPECT_EQ(local, base::PointF(4, 48));
    window->rootSurface()->setInputRegion(base::Region(base::Rect(8, 8, 144, 104)));
    window->rootSurface()->commit();
    EXPECT_EQ(input::findTargetSurface(&model.compositor, {76, 120}, &local), nullptr);
    EXPECT_EQ(input::findTargetSurface(&model.compositor, {150, 84}, &local),
              window->rootSurface());
}

TEST_F(WindowVisualPixelTest, ResizeGeometryAndBufferCommitsKeepRoundedCorners)
{
    for (const auto mode : {Mode::ServerFrame, Mode::ServerExterior}) {
        for (bool exterior : {false, true}) {
            WindowScene model;
            auto *window = model.window(mode, {1, 0, 0, 1}, {160, 120}, exterior);
            auto *shell = model.shells.front().get();
            const auto verify = [&] {
                paint(model, window);
                const auto state = wm::WindowVisualController::evaluate(*window);
                const auto *visual = static_cast<scene::WindowVisualNode *>(window->visualNode());
                EXPECT_EQ(visual->state().shape.bounds, state.shape.bounds);
                EXPECT_EQ(visual->state().shape.radius, 14);
                const auto bounds =
                        state.shape.bounds.translated(window->bounds().topLeft()).toEnclosingRect();
                const auto image = pixels();
                for (const auto corner :
                     {bounds.topLeft(), base::Point(bounds.right() - 1, bounds.y()),
                      base::Point(bounds.x(), bounds.bottom() - 1),
                      base::Point(bounds.right() - 1, bounds.bottom() - 1)})
                    EXPECT_LT(pixel(image, corner.x(), corner.y()), 128);
            };
            renderer->setRasterScale(1);
            verify();
            for (const base::Size size :
                 {base::Size(200, 160), base::Size(120, 80), base::Size(180, 140)}) {
                window->setBounds(base::Rect(base::Point(80, 80), size));
                verify();
                const int margin = exterior ? 8 : 0;
                shell->setGeometry(base::Rect(base::Point(margin, margin), size));
                window->rootSurface()->commit();
                verify();
                model.attach(window->rootSurface(), size + base::Size(2 * margin, 2 * margin),
                             {1, 0, 0, 1});
                verify();
            }
        }
    }
}

TEST_F(WindowVisualPixelTest, CsdResizeGeometryAheadOfBufferKeepsShadowAgainstPaintedBody)
{
    WindowScene model;
    auto *window = model.window(Mode::ServerExterior, {1, 0, 0, 1}, {160, 120}, true);
    auto *shell = model.shells.front().get();
    const base::Rect body(8, 8, 144, 104);
    model.attach(window->rootSurface(), {160, 120}, {1, 0, 0, 1}, body);
    const auto initial = wm::WindowVisualController::evaluate(*window).shape.bounds;
    const auto start = window->bounds().bottomRight() - base::Point(1, 1);
    ASSERT_TRUE(
            model.manager.eventHandler()->beginResize(window, wm::ResizeEdge::BottomRight, start));
    model.manager.eventHandler()->onPointerMotion(start + base::Point(40, 30));
    shell->setGeometry({8, 8, 184, 134});
    window->rootSurface()->commit();
    EXPECT_EQ(wm::WindowVisualController::evaluate(*window).shape.bounds, initial);
    renderer->setRasterScale(1);
    paint(model, window);
    const auto image = pixels();
    const auto corner = window->bounds().topLeft();
    EXPECT_GT(pixel(image, corner.x() + 143, corner.y() + 50, 3), 240);
    EXPECT_GT(pixel(image, corner.x() + 70, corner.y() + 103, 3), 240);
    model.attach(window->rootSurface(), {200, 150}, {1, 0, 0, 1}, {8, 8, 184, 134});
    EXPECT_EQ(wm::WindowVisualController::evaluate(*window).shape.bounds.size(), base::SizeF(184, 134));
    model.manager.eventHandler()->completeDrag(wm::ToplevelEventHandler::DragResult::Success);
}

TEST_F(WindowVisualPixelTest, CsdResizePinsBodyAndShadowWhenGeometryAndBufferArriveSeparately)
{
    for (bool geometryFirst : {false, true}) {
        WindowScene model;
        auto *window = model.window(Mode::ServerExterior, {1, 0, 0, 1}, {160, 120}, true);
        auto *shell = model.shells.front().get();
        uint32_t serial = 0;
        shell->setConfigureCallback([&](const wm::ShellSurface::ConfigureState &) {
            return ++serial;
        });
        const base::Point fixed = window->bounds().bottomRight();
        const base::Point start = window->bounds().topLeft();
        ASSERT_TRUE(
                model.manager.eventHandler()->beginResize(window, wm::ResizeEdge::TopLeft, start));
        model.manager.eventHandler()->onPointerMotion(start - base::Point(40, 30));
        ASSERT_TRUE(shell->acknowledgeConfigure(serial));
        const auto verify = [&](base::Size size) {
            const auto state = wm::WindowVisualController::evaluate(*window);
            const base::Rect body =
                    state.shape.bounds.translated(window->bounds().topLeft()).toEnclosingRect();
            EXPECT_EQ(body.size(), size);
            EXPECT_EQ(body.bottomRight(), fixed);
            auto *shadow =
                    static_cast<decoration::ShadowNode *>(window->node()->children().front());
            EXPECT_EQ(shadow->geometry().marginsRemoved(shadow->margins()),
                      state.shape.bounds.toEnclosingRect());
            EXPECT_EQ(window->rootSurface()->node()->absolutePosition() + base::Point(8, 8),
                      body.topLeft());
        };
        if (geometryFirst) {
            shell->setGeometry({8, 8, 184, 134});
            window->rootSurface()->commit();
            verify({144, 104});
        }
        model.attach(window->rootSurface(), {200, 150}, {1, 0, 0, 1}, {8, 8, 184, 134});
        verify({184, 134});
        if (!geometryFirst) {
            shell->setGeometry({8, 8, 184, 134});
            window->rootSurface()->commit();
            verify({184, 134});
        }
        model.manager.eventHandler()->completeDrag(wm::ToplevelEventHandler::DragResult::Success);
    }
}

TEST_F(WindowVisualPixelTest, FloatingExteriorsRestoreAfterTiledStates)
{
    for (const auto mode : {Mode::ServerFrame, Mode::ServerExterior}) {
        for (bool exterior : {false, true}) {
            WindowScene model;
            auto *window = model.window(mode, {1, 0, 0, 1}, {160, 120}, exterior);
            ASSERT_NE(window, nullptr);
            const auto floating = wm::WindowVisualController::evaluate(*window).shape;
            for (int tiled = 0; tiled < 4; ++tiled) {
                if (tiled == 0)
                    window->maximize();
                else if (tiled == 1)
                    window->setFullscreen(true);
                else if (tiled == 2)
                    window->snapPrimary();
                else
                    window->snapSecondary();
                auto state = wm::WindowVisualController::evaluate(*window);
                if (tiled >= 2) {
                    // Snapped keeps the floating look.
                    EXPECT_GT(state.shape.radius, 0);
                    EXPECT_TRUE(state.drawOutline);
                    EXPECT_TRUE(state.clipContent);
                    EXPECT_NE(model.shadowController.shadowForWindow(window)->elevation(),
                              wm::ShadowElevation::None);
                } else {
                    EXPECT_EQ(state.shape.radius, 0);
                    EXPECT_FALSE(state.drawOutline);
                    EXPECT_EQ(state.clipContent, exterior);
                    EXPECT_EQ(model.shadowController.shadowForWindow(window)->elevation(),
                              wm::ShadowElevation::None);
                }
                if (tiled == 1)
                    window->setFullscreen(false);
                window->restore();
                state = wm::WindowVisualController::evaluate(*window);
                EXPECT_EQ(state.shape.bounds, floating.bounds);
                EXPECT_EQ(state.shape.radius, floating.radius);
                EXPECT_TRUE(state.drawOutline);
                EXPECT_NE(model.shadowController.shadowForWindow(window)->elevation(),
                          wm::ShadowElevation::None);
            }
        }
    }
}

TEST_F(WindowVisualPixelTest, TransparentContentOpaqueFrameAndShadowOutsideAllFourCorners)
{
    for (const auto mode : {Mode::ServerFrame, Mode::ServerExterior}) {
        for (double scale : {1., 1.25, 1.5, 1.75, 2.}) {
            WindowScene model;
            auto *window = model.window(mode, {1, 0, 0, 0.5f});
            renderer->setRasterScale(scale);
            paint(model, window);
            const auto image = pixels();
            const base::RectF bounds =
                    wm::WindowVisualController::evaluate(*window).shape.bounds.translated(
                            window->bounds().topLeft());
            const int l = roundToInt(bounds.x() * scale), t = roundToInt(bounds.y() * scale);
            const int r = roundToInt((bounds.x() + bounds.width()) * scale);
            const int b = roundToInt((bounds.y() + bounds.height()) * scale);
            const int x = (l + r) / 2, y = (t + b) / 2;
            EXPECT_NEAR(pixel(image, x, y, 3), 128, 1);
            EXPECT_EQ(pixel(image, x, y, 1), 0);
            EXPECT_GT(pixel(image, l - 2, y, 3), 0);
            for (auto corner :
                 {base::Point(l, t), base::Point(r - 1, t), base::Point(l, b - 1),
                  base::Point(r - 1, b - 1)}) {
                EXPECT_EQ(pixel(image, corner.x(), corner.y()), 0);
                EXPECT_LT(pixel(image, corner.x(), corner.y(), 3), 100);
            }
            const float h = decoration::WindowVisualStyle().highlight.a;
            EXPECT_NEAR(pixel(image, l, y, 3), 255 * (h + 0.5 * (1 - h)), 1);
            EXPECT_NEAR(pixel(image, l + 1, y, 3), 128, 1);
            EXPECT_NEAR(pixel(image, r - 3, y, 3), 128, 1);
            EXPECT_NEAR(pixel(image, x, b - 3, 3), 128, 1);
            if (mode == Mode::ServerFrame)
                EXPECT_EQ(pixel(image, x, t + 2, 3), 255);
        }
    }
}

TEST_F(WindowVisualPixelTest, SubsurfacesShareWindowClipButPopupsAndResizeMarginsRemainIndependent)
{
    WindowScene model;
    auto *window = model.window(Mode::ServerExterior, {1, 0, 0, 1});
    auto *child = model.surface();
    auto sub = compositor::SubSurface::create(child, window->rootSurface());
    sub->setPosition({-5, -5});
    model.attach(child, {30, 30}, {0, 1, 0, 1});
    window->rootSurface()->commit();
    paint(model, window);
    auto image = pixels();
    EXPECT_EQ(pixel(image, 80, 80, 1), 0);
    EXPECT_EQ(pixel(image, 90, 90, 1), 255);
    base::PointF local;
    EXPECT_EQ(input::findTargetSurface(&model.compositor, {80, 80}, &local), child);
    EXPECT_EQ(input::findTargetSurface(&model.compositor, {90, 90}, &local), child);
    EXPECT_EQ(local, base::PointF(15, 15));
    auto *body = window->visualNode();
    ASSERT_EQ(window->node()->children().size(), 2u);
    EXPECT_EQ(window->node()->children().front()->name(), "shadow");
    EXPECT_EQ(window->rootSurface()->node()->parent(), body);
    EXPECT_EQ(child->node()->parent(), window->rootSurface()->node());
    auto *popupSurface = model.surface();
    wm::ShellSurface popup(&model.manager, popupSurface);
    ASSERT_TRUE(popup.setPopup(model.shells.front().get(), {-10, -10, 30, 30}));
    model.attach(popupSurface, {30, 30}, {0, 0, 1, 1});
    model.compositor.updateTextures(*renderer);
    ASSERT_NE(nullptr, popup.window());
    EXPECT_EQ(popupSurface->node()->parent(), popup.window()->visualNode());
    EXPECT_EQ(popup.window()->node()->parent(), model.compositor.surfaceContainer());
    popupSurface->node()->render(*renderer);
    image = pixels();
    EXPECT_EQ(pixel(image, 80, 80, 2), 255);
    EXPECT_EQ(input::findTargetSurface(&model.compositor, {80, 80}, &local), popupSurface);
    window->setDecorated(true);
    EXPECT_NE(window->frame()->areaAt(window->bounds().topLeft()), wm::FrameArea::None);
}

TEST_F(WindowVisualPixelTest, BlurredShellSurfacesAndPopupsOutlineTheirRoundedBody)
{
    renderer->setRasterScale(1);
    WindowScene model;
    auto *window = model.window(Mode::ServerExterior, {0.5f, 0.5f, 0.5f, 1});
    ASSERT_NE(window, nullptr);
    window->setRole(wm::WindowRole::Dock);
    auto *surface = window->rootSurface();
    surface->setBackgroundBlur({10, 20, 100, 60}, 18);
    surface->commit();
    const auto &style = model.manager.visualStyle();
    auto state = wm::WindowVisualController::evaluate(*window, style);
    EXPECT_EQ(state.mode, Mode::PreserveClient);
    EXPECT_TRUE(state.drawOutline);
    EXPECT_FALSE(state.clipContent);
    EXPECT_EQ(state.shape.bounds, base::RectF(window->presentedBounds()));
    EXPECT_EQ(state.outlineShape.bounds, base::RectF(10, 20, 100, 60));
    EXPECT_EQ(state.outlineShape.radius, 18);
    EXPECT_FLOAT_EQ(state.outlineColor.a, decoration::WindowVisualStyle().dockOutline.a);
    EXPECT_FLOAT_EQ(state.highlightColor.a, decoration::WindowVisualStyle().dockHighlight.a);

    paint(model, window);
    auto image = pixels();
    const auto blend = [](const render::Color &color, float background) {
        return roundToInt(255 * (color.r * color.a + background * (1 - color.a)));
    };
    EXPECT_NEAR(pixel(image, 89, 130), blend(state.outlineColor, 0.5f), 1);
    EXPECT_NEAR(pixel(image, 90, 130), blend(state.highlightColor, 0.5f), 1);
    EXPECT_NEAR(pixel(image, 91, 130), 128, 1);
    EXPECT_NEAR(pixel(image, 85, 130), 128, 1);
    // Damage has to reach the outline outside the body.
    EXPECT_TRUE(window->visualNode()->absolutePaintBounds().contains(base::Rect(89, 99, 102, 62)));

    // A glass rim is the edge, so the outline goes transparent and keeps its shape.
    render::GlassParams glass;
    glass.enabled = true;
    glass.rim = 1;
    surface->setGlass(glass);
    surface->commit();
    state = wm::WindowVisualController::evaluate(*window, style);
    EXPECT_TRUE(state.drawOutline);
    EXPECT_EQ(state.outlineShape.bounds, base::RectF(10, 20, 100, 60));
    EXPECT_FLOAT_EQ(state.outlineColor.a, 0);
    EXPECT_FLOAT_EQ(state.highlightColor.a, 0);
    surface->setGlass({});
    surface->commit();

    // A square blur is not a floating body; the client keeps its own edge.
    surface->setBackgroundBlur({10, 20, 100, 60}, 0);
    surface->commit();
    EXPECT_FALSE(wm::WindowVisualController::evaluate(*window, style).drawOutline);

    auto *popupSurface = model.surface();
    wm::ShellSurface popup(&model.manager, popupSurface);
    ASSERT_TRUE(popup.setPopup(model.shells.front().get(), {-10, -10, 30, 30}));
    popupSurface->setBackgroundBlur({0, 0, 30, 30}, 40);
    model.attach(popupSurface, {30, 30}, {0, 0, 1, 1});
    ASSERT_NE(popup.window(), nullptr);
    state = wm::WindowVisualController::evaluate(*popup.window(), style);
    EXPECT_TRUE(state.drawOutline);
    EXPECT_EQ(state.outlineShape.bounds, base::RectF(0, 0, 30, 30));
    EXPECT_EQ(state.outlineShape.radius, 15);
    EXPECT_FLOAT_EQ(state.outlineColor.a, decoration::WindowVisualStyle().shellOutline.a);
}

TEST_F(WindowVisualPixelTest, FillMappedTextureBlurNestedClipsAndOffscreenOffsets)
{
    for (double scale : {1., 1.25, 1.5, 1.75, 2.}) {
        renderer->setRasterScale(scale);
        render::RoundedShape outer {{41, 43, 160, 120}, 14};
        render::RoundedShape inner {{51, 53, 130, 90}, 14};
        for (int operation = 0; operation < 4; ++operation) {
            renderer->clear({0, 0, 1, 1});
            std::unique_ptr<render::Texture> capture;
            ASSERT_TRUE(renderer->renderToTexture(capture, {27, 29, 200, 160}, [&] {
                renderer->pushRoundedClip(outer);
                renderer->pushRoundedClip(inner);
                if (operation == 0)
                    renderer->fillRect({0, 0, 300, 300}, {1, 0, 0, 1});
                else if (operation == 1)
                    renderer->fillRectF({0.25, 0.25, 300, 300}, {1, 0, 0, 1});
                else if (operation == 2) {
                    const uint8_t rgba[] = {255, 0, 0, 255};
                    auto texture = renderer->createTexture({1, 1}, rgba);
                    base::AffineTransform mapping;
                    mapping.rotate(90);
                    renderer->drawTextureMapped(texture.get(), {0, 0, 300, 300}, 1, mapping);
                } else {
                    renderer->drawBackgroundBlur({0, 0, 300, 300}, 0, {0, 0, 300, 300});
                }
                renderer->popRoundedClip();
                renderer->popRoundedClip();
            }));
            renderer->clear({0, 0, 0, 0});
            renderer->drawTexture(capture.get(), {27, 29, 200, 160}, 1);
            const auto image = pixels();
            EXPECT_EQ(pixel(image, roundToInt(51 * scale), roundToInt(53 * scale), 3), 0);
            EXPECT_EQ(pixel(image, roundToInt(40 * scale), roundToInt(80 * scale), 3), 0);
            EXPECT_EQ(pixel(image, roundToInt(100 * scale), roundToInt(100 * scale), 3), 255);
            EXPECT_EQ(
                    pixel(image, roundToInt(100 * scale), roundToInt(100 * scale),
                          operation == 3 ? 2 : 0),
                    255);
        }
    }
}

TEST_F(WindowVisualPixelTest, CornerCoverageMatchesIndependentAreaSamplingAndHairlineMass)
{
    for (double scale : {1., 1.25, 1.5, 1.75, 2.}) {
        for (double offset : {0., 0.3}) {
            renderer->setRasterScale(scale);
            const render::RoundedShape shape {{40 + offset, 40 + offset, 160, 120}, 14};
            const base::RectF device = shape.bounds.scaled(scale);
            const render::RoundedShape outer {device, 14 * scale};
            for (double stroke : {0., 0.5, 1.}) {
                const bool outline = stroke > 0;
                renderer->clear({0, 0, 0, 0});
                if (outline) {
                    renderer->drawRoundedOutline(shape, {1, 1, 1, 1}, stroke);
                } else {
                    renderer->pushRoundedClip(shape);
                    renderer->fillRect({0, 0, 400, 300}, {1, 1, 1, 1});
                    renderer->popRoundedClip();
                }
                const auto image = pixels();
                const int left = std::floor(device.left()), top = std::floor(device.top());
                const double radius = 21 * scale;
                const int extent = std::ceil(radius) + 2;
                const auto insetTop = [&](double x) {
                    if (x < device.left() + stroke)
                        return device.bottom();
                    if (x >= device.left() + radius)
                        return device.top() + stroke;
                    double low = 0, high = 1;
                    double boundaryY = device.top();
                    for (int i = 0; i < 40; ++i) {
                        const double qx = (low + high) / 2;
                        const double qy = std::cbrt(1 - qx * qx * qx);
                        const double normalLength = std::hypot(qx * qx, qy * qy);
                        const double boundaryX = device.left() + radius * (1 - qx)
                                + stroke * qx * qx / normalLength;
                        boundaryY = device.top() + radius * (1 - qy)
                                + stroke * qy * qy / normalLength;
                        if (boundaryX > x)
                            low = qx;
                        else
                            high = qx;
                    }
                    return boundaryY;
                };
                for (int y = top - 1; y < top + extent; ++y) {
                    for (int x = left - 1; x < left + extent; ++x) {
                        int covered = 0;
                        constexpr int samples = 128;
                        for (int sx = 0; sx < samples; ++sx) {
                            const double innerTop = insetTop(x + (sx + .5) / samples);
                            for (int sy = 0; sy < samples; ++sy) {
                                const base::PointF p(x + (sx + .5) / samples, y + (sy + .5) / samples);
                                covered += outer.contains(p) && (!outline || p.y() < innerTop);
                            }
                        }
                        const double expected = covered * 255. / (samples * samples);
                        ASSERT_NEAR(pixel(image, x, y, 3), expected, 2)
                                << "scale=" << scale << " offset=" << offset
                                << " outline=" << outline << " pixel=" << x << ',' << y;
                    }
                }
                if (outline) {
                    int total = 0;
                    for (int y = top - 2; y <= top + 3; ++y)
                        total += pixel(image, roundToInt(device.center().x()), y, 3);
                    EXPECT_NEAR(total, 255 * stroke, 2);
                }
            }
        }
    }
}

TEST_F(WindowVisualPixelTest, ShapedPopupBlursOutlinesAndShadowsItsOwnOutline)
{
    for (double scale : {1., 2.}) {
        renderer->setRasterScale(scale);
        WindowScene model;
        ASSERT_NE(model.window(Mode::ServerExterior, {0.5f, 0.5f, 0.5f, 1}), nullptr);
        auto *popupSurface = model.surface();
        wm::ShellSurface popup(&model.manager, popupSurface);
        ASSERT_TRUE(popup.setPopup(model.shells.front().get(), {20, 20, 120, 70}));
        ui::Path shape;
        shape.addRoundedRect(ui::RectF {0, 0, 120, 60}, 8, 8);
        ui::Path arrow;
        arrow.moveTo(40, 59);
        arrow.lineTo(60, 59);
        arrow.lineTo(50, 69);
        arrow.close();
        shape = shape.united(arrow);
        popupSurface->setBackgroundBlur({0, 0, 120, 70}, 8);
        popupSurface->setBackgroundBlurShape(shape);
        model.attach(popupSurface, {120, 70}, {0, 0, 0, 0});
        auto *window = popup.window();
        ASSERT_NE(window, nullptr);
        static_cast<scene::WindowAnimation *>(window->node())->finish();
        const auto state =
                wm::WindowVisualController::evaluate(*window, model.manager.visualStyle());
        ASSERT_TRUE(state.drawOutline);
        EXPECT_FALSE(state.outlinePath.isEmpty());
        EXPECT_EQ(state.outlineShape.bounds.toEnclosingRect(), base::Rect(0, 0, 120, 69));

        decoration::ShadowNode *shadow = nullptr;
        for (auto *child : window->node()->children()) {
            if (child->name() == "shadow")
                shadow = static_cast<decoration::ShadowNode *>(child);
        }
        ASSERT_NE(shadow, nullptr);
        EXPECT_FALSE(shadow->silhouette().isEmpty());

        model.compositor.updateTextures(*renderer);
        const base::Point origin = window->node()->absolutePosition();
        const auto device = [&](double x, double y) {
            return base::Point(int((origin.x() + x) * scale), int((origin.y() + y) * scale));
        };
        renderer->clear({1, 1, 1, 1});
        // Dark patches: blurred away inside the shape, untouched outside it.
        renderer->fillRect({origin.x() + 49, origin.y() + 55, 3, 18}, {0, 0, 0, 1});
        renderer->fillRect({origin.x() + 112, origin.y() + 62, 6, 6}, {0, 0, 0, 1});
        window->node()->render(*renderer);
        const auto image = pixels();
        EXPECT_GT(pixel(image, device(50.5, 63).x(), device(50.5, 63).y()), 200) << scale;
        EXPECT_LT(pixel(image, device(50.5, 71.5).x(), device(50.5, 71.5).y()), 10) << scale;
        EXPECT_LT(pixel(image, device(115, 65).x(), device(115, 65).y()), 10) << scale;
        // The hairline follows the arrow's slanted side.
        int darkest = 255;
        for (int x = device(52, 64).x(); x <= device(58, 64).x(); ++x)
            darkest = std::min(darkest, pixel(image, x, device(52, 64).y()));
        EXPECT_LT(darkest, 240) << scale;
    }
}

TEST_F(WindowVisualPixelTest, OutlineBlendsOnceWithClientColor)
{
    for (double scale : {1., 1.25, 1.5, 1.75, 2.}) {
        renderer->setRasterScale(scale);
        const render::RoundedShape shape {{40, 40, 160, 120}, 14};
        for (float client : {0.f, 1.f}) {
            for (double stroke : {0.5, 1.0}) {
                renderer->clear({client, client, client, 1});
                const auto color = decoration::WindowVisualStyle().activeOutline;
                renderer->drawRoundedOutline(shape, color, stroke);
                const auto image = pixels();
                const double coverage = color.a * stroke;
                const int expected = roundToInt(255 * (color.r * coverage + client * (1 - coverage)));
                for (const auto &point :
                     {base::Point(roundToInt(40 * scale), roundToInt(100 * scale)),
                      base::Point(roundToInt(200 * scale) - 1, roundToInt(100 * scale)),
                      base::Point(roundToInt(120 * scale), roundToInt(40 * scale)),
                      base::Point(roundToInt(120 * scale), roundToInt(160 * scale) - 1)}) {
                    EXPECT_NEAR(pixel(image, point.x(), point.y()), expected, 1);
                }
            }
        }
    }
}

TEST_F(WindowVisualPixelTest, AnimationUsesOnePhysicalOutlineAndClipsSquareOldSnapshots)
{
    for (double scale : {1., 1.25, 1.5, 1.75, 2.}) {
        renderer->setRasterScale(scale);
        int64_t now = 0;
        scene::WindowAnimation animation([&] {
            return now;
        });
        const base::Rect from(10, 10, 240, 180), to(40, 40, 160, 120);
        animation.setGeometry(from);
        auto visual = std::make_unique<scene::WindowVisualNode>();
        auto *body = visual.get();
        scene::WindowVisualState state;
        state.mode = Mode::ServerExterior;
        state.shape.bounds = base::RectF(base::Rect(from.size()));
        body->setState(state);
        auto fill = std::make_unique<scene::ColorNode>(render::Color {1, 0, 0, 1});
        auto *content = fill.get();
        fill->setGeometry(base::Rect(from.size()));
        body->addChild(std::move(fill));
        animation.addChild(std::move(visual));
        animation.finish();
        animation.render(*renderer);
        animation.setGeometry(to);
        animation.start(scene::WindowAnimation::Type::Bounds, from, to);
        state.shape = {base::RectF(base::Rect(to.size())), 14};
        state.clipContent = state.drawOutline = true;
        state.outlineWidthPx = 1.0;
        const render::Color outline {1, 1, 1, 0.14f};
        state.outlineColor = outline;
        body->setState(state);
        content->setGeometry(base::Rect(to.size()));
        content->setColor({0, 1, 0, 1});
        const double outlineChannel = 255 * outline.a;
        for (int elapsed : {0, 100, 400}) {
            now = elapsed;
            renderer->clear({0, 0, 0, 0});
            animation.render(*renderer);
            const auto image = pixels();
            const auto sample = scene::WindowAnimation::sample(
                    scene::WindowAnimation::Type::Bounds, from, to,
                    double(elapsed)
                            / scene::WindowAnimation::duration(scene::WindowAnimation::Type::Bounds,
                                                               from, to));
            const int x = roundToInt(sample.bounds.center().x() * scale);
            const int y = std::floor(sample.bounds.y() * scale);
            const int left = std::floor(sample.bounds.x() * scale);
            // The square corners round off and the outline fades in as the
            // window reaches its normal state.
            const double progress =
                    (sample.bounds.width() - from.width()) / (to.width() - from.width());
            if (elapsed == 0)
                EXPECT_EQ(pixel(image, std::ceil(sample.bounds.x() * scale),
                                std::ceil(sample.bounds.y() * scale), 3),
                          255);
            else if (progress < 1)
                EXPECT_LT(pixel(image, left, y, 3), 255);
            else
                EXPECT_EQ(pixel(image, left, y, 3), 0);
            int blueCoverage = 0;
            for (int row = std::max(0, y - 2); row <= y + 3; ++row)
                blueCoverage += pixel(image, x, row, 2);
            EXPECT_NEAR(blueCoverage, outlineChannel * progress, 2);
            // Red comes only from the outline above the final green-content frame.
            if (elapsed == 400) {
                EXPECT_NEAR(pixel(image, x, y - 1), outlineChannel, 1);
                EXPECT_EQ(pixel(image, x, y), 0);
                EXPECT_EQ(pixel(image, x, y, 1), 255);
            }
        }
        animation.close();
        animation.removeChild(body);
        renderer->clear({0, 0, 0, 0});
        animation.render(*renderer);
        now += 60;
        renderer->clear({0, 0, 0, 0});
        animation.render(*renderer);
        EXPECT_EQ(glGetError(), GLenum(GL_NO_ERROR));
    }
}

// A window whose content changes frame after frame skips its snapshot and is
// drawn straight to the output; the two must not differ by a pixel, or every
// periodic snapshot frame would flicker.
TEST_F(WindowVisualPixelTest, DrawingDirectlyMatchesTheSnapshotAtEveryScale)
{
    for (double scale : {1., 1.25, 1.5, 1.75, 2.}) {
        SCOPED_TRACE(scale);
        renderer->setRasterScale(scale);
        WindowScene model;
        auto *window = model.window(Mode::ServerFrame, {0.2f, 0.4f, 0.8f, 1});
        ASSERT_NE(window, nullptr);
        // A fractional device position, where a resampled snapshot would show.
        window->setBounds({base::Point(81, 83), window->bounds().size()});
        // One-pixel stripes, which any resampling blurs.
        const auto stripes = [&] {
            const base::Size size(160, 120);
            std::vector<uint8_t> bytes(size.width() * size.height() * 4, 255);
            for (int y = 0; y < size.height(); ++y)
                for (int x = 0; x < size.width(); x += 2)
                    std::fill_n(bytes.begin() + (y * size.width() + x) * 4, 3, uint8_t(0));
            const int fd = testsupport::makeMemfd(bytes.size());
            ASSERT_GE(fd, 0);
            ASSERT_EQ(pwrite(fd, bytes.data(), bytes.size(), 0), ssize_t(bytes.size()));
            auto pool = compositor::ShmPool::create(fd, bytes.size());
            model.buffers.push_back(pool->createBuffer(size, 0, size.width() * 4,
                                                       render::PixelFormat::Rgba8888, nullptr));
            window->rootSurface()->attach(model.buffers.back(), {});
            window->rootSurface()->damage(base::Rect(base::Point(), size));
            window->rootSurface()->commit();
        };
        paint(model, window);
        paint(model, window);
        stripes();
        paint(model, window);
        const auto snapshot = pixels();
        stripes();
        paint(model, window);
        EXPECT_EQ(pixels(), snapshot);
    }
}

TEST_F(WindowVisualPixelTest, MaximizedSsdHasNoSideOrBottomFrame)
{
    WindowScene model;
    auto *window = model.window(Mode::ServerFrame, {0, 0, 0, 1});
    const auto normalInsets = window->frameInsets();
    auto *shell = model.shells.front().get();
    uint32_t serial = 0;
    shell->setConfigureCallback([&](const wm::ShellSurface::ConfigureState &) {
        return ++serial;
    });
    window->maximize();
    ASSERT_TRUE(shell->acknowledgeConfigure(serial));
    model.attach(window->rootSurface(), window->contentBounds().size(), {0, 0, 0, 1});
    static_cast<scene::WindowAnimation *>(window->node())->finish();
    paint(model, window, {1, 0, 1, 1});
    const auto image = pixels();
    for (int y = normalInsets.top; y < height; ++y) {
        for (int x : {0, width - 1}) {
            EXPECT_EQ(pixel(image, x, y), 0);
            EXPECT_EQ(pixel(image, x, y, 2), 0);
        }
    }
    for (int x = 0; x < width; ++x)
        EXPECT_EQ(pixel(image, x, height - 1), 0);
    EXPECT_FALSE(wm::WindowVisualController::evaluate(*window).drawOutline);
    window->restore();
    EXPECT_EQ(window->frameInsets(), normalInsets);
    EXPECT_TRUE(wm::WindowVisualController::evaluate(*window).drawOutline);
}

TEST_F(WindowVisualPixelTest, ConfigureAcknowledgementAndOldBufferKeepPresentedSsdShape)
{
    WindowScene model;
    auto *window = model.window(Mode::ServerFrame, {1, 0, 0, 1});
    auto *shell = model.shells.front().get();
    uint32_t serial = 0;
    shell->setConfigureCallback([&](const wm::ShellSurface::ConfigureState &) {
        return ++serial;
    });
    paint(model, window);
    const auto original = pixels();
    const auto shape = wm::WindowVisualController::evaluate(*window).shape.bounds;
    const auto requested = window->bounds();
    window->requestBounds({requested.topLeft(), requested.size() + base::Size(40, 20)});
    const auto future = window->bounds();
    ASSERT_GT(serial, 0u);
    ASSERT_TRUE(shell->acknowledgeConfigure(serial));
    window->rootSurface()->commit();
    EXPECT_EQ(wm::WindowVisualController::evaluate(*window).shape.bounds.size(), shape.size());
    model.attach(window->rootSurface(), {160, 120}, {1, 0, 0, 1});
    paint(model, window);
    EXPECT_EQ(pixels(), original);
    model.attach(window->rootSurface(), {200, 140}, {0, 1, 0, 1});
    paint(model, window);
    EXPECT_EQ(wm::WindowVisualController::evaluate(*window).shape.bounds.size(),
              base::SizeF(future.size()));
    EXPECT_EQ(window->contentBounds().size(), base::Size(200, 140));
}

TEST_F(WindowVisualPixelTest, ShapedGlassPopupOutlineMeetsSnappedBodyAtFractionalScale)
{
    const double scale = 1.5;
    renderer->setRasterScale(scale);
    // An odd logical position lands the popup on half a physical pixel.
    for (int x : {20, 21}) {
        WindowScene model;
        ASSERT_NE(model.window(Mode::ServerExterior, {0.5f, 0.5f, 0.5f, 1}), nullptr);
        auto *popupSurface = model.surface();
        wm::ShellSurface popup(&model.manager, popupSurface);
        ASSERT_TRUE(popup.setPopup(model.shells.front().get(), {x, 21, 120, 70}));
        ui::Path shape;
        shape.addRoundedRect(ui::RectF {0, 0, 120, 60}, 8, 8);
        popupSurface->setBackgroundBlur({0, 0, 120, 60}, 8);
        popupSurface->setBackgroundBlurShape(shape);
        render::GlassParams glass;
        glass.enabled = true;
        glass.displacement = 0;
        glass.tintRed = glass.tintGreen = glass.tintBlue = 1;
        glass.tintAlpha = 0.6f;
        popupSurface->setGlass(glass);
        model.attach(popupSurface, {120, 70}, {0, 0, 0, 0});
        auto *window = popup.window();
        ASSERT_NE(window, nullptr);
        static_cast<scene::WindowAnimation *>(window->node())->finish();
        model.compositor.updateTextures(*renderer);
        const base::Point origin = window->node()->absolutePosition();
        renderer->clear({1, 1, 1, 1});
        window->node()->render(*renderer);
        const auto image = pixels();

        // The body starts where the surface is snapped; the hairline is the
        // one pixel outside it, darker than the shadow beyond.
        const int left = roundToInt(origin.x() * scale);
        const int top = roundToInt(origin.y() * scale);
        const int row = int((origin.y() + 30) * scale);
        const int column = int((origin.x() + 60) * scale);
        EXPECT_GE(pixel(image, left, row), 245) << x;
        EXPECT_LT(pixel(image, left - 1, row), pixel(image, left - 2, row)) << x;
        EXPECT_GE(pixel(image, column, top), 245) << x;
        EXPECT_LT(pixel(image, column, top - 1), pixel(image, column, top - 2)) << x;
    }
}

} // namespace
