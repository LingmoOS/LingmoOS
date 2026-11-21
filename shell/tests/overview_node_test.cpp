// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// The overview node's progress - the tween, the fingers, the ends and
// the hand-offs between them - on a fake frame clock, and where it draws the
// windows it moves.

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <format>
#include <vector>

#include <gtest/gtest.h>

#include "fake_renderer.h"
#include "scene/overview_node.h"
#include "scene/scene.h"
#include "scene/window_animation.h"
#include "base/geometry.h"

using namespace Lingmo;
using Node = scene::OverviewNode;
using Track = scene::motion::FingerTrack;

namespace {

class FrameTexture : public render::Texture
{
public:
    explicit FrameTexture(base::Size size)
        : m_size(size)
    {
    }

    base::Size size() const override { return m_size; }

private:
    base::Size m_size;
};

class FrameRenderer : public testsupport::FakeRenderer
{
public:
    bool renderToTexture(std::unique_ptr<render::Texture> &texture, const base::Rect &bounds,
                         const std::function<void()> &paint) override
    {
        paint();
        if (!texture || texture->size() != bounds.size())
            texture = std::make_unique<FrameTexture>(bounds.size());
        return true;
    }

    void drawTextureF(render::Texture *texture, const base::RectF &target, float alpha) override
    {
        FakeRenderer::drawTextureF(texture, target, alpha);
        calls.back().target = target;
    }

    std::vector<render::Texture *> trilinear;

    void enableTrilinearFiltering(render::Texture *texture) override { trilinear.push_back(texture); }

    struct Outline {
        base::RectF bounds;
        float alpha = 0;
    };
    std::vector<Outline> outlines;

    void drawRoundedOutline(const render::RoundedShape &shape, const render::Color &color,
                            double) override
    {
        outlines.push_back({shape.bounds, color.a});
    }
};

const base::Rect kViewport(0, 0, 1000, 800);
const base::Rect kWindow(100, 100, 400, 300);
const base::RectF kPlace(600, 400, 200, 150);
constexpr int kSettleMs = 1500;

struct Fixture {
    int64_t now = 1000;
    scene::Scene scene;
    scene::Node *container =
            scene.root()->addChild(std::make_unique<scene::Node>("surfaces"));
    scene::ColorNode *desktop = static_cast<scene::ColorNode *>(
            container->addChild(std::make_unique<scene::ColorNode>(render::Color {0, 0, 1, 1})));
    scene::WindowAnimation *window = static_cast<scene::WindowAnimation *>(
            container->addChild(std::make_unique<scene::WindowAnimation>([this] {
                return double(now);
            })));
    Node *node = static_cast<Node *>(container->addChild(std::make_unique<Node>([this] {
        return now;
    })));
    FrameRenderer renderer;
    std::vector<double> finished;

    Fixture()
    {
        scene.setSize(kViewport.size());
        desktop->setGeometry(kViewport);
        window->setGeometry(kWindow);
        window->addChild(std::make_unique<scene::ColorNode>(render::Color {1, 0, 0, 1}))
                ->setGeometry(base::Rect(base::Point(), kWindow.size()));
        // Already open: no animation of its own.
        window->finish();
        node->setGeometry(kViewport);
        node->setBackdrop({desktop});
        node->setItems({item()}, false);
        node->setFinishedCallback([this](double target) {
            finished.push_back(target);
        });
    }

    Node::Item item(const base::RectF &to = kPlace) const
    {
        Node::Item item;
        item.windows = {window};
        item.to = to;
        item.title = "Window";
        return item;
    }

    void paint()
    {
        renderer.calls.clear();
        renderer.outlines.clear();
        renderer.setOutputContext({"out", kViewport, kViewport.size(), 1, 0, 0, {}});
        renderer.frameTime = double(now);
        container->render(renderer);
    }

    void settle()
    {
        paint();
        now += kSettleMs;
        paint();
    }

    base::RectF rect(double progress) const
    {
        return node->presentedRect(0, progress, double(now));
    }
};

base::RectF lerp(const base::RectF &a, const base::RectF &b, double t)
{
    return base::RectF(a.x() + (b.x() - a.x()) * t, a.y() + (b.y() - a.y()) * t,
                  a.width() + (b.width() - a.width()) * t,
                  a.height() + (b.height() - a.height()) * t);
}

} // namespace

