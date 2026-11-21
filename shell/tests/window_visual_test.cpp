// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "decoration/decoration.h"
#include "decoration/decoration_theme.h"
#include "wm/window.h"
#include "wm/window_manager.h"
#include "wm/window_visual_controller.h"

using namespace Lingmo;
using Mode = scene::WindowVisualMode;

TEST(WindowVisualPolicy, ContinuousCornersExtendBeyondCircularTangency)
{
    const render::RoundedShape shape {{0, 0, 100, 80}, 20};
    EXPECT_FALSE(shape.contains({20, 0}));
    EXPECT_TRUE(shape.contains({30, 0}));
    EXPECT_FALSE(shape.contains({6, 6}));
    EXPECT_TRUE(shape.contains({7, 7}));
    for (const base::PointF point : {base::PointF(7, 7), {93, 7}, {7, 73}, {93, 73}})
        EXPECT_TRUE(shape.contains(point));

    const render::RoundedShape stretched {{10, 20, 200, 40}, 20, {2, 0.5}};
    for (int y = 0; y <= 80; ++y) {
        for (int x = 0; x <= 100; ++x)
            EXPECT_EQ(shape.contains({double(x), double(y)}),
                      stretched.contains({10.0 + x * 2, 20.0 + y * 0.5}));
    }
    const render::RoundedShape square {{0, 0, 100, 80}, 0};
    EXPECT_TRUE(square.contains({0, 0}));
    EXPECT_FALSE(square.contains({-1, 0}));
}

TEST(WindowVisualPolicy, CircularCornersFollowTheSessionStyle)
{
    struct Restore {
        ~Restore() { render::setContinuousCorners(true); }
    } restore;
    render::setContinuousCorners(false);
    const render::RoundedShape shape {{0, 0, 100, 80}, 20};
    // The arc is tangent at the radius; the continuous corner reaches 1.5x it.
    EXPECT_TRUE(shape.contains({20, 0}));
    EXPECT_FALSE(shape.contains({19, 0}));
    EXPECT_FALSE(shape.contains({5, 5}));
    EXPECT_TRUE(shape.contains({6, 6}));
    const render::RoundedShape stretched {{10, 20, 200, 40}, 20, {2, 0.5}};
    for (int y = 0; y <= 80; ++y) {
        for (int x = 0; x <= 100; ++x)
            EXPECT_EQ(shape.contains({double(x), double(y)}),
                      stretched.contains({10.0 + x * 2, 20.0 + y * 0.5}));
    }
}

TEST(WindowVisualPolicy, OwnershipGeometryStateAndActivation)
{
    decoration::Theme theme;
    decoration::DecorationProvider frames(&theme);
    wm::WindowManager manager(nullptr, nullptr);
    manager.setFrameProvider(&frames);
    manager.setOutputBounds({0, 0, 1200, 900});
    auto *window = manager.createWindow(nullptr, {.requestedSize = {400, 300}});
    auto state = [&] {
        return wm::WindowVisualController::evaluate(*window, theme.visualStyle());
    };
    EXPECT_EQ(state().mode, Mode::ServerExterior);
    EXPECT_EQ(window->contentBounds(), window->bounds());
    EXPECT_TRUE(window->frameInsets().isNull());
    EXPECT_TRUE(state().clipContent);
    EXPECT_TRUE(state().drawOutline);
    EXPECT_EQ(state().shape.radius, 14);
    EXPECT_EQ(state().outlineWidthPx, 1.0);
    EXPECT_FLOAT_EQ(state().outlineColor.a, decoration::WindowVisualStyle().activeOutline.a);
    const auto inactive = state().outlineColor;
    window->activate();
    EXPECT_EQ(state().outlineColor.r, inactive.r);
    EXPECT_FLOAT_EQ(state().outlineColor.r, decoration::WindowVisualStyle().activeOutline.r);
    EXPECT_FLOAT_EQ(state().highlightColor.a, decoration::WindowVisualStyle().highlight.a);
    window->setClientExteriorInsets({12, 12, 12, 12});
    EXPECT_EQ(state().mode, Mode::ServerExterior);
    EXPECT_TRUE(state().clipContent);
    EXPECT_TRUE(state().drawOutline);
    const auto content = window->contentBounds();
    window->setDecorated(true);
    EXPECT_EQ(state().mode, Mode::ServerFrame);
    EXPECT_EQ(window->contentBounds(), content);
    EXPECT_EQ(window->frameInsets(), theme.metrics().insets());
    EXPECT_EQ(window->frameInsets().left, 0);
    EXPECT_EQ(window->frameInsets().right, 0);
    EXPECT_EQ(window->frameInsets().bottom, 0);
    EXPECT_EQ(window->bounds().width(), content.width());
    for (bool fullscreen : {false, true}) {
        if (fullscreen)
            window->setFullscreen(true);
        else
            window->maximize();
        EXPECT_EQ(state().shape.radius, 0);
        EXPECT_FALSE(state().drawOutline);
        EXPECT_TRUE(state().clipContent);
        if (fullscreen)
            window->setFullscreen(false);
        window->restore();
        EXPECT_EQ(state().shape.radius, 14);
        EXPECT_TRUE(state().drawOutline);
    }
    window->setDecorated(false);
    EXPECT_EQ(state().mode, Mode::ServerExterior);
    window->setClientExteriorInsets({0, 0, 20, 20});
    EXPECT_EQ(state().mode, Mode::ServerExterior);
    window->setRole(wm::WindowRole::Notification);
    EXPECT_EQ(state().mode, Mode::PreserveClient);
    manager.destroyWindow(window);
}

