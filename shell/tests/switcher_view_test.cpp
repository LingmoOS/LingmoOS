// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <chrono>
#include <thread>

#include <gtest/gtest.h>

#include "compositor/compositor.h"
#include "compositor/surface.h"
#include "core/main_loop.h"
#include "fake_renderer.h"
#include "input/seat.h"
#include "scene/node.h"
#include "scene/scene.h"
#include "shm_test_util.h"
#include "decoration/decoration_theme.h"
#include "decoration/shadow_node.h"
#include "switcher/application_info.h"
#include "switcher/switcher_view.h"
#include "temporary_directory.h"
#include "wm/window.h"
#include "wm/window_manager.h"

using namespace Lingmo;

namespace {

struct Desktop {
    scene::Scene scene;
    scene::Node *surfaces =
            scene.root()->addChild(std::make_unique<scene::Node>("surfaces"));
    scene::Node *switcher =
            scene.root()->addChild(std::make_unique<scene::Node>("switcher"));
    compositor::Compositor compositor {surfaces};
    testsupport::FakeRenderer renderer;
    input::Seat seat {&compositor};
    wm::WindowManager manager {&compositor, &seat};
    std::vector<std::unique_ptr<compositor::Surface>> owned;
    std::shared_ptr<compositor::ShmPool> pool = testsupport::makePool(1 << 20);

    Desktop() { manager.setOutputBounds(base::Rect(0, 0, 1000, 800)); }
    ~Desktop() { seat.shutdown(); }

    wm::Window *open(const std::string &applicationId, const std::string &title)
    {
        owned.push_back(compositor.createSurface());
        auto *surface = owned.back().get();
        surface->attach(pool->createBuffer({200, 100}, 0, 800, render::PixelFormat::Bgrx8888,
                                           nullptr),
                        {});
        surface->commit();
        compositor.updateTextures(renderer);
        return manager.createWindow(surface, {.requestedSize = {200, 100},
                                              .title = title,
                                              .applicationId = applicationId});
    }
};

// renderImage()'s alpha at a point.
template <typename Image>
int alphaAt(const Image &image, const base::Point &point)
{
    return int(uint32_t(image.pixel(point.x(), point.y())) >> 24);
}

template <typename Image>
base::Size imageSize(const Image &image)
{
    return base::Size(image.width(), image.height());
}

} // namespace

TEST(SwitcherViewTest, FindsEntriesByIdAndWindowClassAndRemembersMisses)
{
    test::TemporaryDirectory applications;
    const auto writeEntry = [&](const char *name, const std::string &body) {
        ASSERT_TRUE(applications.write(name, "[Desktop Entry]\nType=Application\n" + body));
    };
    writeEntry("org.example.Editor.desktop", "Name=Editor\nIcon=editor-icon\n");
    writeEntry("viewer.desktop", "Name=Viewer\nIcon=viewer-icon\nStartupWMClass=ViewerWindow\n");

    switcher::ApplicationInfoCache cache({applications.path()});
    EXPECT_EQ(cache.lookup("org.example.Editor", "x").name, std::string("Editor"));
    EXPECT_EQ(cache.lookup("org.example.Editor", "x").icon, std::string("editor-icon"));
    EXPECT_EQ(cache.lookup("viewerwindow", "x").name, std::string("Viewer"));
    EXPECT_EQ(cache.lookup("paint", "x").name, std::string("paint"));
    EXPECT_EQ(cache.lookup("", "Untitled").name, std::string("Untitled"));

    // An application installed since is found.
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    writeEntry("paint.desktop", "Name=Paint\nIcon=paint-icon\n");
    cache.refresh();
    EXPECT_EQ(cache.lookup("paint", "x").name, std::string("Paint"));
}