TEST(OverviewNode, AtZeroItCoversNothingAndDrawsNothing)
{
    Fixture f;
    EXPECT_FALSE(f.node->isActive());
    EXPECT_EQ(scene::Node::Content::None, f.node->content());
    EXPECT_FALSE(f.node->occludesOutput(f.renderer));
    f.paint();
    // The desk as it is: the desktop and the window, each drawn once.
    const auto kinds = f.renderer.kinds();
    EXPECT_EQ(2, std::count(kinds.begin(), kinds.end(), "fill"));
}

TEST(OverviewNode, TheShortcutSetsOffAtOnceAndLandsOnTime)
{
    Fixture f;
    const int64_t start = f.now;
    f.node->animateTo(1);
    EXPECT_TRUE(f.node->isActive());
    EXPECT_TRUE(f.node->occludesOutput(f.renderer));
    double last = 0;
    while (f.now - start < int64_t(Node::kTransitionDurationMs)) {
        f.now += 4;
        f.paint();
        const double progress = f.node->progress(double(f.now));
        EXPECT_GE(progress, last);
        EXPECT_LE(progress, 1);
        last = progress;
        // From rest it gets going at once with no jump: the first frame
        // covers under 1%, an eighth of the time over a tenth of the way.
        if (f.now - start == 4)
            EXPECT_LT(progress, 0.01);
        if (f.now - start == 32)
            EXPECT_GT(progress, 0.1);
    }
    // No tail creeping in after its time.
    EXPECT_EQ(1, last);
    f.paint();
    EXPECT_EQ(std::vector<double>({1}), f.finished);
    EXPECT_TRUE(f.node->isSettled());
    EXPECT_EQ(1, f.node->target());

    // And back, to the desk as it was, where it covers nothing again.
    f.node->animateTo(0);
    f.settle();
    EXPECT_EQ(std::vector<double>({1, 0}), f.finished);
    EXPECT_FALSE(f.node->isActive());
}

TEST(OverviewNode, AWindowMovesFromWhereItIsToItsPlace)
{
    Fixture f;
    EXPECT_EQ(base::RectF(kWindow), f.rect(0));
    EXPECT_EQ(kPlace, f.rect(1));
    EXPECT_EQ(lerp(kWindow, kPlace, 0.5), f.rect(0.5));
}

TEST(OverviewNode, TheWindowIsDrawnScaledOntoItsPlace)
{
    Fixture f;
    f.node->jumpTo(1);
    f.paint();
    // The window's texture holds its shadow margins, scaled along with it.
    const double scale = kPlace.width() / kWindow.width();
    const base::RectF coverage = base::RectF(kWindow).adjusted(-64, -64, 64, 64);
    const base::RectF expected(kPlace.topLeft() + (coverage.topLeft() - kWindow.topLeft()) * scale,
                          coverage.size() * scale);
    const auto drawn = std::find_if(f.renderer.calls.begin(), f.renderer.calls.end(),
                                    [](const auto &call) {
                                        return call.kind == "texture";
                                    });
    ASSERT_NE(f.renderer.calls.end(), drawn);
    EXPECT_EQ(expected, drawn->target);
    // Half its size, it is sampled through mipmaps.
    EXPECT_EQ(1, f.renderer.trilinear.size());
    // The desktop covers the output: darkened in place, first, with no clear
    // before it and no dim pass over it.
    ASSERT_GE(f.renderer.calls.size(), 2);
    EXPECT_EQ("fill", f.renderer.calls[0].kind);
    EXPECT_FLOAT_EQ(1 - Node::kDimOpacity, f.renderer.calls[0].color.b);
    EXPECT_FLOAT_EQ(1, f.renderer.calls[0].color.a);
    for (const auto &call : f.renderer.calls) {
        const bool black = call.color.r == 0 && call.color.g == 0 && call.color.b == 0;
        EXPECT_FALSE(call.kind == "fill" && black && call.color.a > 0);
    }
}

