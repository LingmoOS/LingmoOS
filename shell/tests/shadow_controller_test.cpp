// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/wm/core/shadow_controller_unittest.cc
//           chromium/src/ash/wm/wm_shadow_controller_delegate.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Compositor-owned window shadows, from the window manager's side.
//
// Adapted for Lingmo: exercises the corresponding behavior with native
// compositor test doubles and APIs.
//
// The cases upstream has and Lingmo keeps are ShadowBounds, ShadowStyle,
// ShowState and teardown. The one it cannot have is the CSD pair: ChromeOS
// windows never draw their own exterior, so nothing upstream has to decide
// whether the client already owns the space around its window geometry.

#include <gtest/gtest.h>
#include <memory>
#include <vector>

#include "base/geometry.h"
#include "compositor/compositor.h"
#include "compositor/shm_pool.h"
#include "compositor/surface.h"
#include "decoration/decoration.h"
#include "decoration/decoration_theme.h"
#include "decoration/raster.h"
#include "decoration/shadow_node.h"
#include "fake_renderer.h"
#include "render/scanout.h"
#include "scene/node.h"
#include "scene/overlay_candidate.h"
#include "scene/scene.h"
#include "scene/surface_node.h"
#include "scene/window_visual_node.h"
#include "shm_test_util.h"
#include "wm/activation.h"
#include "wm/shadow_controller.h"
#include "wm/shell_surface.h"
#include "wm/toplevel_event_handler.h"
#include "wm/window.h"
#include "wm/window_manager.h"
#include "wm/window_state.h"
#include "wm/wm_shadow_controller_delegate.h"

using namespace Lingmo;
using namespace Lingmo::Wm;
using Lingmo::decoration::Corners;

namespace {

const base::Rect kOutput(0, 0, 1000, 800);

struct Recorder {
    std::vector<uint32_t> serials;
    uint32_t nextSerial = 1;
};

// A buffer a hardware plane could show. Copied from overlay_candidate_test:
// the planner only checks that one is there, never what is in it.
class TestScanoutBuffer final : public render::ScanoutBuffer
{
public:
    explicit TestScanoutBuffer(const base::Size &size)
    {
        m_attributes.size = size;
        m_attributes.format = 0x34325258; // DRM_FORMAT_XRGB8888
        m_attributes.modifier = 0;
        m_attributes.planes.push_back(render::DmaBufPlane {ScopedFd(), 0, 4096});
    }

    const render::DmaBufAttributes &scanoutAttributes() const override { return m_attributes; }

private:
    render::DmaBufAttributes m_attributes;
};

// Stands in for real content; the planner only checks that one is there.
class TestTexture final : public render::Texture
{
public:
    explicit TestTexture(const base::Size &size)
        : m_size(size)
    {
    }

    base::Size size() const override { return m_size; }

private:
    base::Size m_size;
};

class ShadowControllerTest : public testing::Test
{
protected:
    scene::Scene scene;
    scene::Node *surfaceContainer =
            scene.root()->addChild(std::make_unique<scene::Node>("surfaces"));
    compositor::Compositor compositor {surfaceContainer};
    testsupport::FakeRenderer renderer;
    WindowManager manager {&compositor, nullptr};
    decoration::Theme theme;
    decoration::DecorationProvider frames {&theme};
    decoration::ShadowProvider shadows {&theme};
    WmShadowControllerDelegate delegate;
    std::unique_ptr<ShadowController> controller;

    std::vector<std::unique_ptr<compositor::Surface>> surfaces;
    std::vector<std::unique_ptr<ShellSurface>> shells;
    std::vector<std::unique_ptr<Recorder>> recorders;
    std::shared_ptr<compositor::ShmPool> pool = testsupport::makePool(8 << 20);

    void SetUp() override
    {
        scene.setSize(kOutput.size());
        manager.setOutputBounds(kOutput);
        manager.setFrameProvider(&frames);
        controller = std::make_unique<ShadowController>(&manager, &shadows, &delegate);
    }

    void TearDown() override
    {
        shells.clear();
        controller.reset();
    }

    void attach(compositor::Surface *surface, const base::Size &size)
    {
        auto buffer = pool->createBuffer(size, 0, size.width() * 4, render::PixelFormat::Bgrx8888,
                                         nullptr);
        surface->attach(buffer, {});
        surface->damage(base::Rect(base::Point(), size));
    }

