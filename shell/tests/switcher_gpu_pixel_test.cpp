// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// The application switcher composited as an output draws it: its shadow, its
// glass, the panel's texture, the outline and a node drawn after it, through
// the real GL renderer, with the panel drawn on the GPU against the CPU.

#include <GLES3/gl3.h>
#include <gtest/gtest.h>

#include "compositor/compositor.h"
#include "compositor/surface.h"
#include "decoration/decoration_theme.h"
#include "display/display_manager.h"
#include "egl_test_util.h"
#include "fake_renderer.h"
#include "input/seat.h"
#include "scene/node.h"
#include "scene/scene.h"
#include "shm_test_util.h"
#include "switcher/switcher_view.h"
#include "toolkit/toolkit_host.h"
#include "ui/bitmap.h"
#include "wm/window.h"
#include "wm/window_manager.h"

using namespace Lingmo;
using namespace Lingmo::Render;

namespace {

const base::Rect kOutput(0, 0, 640, 320);

// A node drawn after the switcher, as anything above it in the scene is.
class MarkerNode final : public scene::Node
{
public:
    MarkerNode()
        : Node("marker")
    {
    }

protected:
    void paint(Renderer &renderer, const base::Rect &bounds) const override
    {
        renderer.fillRect(bounds, {0, 0.5f, 0, 0.5f});
    }
};

struct Param {
    double scale;
    bool dark;
};

class SwitcherGpuPixels : public testing::TestWithParam<Param>
{
protected:
    void SetUp() override
    {
        const double scale = GetParam().scale;
        renderer = EglRendererTestAccess::create();
        ASSERT_TRUE(renderer);
        native = base::Size(base::roundToInt(kOutput.width() * scale),
                            base::roundToInt(kOutput.height() * scale));
        const EGLint attributes[] = {EGL_WIDTH, native.width(), EGL_HEIGHT, native.height(),
                                     EGL_NONE};
        const auto surface =
                eglCreatePbufferSurface(renderer->eglDisplay(), renderer->eglConfig(), attributes);
        ASSERT_NE(surface, EGL_NO_SURFACE);
        target = std::make_unique<EglRenderTarget>(renderer->eglDisplay(), surface, nullptr,
                                                   native);

        manager.setOutputBounds(kOutput);
        display::DisplaySnapshot physical;
        physical.sessionId = display::DisplayId("a");
        physical.persistentKey = "edid-a";
        physical.portKey = "port-a";
        physical.connectorName = "a";
        physical.devicePath = "/dev/dri/card0";
        physical.physicalSizeMm = {600, 340};
        physical.modes = {{native, 60000, 148500, native.width() + 100,
                           native.height() + 50, 0, "mode", true}};
        physical.currentMode = physical.modes.front();
        ASSERT_TRUE(displays.installHardware({"instance", "seat0", 1, {physical}}));
        const display::LogicalDisplay logical {
                display::DisplayId("a"),
                {display::DisplayId("a")},
                "Lingmo-a",
                kOutput,
                kOutput,
                *compositor::OutputScale::fromNumerator(uint32_t(base::roundToInt(scale * 120))),
                display::DisplayRotation::Normal,
                true};
        ASSERT_TRUE(displays.installApplied({"instance", 1, 1, {}, {logical}, {}}));
        manager.setDisplayManager(&displays);
        theme.setDarkMode(GetParam().dark);
        view = std::make_unique<switcher::SwitcherView>(switcherContainer, &manager, &theme);
        for (const char *id : {"one", "two", "three"})
            windows.push_back(open(id));
        marker = scene.root()->addChild(std::make_unique<MarkerNode>());
    }

    void TearDown() override
    {
        toolkit::setGpuRenderingAllowed(true);
        if (renderer && target) {
            renderer->setOutputContext(context());
            ASSERT_TRUE(renderer->beginFrame(target.get()));
            view->releaseRenderResources();
            renderer->cancelFrame();
        }
        view.reset();
        manager.setDisplayManager(nullptr);
        seat.shutdown();
        if (renderer && target)
            renderer->releaseTarget(target.get());
        target.reset();
        renderer.reset();
    }