TEST(OverviewNode, ADesktopShortOfTheOutputIsDrawnOverBlack)
{
    Fixture f;
    f.desktop->setGeometry(kViewport.adjusted(0, 0, 0, -100));
    f.node->jumpTo(1);
    f.paint();
    ASSERT_GE(f.renderer.calls.size(), 2);
    EXPECT_EQ("fill", f.renderer.calls[0].kind);
    EXPECT_EQ(kViewport, f.renderer.calls[0].rect);
    EXPECT_FLOAT_EQ(0, f.renderer.calls[0].color.b);
    EXPECT_FLOAT_EQ(1, f.renderer.calls[0].color.a);
    EXPECT_FLOAT_EQ(1, f.renderer.calls[1].color.b);
    EXPECT_FLOAT_EQ(1 - Node::kDimOpacity, f.renderer.calls[1].color.a);
}

TEST(OverviewNode, TheFingersMoveItAndStopAtTheEnds)
{
    Fixture f;
    f.node->startTracking();
    for (int step = 1; step <= 5; ++step) {
        f.now += 10;
        f.node->updateTracking(0.1, double(f.now));
    }
    // Shown where the fingers were a resampling latency ago.
    const double shown = f.node->progress(double(f.now) + Track::kResampleLatencyMs);
    EXPECT_NEAR(0.5, shown, 1e-9);

    // Far past the end, it stays at the end, and turning back moves it at once.
    for (int step = 0; step < 20; ++step) {
        f.now += 10;
        f.node->updateTracking(0.1, double(f.now));
        EXPECT_LE(f.node->progress(double(f.now)), 1);
    }
    EXPECT_DOUBLE_EQ(1, f.node->progress(double(f.now) + 100));
    f.now += 10;
    f.node->updateTracking(-0.1, double(f.now));
    f.now += 100;
    EXPECT_LT(f.node->progress(double(f.now)), 1);
}

TEST(OverviewNode, AHeldReleaseGoesToTheNearerEndAndAFlickGoesOn)
{
    for (const bool flick : {false, true}) {
        SCOPED_TRACE(flick);
        Fixture f;
        f.node->startTracking();
        if (flick) {
            // A third of the way, fast.
            for (int step = 0; step < 4; ++step) {
                f.now += 8;
                f.node->updateTracking(0.3 / 4, double(f.now));
            }
        } else {
            // Past half way, then back to a third and held there.
            for (int step = 0; step < 6; ++step) {
                f.now += 8;
                f.node->updateTracking(0.1, double(f.now));
            }
            for (int step = 0; step < 6; ++step) {
                f.now += 8;
                f.node->updateTracking(-0.05, double(f.now));
            }
            f.now += 150;
        }
        EXPECT_EQ(flick ? 1 : 0, f.node->endTracking(double(f.now)));
        f.settle();
        EXPECT_EQ(std::vector<double>({flick ? 1.0 : 0.0}), f.finished);
    }
}

TEST(OverviewNode, ReleasedItCarriesOnAtTheFingersSpeed)
{
    Fixture f;
    f.node->startTracking();
    // Past half way slower than a flick: 0.4 a second towards the nearer end.
    for (int step = 0; step < 150; ++step) {
        f.now += 10;
        f.node->updateTracking(0.004, double(f.now));
    }
    EXPECT_EQ(1, f.node->endTracking(double(f.now)));
    const double now = double(f.now);
    const double speed = (f.node->progress(now + 0.01) - f.node->progress(now)) * 100000;
    EXPECT_NEAR(0.4, speed, 0.02);
    // On the quick curve: the rest of the way within the shorter time.
    f.now += int64_t(Node::kTransitionDurationMs);
    f.paint();
    EXPECT_EQ(std::vector<double>({1}), f.finished);
    f.settle();
    EXPECT_EQ(std::vector<double>({1}), f.finished);
}