    // The xdg-shell handshake. `geometry`, when non-null, is committed as the
    // window geometry before the first buffer, which is how a CSD client
    // declares that part of its surface is exterior decoration.
    ShellSurface *makeToplevel(const base::Size &bufferSize, bool serverSide,
                               const base::Rect *geometry = nullptr)
    {
        recorders.push_back(std::make_unique<Recorder>());
        Recorder *recorder = recorders.back().get();
        surfaces.push_back(compositor.createSurface());
        shells.push_back(std::make_unique<ShellSurface>(&manager, surfaces.back().get()));
        ShellSurface *shell = shells.back().get();
        shell->setConfigureCallback([recorder](const ShellSurface::ConfigureState &) {
            recorder->serials.push_back(recorder->nextSerial);
            return recorder->nextSerial++;
        });

        shell->setToplevel();
        shell->setHasDecorationObject(true);
        shell->setRequestedDecorationMode(serverSide ? ShellSurface::DecorationMode::ServerSide
                                                     : ShellSurface::DecorationMode::ClientSide);
        shell->rootSurface()->commit();
        EXPECT_TRUE(shell->acknowledgeConfigure(recorder->serials.back()));
        if (geometry)
            shell->setGeometry(*geometry);
        attach(shell->rootSurface(), bufferSize);
        shell->rootSurface()->commit();
        compositor.updateTextures(renderer);
        return shell;
    }

    decoration::Shadow *shadowOf(Window *window)
    {
        return static_cast<decoration::Shadow *>(controller->shadowForWindow(window));
    }