TEST(SwitcherViewTest, DrawsARowCentredOnTheOutputAndFindsItsEntries)
{
    Desktop desktop;
    wm::Window *editor = desktop.open("no-such-editor", "Doc");
    wm::Window *viewer = desktop.open("", "Viewer");
    switcher::SwitcherView view(desktop.switcher, &desktop.manager, nullptr);

    view.showWindowCycle({editor, viewer}, 1, std::string());
    const base::Rect bounds = view.bounds();
    const double cell = switcher::SwitcherView::kCellSize;
    EXPECT_EQ(bounds.size(),
              base::Size(base::roundToInt(2 * cell + 2 * switcher::SwitcherView::kPadding),
                         base::roundToInt(cell + 2 * switcher::SwitcherView::kVerticalPadding)));
    // Both extents are even, so whole-pixel centres compare as exact ones.
    EXPECT_EQ(bounds.center(), base::Rect(0, 0, 1000, 800).center());

    const auto image = view.renderImage();
    EXPECT_TRUE(!image.isNull());
    EXPECT_EQ(imageSize(image), bounds.size());
    const int padding = base::roundToInt(switcher::SwitcherView::kPadding);
    const int top = base::roundToInt(switcher::SwitcherView::kVerticalPadding);
    // The selection's glass sits centred on the second cell.
    const base::Rect selection = view.selectionBounds();
    const base::PointF secondCell = base::PointF(bounds.topLeft() + base::Point(padding, top))
            + base::PointF(cell * 1.5, cell / 2);
    EXPECT_TRUE(base::fuzzyCompare(base::RectF(selection).center(), secondCell));
    EXPECT_EQ(selection.width(), base::roundToInt(cell * switcher::SwitcherView::kSelectionRatio));

    // Moving the selection repaints only what changed; the rest stays.
    const base::Point firstIcon(padding + base::roundToInt(cell / 2), top + base::roundToInt(cell / 2));
    EXPECT_TRUE(alphaAt(image, firstIcon) > 0);
    view.selectWindowCycleEntry(0);
    const auto moved = view.renderImage();
    EXPECT_TRUE(alphaAt(moved, firstIcon + base::Point(base::roundToInt(cell), 0)) > 0);
    EXPECT_TRUE(alphaAt(moved, firstIcon) > 0);
    EXPECT_EQ(view.selectionBounds().translated(base::roundToInt(cell), 0), selection);

    const base::PointF row = base::PointF(bounds.topLeft()) + base::PointF(padding, top);
    EXPECT_EQ(view.windowCycleEntryAt(row + base::PointF(10, 10)), 0);
    EXPECT_EQ(view.windowCycleEntryAt(row + base::PointF(cell + 10, 10)), 1);
    EXPECT_EQ(view.windowCycleEntryAt(row + base::PointF(10, cell + 10)), -1);
    EXPECT_EQ(view.windowCycleEntryAt(base::PointF(bounds.topLeft()) - base::PointF(5, 5)), -1);
    EXPECT_TRUE(view.windowCycleViewContains(base::PointF(bounds.center())));

    view.hideWindowCycle();
    EXPECT_TRUE(!view.windowCycleViewContains(base::PointF(bounds.center())));
    EXPECT_TRUE(view.renderImage().isNull());
}

TEST(SwitcherViewTest, AListOfAnotherLengthGetsATextureOfItsSize)
{
    Desktop desktop;
    wm::Window *first = desktop.open("one", "");
    wm::Window *second = desktop.open("two", "");
    wm::Window *third = desktop.open("three", "");
    switcher::SwitcherView view(desktop.switcher, &desktop.manager, nullptr);
    const auto drawnSize = [&desktop] {
        desktop.renderer.calls.clear();
        desktop.switcher->render(desktop.renderer);
        for (const auto &call : desktop.renderer.calls) {
            if (call.kind == "texture")
                return call.texture->size();
        }
        return base::Size(-1, -1);
    };

    view.showWindowCycle({first}, 0, std::string());
    EXPECT_EQ(drawnSize(), imageSize(view.renderImage()));
    view.hideWindowCycle();
    view.showWindowCycle({first, second, third}, 1, std::string());
    EXPECT_EQ(drawnSize(), imageSize(view.renderImage()));
}

TEST(SwitcherViewTest, ThePanelCastsAShadow)
{
    Desktop desktop;
    wm::Window *window = desktop.open("one", "");
    decoration::Theme theme;
    switcher::SwitcherView view(desktop.switcher, &desktop.manager, &theme);
    view.showWindowCycle({window}, 0, std::string());
    auto *shadow = dynamic_cast<decoration::ShadowNode *>(desktop.switcher->children().front());
    ASSERT_TRUE(shadow);
    EXPECT_TRUE(shadow->isVisible());
    EXPECT_TRUE(shadow->elevation() != wm::ShadowElevation::None);
    EXPECT_TRUE(shadow->absoluteGeometry().contains(view.bounds()));
    // The glass's own corner: a tighter one shows past it as a dark crescent.
    EXPECT_EQ(shadow->radius(), switcher::SwitcherView::kRadius);
}

TEST(SwitcherViewTest, LeavesNoNodesBehindInTheScene)
{
    Desktop desktop;
    wm::Window *window = desktop.open("one", "");
    decoration::Theme theme;
    {
        switcher::SwitcherView view(desktop.switcher, &desktop.manager, &theme);
        view.showWindowCycle({window}, 0, std::string());
        EXPECT_EQ(desktop.switcher->children().size(), 2u);
    }
    EXPECT_TRUE(desktop.switcher->children().empty());
    // The compositor releases the scene after the view is gone.
    desktop.scene.root()->releaseRenderResources();
}

TEST(SwitcherViewTest, ShrinksTheCellsToFitTheOutput)
{
    Desktop desktop;
    std::vector<wm::Window *> windows;
    for (int i = 0; i < 12; ++i)
        windows.push_back(desktop.open("app" + std::to_string(i), ""));
    switcher::SwitcherView view(desktop.switcher, &desktop.manager, nullptr);
    view.showWindowCycle(windows, 0, std::string());
    EXPECT_TRUE(view.bounds().width() <= 1000 - 2 * switcher::SwitcherView::kScreenMargin);
    EXPECT_TRUE(view.bounds().left() >= 0);
}