TEST(OverviewNode, ASlowReleaseSpeedsUpGentlyInsteadOfLunging)
{
    Fixture f;
    f.node->startTracking();
    for (int step = 0; step < 150; ++step) {
        f.now += 10;
        f.node->updateTracking(0.004, double(f.now));
    }
    ASSERT_EQ(1, f.node->endTracking(double(f.now)));
    const double start = double(f.now);
    const double from = f.node->progress(start);
    const double duration = Node::kTransitionDurationMs * std::sqrt(1 - from);
    const double average = (1 - from) / duration * 1000;
    double fastest = 0;
    for (double t = start; t < start + duration; t += 1)
        fastest = std::max(fastest, (f.node->progress(t + 1) - f.node->progress(t)) * 1000);
    EXPECT_LT(fastest, 2 * average);
}

TEST(OverviewNode, TheFingersCatchTheTweenWhereItIsShown)
{
    Fixture f;
    f.node->animateTo(1);
    f.now += 120;
    const double before = f.node->progress(double(f.now));
    ASSERT_GT(before, 0.1);
    ASSERT_LT(before, 0.9);
    f.node->startTracking();
    EXPECT_NEAR(before, f.node->progress(double(f.now)), 1e-9);
    // And the shortcut catches the fingers the same way.
    f.node->updateTracking(0.1, double(f.now));
    f.now += 20;
    const double tracked = f.node->progress(double(f.now));
    f.node->animateTo(0);
    EXPECT_NEAR(tracked, f.node->progress(double(f.now)), 1e-9);
}

TEST(OverviewNode, ANewLayoutMovesWindowsThereOnATween)
{
    Fixture f;
    f.node->jumpTo(1);
    const base::RectF moved(200, 150, 300, 225);
    f.node->setItems({f.item(moved)}, true);
    EXPECT_EQ(kPlace, f.rect(1));
    f.now += 100;
    const base::RectF partway = f.rect(1);
    EXPECT_NE(kPlace, partway);
    EXPECT_NE(moved, partway);
    f.now += kSettleMs;
    EXPECT_EQ(moved, f.rect(1));

    // A dragged window heads home from where it was let go.
    const base::RectF dragged(700, 50, 300, 225);
    f.node->setDragged(0, dragged);
    EXPECT_EQ(dragged, f.rect(1));
    f.node->setItems({f.item(moved)}, true);
    EXPECT_EQ(dragged, f.rect(1));
    f.now += kSettleMs;
    EXPECT_EQ(moved, f.rect(1));
}

TEST(OverviewNode, TheTitleShowsOnceItIsIn)
{
    Fixture f;
    int painted = 0;
    f.node->setTextPainter([&painted](const std::string &, int, const render::Color &, double, int) {
        ++painted;
        ui::Bitmap image(60, 16, ui::Bitmap::Format::Rgba8888Premultiplied);
        image.fill(0xffffffff);
        return image;
    });
    f.node->setChrome({0, -1});
    f.node->animateTo(1);
    f.now += 16;
    f.paint();
    // Not while it moves.
    EXPECT_EQ(0, painted);
    f.settle();
    f.now += 1000;
    f.paint();
    EXPECT_EQ(1, painted);
    // In the middle of the window.
    const auto targets = [&] {
        std::vector<base::RectF> result;
        for (const auto &call : f.renderer.calls) {
            if (call.kind == "texture")
                result.push_back(call.target);
        }
        return result;
    }();
    EXPECT_TRUE(std::any_of(targets.begin(), targets.end(), [](const base::RectF &target) {
        return target.size() == base::SizeF(60, 16) && kPlace.contains(target)
                && std::abs(target.center().x() - kPlace.center().x()) <= 1
                && std::abs(target.center().y() - kPlace.center().y()) <= 1;
    }));
}