TEST(WindowVisualPolicy, OnlyNormalWindowsBlockedByAModalAreDimmed)
{
    decoration::Theme theme;
    wm::WindowManager manager(nullptr, nullptr);
    manager.setOutputBounds({0, 0, 1200, 900});
    auto *parent = manager.createWindow(nullptr, {.requestedSize = {400, 300}});
    auto *child = manager.createWindow(nullptr, {.requestedSize = {200, 100}});
    auto dim = [&](wm::Window *window) {
        return wm::WindowVisualController::evaluate(*window, theme.visualStyle()).dimOpacity;
    };
    ASSERT_TRUE(child->setTransientParent(parent));
    EXPECT_FLOAT_EQ(dim(parent), 0);
    child->setModal(true);
    EXPECT_FLOAT_EQ(dim(parent), theme.visualStyle().dimOpacity);
    EXPECT_FLOAT_EQ(dim(child), 0);
    parent->maximize();
    EXPECT_FLOAT_EQ(dim(parent), theme.visualStyle().dimOpacity);
    parent->setRole(wm::WindowRole::Notification);
    EXPECT_FLOAT_EQ(dim(parent), 0);
    manager.destroyWindow(child);
    manager.destroyWindow(parent);

    theme.setDarkMode(true);
    EXPECT_GT(theme.visualStyle().dimOpacity, decoration::WindowVisualStyle().dimOpacity);
}

TEST(WindowVisualPolicy, AnUnresponsiveWindowIsDimmedLikeABlockedOne)
{
    decoration::Theme theme;
    wm::WindowManager manager(nullptr, nullptr);
    manager.setOutputBounds({0, 0, 1200, 900});
    auto *window = manager.createWindow(nullptr, {.requestedSize = {400, 300}});
    auto dim = [&] {
        return wm::WindowVisualController::evaluate(*window, theme.visualStyle()).dimOpacity;
    };
    EXPECT_FLOAT_EQ(dim(), 0);
    window->setResponsive(false);
    EXPECT_FLOAT_EQ(dim(), theme.visualStyle().dimOpacity);
    window->setResponsive(true);
    EXPECT_FLOAT_EQ(dim(), 0);
    manager.destroyWindow(window);
}

TEST(WindowVisualPolicy, ASheetIsClippedAndOutlinedLikeAWindow)
{
    decoration::Theme theme;
    wm::WindowManager manager(nullptr, nullptr);
    manager.setOutputBounds({0, 0, 1200, 900});
    auto *sheet = manager.createWindow(nullptr, {.requestedSize = {400, 140}});
    sheet->setRole(wm::WindowRole::Sheet);
    const auto state = wm::WindowVisualController::evaluate(*sheet, theme.visualStyle());
    EXPECT_EQ(Mode::ServerExterior, state.mode);
    EXPECT_TRUE(state.clipContent);
    EXPECT_TRUE(state.drawOutline);
    EXPECT_GT(state.shape.radius, 0);
    manager.destroyWindow(sheet);
}

TEST(WindowVisualPolicy, SmallWindowsClampAllCornersAndDarkModeDeepensTheOutline)
{
    wm::Window window(nullptr, nullptr);
    window.setBounds({0, 0, 18, 10});
    const auto shape = wm::WindowVisualController::evaluate(window).shape;
    EXPECT_EQ(shape.radius, 5);
    for (const base::PointF corner : {base::PointF(0, 0), {18, 0}, {0, 10}, {18, 10}})
        EXPECT_FALSE(shape.contains(corner));
    EXPECT_TRUE(shape.contains({9, 0}));
    decoration::Theme theme;
    theme.setDarkMode(true);
    const auto light = decoration::WindowVisualStyle();
    for (const auto color :
         {theme.visualStyle().activeOutline, theme.visualStyle().inactiveOutline}) {
        EXPECT_GT(color.a, light.activeOutline.a);
        EXPECT_FLOAT_EQ(color.r, light.activeOutline.r);
    }
    EXPECT_GT(theme.visualStyle().dockOutline.a, light.dockOutline.a);
    EXPECT_EQ(theme.visualStyle().radius, light.radius);
}
