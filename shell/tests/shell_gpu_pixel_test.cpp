// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// The shell's windows rendered by Skia on the renderer's own GL context, all
// in the one pass an output frame starts with, and shown through their
// surfaces' textures.

#include <functional>

#include <gtest/gtest.h>

#include "compositor/compositor.h"
#include "compositor/output_scale.h"
#include "compositor/surface.h"
#include "egl_test_util.h"
#include "input/seat.h"
#include "render/gl_texture.h"
#include "scene/node.h"
#include "scene/scene.h"
#include "scene/surface_node.h"
#include "shell/shell.h"
#include "shell/shell_host.h"
#include "shell/shell_window.h"
#include "shell/window_renderer.h"
#include "toolkit/toolkit_host.h"
#include "ui/bitmap.h"
#include "ui/embedded_window.h"
#include "ui/frame_client.h"
#include "ui/painter.h"
#include "ui/view.h"
#include "wm/window.h"
#include "wm/window_manager.h"

using namespace Lingmo;
using namespace Lingmo::Render;

namespace {

const base::Rect kOutput(0, 0, 400, 200);

class Fill : public ui::View
{
public:
    explicit Fill(ui::Color color)
        : m_color(color)
    {
    }

    void setColor(ui::Color color)
    {
        m_color = color;
        requestPaint();
    }

    void paint(ui::Painter &painter) override { painter.fillRect(localRect(), m_color); }

private:
    ui::Color m_color;
};

class ShellGpuPixels : public testing::TestWithParam<double>
{
protected:
    void SetUp() override
    {
        renderer = EglRendererTestAccess::create();
        ASSERT_TRUE(renderer);
        native = base::Size(base::roundToInt(kOutput.width() * scale()),
                            base::roundToInt(kOutput.height() * scale()));
        const EGLint attributes[] = {EGL_WIDTH, native.width(), EGL_HEIGHT, native.height(),
                                     EGL_NONE};
        const auto surface =
                eglCreatePbufferSurface(renderer->eglDisplay(), renderer->eglConfig(), attributes);
        ASSERT_NE(surface, EGL_NO_SURFACE);
        target = std::make_unique<EglRenderTarget>(renderer->eglDisplay(), surface, nullptr, native);

        compositor.setOutputScale(
                *compositor::OutputScale::fromNumerator(uint32_t(scale() * 120)));
        manager.setOutputBounds(kOutput);
        shell = std::make_unique<shell::Shell>(
                shell::ShellEnvironment {&compositor, &manager, &seat, nullptr});
    }

    void TearDown() override
    {
        if (renderer && target) {
            ASSERT_TRUE(renderer->beginFrame(target.get()));
            shell.reset();
            // Skia's objects go with the context current.
            toolkit::PanelTexture(nullptr).releaseRenderResources();
            renderer->endFrame();
        }
        shell.reset();
        seat.shutdown();
        if (renderer && target)
            renderer->releaseTarget(target.get());
        target.reset();
        renderer.reset();
    }

    double scale() const { return GetParam(); }

    // One output frame: the shell's pass, the surfaces' textures, then each
    // window's surface drawn where it is.
    ui::Bitmap frame()
    {
        renderer->setOutputContext({"out", kOutput, native, scale(), 0, ++frameCount});
        renderer->frameTime = 1000.0 * double(frameCount);
        EXPECT_TRUE(renderer->beginFrame(target.get()));
        const ui::Bitmap image = renderer->captureFrame([&] {
            renderer->clear({0, 0, 0, 0});
            compositor.updateTextures(*renderer);
            for (shell::ShellWindow *window : shell->windows()) {
                scene::SurfaceNode *node = window->surface()->node();
                if (node->texture())
                    renderer->drawTextureMapped(node->texture(), window->drawnBounds(), 1.0f,
                                                base::AffineTransform());
            }
        });
        renderer->cancelFrame();
        return image.converted(ui::Bitmap::Format::Rgba8888Premultiplied);
    }

    uint32_t pixelAt(const ui::Bitmap &image, const base::Point &logical) const
    {
        const uint8_t *pixel = image.constScanLine(base::roundToInt(logical.y() * scale()))
                + size_t(base::roundToInt(logical.x() * scale())) * 4;
        return uint32_t(pixel[3]) << 24 | uint32_t(pixel[0]) << 16 | uint32_t(pixel[1]) << 8
                | pixel[2];
    }

    std::unique_ptr<EglRenderer> renderer;
    std::unique_ptr<EglRenderTarget> target;
    base::Size native;
    uint64_t frameCount = 0;
    scene::Scene scene;
    scene::Node *surfaces = scene.root()->addChild(std::make_unique<scene::Node>("surfaces"));
    compositor::Compositor compositor {surfaces};
    input::Seat seat {&compositor};
    wm::WindowManager manager {&compositor, &seat};
    std::unique_ptr<shell::Shell> shell;
};

} // namespace