    ShadowElevation elevationOf(Window *window)
    {
        WindowShadow *shadow = controller->shadowForWindow(window);
        return shadow ? shadow->elevation() : ShadowElevation::None;
    }
};

////////////////////////////////////////////////////////////////////////////////
// Which windows get a shadow at all.

// Upstream: ShadowControllerTest.Shadow.
TEST_F(ShadowControllerTest, ServerSideDecoratedWindowGetsShadow)
{
    Window *window = makeToplevel({400, 300}, true)->window();
    ASSERT_TRUE(window);
    ASSERT_TRUE(window->isDecorated());

    decoration::Shadow *shadow = shadowOf(window);
    ASSERT_TRUE(shadow);
    EXPECT_EQ(shadow->elevation(), ShadowElevation::Active);

    // The shadow's node is the window's own, not the decoration's, and it is
    // underneath everything else the window draws.
    ASSERT_TRUE(shadow->node());
    EXPECT_EQ(shadow->node()->parent(), window->node());
    EXPECT_EQ(window->node()->children().front(), shadow->node());
    EXPECT_EQ(shadow->node()->content(), scene::Node::Content::Translucent);

    // It reaches outside the window by the style's exterior margins.
    const base::Insets margins = shadow->node()->margins();
    EXPECT_GT(margins.left(), 0);
    EXPECT_GT(margins.top(), 0);
    EXPECT_EQ(shadow->shadowBounds(), window->bounds().marginsAdded(margins));
}

TEST_F(ShadowControllerTest, DockShadowFollowsPanelGlass)
{
    ShellSurface *shell = makeToplevel({800, 86}, false);
    Window *window = shell->window();
    window->setRole(WindowRole::Dock);
    const base::Rect panel(200, 0, 400, 86);
    shell->rootSurface()->setBackgroundBlur(panel, 28);
    shell->rootSurface()->commit();
    controller->handlePossibleShadowVisibilityChange(window);

    decoration::Shadow *shadow = shadowOf(window);
    ASSERT_TRUE(shadow);
    EXPECT_EQ(shadow->elevation(), ShadowElevation::Dock);
    EXPECT_EQ(shadow->node()->geometry(), panel.marginsAdded(shadow->node()->margins()));
    window->setVisible(false);
    EXPECT_EQ(shadow->elevation(), ShadowElevation::None);
}

// The largest dock icons make a panel whose corner radius, 28/86 of its
// thickness, is past the largest shadow corner. It keeps a shadow.
TEST_F(ShadowControllerTest, TallDockKeepsItsShadow)
{
    ShellSurface *shell = makeToplevel({1200, 204}, false);
    Window *window = shell->window();
    window->setRole(WindowRole::Dock);
    const base::Rect panel(200, 0, 800, 204);
    shell->rootSurface()->setBackgroundBlur(panel, 66);
    shell->rootSurface()->commit();
    controller->handlePossibleShadowVisibilityChange(window);

    decoration::Shadow *shadow = shadowOf(window);
    ASSERT_TRUE(shadow);
    EXPECT_EQ(shadow->elevation(), ShadowElevation::Dock);
    EXPECT_TRUE(shadow->node()->isVisible());
    EXPECT_EQ(shadow->node()->geometry(), panel.marginsAdded(shadow->node()->margins()));
}

// A FishUI window is frameless and declares its whole surface as its window
// geometry, so there is no exterior area and the compositor owns the shadow.
TEST_F(ShadowControllerTest, FramelessClientDecoratedWindowGetsShadow)
{
    const base::Rect geometry(0, 0, 400, 300);
    Window *window = makeToplevel({400, 300}, false, &geometry)->window();
    ASSERT_TRUE(window);
    EXPECT_FALSE(window->isDecorated());
    EXPECT_EQ(window->clientExteriorInsets(), Insets());
    EXPECT_FALSE(window->hasClientExteriorDecoration());
    EXPECT_EQ(elevationOf(window), ShadowElevation::Active);
}

// A window that draws a shaped body in a transparent surface, a bubble with an
// arrow, casts its shadow from that outline, and is neither clipped nor
// outlined as a rounded rectangle.
TEST_F(ShadowControllerTest, ABodyShapeCastsTheShadowAndKeepsTheWindowAsDrawn)
{
    Window *window = makeToplevel({200, 160}, false)->window();
    ASSERT_TRUE(window);
    ui::Path bubble;
    bubble.moveTo(100, 0);
    bubble.lineTo(112, 12);
    bubble.lineTo(200, 12);
    bubble.lineTo(200, 160);
    bubble.lineTo(0, 160);
    bubble.lineTo(0, 12);
    bubble.lineTo(88, 12);
    bubble.close();
    window->rootSurface()->setBodyShape(bubble);
    EXPECT_EQ(elevationOf(window), ShadowElevation::Active);
    decoration::ShadowNode *node = shadowOf(window)->node();
    ASSERT_TRUE(node);
    EXPECT_FALSE(node->silhouette().isEmpty());
    const auto *visual = static_cast<scene::WindowVisualNode *>(window->visualNode());
    EXPECT_EQ(scene::WindowVisualMode::PreserveClient, visual->state().mode);
    EXPECT_FALSE(visual->state().clipContent);
    EXPECT_FALSE(visual->state().drawOutline);

    window->rootSurface()->setBodyShape({});
    EXPECT_TRUE(node->silhouette().isEmpty());
    EXPECT_TRUE(static_cast<scene::WindowVisualNode *>(window->visualNode())->state().drawOutline);
}

// A client that draws its own shadow inside a transparent surface asks for
// none: with its body shape the window shows exactly as drawn.
TEST_F(ShadowControllerTest, AClientCanDeclineTheShadow)
{
    Window *window = makeToplevel({200, 160}, false)->window();
    ASSERT_TRUE(window);
    ui::Path body;
    body.moveTo(40, 40);
    body.lineTo(160, 40);
    body.lineTo(160, 120);
    body.lineTo(40, 120);
    body.close();
    window->rootSurface()->setBodyShape(body);
    window->rootSurface()->setCastsShadow(false);
    EXPECT_EQ(elevationOf(window), ShadowElevation::None);
    const auto *visual = static_cast<scene::WindowVisualNode *>(window->visualNode());
    EXPECT_FALSE(visual->state().clipContent);
    EXPECT_FALSE(visual->state().drawOutline);

    window->rootSurface()->setCastsShadow(true);
    EXPECT_EQ(elevationOf(window), ShadowElevation::Active);
}

// A client that never sets a window geometry is the same case: its buffer is
// its window.
TEST_F(ShadowControllerTest, ClientWithoutWindowGeometryGetsShadow)
{
    Window *window = makeToplevel({400, 300}, false)->window();
    ASSERT_TRUE(window);
    EXPECT_FALSE(window->hasClientExteriorDecoration());
    EXPECT_EQ(elevationOf(window), ShadowElevation::Active);
}

TEST_F(ShadowControllerTest, ExteriorClientDecorationGetsCompositorShadowAroundBody)
{
    const base::Rect geometry(30, 20, 400, 300);
    ShellSurface *shell = makeToplevel({460, 350}, false, &geometry);
    Window *window = shell->window();
    ASSERT_TRUE(window);

    EXPECT_EQ(window->clientExteriorInsets(), (Insets {30, 20, 30, 30}));
    EXPECT_TRUE(window->hasClientExteriorDecoration());
    EXPECT_EQ(elevationOf(window), ShadowElevation::Active);
    ASSERT_TRUE(shadowOf(window)->node());
    EXPECT_TRUE(shadowOf(window)->node()->isVisible());
    EXPECT_EQ(shadowOf(window)->node()->geometry(),
              base::Rect(0, 0, 400, 300).marginsAdded(shadowOf(window)->node()->margins()));

    // The same window after it stops drawing its own exterior.
    const base::Rect full(0, 0, 460, 350);
    shell->setGeometry(full);
    shell->rootSurface()->commit();
    EXPECT_FALSE(window->hasClientExteriorDecoration());
    EXPECT_EQ(elevationOf(window), ShadowElevation::Active);
}

// A menu that paints its own shadow may declare its whole surface as window
// geometry and only keep the shadow band out of its input region.
TEST_F(ShadowControllerTest, PopupWithInputMarginDrawsItsOwnShadow)
{
    ShellSurface *parent = makeToplevel({400, 300}, false);
    surfaces.push_back(compositor.createSurface());
    shells.push_back(std::make_unique<ShellSurface>(&manager, surfaces.back().get()));
    ShellSurface *popup = shells.back().get();
    ASSERT_TRUE(popup->setPopup(parent, base::Rect(10, 20, 120, 90)));
    popup->setGeometry(base::Rect(0, 0, 120, 90));
    attach(popup->rootSurface(), {120, 90});
    popup->rootSurface()->commit();
    Window *window = popup->window();
    ASSERT_TRUE(window);
    EXPECT_EQ(window->clientExteriorInsets(), Insets());
    EXPECT_EQ(elevationOf(window), ShadowElevation::MenuOrTooltip);

    popup->rootSurface()->setInputRegion(base::Region(base::Rect(4, 4, 112, 82)));
    popup->rootSurface()->commit();
    EXPECT_EQ(window->clientInputInsets(), (Insets {4, 4, 4, 4}));
    EXPECT_EQ(elevationOf(window), ShadowElevation::None);

    // An input region flush with any edge is not a surrounding band.
    popup->rootSurface()->setInputRegion(base::Region(base::Rect(0, 4, 116, 82)));
    popup->rootSurface()->commit();
    EXPECT_FALSE(window->hasClientInputMargin());
    EXPECT_EQ(elevationOf(window), ShadowElevation::MenuOrTooltip);
}

// When the compositor drew the frame it owns the space around it, whatever the
// client's window geometry still says about its own exterior.
TEST_F(ShadowControllerTest, ServerSideDecorationOverridesClientExterior)
{
    const base::Rect geometry(30, 20, 400, 300);
    Window *window = makeToplevel({460, 350}, true, &geometry)->window();
    ASSERT_TRUE(window->isDecorated());
    EXPECT_TRUE(window->hasClientExteriorDecoration());
    EXPECT_EQ(elevationOf(window), ShadowElevation::Active);
}

// Both compositor-owned modes use the same complete rounded silhouette.
TEST_F(ShadowControllerTest, SilhouetteMatchesBothCompositorOwnedModes)
{
    Window *decorated = makeToplevel({400, 300}, true)->window();
    ASSERT_TRUE(decorated->isDecorated());
    const Corners ssd = shadowOf(decorated)->node()->corners();
    EXPECT_TRUE(ssd.topLeft);
    EXPECT_TRUE(ssd.bottomLeft);
    EXPECT_TRUE(ssd.bottomRight);

    const base::Rect geometry(0, 0, 400, 300);
    Window *frameless = makeToplevel({400, 300}, false, &geometry)->window();
    ASSERT_FALSE(frameless->isDecorated());
    const Corners csd = shadowOf(frameless)->node()->corners();
    EXPECT_TRUE(csd.topLeft);
    EXPECT_TRUE(csd.bottomLeft);
    EXPECT_TRUE(csd.bottomRight);

    // Both are cut with the window radius the whole desktop shares. Cutting a
    // frameless window with anything smaller than the radius its own toolkit
    // rounds by leaves an unshadowed sliver outside its arc.
    EXPECT_EQ(shadowOf(frameless)->node()->radius(), theme.metrics().cornerRadius);
    EXPECT_EQ(shadowOf(decorated)->node()->radius(), theme.metrics().cornerRadius);

    // And it follows the frame appearing and disappearing later on.
    frameless->setDecorated(true);
    EXPECT_TRUE(shadowOf(frameless)->node()->corners().bottomLeft);
    frameless->setDecorated(false);
    EXPECT_TRUE(shadowOf(frameless)->node()->corners().bottomLeft);
}

////////////////////////////////////////////////////////////////////////////////
// Elevation.

// Upstream: ShadowControllerTest.ShadowStyle.
TEST_F(ShadowControllerTest, ActivationChangesElevation)
{
    Window *first = makeToplevel({400, 300}, true)->window();
    Window *second = makeToplevel({400, 300}, true)->window();

    manager.activation()->activate(first);
    EXPECT_EQ(elevationOf(first), ShadowElevation::Active);
    EXPECT_EQ(elevationOf(second), ShadowElevation::Inactive);

    manager.activation()->activate(second);
    EXPECT_EQ(elevationOf(first), ShadowElevation::Inactive);
    EXPECT_EQ(elevationOf(second), ShadowElevation::Active);

    // Both elevations keep the same exterior margins, so activation never
    // moves the window's scene bounds.
    EXPECT_EQ(shadowOf(first)->node()->margins(), shadowOf(second)->node()->margins());
}

// A window being moved reads as lifted, so it keeps the active elevation even
// while something else holds activation.
TEST_F(ShadowControllerTest, DraggedWindowKeepsActiveElevation)
{
    Window *dragged = makeToplevel({400, 300}, true)->window();
    Window *other = makeToplevel({400, 300}, true)->window();
    manager.activation()->activate(other);
    ASSERT_EQ(elevationOf(dragged), ShadowElevation::Inactive);

    ASSERT_TRUE(manager.eventHandler()->beginMove(dragged, centerPixel(dragged->bounds())));
    EXPECT_TRUE(dragged->isDragged());
    EXPECT_EQ(elevationOf(dragged), ShadowElevation::Active);

    manager.eventHandler()->completeDrag(ToplevelEventHandler::DragResult::Success);
    EXPECT_FALSE(dragged->isDragged());
    EXPECT_EQ(elevationOf(dragged), ShadowElevation::Inactive);
}

////////////////////////////////////////////////////////////////////////////////
// Window state.

// Upstream: ShadowControllerTest.ShowState.
TEST_F(ShadowControllerTest, MaximizeAndFullscreenRemoveShadowAndRestoreBringsItBack)
{
    Window *window = makeToplevel({400, 300}, true)->window();
    ASSERT_EQ(elevationOf(window), ShadowElevation::Active);
    const base::Rect normalBounds = window->bounds();

    window->maximize();
    ASSERT_EQ(window->stateType(), WindowStateType::Maximized);
    EXPECT_EQ(elevationOf(window), ShadowElevation::None);
    EXPECT_FALSE(shadowOf(window)->node()->isVisible());

    window->restore();
    EXPECT_EQ(elevationOf(window), ShadowElevation::Active);
    EXPECT_EQ(shadowOf(window)->contentBounds(), normalBounds);

    window->setFullscreen(true);
    ASSERT_EQ(window->stateType(), WindowStateType::Fullscreen);
    EXPECT_EQ(elevationOf(window), ShadowElevation::None);

    window->setFullscreen(false);
    EXPECT_EQ(elevationOf(window), ShadowElevation::Active);

    window->minimize();
    EXPECT_FALSE(window->isVisible());
    EXPECT_EQ(elevationOf(window), ShadowElevation::None);

    // Minimizing gave activation away, so the restored window is shadowed
    // again but at the inactive elevation.
    window->restore();
    EXPECT_TRUE(window->isVisible());
    EXPECT_EQ(elevationOf(window), ShadowElevation::Inactive);
}

TEST_F(ShadowControllerTest, DraggingOutOfAMaximizedStateWaitsForTheSmallerBuffer)
{
    auto *shell = makeToplevel({400, 300}, false);
    Window *window = shell->window();
    window->maximize();
    ASSERT_EQ(elevationOf(window), ShadowElevation::None);
    const base::Size maximized = window->bounds().size();
    attach(shell->rootSurface(), maximized);
    shell->rootSurface()->commit();
    compositor.updateTextures(renderer);
    ASSERT_EQ(window->presentedBounds().size(), maximized);

    // Past the threshold the window is presented as a restored one, but the
    // client is still showing the maximized buffer. A shadow around that would
    // sit at the edges of the work area, not at the edges of the window.
    ASSERT_TRUE(manager.eventHandler()->beginMove(window, base::PointF(500, 10)));
    manager.eventHandler()->onPointerMotion(base::PointF(500, 300));
    ASSERT_TRUE(window->isRestoringFromDrag());
    ASSERT_LT(window->bounds().width(), maximized.width());
    EXPECT_EQ(elevationOf(window), ShadowElevation::None);

    // The commit that carries the restored size is what brings it back.
    attach(shell->rootSurface(), window->bounds().size());
    shell->rootSurface()->commit();
    compositor.updateTextures(renderer);
    EXPECT_EQ(elevationOf(window), ShadowElevation::Active);
    EXPECT_EQ(shadowOf(window)->contentBounds().size(), window->bounds().size());

    manager.eventHandler()->completeDrag(ToplevelEventHandler::DragResult::Success);
    EXPECT_EQ(elevationOf(window), ShadowElevation::Active);
}

TEST_F(ShadowControllerTest, DragRestoreWithClientMarginsRestoresFloatingVisuals)
{
    const base::Rect geometry(12, 12, 400, 300);
    auto *shell = makeToplevel({424, 324}, false, &geometry);
    auto *window = shell->window();
    auto *visual = static_cast<scene::WindowVisualNode *>(window->visualNode());
    window->maximize();
    const base::Size maximized = window->bounds().size();
    shell->setGeometry(base::Rect(base::Point(), maximized));
    attach(shell->rootSurface(), maximized);
    shell->rootSurface()->commit();
    ASSERT_TRUE(manager.eventHandler()->beginMove(window, base::PointF(500, 10)));
    manager.eventHandler()->onPointerMotion(base::PointF(500, 300));
    ASSERT_TRUE(window->isRestoringFromDrag());
    EXPECT_EQ(elevationOf(window), ShadowElevation::None);
    EXPECT_EQ(visual->state().shape.radius, 0);

    const base::Size restored = window->contentBounds().size();
    shell->acknowledgeConfigure(recorders.back()->serials.back());
    shell->setGeometry(base::Rect(base::Point(12, 12), restored));
    attach(shell->rootSurface(), restored + base::Size(24, 24));
    shell->rootSurface()->commit();
    EXPECT_EQ(elevationOf(window), ShadowElevation::Active);
    EXPECT_EQ(visual->state().shape.radius, 14);
    EXPECT_TRUE(visual->state().drawOutline);
    manager.eventHandler()->completeDrag(ToplevelEventHandler::DragResult::Success);
    EXPECT_EQ(elevationOf(window), ShadowElevation::Active);
    EXPECT_EQ(visual->state().shape.radius, 14);
}

// Upstream: ShadowControllerTest.ShadowBounds.
TEST_F(ShadowControllerTest, BoundsChangesUpdateShadow)
{
    auto *shell = makeToplevel({400, 300}, true);
    Window *window = shell->window();
    decoration::Shadow *shadow = shadowOf(window);
    const base::Insets margins = shadow->node()->margins();

    const base::Rect moved(120, 90, window->bounds().width(), window->bounds().height());
    window->setBounds(moved);
    EXPECT_EQ(shadow->contentBounds(), moved);
    EXPECT_EQ(shadow->shadowBounds(), moved.marginsAdded(margins));

    const base::Rect resized(120, 90, 640, 480);
    window->setBounds(resized);
    EXPECT_EQ(shadow->node()->geometry(), base::Rect(base::Point(), moved.size()).marginsAdded(margins));
    ASSERT_TRUE(shell->acknowledgeConfigure(recorders.back()->serials.back()));
    attach(shell->rootSurface(), window->contentBounds().size());
    shell->rootSurface()->commit();
    EXPECT_EQ(shadow->node()->geometry(), base::Rect(base::Point(0, 0), resized.size()).marginsAdded(margins));
    EXPECT_EQ(shadow->shadowBounds(), resized.marginsAdded(margins));
}

////////////////////////////////////////////////////////////////////////////////
// Resize.

// An interactive resize moves the window manager's bounds on every pointer
// motion, but the client's buffer only catches up a round trip later. The
// shadow is cut around the window's silhouette, so a shadow sized from the
// requested bounds while the old buffer is still on screen leaves a band of
// bare desktop between the client's real edge and the cut-out - which flickers
// as the client catches up.
TEST_F(ShadowControllerTest, ShadowTracksCommittedContentDuringAnInteractiveResize)
{
    ShellSurface *shell = makeToplevel({400, 300}, false);
    Window *window = shell->window();
    auto *surface = window->surfaceNode();
    ASSERT_TRUE(surface);
    ASSERT_EQ(surface->geometry().size(), base::Size(400, 300));
    const base::Insets margins = shadowOf(window)->node()->margins();
    ASSERT_EQ(shadowOf(window)->node()->geometry(), base::Rect(0, 0, 400, 300).marginsAdded(margins));

    ASSERT_TRUE(manager.eventHandler()->beginResize(window, ResizeEdge::BottomRight,
                                                    window->bounds().bottomRight() - base::Point(1, 1)));
    manager.eventHandler()->onPointerMotion(window->bounds().bottomRight() - base::Point(1, 1)
                                            + base::PointF(100, 80));

    // The window manager has already grown; the client has not committed yet.
    EXPECT_EQ(window->bounds().size(), base::Size(500, 380));
    EXPECT_EQ(surface->geometry().size(), base::Size(400, 300));
    // The shadow must hug what is actually on screen, not the request.
    EXPECT_EQ(shadowOf(window)->node()->geometry(), base::Rect(0, 0, 400, 300).marginsAdded(margins));

    // The commit that carries the new size is what moves the shadow.
    attach(shell->rootSurface(), {500, 380});
    shell->rootSurface()->commit();
    compositor.updateTextures(renderer);
    EXPECT_EQ(surface->geometry().size(), base::Size(500, 380));
    EXPECT_EQ(shadowOf(window)->node()->geometry(), base::Rect(0, 0, 500, 380).marginsAdded(margins));

    manager.eventHandler()->completeDrag(ToplevelEventHandler::DragResult::Success);
}

// A client's buffer routinely runs ahead of the window geometry it has declared
// for it - the same render-thread race the commit path already compensates for
// in the other direction. That is trailing slack, not exterior decoration, and
// reading it as decoration switches the shadow off for those frames, which is
// what makes a resize flicker.
TEST_F(ShadowControllerTest, ABufferAheadOfItsDeclaredGeometryIsNotExteriorDecoration)
{
    const base::Rect initial(0, 0, 400, 300);
    ShellSurface *shell = makeToplevel({400, 300}, false, &initial);
    Window *window = shell->window();
    manager.activation()->activate(window);
    ASSERT_EQ(elevationOf(window), ShadowElevation::Active);

    // A bigger buffer, with set_window_geometry still describing the old size.
    attach(shell->rootSurface(), {500, 380});
    shell->rootSurface()->commit();
    compositor.updateTextures(renderer);

    // Measured honestly...
    EXPECT_EQ(window->clientExteriorInsets(), (Insets {0, 0, 100, 80}));
    // ...but the window has reserved nothing around itself, so the shadow stays.
    EXPECT_FALSE(window->hasClientExteriorDecoration());
    EXPECT_EQ(elevationOf(window), ShadowElevation::Active);

    // And the shadow follows the content that is actually up.
    EXPECT_EQ(window->presentedBounds(), base::Rect(0, 0, 500, 380));

    shell->setGeometry(base::Rect(0, 0, 500, 380));
    shell->rootSurface()->commit();
    EXPECT_EQ(window->clientExteriorInsets(), Insets());
    EXPECT_EQ(elevationOf(window), ShadowElevation::Active);
}

////////////////////////////////////////////////////////////////////////////////
// Lifetime.

TEST_F(ShadowControllerTest, ShadowSurvivesTheCloseAnimationAndLeavesNothingBehind)
{
    ShellSurface *shell = makeToplevel({400, 300}, true);
    Window *window = shell->window();
    scene::Node *windowNode = window->node();
    ASSERT_TRUE(controller->shadowForWindow(window));

    // The close animation keeps the window's node in the scene after the
    // window is gone; nothing of the shadow may outlive the window itself.
    shells.clear();
    EXPECT_TRUE(manager.windows().empty());
    for (const scene::Node *child : windowNode->children())
        EXPECT_NE(child->name(), "shadow");

    // Re-asking after destruction must not resurrect anything either.
    EXPECT_EQ(controller->shadowForWindow(window), nullptr);
}

TEST_F(ShadowControllerTest, ControllerAdoptsWindowsThatAlreadyExist)
{
    controller.reset();
    Window *window = makeToplevel({400, 300}, true)->window();
    controller = std::make_unique<ShadowController>(&manager, &shadows, &delegate);
    EXPECT_EQ(elevationOf(window), ShadowElevation::Active);
}

// A window that gets no shadow is not observed either: nothing would take the
// controller out of it again, and the window would call a controller that is
// gone.
TEST_F(ShadowControllerTest, WindowWithoutAShadowOutlivesTheController)
{
    controller = std::make_unique<ShadowController>(&manager, nullptr, &delegate);
    Window *window = makeToplevel({400, 300}, true)->window();
    ASSERT_TRUE(window);
    EXPECT_EQ(controller->shadowForWindow(window), nullptr);
    EXPECT_FALSE(window->hasObserver(controller.get()));

    controller.reset();
    shells.clear();
    EXPECT_TRUE(manager.windows().empty());
}

////////////////////////////////////////////////////////////////////////////////
// Presentation.

// A shadow reaches outside its window, over whatever is behind it. Because it
// is a scene node of its own - rather than overflow painted by the frame - the
// planner sees those pixels and will not lift the window underneath onto a
// hardware plane, which would draw it over the shadow.
TEST_F(ShadowControllerTest, VisibleShadowBlocksOverlayPromotionOfTheWindowBehindIt)
{
    // Built directly on the scene: proposeOverlays works on nodes, and a real
    // window's animation node hides its whole subtree from the planner anyway.
    auto *behind = static_cast<scene::SurfaceNode *>(scene.root()->addChild(
            std::make_unique<scene::SurfaceNode>("surface")));
    behind->setGeometry(base::Rect(0, 0, 400, 300));
    behind->setTexture(std::make_shared<TestTexture>(base::Size(400, 300)));
    behind->setScanoutBuffer(std::make_shared<TestScanoutBuffer>(base::Size(400, 300)));
    behind->setOpaque(true);

    auto *shadow = static_cast<decoration::ShadowNode *>(
            scene.root()->addChild(std::make_unique<decoration::ShadowNode>(&theme)));
    shadow->update(base::Rect(0, 0, 200, 200), ShadowElevation::Active, decoration::Corners::all(),
                   theme.metrics().cornerRadius);
    // The window itself clears `behind`; only its shadow's left margin does not.
    const base::Rect window(430, 100, 200, 200);
    shadow->setGeometry(shadow->geometry().translated(window.topLeft()));
    ASSERT_TRUE(shadow->absoluteGeometry().intersects(behind->absoluteGeometry()));
    ASSERT_FALSE(window.intersects(behind->absoluteGeometry()));

    EXPECT_TRUE(scene::proposeOverlays(*scene.root(), kOutput.size(), 2).overlays.empty());

    // With no shadow drawn there is nothing over it, and the plane is offered.
    shadow->update(base::Rect(0, 0, 200, 200), ShadowElevation::None, decoration::Corners::all(),
                   theme.metrics().cornerRadius);
    EXPECT_EQ(scene::proposeOverlays(*scene.root(), kOutput.size(), 2).overlays.size(), 1u);
}

} // namespace