    wm::Window *open(const std::string &id)
    {
        owned.push_back(compositor.createSurface());
        auto *surface = owned.back().get();
        surface->attach(pool->createBuffer({200, 100}, 0, 800, PixelFormat::Bgrx8888, nullptr), {});
        surface->commit();
        compositor.updateTextures(fake);
        return manager.createWindow(surface, {.requestedSize = {200, 100}, .title = id,
                                              .applicationId = id});
    }

    OutputRenderContext context()
    {
        return {"out", kOutput, native, GetParam().scale, 0,
                ++frame};
    }

    // A busy backdrop, the switcher over it and a node after it, as an output
    // draws them.
    ui::Bitmap composite()
    {
        renderer->setOutputContext(context());
        EXPECT_TRUE(renderer->beginFrame(target.get()));
        const ui::Bitmap image = renderer->captureFrame([&] {
            renderer->clear({0.2f, 0.4f, 0.8f, 1});
            for (int x = 0; x < kOutput.width(); x += 24)
                renderer->fillRect(base::Rect(x, 0, 12, kOutput.height()), {0.95f, 0.85f, 0.2f, 1});
            for (int y = 0; y < kOutput.height(); y += 40)
                renderer->fillRect(base::Rect(0, y, kOutput.width(), 6), {0.1f, 0.1f, 0.1f, 1});
            scene.root()->render(*renderer);
        });
        renderer->cancelFrame();
        return image;
    }

    static int differing(const ui::Bitmap &a, const ui::Bitmap &b, int threshold, base::Rect *where)
    {
        int count = 0;
        for (int y = 0; y < a.height(); ++y) {
            for (int x = 0; x < a.width(); ++x) {
                const uint32_t p = a.pixel(x, y), q = b.pixel(x, y);
                int d = 0;
                for (int shift = 0; shift < 32; shift += 8)
                    d = std::max(d, std::abs(int((p >> shift) & 0xff) - int((q >> shift) & 0xff)));
                if (d > threshold) {
                    ++count;
                    *where |= base::Rect(x, y, 1, 1);
                }
            }
        }
        return count;
    }

    std::unique_ptr<EglRenderer> renderer;
    std::unique_ptr<EglRenderTarget> target;
    base::Size native;
    uint64_t frame = 0;
    scene::Scene scene;
    scene::Node *surfaces = scene.root()->addChild(std::make_unique<scene::Node>("surfaces"));
    scene::Node *switcherContainer = scene.root()->addChild(std::make_unique<scene::Node>("switcher"));
    scene::Node *marker = nullptr;
    compositor::Compositor compositor {surfaces};
    testsupport::FakeRenderer fake;
    input::Seat seat {&compositor};
    wm::WindowManager manager {&compositor, &seat};
    display::DisplayManager displays;
    decoration::Theme theme;
    std::vector<std::unique_ptr<compositor::Surface>> owned;
    std::shared_ptr<compositor::ShmPool> pool = testsupport::makePool(1 << 20);
    std::vector<wm::Window *> windows;
    std::unique_ptr<switcher::SwitcherView> view;
};

TEST_P(SwitcherGpuPixels, GpuPanelCompositesAsTheCpuOne)
{
    view->showWindowCycle(windows, 1, "Lingmo-a");
    marker->setGeometry(view->bounds().adjusted(-20, -20, 20, 20));

    toolkit::setGpuRenderingAllowed(false);
    const ui::Bitmap cpu = composite();
    view->hideWindowCycle();
    view->showWindowCycle(windows, 1, "Lingmo-a");
    toolkit::setGpuRenderingAllowed(true);
    const ui::Bitmap gpu = composite();
    // Twice: the second frame reuses the texture the first one drew.
    const ui::Bitmap again = composite();

    base::Rect where;
    const int whole = differing(cpu, gpu, 2, &where);
    EXPECT_EQ(whole, 0) << "pixels over 2 in " << where.x() << "," << where.y() << " "
                        << where.width() << "x" << where.height();
    base::Rect whereAgain;
    EXPECT_EQ(differing(gpu, again, 2, &whereAgain), 0)
            << "in " << whereAgain.x() << "," << whereAgain.y() << " " << whereAgain.width() << "x"
            << whereAgain.height();
}

INSTANTIATE_TEST_SUITE_P(States, SwitcherGpuPixels,
                         testing::Values(Param {1.0, false}, Param {1.0, true}, Param {1.5, false},
                                         Param {1.5, true}));

} // namespace