TEST_P(ShellGpuPixels, WindowsRenderOnTheGpuInOnePassAndOnlyWhatChanged)
{
    shell::ShellWindow panel(shell.get(), wm::WindowRole::Panel);
    auto red = std::make_unique<Fill>(ui::Color::rgb(0xff0000));
    panel.setContentView(std::move(red));
    panel.setGeometry(base::Rect(0, 0, 200, 40));
    panel.show();
    shell::ShellWindow osd(shell.get(), wm::WindowRole::Osd);
    auto blue = std::make_unique<Fill>(ui::Color::rgb(0x0000ff));
    Fill *osdFill = blue.get();
    osd.setContentView(std::move(blue));
    osd.setGeometry(base::Rect(300, 100, 100, 50));
    osd.show();
    EXPECT_EQ(scale(), panel.scale());

    const uint64_t passes = shell->renderer()->passCount();
    ui::Bitmap image = frame();
    EXPECT_EQ(passes + 1, shell->renderer()->passCount());
    EXPECT_EQ(2u, shell->renderer()->lastPass().size());
    EXPECT_TRUE(panel.lastFrameOnGpu());
    EXPECT_TRUE(osd.lastFrameOnGpu());
    auto *texture = dynamic_cast<GlTexture *>(osd.surface()->node()->texture());
    ASSERT_NE(nullptr, texture);
    EXPECT_EQ(base::Size(base::roundToInt(100 * scale()), base::roundToInt(50 * scale())),
              texture->size());
    EXPECT_EQ(0xffff0000u, pixelAt(image, {100, 20}));
    EXPECT_EQ(0xff0000ffu, pixelAt(image, {350, 125}));
    EXPECT_EQ(0u, pixelAt(image, {250, 20}));

    // One window changed: rendered alone, into the same texture, the other
    // kept as it was.
    osdFill->setColor(ui::Color::rgb(0x00ff00));
    image = frame();
    ASSERT_EQ(1u, shell->renderer()->lastPass().size());
    EXPECT_EQ(&osd, shell->renderer()->lastPass().front());
    EXPECT_EQ(texture, osd.surface()->node()->texture());
    EXPECT_EQ(0xff00ff00u, pixelAt(image, {350, 125}));
    EXPECT_EQ(0xffff0000u, pixelAt(image, {100, 20}));

    // A new size takes a new texture in the middle of the pass; the batch
    // around the other window's render survives it.
    osd.setSize(base::Size(120, 60));
    osdFill->setColor(ui::Color::rgb(0xffffff));
    panel.window()->contentView()->requestPaint();
    image = frame();
    EXPECT_EQ(2u, shell->renderer()->lastPass().size());
    EXPECT_EQ(0xffffffffu, pixelAt(image, {390, 150}));
    EXPECT_EQ(0xffff0000u, pixelAt(image, {100, 20}));
}

TEST_P(ShellGpuPixels, APopupGrowingInItsOwnFrameShowsItsNewSizeInThatFrame)
{
    // Runs once on the next frame of its window.
    class Step : public ui::FrameClient
    {
    public:
        using ui::FrameClient::FrameClient;

        void start() { startTicking(); }

        std::function<void()> onTick;

    protected:
        void tick(uint64_t) override
        {
            stopTicking();
            onTick();
        }
    };

    shell::ShellWindow panel(shell.get(), wm::WindowRole::Panel);
    panel.setContentView(std::make_unique<Fill>(ui::Color::rgb(0xff0000)));
    panel.setGeometry(base::Rect(0, 0, 400, 20));
    panel.show();
    ui::PopupPlacement placement;
    placement.anchorRect = {100, 20, 1, 1};
    placement.anchor = ui::PopupAnchor::TopLeft;
    placement.gravity = ui::PopupAnchor::BottomRight;
    placement.flipX = placement.flipY = placement.slide = false;
    std::unique_ptr<ui::Window> popup = ui::Window::createPopup(panel.window(), placement, false);
    auto content = std::make_unique<Fill>(ui::Color::rgb(0x0000ff));
    Fill *fill = content.get();
    popup->setContentView(std::move(content));
    popup->resize({100, 40});
    popup->show();
    shell::ShellWindow *window = shell->host()->windowFor(static_cast<ui::EmbeddedWindow *>(popup.get()));
    ASSERT_NE(nullptr, window);
    window->setBackgroundBlur(base::Rect(0, 0, 100, 40), 8);
    ui::Bitmap image = frame();
    EXPECT_TRUE(window->lastFrameOnGpu());
    EXPECT_EQ(0xff0000ffu, pixelAt(image, {150, 50}));
    EXPECT_EQ(0u, pixelAt(image, {150, 90}));

    // Its own animation grows it, recolours it and moves its glass, as a
    // status panel's height does: all of it in this frame, drawn once.
    Step step(popup.get());
    step.onTick = [&] {
        popup->resize({100, 80});
        fill->setColor(ui::Color::rgb(0x00ff00));
        window->setBackgroundBlur(base::Rect(0, 0, 100, 80), 8);
    };
    step.start();
    image = frame();
    EXPECT_TRUE(window->lastFrameOnGpu());
    EXPECT_EQ(base::Rect(100, 20, 100, 80), window->geometry());
    EXPECT_EQ(base::Size(base::roundToInt(100 * scale()), base::roundToInt(80 * scale())),
              window->surface()->node()->texture()->size());
    EXPECT_EQ(base::Rect(0, 0, 100, 80), window->surface()->backgroundBlurRect());
    EXPECT_EQ(0xff00ff00u, pixelAt(image, {150, 30}));
    EXPECT_EQ(0xff00ff00u, pixelAt(image, {150, 90}));
    popup.reset();
}

INSTANTIATE_TEST_SUITE_P(Scales, ShellGpuPixels, testing::Values(1.0, 1.5));