TEST(OverviewNode, EveryWindowGetsTheOverviewsCornersShadowAndOutline)
{
    Fixture f;
    struct Drawn {
        base::RectF body;
        double radius;
        double uniform;
        float opacity;
    };
    std::vector<Drawn> shadows;
    std::vector<Drawn> outlines;
    scene::OverviewWindowFrame frame;
    frame.radius = 14;
    frame.shadow = [&](render::Renderer &, const scene::WindowAnimation &,
                       const render::RoundedShape &body, double uniform, float opacity, bool) {
        shadows.push_back({body.bounds, body.radius, uniform, opacity});
    };
    frame.outline = [&](render::Renderer &, const render::RoundedShape &body, float opacity) {
        outlines.push_back({body.bounds, body.radius, 1, opacity});
    };
    f.node->setWindowFrame(frame);

    f.node->jumpTo(1);
    f.paint();
    ASSERT_EQ(1, shadows.size());
    EXPECT_EQ(kPlace, shadows[0].body);
    // A normal window's corners, at the window's half size.
    EXPECT_EQ(7, shadows[0].radius);
    EXPECT_EQ(1, shadows[0].uniform);
    ASSERT_EQ(1, outlines.size());
    EXPECT_EQ(kPlace, outlines[0].body);
    EXPECT_FLOAT_EQ(1, outlines[0].opacity);

    // Halfway in, at three quarters of its size, the window's own square
    // corners are halfway to the overview's, and so is its outline.
    shadows.clear();
    outlines.clear();
    f.node->jumpTo(0.5);
    f.paint();
    ASSERT_EQ(1, shadows.size());
    EXPECT_DOUBLE_EQ(14 * 0.75 / 2, shadows[0].radius);
    EXPECT_DOUBLE_EQ(0.5, shadows[0].uniform);
    ASSERT_EQ(1, outlines.size());
    EXPECT_FLOAT_EQ(0.5, outlines[0].opacity);
}

TEST(OverviewNode, ADraggedWindowIsDrawnOverTheDeskBar)
{
    Fixture f;
    f.container->addChild(std::make_unique<scene::OverviewDeskBarNode>(f.node));
    f.node->jumpTo(1);
    const base::RectF dragged(50, 20, 100, 75);
    f.node->setDragged(0, dragged);
    f.paint();
    std::vector<base::RectF> targets;
    for (const auto &call : f.renderer.calls) {
        if (call.kind == "texture")
            targets.push_back(call.target);
    }
    // Once, by the bar, after everything the overview draws.
    ASSERT_EQ(1, targets.size());
    const double scale = dragged.width() / kWindow.width();
    const base::RectF coverage = base::RectF(kWindow).adjusted(-64, -64, 64, 64);
    EXPECT_EQ(base::RectF(dragged.topLeft() + (coverage.topLeft() - kWindow.topLeft()) * scale,
                     coverage.size() * scale),
              targets[0]);
    EXPECT_EQ("texture", f.renderer.calls.back().kind);
}

namespace {

using Bar = scene::OverviewDeskBarNode::Bar;
using DeskBar = scene::OverviewDeskBarNode;

// The outline of an inactive desk preview, drawn on its body.
constexpr float kOutlineAlpha = 0.22f;

DeskBar::Desk barDesk(uintptr_t id, double x)
{
    DeskBar::Desk desk;
    desk.id = id;
    desk.bounds = base::RectF(x, 20, 100, 60);
    desk.name = std::format("Desk {}", id);
    return desk;
}

Bar bar(std::vector<DeskBar::Desk> desks, double newDeskX)
{
    Bar bar;
    bar.output = kViewport;
    bar.bounds = base::RectF(0, 0, 1000, 100);
    bar.desks = std::move(desks);
    bar.newDesk = base::RectF(newDeskX, 20, 50, 60);
    return bar;
}

} // namespace

TEST(OverviewNode, ABarPutUpAnewStandsWhereItIsPut)
{
    Fixture f;
    auto *deskBar = static_cast<DeskBar *>(
            f.container->addChild(std::make_unique<DeskBar>(f.node)));
    f.node->jumpTo(1);
    deskBar->setBars({bar({barDesk(1, 340), barDesk(2, 450)}, 560)});
    f.paint();
    ASSERT_EQ(2, f.renderer.outlines.size());
    EXPECT_EQ(base::RectF(340, 20, 100, 60), f.renderer.outlines[0].bounds);
    EXPECT_EQ(base::RectF(450, 20, 100, 60), f.renderer.outlines[1].bounds);
    EXPECT_FLOAT_EQ(kOutlineAlpha, f.renderer.outlines[1].alpha);
}

TEST(OverviewNode, OnlyTheActiveDeskIsRingedAndAPressedOneDims)
{
    Fixture f;
    auto *deskBar = static_cast<DeskBar *>(
            f.container->addChild(std::make_unique<DeskBar>(f.node)));
    f.node->jumpTo(1);
    DeskBar::Desk active = barDesk(1, 340);
    active.active = true;
    DeskBar::Desk pressed = barDesk(2, 450);
    pressed.pressed = true;
    deskBar->setBars({bar({active, pressed}, 560)});
    f.paint();
    ASSERT_EQ(2, f.renderer.outlines.size());
    // The ring stands off the active desk's body; the pressed one keeps the
    // plain outline on its body.
    EXPECT_EQ(base::RectF(337, 17, 106, 66), f.renderer.outlines[0].bounds);
    EXPECT_EQ(base::RectF(450, 20, 100, 60), f.renderer.outlines[1].bounds);
    EXPECT_FLOAT_EQ(kOutlineAlpha, f.renderer.outlines[1].alpha);
    const auto dims = std::count_if(f.renderer.calls.begin(), f.renderer.calls.end(),
                                    [](const auto &call) {
                                        return call.kind == "fill"
                                                && call.rect == base::Rect(450, 20, 100, 60)
                                                && call.color.r == 0 && call.color.a > 0
                                                && call.color.a < 1;
                                    });
    EXPECT_EQ(1, dims);
}

TEST(OverviewNode, ADraggedDeskFollowsAtOnceOverTheOnesThatEaseAside)
{
    Fixture f;
    auto *deskBar = static_cast<DeskBar *>(
            f.container->addChild(std::make_unique<DeskBar>(f.node)));
    f.node->jumpTo(1);
    deskBar->setBars({bar({barDesk(1, 340), barDesk(2, 450)}, 560)});
    f.paint();

    // The first one dragged past the second, which makes way.
    DeskBar::Desk dragged = barDesk(1, 420);
    dragged.dragged = true;
    deskBar->setBars({bar({barDesk(2, 340), dragged}, 560)});
    f.now += DeskBar::kShiftDurationMs / 2;
    f.paint();
    ASSERT_EQ(2, f.renderer.outlines.size());
    // The second is on its way; the dragged one is where it is put, drawn last.
    EXPECT_GT(f.renderer.outlines[0].bounds.x(), 340);
    EXPECT_LT(f.renderer.outlines[0].bounds.x(), 450);
    EXPECT_EQ(base::RectF(420, 20, 100, 60), f.renderer.outlines[1].bounds);

    // Let go, it eases into its place from where it was dropped.
    deskBar->setBars({bar({barDesk(2, 340), barDesk(1, 450)}, 560)});
    f.now += DeskBar::kShiftDurationMs / 2;
    f.paint();
    EXPECT_GT(f.renderer.outlines[1].bounds.x(), 420);
    EXPECT_LT(f.renderer.outlines[1].bounds.x(), 450);
    f.now += DeskBar::kShiftDurationMs;
    f.paint();
    EXPECT_EQ(base::RectF(450, 20, 100, 60), f.renderer.outlines[1].bounds);
}

TEST(OverviewNode, ANewDeskScalesUpAndFadesInWhileTheRestMoveOver)
{
    Fixture f;
    auto *deskBar = static_cast<DeskBar *>(
            f.container->addChild(std::make_unique<DeskBar>(f.node)));
    f.node->jumpTo(1);
    deskBar->setBars({bar({barDesk(1, 400)}, 510)});
    f.paint();
    deskBar->setBars({bar({barDesk(1, 340), barDesk(2, 450)}, 560)});

    // The new desk waits out its fade's delay; the old one sets off from
    // where it was.
    f.paint();
    ASSERT_EQ(1, f.renderer.outlines.size());
    EXPECT_EQ(base::RectF(400, 20, 100, 60), f.renderer.outlines[0].bounds);

    f.now += 125;
    f.paint();
    ASSERT_EQ(2, f.renderer.outlines.size());
    const base::RectF moving = f.renderer.outlines[0].bounds;
    EXPECT_LT(340, moving.x());
    EXPECT_GT(400, moving.x());
    EXPECT_EQ(base::SizeF(100, 60), moving.size());
    // Scaled about the middle of its top edge, partly faded in.
    const base::RectF growing = f.renderer.outlines[1].bounds;
    EXPECT_EQ(20, growing.top());
    EXPECT_NEAR(500, growing.center().x(), 0.5);
    EXPECT_GT(100, growing.width());
    EXPECT_LT(10, growing.width());
    EXPECT_NEAR(kOutlineAlpha * 0.25f, f.renderer.outlines[1].alpha, 0.01f);

    f.now += kSettleMs;
    f.paint();
    ASSERT_EQ(2, f.renderer.outlines.size());
    EXPECT_EQ(base::RectF(340, 20, 100, 60), f.renderer.outlines[0].bounds);
    EXPECT_EQ(base::RectF(450, 20, 100, 60), f.renderer.outlines[1].bounds);
    EXPECT_FLOAT_EQ(kOutlineAlpha, f.renderer.outlines[1].alpha);
}

TEST(OverviewNode, ARemovedDeskFadesOutWhereItWasUnderTheRest)
{
    Fixture f;
    auto *deskBar = static_cast<DeskBar *>(
            f.container->addChild(std::make_unique<DeskBar>(f.node)));
    f.node->jumpTo(1);
    deskBar->setBars({bar({barDesk(1, 340), barDesk(2, 450)}, 560)});
    f.paint();
    deskBar->setBars({bar({barDesk(1, 400)}, 510)});

    f.now += 100;
    f.paint();
    ASSERT_EQ(2, f.renderer.outlines.size());
    EXPECT_EQ(base::RectF(450, 20, 100, 60), f.renderer.outlines[0].bounds);
    EXPECT_GT(kOutlineAlpha, f.renderer.outlines[0].alpha);
    EXPECT_LT(0, f.renderer.outlines[0].alpha);
    EXPECT_LT(340, f.renderer.outlines[1].bounds.x());
    EXPECT_GT(400, f.renderer.outlines[1].bounds.x());

    f.now += kSettleMs;
    f.paint();
    ASSERT_EQ(1, f.renderer.outlines.size());
    EXPECT_EQ(base::RectF(400, 20, 100, 60), f.renderer.outlines[0].bounds);
}

namespace {

// Counts the textures alive, as many as the bar keeps for its desks.
class PreviewTexture : public render::Texture
{
public:
    PreviewTexture(base::Size size, int &live)
        : m_size(size)
        , m_live(live)
    {
        ++m_live;
    }

    ~PreviewTexture() override { --m_live; }

    base::Size size() const override { return m_size; }

private:
    base::Size m_size;
    int &m_live;
};

class PreviewRenderer : public FrameRenderer
{
public:
    bool renderToTexture(std::unique_ptr<render::Texture> &texture, const base::Rect &bounds,
                         const std::function<void()> &paint) override
    {
        if (bounds.size() != base::Size(100, 60))
            return FrameRenderer::renderToTexture(texture, bounds, paint);
        paint();
        if (!texture || texture->size() != bounds.size())
            texture = std::make_unique<PreviewTexture>(bounds.size(), previews);
        return true;
    }

    int previews = 0;
};

} // namespace

TEST(OverviewNode, APreviewGoesWithItsDeskAndItsOutput)
{
    Fixture f;
    PreviewRenderer renderer;
    auto *deskBar = static_cast<DeskBar *>(
            f.container->addChild(std::make_unique<DeskBar>(f.node)));
    const auto paint = [&] {
        renderer.setOutputContext({"out", kViewport, kViewport.size(), 1, 0, 0, {}});
        renderer.frameTime = double(f.now);
        f.container->render(renderer);
        f.now += 16;
    };
    f.node->jumpTo(1);
    deskBar->setBars({bar({barDesk(1, 340), barDesk(2, 450)}, 560)});
    paint();
    paint();
    EXPECT_EQ(2, renderer.previews);

    // Taken down and put up again without the second desk, removed meanwhile.
    deskBar->setBars({});
    EXPECT_EQ(2, renderer.previews);
    deskBar->setBars({bar({barDesk(1, 400)}, 510)});
    EXPECT_EQ(1, renderer.previews);

    // The output's bounds changed.
    Bar moved = bar({barDesk(1, 400)}, 510);
    moved.output = base::Rect(0, 0, 1200, 800);
    deskBar->setBars({moved});
    EXPECT_EQ(0, renderer.previews);
}
