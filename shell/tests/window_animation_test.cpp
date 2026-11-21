// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <optional>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "fake_renderer.h"
#include "render/damage.h"
#include "scene/scene.h"
#include "scene/surface_node.h"
#include "scene/window_animation.h"
#include "scene/window_visual_node.h"
#include "base/region.h"

using namespace Lingmo;
using Animation = scene::WindowAnimation;
using Type = Animation::Type;

namespace {

int countKind(const testsupport::FakeRenderer &renderer, const char *kind)
{
    const auto kinds = renderer.kinds();
    return int(std::count(kinds.begin(), kinds.end(), kind));
}

class SnapshotTexture : public render::Texture
{
public:
    explicit SnapshotTexture(base::Size size)
        : m_size(size)
    {
        ++live;
    }

    ~SnapshotTexture() override { --live; }

    static inline int live = 0;

    base::Size size() const override { return m_size; }

private:
    base::Size m_size;
};

class SnapshotRenderer : public testsupport::FakeRenderer
{
public:
    int captures = 0;
    base::RectF lastTarget;
    std::vector<render::Texture *> trilinear;

    void enableTrilinearFiltering(render::Texture *texture) override { trilinear.push_back(texture); }

    std::vector<int> mipmapLevels;

    void limitMipmapLevel(render::Texture *, int level) override { mipmapLevels.push_back(level); }

    std::vector<render::RoundedShape> clips;
    std::vector<float> outlines;

    void pushRoundedClip(const render::RoundedShape &shape) override
    {
        clips.push_back(shape);
        FakeRenderer::pushRoundedClip(shape);
    }

    void drawRoundedOutline(const render::RoundedShape &, const render::Color &color,
                            double) override
    {
        outlines.push_back(color.a);
    }

    int blurs = 0;

    void drawBackgroundBlur(const base::Rect &, int, const base::Rect &, const render::GlassParams &,
                            const ui::Path &, uint64_t) override
    {
        ++blurs;
    }

    void drawTextureF(render::Texture *texture, const base::RectF &target, float alpha) override
    {
        lastTarget = target;
        FakeRenderer::drawTextureF(texture, target, alpha);
    }

    bool renderToTexture(std::unique_ptr<render::Texture> &texture, const base::Rect &bounds,
                         const std::function<void()> &paint) override
    {
        ++captures;
        paint();
        texture = std::make_unique<SnapshotTexture>(bounds.size());
        return true;
    }

    base::Rect lastRepaint;
    std::vector<base::Rect> repaintRects;

    bool renderToTexture(std::unique_ptr<render::Texture> &texture, const base::Rect &bounds,
                         const base::Rect &repaint, const std::function<void()> &paint) override
    {
        lastRepaint = repaint;
        repaintRects.push_back(repaint);
        return renderToTexture(texture, bounds, paint);
    }

    // The logical part of the output this frame draws again; all of it if unset.
    std::optional<base::Rect> repainted;

    bool repaints(const base::Rect &logical) const override
    {
        return !repainted || repainted->intersects(logical);
    }

    std::vector<base::Rect> imageDraws;

    ui::Bitmap renderToImage(const base::Size &size, const std::function<void()> &paint) override
    {
        const auto first = calls.size();
        paint();
        for (auto index = first; index < calls.size(); ++index)
            imageDraws.push_back(calls[index].rect);
        calls.erase(calls.begin() + first, calls.end());
        ui::Bitmap image(size, ui::Bitmap::Format::Argb32Premultiplied);
        image.fill(0x80402010);
        return image;
    }
};

} // namespace

TEST(WindowAnimation, DockTransitionsSettleSmoothly)
{
    const base::Rect from(100, 50, 800, 600), to(500, 900, 1, 1);
    EXPECT_EQ(Animation::duration(Type::Minimize, from, to), 320);
    EXPECT_EQ(Animation::duration(Type::Restore, to, from), 320);
    const auto halfway = Animation::sample(Type::Minimize, from, to, .5);
    EXPECT_GT(halfway.bounds.x(), 350);
    EXPECT_LT(halfway.bounds.x(), to.x());
    EXPECT_EQ(halfway.opacity, 1);
    // Opaque all the way into the dock thumbnail.
    EXPECT_EQ(Animation::sample(Type::Minimize, from, to, .99).opacity, 1);
    EXPECT_EQ(Animation::sample(Type::Restore, to, from, .3).opacity, 1);
    EXPECT_LT(Animation::sample(Type::Restore, to, from, .5).bounds.x(), 250);
}

TEST(WindowAnimation, VisibilityCurvesAreBoundedAndHaveSoftEndpoints)
{
    const base::RectF from(100, 50, 800, 600), to(500, 900, 48, 40);
    for (const auto type : {Type::Open, Type::Close, Type::Minimize, Type::Restore}) {
        EXPECT_EQ(Animation::sample(type, from, to, -1).bounds, from);
        EXPECT_EQ(Animation::sample(type, from, to, 2).bounds, to);
        double lastX = from.x();
        for (int frame = 0; frame <= 240; ++frame) {
            const auto value = Animation::sample(type, from, to, frame / 240.0);
            EXPECT_GE(value.bounds.x(), lastX);
            EXPECT_LE(value.bounds.x(), to.x());
            EXPECT_GT(value.bounds.width(), 0);
            EXPECT_GE(value.opacity, 0);
            EXPECT_LE(value.opacity, 1);
            lastX = value.bounds.x();
        }
        const double epsilon = .0001;
        EXPECT_NEAR(Animation::sample(type, from, to, epsilon).bounds.x(), from.x(), 0.001);
        EXPECT_NEAR(Animation::sample(type, from, to, 1 - epsilon).bounds.x(), to.x(), 0.001);
    }
}

TEST(WindowAnimation, ChromiumAreaDependentCrossFadeDuration)
{
    EXPECT_EQ(Animation::duration(Type::Bounds, {0, 0, 100, 100}, {0, 0, 100, 100}), 200);
    EXPECT_EQ(Animation::duration(Type::Bounds, {0, 0, 100, 100}, {0, 0, 200, 200}), 350);
    EXPECT_EQ(Animation::duration(Type::Bounds, {}, {}), 200);
    EXPECT_EQ(Animation::duration(Type::Open, {}, {}), 360);
    EXPECT_EQ(Animation::duration(Type::Close, {}, {}), 220);
}

TEST(WindowAnimation, OpenScalesFromCenterAndEndsWithoutFurtherFrames)
{
    int64_t now = 0;
    scene::Scene scene;
    SnapshotRenderer renderer;
    auto *node = static_cast<Animation *>(scene.root()->addChild(std::make_unique<Animation>([&] {
        return now;
    })));
    node->setGeometry({100, 100, 400, 300});
    int requests = 0;
    scene.root()->setChangeCallback([&] {
        ++requests;
    });
    scene.render(renderer);
    EXPECT_FLOAT_EQ(renderer.calls.back().alpha, 0);
    now = 90;
    scene.render(renderer);
    EXPECT_FLOAT_EQ(renderer.calls.back().alpha, Animation::sample(Type::Open, {}, {}, 90.0 / Animation::duration(Type::Open, {}, {})).opacity);
    now = Animation::duration(Type::Open, {}, {});
    scene.render(renderer);
    EXPECT_FLOAT_EQ(renderer.calls.back().alpha, 1);
    requests = 0;
    scene.render(renderer);
    EXPECT_EQ(requests, 0);
}

TEST(WindowAnimation, APresentationScaleShrinksAboutItsAnchorAndGrowsBack)
{
    int64_t now = 0;
    scene::Scene scene;
    SnapshotRenderer renderer;
    auto *node = static_cast<Animation *>(scene.root()->addChild(std::make_unique<Animation>([&] {
        return now;
    })));
    node->setStyle(Animation::Style::None);
    node->setGeometry({100, 100, 400, 300});
    // Half size about (100, 50) in the window: the texture, which covers 64px
    // around it, shrinks towards (200, 150).
    node->setPresentationScale(0.5, {100, 50}, false);
    scene.render(renderer);
    EXPECT_EQ(base::RectF(118, 93, 264, 214), renderer.lastTarget);

    int requests = 0;
    scene.root()->setChangeCallback([&] {
        ++requests;
    });
    node->setPresentationScale(1, {100, 50}, true);
    scene.render(renderer);
    EXPECT_EQ(base::RectF(118, 93, 264, 214), renderer.lastTarget);
    now = 100;
    scene.render(renderer);
    EXPECT_GT(renderer.lastTarget.width(), 264);
    EXPECT_LT(renderer.lastTarget.width(), 528);
    now = 1000;
    scene.render(renderer);
    EXPECT_EQ(base::RectF(36, 36, 528, 428), renderer.lastTarget);
    EXPECT_DOUBLE_EQ(1, node->presentationScale());
    requests = 0;
    scene.render(renderer);
    EXPECT_EQ(0, requests);
}

TEST(WindowAnimation, StepsOnTheRendererFrameTimeInsteadOfTheClock)
{
    int64_t now = 0;
    scene::Scene scene;
    SnapshotRenderer renderer;
    auto *node = static_cast<Animation *>(scene.root()->addChild(std::make_unique<Animation>([&] {
        return now;
    })));
    node->setGeometry({100, 100, 400, 300});
    renderer.frameTime = 5000;
    scene.render(renderer);
    EXPECT_FLOAT_EQ(renderer.calls.back().alpha, 0);
    // The wall clock has raced ahead, but the display has only shown 90 ms.
    now = 1000;
    renderer.frameTime = 5090;
    scene.render(renderer);
    EXPECT_FLOAT_EQ(renderer.calls.back().alpha, Animation::sample(Type::Open, {}, {}, 90.0 / Animation::duration(Type::Open, {}, {})).opacity);
    renderer.frameTime = 5000 + Animation::duration(Type::Open, {}, {});
    scene.render(renderer);
    EXPECT_FLOAT_EQ(renderer.calls.back().alpha, 1);
}

TEST(WindowAnimation, AnOpeningWindowAlreadyMovesInItsFirstFrame)
{
    scene::Scene scene;
    SnapshotRenderer renderer;
    auto *node = static_cast<Animation *>(scene.root()->addChild(std::make_unique<Animation>()));
    node->setGeometry({100, 100, 400, 300});
    const int duration = Animation::duration(Type::Open, {}, {});
    renderer.framePeriod = 16;
    renderer.frameTime = 5000;
    scene.render(renderer);
    const float first = Animation::sample(Type::Open, {}, {}, 16.0 / duration).opacity;
    EXPECT_FLOAT_EQ(renderer.calls.back().alpha, first);
    EXPECT_GT(first, .1);

    // A slow first frame still costs only one step, not the slots it missed.
    renderer.frameTime = 5080;
    scene.render(renderer);
    EXPECT_FLOAT_EQ(renderer.calls.back().alpha,
                    Animation::sample(Type::Open, {}, {}, 32.0 / duration).opacity);
}

TEST(WindowAnimation, ASlowFirstFrameDelaysTheAnimationInsteadOfSkippingAhead)
{
    scene::Scene scene;
    SnapshotRenderer renderer;
    auto *node = static_cast<Animation *>(scene.root()->addChild(std::make_unique<Animation>()));
    const base::Rect bounds(100, 100, 800, 600), slot(500, 900, 48, 36);
    node->setGeometry(bounds);
    node->finish();
    scene.render(renderer);
    node->setVisible(false);
    node->start(Type::Minimize, bounds, slot);
    const int duration = Animation::duration(Type::Minimize, bounds, slot);
    renderer.framePeriod = 13;
    renderer.frameTime = 5000;
    scene.render(renderer);
    EXPECT_EQ(renderer.lastTarget.x(), bounds.x() - 64);

    // The first frame missed four slots; the next one still shows one step.
    renderer.frameTime = 5065;
    scene.render(renderer);
    const auto step = Animation::sample(Type::Minimize, bounds, slot, 13.0 / duration);
    const double scale = step.bounds.width() / bounds.width();
    EXPECT_NEAR(renderer.lastTarget.x(), step.bounds.x() - 64 * scale, 1e-6);

    // Later frames step on the display's timeline again, from the new start.
    renderer.frameTime = 5078;
    scene.render(renderer);
    const auto next = Animation::sample(Type::Minimize, bounds, slot, 26.0 / duration);
    EXPECT_NEAR(renderer.lastTarget.x(), next.bounds.x() - 64 * next.bounds.width() / bounds.width(),
                1e-6);
}

// A 75 Hz slot is 13.33 ms; stepping on whole milliseconds would advance the
// flight by 13 or 14 ms a frame at random, and it would visibly judder.
TEST(WindowAnimation, StepsOnFractionalFrameTimes)
{
    scene::Scene scene;
    SnapshotRenderer renderer;
    auto *node = static_cast<Animation *>(scene.root()->addChild(std::make_unique<Animation>()));
    const base::Rect bounds(100, 100, 800, 600), slot(500, 900, 48, 36);
    node->setGeometry(bounds);
    node->finish();
    scene.render(renderer);
    node->setVisible(false);
    node->start(Type::Minimize, bounds, slot);
    const int duration = Animation::duration(Type::Minimize, bounds, slot);
    const double period = 1000.0 / 75;
    renderer.framePeriod = period;
    for (int frame = 0; frame < 5; ++frame) {
        renderer.frameTime = 5000 + frame * period;
        scene.render(renderer);
        const auto sample = Animation::sample(Type::Minimize, bounds, slot, frame * period / duration);
        const double scale = sample.bounds.width() / bounds.width();
        EXPECT_NEAR(renderer.lastTarget.x(), sample.bounds.x() - 64 * scale, 1e-6) << frame;
    }
}

TEST(WindowAnimation, LiveContentInTheSnapshotPassIsPresented)
{
    // Like the EGL pass, the snapshot suspends presentation recording.
    class SuspendingRenderer : public SnapshotRenderer
    {
    public:
        bool renderToTexture(std::unique_ptr<render::Texture> &texture, const base::Rect &bounds,
                             const std::function<void()> &paint) override
        {
            auto *submission = std::exchange(presentationSubmission, nullptr);
            const bool ok = SnapshotRenderer::renderToTexture(texture, bounds, paint);
            presentationSubmission = submission;
            return ok;
        }
    };

    class ContentNode : public scene::Node
    {
    public:
        std::shared_ptr<render::PresentationContent> content =
                std::make_shared<render::PresentationContent>();

    protected:
        void paint(render::Renderer &renderer, const base::Rect &geometry) const override
        {
            renderer.recordPresentation(content, geometry);
        }
    };

    scene::Scene scene;
    SuspendingRenderer renderer;
    auto *node = static_cast<Animation *>(scene.root()->addChild(std::make_unique<Animation>()));
    node->setGeometry({100, 100, 400, 300});
    auto *content = static_cast<ContentNode *>(node->addChild(std::make_unique<ContentNode>()));
    content->setGeometry({0, 0, 400, 300});
    render::PresentationSubmission submission;
    renderer.presentationSubmission = &submission;
    scene.render(renderer);
    EXPECT_TRUE(submission.contains(content->content));
    EXPECT_EQ(renderer.presentationSubmission, &submission);
    renderer.presentationSubmission = nullptr;
}

// An idle window whose subtree has not changed is drawn from the snapshot it
// left last frame, as viz skips such a render pass (CanSkipRenderPass).
class SnapshotReuse : public testing::Test
{
protected:
    scene::Scene scene;
    SnapshotRenderer renderer;
    Animation *window = static_cast<Animation *>(
            scene.root()->addChild(std::make_unique<Animation>()));
    scene::SurfaceNode *surface = nullptr;
    const std::vector<uint8_t> pixels = std::vector<uint8_t>(400 * 300 * 4, 255);

    void SetUp() override
    {
        window->setGeometry({100, 100, 400, 300});
        window->finish();
        surface = static_cast<scene::SurfaceNode *>(
                window->addChild(std::make_unique<scene::SurfaceNode>()));
        surface->setGeometry({0, 0, 400, 300});
        surface->setTexture(std::make_shared<testsupport::FakeTexture>(
                base::Size(400, 300), pixels.data(), 400 * 4, render::PixelFormat::Rgba8888));
        scene.render(renderer);
        ASSERT_EQ(renderer.captures, 1);
    }
};

TEST_F(SnapshotReuse, UnchangedContentIsDrawnFromTheLastSnapshotAndStillPresented)
{
    const auto content = std::make_shared<render::PresentationContent>();
    surface->setPresentationContent(content);
    render::PresentationSubmission submission;
    renderer.presentationSubmission = &submission;
    renderer.calls.clear();
    scene.render(renderer);
    renderer.presentationSubmission = nullptr;
    EXPECT_EQ(renderer.captures, 1);
    EXPECT_EQ(countKind(renderer, "texture"), 1);
    EXPECT_TRUE(submission.contains(content));
}

TEST_F(SnapshotReuse, DamageRedrawsItsPartAndAWholePixelMoveKeepsTheSnapshot)
{
    // One more pixel on each side for linear filtering.
    surface->notifyDamage(base::Rect(0, 0, 10, 10));
    scene.render(renderer);
    EXPECT_EQ(renderer.captures, 2);
    EXPECT_EQ(renderer.lastRepaint, base::Rect(100, 100, 11, 11));
    // Moved by whole device pixels, the subtree is unchanged in its own space
    // and the snapshot is drawn where the window now is.
    window->setPosition({120, 100});
    renderer.calls.clear();
    scene.render(renderer);
    EXPECT_EQ(renderer.captures, 2);
    EXPECT_EQ(countKind(renderer, "texture"), 1);
    scene.render(renderer);
    EXPECT_EQ(renderer.captures, 2);
}

TEST_F(SnapshotReuse, ContentChangingEveryFrameSkipsTheSnapshotAllButEveryEighthFrame)
{
    const auto drawFrames = [&](int count, bool damage) {
        std::vector<bool> snapshots;
        for (int frame = 0; frame < count; ++frame) {
            if (damage)
                surface->notifyDamage(base::Rect(0, 0, 400, 300));
            const int before = renderer.captures;
            scene.render(renderer);
            snapshots.push_back(renderer.captures > before);
        }
        return snapshots;
    };
    drawFrames(1, false);
    // A change on its own still goes through the snapshot; from the second
    // frame in a row the window is drawn straight to the output.
    EXPECT_EQ(drawFrames(10, true),
              std::vector<bool>({true, false, false, false, false, false, false, false, true, false}));
    // The next frame drawn without a change draws the snapshot again, and then
    // it is kept.
    EXPECT_EQ(drawFrames(2, false), std::vector<bool>({true, false}));

    // Blended by the client's opacity, it always goes through the snapshot.
    window->setOpacity(0.5f);
    drawFrames(1, false);
    EXPECT_EQ(drawFrames(3, true), std::vector<bool>({true, true, true}));
}

TEST(WindowAnimation, ChangesFarApartGoThroughTheSnapshot)
{
    double now = 1000;
    scene::Scene scene;
    SnapshotRenderer renderer;
    renderer.framePeriod = 16;
    auto *window = static_cast<Animation *>(scene.root()->addChild(
            std::make_unique<Animation>([&] { return now; })));
    window->setGeometry({100, 100, 400, 300});
    window->finish();
    auto *surface = static_cast<scene::SurfaceNode *>(
            window->addChild(std::make_unique<scene::SurfaceNode>()));
    surface->setGeometry({0, 0, 400, 300});
    const std::vector<uint8_t> pixels(400 * 300 * 4, 255);
    surface->setTexture(std::make_shared<testsupport::FakeTexture>(
            base::Size(400, 300), pixels.data(), 400 * 4, render::PixelFormat::Rgba8888));
    const auto drawChangeAt = [&](double time) {
        now = time;
        renderer.frameTime = time;
        surface->notifyDamage(base::Rect(0, 0, 400, 300));
        const int before = renderer.captures;
        scene.render(renderer);
        return renderer.captures > before;
    };
    now = renderer.frameTime = 1000;
    scene.render(renderer);
    now = renderer.frameTime = 1016;
    scene.render(renderer);

    // Every change is the next frame drawn, as with a blinking caret, but each
    // comes half a second after the last: a close, a minimize or a cross-fade
    // must not start from a snapshot several blinks old.
    EXPECT_TRUE(drawChangeAt(1500));
    EXPECT_TRUE(drawChangeAt(2000));
    EXPECT_TRUE(drawChangeAt(2500));
    // Frame after frame, the run still skips the snapshot until it is eight
    // frames old.
    EXPECT_FALSE(drawChangeAt(2516));
    EXPECT_FALSE(drawChangeAt(2532));
    EXPECT_FALSE(drawChangeAt(2548));
    EXPECT_FALSE(drawChangeAt(2564));
    // A pause shorter than the run's end still ends it once the snapshot is
    // that old.
    EXPECT_TRUE(drawChangeAt(2640));
    EXPECT_FALSE(drawChangeAt(2656));
}

TEST_F(SnapshotReuse, AFractionalPixelMoveRedrawsAll)
{
    renderer.setRasterScale(1.5);
    scene.render(renderer);
    EXPECT_EQ(renderer.captures, 2);
    // One logical pixel is a pixel and a half: the content's subpixel phase changes.
    window->setPosition({101, 100});
    scene.render(renderer);
    EXPECT_EQ(renderer.captures, 3);
    EXPECT_TRUE(renderer.lastRepaint.isEmpty());
    // Two more are three device pixels.
    window->setPosition({103, 100});
    scene.render(renderer);
    EXPECT_EQ(renderer.captures, 3);
}

TEST_F(SnapshotReuse, AWindowMovedWhileDrawnDirectlyClosesWhereItWasDrawn)
{
    scene.render(renderer);
    const base::RectF captured = renderer.lastTarget;
    // Content changing every frame is drawn directly from the second frame on,
    // and the snapshot stays where it was captured.
    for (int x = 101; x <= 104; ++x) {
        window->setPosition({x, 100});
        surface->notifyDamage(base::Rect(0, 0, 400, 300));
        scene.render(renderer);
    }
    ASSERT_EQ(renderer.captures, 2);
    window->close();
    scene.render(renderer);
    // The close has run for the microseconds the real clock moved on.
    const base::RectF expected = captured.translated(4, 0);
    EXPECT_NEAR(renderer.lastTarget.x(), expected.x(), 0.01);
    EXPECT_NEAR(renderer.lastTarget.y(), expected.y(), 0.01);
    EXPECT_NEAR(renderer.lastTarget.width(), expected.width(), 0.01);
}

TEST_F(SnapshotReuse, APopupChangingEveryFrameIsDrawnThroughItsSnapshot)
{
    // A menu lights another row on every pointer motion and closes on the
    // click: drawn directly, its close would fade out a snapshot frames old.
    window->setStyle(Animation::Style::Popup);
    for (int frame = 0; frame < 4; ++frame) {
        surface->notifyDamage(base::Rect(0, 0, 400, 300));
        scene.render(renderer);
        EXPECT_EQ(renderer.captures, frame + 2);
    }
    window->close();
    scene.render(renderer);
    EXPECT_EQ(renderer.captures, 5);
    EXPECT_EQ(renderer.kinds().back(), "texture");
}

TEST_F(SnapshotReuse, APlanePromotionIsNeverKeptForALaterFrame)
{
    // The promoted surface is left out of the snapshot drawn while it is on a
    // plane, so that snapshot cannot serve the frame after the promotion.
    surface->setPromotedToOverlay(true);
    scene.render(renderer);
    EXPECT_EQ(renderer.captures, 2);
    surface->setPromotedToOverlay(false);
    scene.render(renderer);
    EXPECT_EQ(renderer.captures, 3);
    scene.render(renderer);
    EXPECT_EQ(renderer.captures, 3);
}

TEST_F(SnapshotReuse, ClientOpacityBlendsTheKeptSnapshot)
{
    window->setOpacity(.5f);
    EXPECT_TRUE(window->paintsSubtree());
    EXPECT_FALSE(window->requiresFullRepaint());
    scene.render(renderer);
    const int captures = renderer.captures;
    scene.render(renderer);
    EXPECT_EQ(renderer.captures, captures);
    EXPECT_FLOAT_EQ(renderer.calls.back().alpha, .5f);
    window->setOpacity(0);
    scene.render(renderer);
    EXPECT_FLOAT_EQ(renderer.calls.back().alpha, 0);
    window->setOpacity(1);
    EXPECT_FALSE(window->paintsSubtree());
    scene.render(renderer);
    EXPECT_FLOAT_EQ(renderer.calls.back().alpha, 1);
}

TEST_F(SnapshotReuse, AnotherOutputDoesNotDrawAnIdleWindow)
{
    render::OutputRenderContext context;
    context.outputId = "right";
    context.logicalViewport = base::Rect(1920, 0, 1920, 1080);
    context.scale = 2;
    renderer.setOutputContext(context);
    renderer.calls.clear();
    surface->notifyDamage(base::Rect(0, 0, 10, 10));
    scene.render(renderer);
    EXPECT_EQ(renderer.captures, 1);
    EXPECT_EQ(countKind(renderer, "texture"), 0);
}

TEST_F(SnapshotReuse, CapturesShareTheSnapshotButScalesAndAppearanceChangesDrawAgain)
{
    const auto capture = [&] {
        renderer.capturing = true;
        scene.render(renderer);
        renderer.capturing = false;
    };
    // A capture of what the screen shows draws the same snapshot.
    capture();
    EXPECT_EQ(renderer.captures, 1);
    // Changed content is drawn once, by whichever comes first.
    surface->notifyDamage(base::Rect(0, 0, 10, 10));
    capture();
    EXPECT_EQ(renderer.captures, 2);
    scene.render(renderer);
    EXPECT_EQ(renderer.captures, 2);
    // A capture draws promoted surfaces too, so the screen does not reuse it.
    surface->setPromotedToOverlay(true);
    capture();
    surface->setPromotedToOverlay(false);
    EXPECT_EQ(renderer.captures, 3);
    scene.render(renderer);
    EXPECT_EQ(renderer.captures, 4);
    scene.render(renderer);
    EXPECT_EQ(renderer.captures, 4);
    renderer.setRasterScale(2);
    scene.render(renderer);
    EXPECT_EQ(renderer.captures, 5);
    // Session-wide appearance, such as the corner style, notifies the root.
    scene.root()->notifyChanged();
    scene.render(renderer);
    EXPECT_EQ(renderer.captures, 6);
    scene.render(renderer);
    EXPECT_EQ(renderer.captures, 6);
}

// A blur in the snapshot reads the output behind the window, which the
// window's own damage never shows changing.
class BlurredSnapshot : public SnapshotReuse
{
protected:
    scene::SurfaceNode *blur = nullptr;
    base::Rect sampled;
    int captures = 0;

    void SetUp() override
    {
        scene.setSize({1000, 800});
        SnapshotReuse::SetUp();
        blur = static_cast<scene::SurfaceNode *>(
                window->addChild(std::make_unique<scene::SurfaceNode>()));
        blur->setGeometry({150, 100, 100, 100});
        blur->setBackgroundBlur({0, 0, 100, 100}, 12);
        const int halo = render::kBackdropBlurSupport + 2;
        sampled = base::Rect(250, 200, 100, 100).adjusted(-halo, -halo, halo, halo);
        scene.render(renderer);
        // Idle once, so a change is not the second in a row, drawn directly.
        scene.render(renderer);
        captures = renderer.captures;
    }

    bool redrawn() const
    {
        return window->redrawsSubtree(scene.root()->geometry(), renderer.rasterScale());
    }
};

TEST_F(BlurredSnapshot, IsKeptWhileNothingItsBlurReadsIsDrawnAgain)
{
    renderer.repainted = base::Rect(900, 700, 10, 10);
    scene.render(renderer);
    EXPECT_EQ(renderer.captures, captures);
    // What the scene draws again there may have changed.
    renderer.repainted = base::Rect(160, 110, 10, 10);
    scene.render(renderer);
    EXPECT_EQ(renderer.captures, captures + 1);
    EXPECT_EQ(renderer.lastRepaint, sampled);
}

TEST_F(BlurredSnapshot, DrawnWholeItHasTheSceneRepaintWhatItsBlursRead)
{
    scene::DamageTracker tracker;
    const base::Rect viewport = scene.root()->geometry();
    const auto kept = [](uint64_t) {
        return true;
    };
    tracker.update(*scene.root(), viewport);
    EXPECT_FALSE(redrawn());
    EXPECT_TRUE(tracker.update(*scene.root(), viewport).isEmpty());
    EXPECT_TRUE(tracker.samples(kept).empty());
    // A frame with a promoted surface is not kept, so the next one draws whole.
    surface->setPromotedToOverlay(true);
    scene.render(renderer);
    surface->setPromotedToOverlay(false);
    EXPECT_TRUE(redrawn());
    EXPECT_FALSE(tracker.update(*scene.root(), viewport).isEmpty());
    EXPECT_EQ(tracker.samples(kept), std::vector<base::Rect> {sampled});
    scene.render(renderer);
    EXPECT_FALSE(redrawn());
}

TEST_F(BlurredSnapshot, ABlurDrawnFromWhatTheOutputLeftIsNotKept)
{
    // Changes on either side of the blur, whose one repaint rectangle crosses
    // it, while the output draws none of what the blur reads again.
    renderer.repainted = base::Rect(100, 100, 11, 11);
    surface->notifyDamage(base::Rect(0, 0, 10, 10));
    surface->notifyDamage(base::Rect(390, 290, 10, 10));
    scene.render(renderer);
    EXPECT_EQ(renderer.captures, captures + 1);
    EXPECT_TRUE(renderer.lastRepaint.contains(sampled));
    EXPECT_TRUE(redrawn());
    renderer.repainted.reset();
    scene.render(renderer);
    EXPECT_FALSE(redrawn());
}

TEST_F(SnapshotReuse, AScaledPresentationDrawsNoBlurs)
{
    // Scaled, the window is not where its blurs would read the screen.
    surface->setBackgroundBlur({0, 0, 400, 300}, 12);
    scene.render(renderer);
    EXPECT_EQ(renderer.blurs, 1);
    window->setPresentationScale(0.5, {200, 150}, false);
    scene.render(renderer);
    EXPECT_EQ(renderer.blurs, 1);
    window->setPresentationScale(1, {200, 150}, false);
    window->setOpacity(0.5);
    scene.render(renderer);
    EXPECT_EQ(renderer.blurs, 2);
}

TEST(WindowAnimation, CloseRetainsOnlyPixelsAndCollectsAfterCompletion)
{
    int64_t now = 0;
    scene::Scene scene;
    SnapshotRenderer renderer;
    auto *node = static_cast<Animation *>(scene.root()->addChild(std::make_unique<Animation>([&] {
        return now;
    })));
    node->setGeometry({100, 100, 400, 300});
    node->finish();
    scene.render(renderer);
    ASSERT_EQ(renderer.captures, 1);
    node->close();
    scene.render(renderer);
    EXPECT_EQ(renderer.captures, 1);
    now = 90;
    scene.render(renderer);
    EXPECT_FLOAT_EQ(renderer.calls.back().alpha,
                    Animation::sample(Type::Close, {}, {}, 90.0 / Animation::duration(Type::Close, {}, {}))
                            .opacity);
    now = Animation::duration(Type::Close, {}, {});
    scene.render(renderer);
    EXPECT_TRUE(scene.root()->children().empty());
    EXPECT_EQ(renderer.captures, 1);
}

TEST(WindowAnimation, CloseStartsFromTheSnapshotBoundsNotTheLiveGeometry)
{
    int64_t now = 0;
    scene::Scene scene;
    SnapshotRenderer renderer;
    auto *node = static_cast<Animation *>(scene.root()->addChild(std::make_unique<Animation>([&] {
        return now;
    })));
    node->setGeometry({100, 100, 400, 300});
    node->finish();
    scene.render(renderer);
    const base::Rect captured = renderer.calls.back().rect;
    // The frame was dropped, but nothing repainted before the surface went.
    node->setGeometry({100, 130, 400, 270});
    node->close();
    renderer.calls.clear();
    scene.render(renderer);
    ASSERT_FALSE(renderer.calls.empty());
    EXPECT_EQ(renderer.calls.back().rect, captured);
}

TEST(WindowAnimation, UnpaintedWindowCanCloseWithoutWaitingForAnAnimation)
{
    scene::Scene scene;
    SnapshotRenderer renderer;
    auto *node = static_cast<Animation *>(scene.root()->addChild(std::make_unique<Animation>()));
    node->close();
    scene.render(renderer);
    EXPECT_TRUE(scene.root()->children().empty());
    EXPECT_EQ(renderer.captures, 0);
}

TEST(WindowAnimation, HiddenMinimizeUsesSnapshotAndCanReverse)
{
    int64_t now = 0;
    scene::Scene scene;
    SnapshotRenderer renderer;
    auto *node = static_cast<Animation *>(scene.root()->addChild(std::make_unique<Animation>([&] {
        return now;
    })));
    const base::Rect bounds(100, 100, 400, 300), target(500, 900, 48, 40);
    node->setGeometry(bounds);
    node->finish();
    scene.render(renderer);
    node->setVisible(false);
    node->start(Type::Minimize, bounds, target);
    scene.render(renderer);
    now = 175;
    scene.render(renderer);
    EXPECT_FALSE(node->isEffectivelyVisible());
    EXPECT_EQ(renderer.captures, 1);
    const auto previous = renderer.calls.back();
    const auto sample = Animation::sample(
            Type::Minimize, bounds, target,
            double(now) / Animation::duration(Type::Minimize, bounds, target));
    const double sx = sample.bounds.width() / bounds.width();
    const double sy = sample.bounds.height() / bounds.height();
    EXPECT_EQ(previous.rect,
              base::RectF(sample.bounds.x() - 64 * sx, sample.bounds.y() - 64 * sy,
                     (bounds.width() + 128) * sx, (bounds.height() + 128) * sy)
                      .toEnclosingRect());
    node->setVisible(true);
    node->start(Type::Restore, target, bounds);
    scene.render(renderer);
    EXPECT_EQ(renderer.calls.back().rect, previous.rect);
    EXPECT_FLOAT_EQ(renderer.calls.back().alpha, previous.alpha);
    now += 380;
    scene.render(renderer);
    EXPECT_FLOAT_EQ(renderer.calls.back().alpha, 1);
}

TEST(WindowAnimation, MinimizeExportsPreviewThenTheExactLandingImage)
{
    int64_t now = 0;
    scene::Scene scene;
    SnapshotRenderer renderer;
    auto *node = static_cast<Animation *>(scene.root()->addChild(std::make_unique<Animation>([&] {
        return now;
    })));
    const base::Rect bounds(100, 100, 800, 400);
    node->setGeometry(bounds);
    node->finish();
    scene.render(renderer);
    node->setVisible(false);
    const base::Rect slot(500, 900, 48, 24);
    node->start(Type::Minimize, bounds, slot);
    node->setHoldAtEnd(true);
    std::vector<ui::Bitmap> snapshots;
    int landed = 0;
    node->setFinishedCallback([&] {
        EXPECT_EQ(snapshots.size(), 2u);
        ++landed;
    });
    node->requestSnapshot(
            [&](const ui::Bitmap &image) {
                snapshots.push_back(image);
            },
            200);
    scene.render(renderer);
    now = 100;
    scene.render(renderer);
    ASSERT_EQ(snapshots.size(), 1u);
    EXPECT_EQ(snapshots.front().size(), base::Size(200, 100));
    EXPECT_EQ(renderer.captures, 1);
    // The shrinking snapshot and both images read from it are mipmapped.
    ASSERT_FALSE(renderer.trilinear.empty());
    EXPECT_NE(renderer.trilinear.front(), nullptr);
    EXPECT_EQ(std::ranges::count(renderer.trilinear, renderer.trilinear.front()),
              std::ssize(renderer.trilinear));
    // Cropped to the window: the shadow margins land outside the image.
    EXPECT_EQ(renderer.imageDraws.back(), base::Rect(-16, -16, 232, 132));
    // Read from the level a quarter of the snapshot's size, 232 pixels across
    // like the image, and never from a smaller one.
    EXPECT_EQ(renderer.mipmapLevels, std::vector<int>({2, -1}));

    now = Animation::duration(Type::Minimize, bounds, slot);
    scene.render(renderer);
    ASSERT_EQ(snapshots.size(), 2u);
    EXPECT_EQ(snapshots.back().size(), slot.size());
    // 928 snapshot pixels onto 56: level 4 is 58 across.
    EXPECT_EQ(renderer.mipmapLevels, std::vector<int>({2, -1, 4, -1}));
    EXPECT_EQ(renderer.lastTarget, base::RectF(slot));
    EXPECT_EQ(landed, 1);
    const auto *texture = dynamic_cast<testsupport::FakeTexture *>(renderer.calls.back().texture);
    ASSERT_NE(texture, nullptr);
    const ui::Bitmap rgba = snapshots.back().converted(ui::Bitmap::Format::Rgba8888Premultiplied);
    EXPECT_EQ(texture->pixels(),
              std::vector<uint8_t>(rgba.constBits(), rgba.constBits() + rgba.sizeInBytes()));
    now += 100;
    scene.render(renderer);
    EXPECT_EQ(snapshots.size(), 2u);
    EXPECT_EQ(renderer.calls.back().texture, texture);
    EXPECT_EQ(landed, 1);
}

TEST(WindowAnimation, MinimizeFinishesAnInterruptedOpeningFadeBeforeLanding)
{
    int64_t now = 0;
    scene::Scene scene;
    SnapshotRenderer renderer;
    auto *node = static_cast<Animation *>(scene.root()->addChild(std::make_unique<Animation>([&] {
        return now;
    })));
    const base::Rect bounds(100, 100, 400, 300), slot(500, 900, 48, 36);
    node->setGeometry(bounds);
    scene.render(renderer);
    now = 50;
    scene.render(renderer);
    const float openingOpacity = renderer.calls.back().alpha;
    ASSERT_GT(openingOpacity, 0);
    ASSERT_LT(openingOpacity, 1);
    node->setVisible(false);
    node->start(Type::Minimize, bounds, slot);
    scene.render(renderer);
    EXPECT_FLOAT_EQ(renderer.calls.back().alpha, openingOpacity);
    now += Animation::duration(Type::Minimize, bounds, slot);
    scene.render(renderer);
    EXPECT_FLOAT_EQ(renderer.calls.back().alpha, 1);
}

TEST(WindowAnimation, RestoringDuringMinimizeCancelsTheLandingSnapshot)
{
    int64_t now = 0;
    scene::Scene scene;
    SnapshotRenderer renderer;
    auto *node = static_cast<Animation *>(scene.root()->addChild(std::make_unique<Animation>([&] {
        return now;
    })));
    const base::Rect bounds(100, 100, 400, 300), slot(500, 900, 48, 36);
    node->setGeometry(bounds);
    node->finish();
    scene.render(renderer);
    node->setVisible(false);
    node->start(Type::Minimize, bounds, slot);
    int snapshots = 0;
    node->requestSnapshot(
            [&](const ui::Bitmap &) {
                ++snapshots;
            },
            384);
    scene.render(renderer);
    EXPECT_EQ(snapshots, 1);
    now = 100;
    node->setVisible(true);
    node->start(Type::Restore, slot, bounds);
    scene.render(renderer);
    now += Animation::duration(Type::Restore, slot, bounds);
    scene.render(renderer);
    EXPECT_EQ(snapshots, 1);
    EXPECT_EQ(renderer.createdTextures, 0);
}

TEST(WindowAnimation, MinimizeWithoutADockKeepsAUsablePreview)
{
    int64_t now = 0;
    scene::Scene scene;
    SnapshotRenderer renderer;
    auto *node = static_cast<Animation *>(scene.root()->addChild(std::make_unique<Animation>([&] {
        return now;
    })));
    const base::Rect bounds(100, 100, 800, 400), fallback(500, 900, 1, 1);
    node->setGeometry(bounds);
    node->finish();
    scene.render(renderer);
    node->setVisible(false);
    node->start(Type::Minimize, bounds, fallback);
    std::vector<ui::Bitmap> snapshots;
    node->requestSnapshot(
            [&](const ui::Bitmap &image) {
                snapshots.push_back(image);
            },
            384);
    scene.render(renderer);
    now = Animation::duration(Type::Minimize, bounds, fallback);
    scene.render(renderer);
    ASSERT_EQ(snapshots.size(), 1u);
    EXPECT_EQ(snapshots.back().size(), base::Size(384, 192));
    EXPECT_EQ(renderer.createdTextures, 0);
}

TEST(WindowAnimation, DockTransitionsStayOpaqueAndPassOverThePanels)
{
    int64_t now = 0;
    scene::Scene scene;
    SnapshotRenderer renderer;
    auto *node = static_cast<Animation *>(scene.root()->addChild(std::make_unique<Animation>([&] {
        return now;
    })));
    const base::Rect bounds(100, 100, 400, 300), slot(500, 900, 48, 36);
    node->setGeometry(bounds);
    node->finish();
    scene.render(renderer);
    node->setVisible(false);
    int finished = 0;
    node->setFinishedCallback([&] {
        ++finished;
    });
    node->start(Type::Minimize, bounds, {500, 900, 40, 30});
    EXPECT_TRUE(node->drawsAbovePanels());
    scene.render(renderer);
    now = 100;
    // The dock opened the slot somewhere else.
    node->setMinimizeTarget(slot);
    scene.render(renderer);
    EXPECT_FLOAT_EQ(renderer.calls.back().alpha, 1);
    now = Animation::duration(Type::Minimize, bounds, slot) - 1;
    scene.render(renderer);
    EXPECT_FLOAT_EQ(renderer.calls.back().alpha, 1);
    ++now;
    scene.render(renderer);
    EXPECT_EQ(finished, 1);
    EXPECT_FALSE(node->drawsAbovePanels());

    node->setVisible(true);
    node->start(Type::Restore, slot, bounds);
    renderer.calls.clear();
    scene.render(renderer);
    // Out of the thumbnail at full opacity.
    EXPECT_FLOAT_EQ(renderer.calls.back().alpha, 1);
    const double scale = slot.width() / double(bounds.width());
    EXPECT_EQ(renderer.calls.back().rect,
              base::RectF(slot).adjusted(-64 * scale, -64 * scale, 64 * scale, 64 * scale)
                      .toEnclosingRect());
}

TEST(WindowAnimation, MinimizeHoldsOnTheSlotUntilTheDockTakesOver)
{
    int64_t now = 0;
    scene::Scene scene;
    SnapshotRenderer renderer;
    auto *node = static_cast<Animation *>(scene.root()->addChild(std::make_unique<Animation>([&] {
        return now;
    })));
    const base::Rect bounds(100, 100, 400, 300), slot(500, 900, 48, 36);
    node->setGeometry(bounds);
    node->finish();
    scene.render(renderer);
    node->setVisible(false);
    int landed = 0;
    node->setFinishedCallback([&] {
        ++landed;
    });
    node->setHoldAtEnd(true);
    node->start(Type::Minimize, bounds, slot);
    scene.render(renderer);

    // Halfway the shadow is on its way out: a translucent exterior pass under
    // the body drawn inside its shape.
    now = Animation::duration(Type::Minimize, bounds, slot) / 2;
    renderer.calls.clear();
    scene.render(renderer);
    ASSERT_EQ(renderer.calls.size(), 3);
    EXPECT_GT(renderer.calls[1].alpha, 0);
    EXPECT_LT(renderer.calls[1].alpha, 1);
    EXPECT_FLOAT_EQ(renderer.calls[2].alpha, 1);

    now = Animation::duration(Type::Minimize, bounds, slot);
    scene.render(renderer);
    EXPECT_EQ(landed, 1);
    EXPECT_TRUE(node->isHolding());
    EXPECT_TRUE(node->drawsAbovePanels());

    // Still there, and only the body, as long as the dock has not taken over.
    const double scale = slot.width() / double(bounds.width());
    const base::Rect landedRect =
            base::RectF(slot).adjusted(-64 * scale, -64 * scale, 64 * scale, 64 * scale).toEnclosingRect();
    int requests = 0;
    scene.root()->setChangeCallback([&] {
        ++requests;
    });
    for (now += 100; now <= 1000; now += 100) {
        renderer.calls.clear();
        scene.render(renderer);
        ASSERT_EQ(renderer.calls.size(), 2);
        EXPECT_EQ(renderer.calls.back().rect, landedRect);
        EXPECT_FLOAT_EQ(renderer.calls.back().alpha, 1);
    }
    EXPECT_EQ(requests, 0);

    EXPECT_EQ(SnapshotTexture::live, 1);
    node->releaseHold();
    EXPECT_GT(requests, 0);
    EXPECT_FALSE(node->drawsAbovePanels());
    // Minimized, the window keeps no snapshot on the GPU.
    EXPECT_EQ(SnapshotTexture::live, 0);
    renderer.calls.clear();
    scene.render(renderer);
    EXPECT_EQ(renderer.calls.size(), 1); // Scene clear only.

    // A restore draws the live window again.
    const int captures = renderer.captures;
    node->setVisible(true);
    node->start(Type::Restore, slot, bounds);
    renderer.calls.clear();
    scene.render(renderer);
    EXPECT_EQ(renderer.captures, captures + 1);
    EXPECT_EQ(SnapshotTexture::live, 1);
    EXPECT_GT(renderer.calls.size(), 1);
}

TEST(WindowAnimation, MovingDockTargetsPreservePositionAndVelocity)
{
    int64_t now = 0;
    scene::Scene scene;
    SnapshotRenderer renderer;
    auto *node = static_cast<Animation *>(scene.root()->addChild(std::make_unique<Animation>([&] {
        return now;
    })));
    const base::Rect bounds(100, 100, 800, 600);
    base::Rect slot(700, 900, 64, 48);
    node->setGeometry(bounds);
    node->finish();
    scene.render(renderer);
    node->setVisible(false);
    node->setHoldAtEnd(true);
    int landed = 0;
    node->setFinishedCallback([&] {
        ++landed;
    });
    node->start(Type::Minimize, bounds, slot);
    scene.render(renderer);

    const auto renderAt = [&](int64_t time) {
        now = time;
        scene.render(renderer);
        return renderer.lastTarget;
    };
    for (const int64_t time : {100, 180, 280}) {
        const base::RectF before = renderAt(time - 1);
        const base::RectF at = renderAt(time);
        slot.translate(-40, 12);
        slot.setSize(slot.size() + base::Size(8, 6));
        node->setMinimizeTarget(slot);
        const base::RectF retargeted = renderAt(time);
        EXPECT_NEAR(retargeted.x(), at.x(), 1e-9);
        EXPECT_NEAR(retargeted.y(), at.y(), 1e-9);
        EXPECT_NEAR(retargeted.width(), at.width(), 1e-9);
        EXPECT_NEAR(retargeted.height(), at.height(), 1e-9);

        const base::RectF after = renderAt(time + 1);
        EXPECT_NEAR(after.x() - at.x(), at.x() - before.x(), .15);
        EXPECT_NEAR(after.y() - at.y(), at.y() - before.y(), .15);
        EXPECT_NEAR(after.width() - at.width(), at.width() - before.width(), .15);
        EXPECT_NEAR(after.height() - at.height(), at.height() - before.height(), .15);
        EXPECT_EQ(landed, 0);
    }

    const double scale = slot.width() / double(bounds.width());
    const base::RectF expected = base::RectF(slot).adjusted(-64 * scale, -64 * scale, 64 * scale, 64 * scale);
    const base::RectF landedAt = renderAt(Animation::duration(Type::Minimize, bounds, slot));
    EXPECT_TRUE(base::fuzzyCompare(landedAt, expected)) << landedAt << " " << expected;
    EXPECT_TRUE(node->isHolding());
    EXPECT_EQ(landed, 1);
    // Once handed over, the held image follows the dock's actual displayed slot.
    slot.translate(20, 0);
    node->setMinimizeTarget(slot);
    const base::RectF followed = renderAt(now + 16);
    EXPECT_TRUE(base::fuzzyCompare(followed, expected.translated(20, 0))) << followed;
    EXPECT_EQ(landed, 1);
}

TEST(WindowAnimation, CrossFadeKeepsHigherResolutionSnapshotOnTop)
{
    int64_t now = 0;
    scene::Scene scene;
    SnapshotRenderer renderer;
    auto *node = static_cast<Animation *>(scene.root()->addChild(std::make_unique<Animation>([&] {
        return now;
    })));
    const base::Rect oldBounds(0, 0, 800, 600), newBounds(100, 100, 400, 300);
    node->setGeometry(oldBounds);
    node->finish();
    scene.render(renderer);
    auto *oldTexture = renderer.calls.back().texture;
    node->setGeometry(newBounds);
    node->start(Type::Bounds, oldBounds, newBounds);
    scene.render(renderer);
    EXPECT_EQ(renderer.calls.back().texture, oldTexture);
    now = 175;
    scene.render(renderer);
    EXPECT_EQ(renderer.calls.back().texture, oldTexture);
    EXPECT_FLOAT_EQ(renderer.calls.back().alpha, .25);
    EXPECT_FLOAT_EQ(renderer.calls[renderer.calls.size() - 2].alpha, 1);
}

namespace {

scene::WindowVisualState frameVisual(const base::Size &size, bool normal)
{
    scene::WindowVisualState state;
    state.mode = scene::WindowVisualMode::ServerFrame;
    state.shape = {base::RectF(base::PointF(), size), normal ? 12.0 : 0.0};
    state.clipContent = normal;
    state.drawOutline = normal;
    state.outlineWidthPx = 1;
    state.outlineColor = {0, 0, 0, 1};
    return state;
}

struct ExteriorMorph {
    scene::Scene scene;
    SnapshotRenderer renderer;
    int64_t now = 0;
    Animation *node = nullptr;
    render::Texture *oldTexture = nullptr;
    double progress = 0;
    base::RectF bounds;

    // Renders the cross-fade from `from` to `to` halfway through.
    ExteriorMorph(const base::Rect &from, const base::Rect &to)
    {
        node = static_cast<Animation *>(scene.root()->addChild(std::make_unique<Animation>([this] {
            return now;
        })));
        auto *visual = static_cast<scene::WindowVisualNode *>(
                node->addChild(std::make_unique<scene::WindowVisualNode>()));
        visual->setState(frameVisual(from.size(), from.width() < to.width()));
        node->setGeometry(from);
        node->finish();
        scene.render(renderer);
        oldTexture = renderer.calls.back().texture;

        visual->setState(frameVisual(to.size(), from.width() > to.width()));
        node->setGeometry(to);
        node->start(Type::Bounds, from, to);
        scene.render(renderer);
        now = Animation::duration(Type::Bounds, from, to) / 2;
        const auto sample = Animation::sample(Type::Bounds, from, to, double(now)
                                                      / Animation::duration(Type::Bounds, from, to));
        bounds = sample.bounds;
        progress = (bounds.width() - from.width()) / (to.width() - from.width());
        renderer.calls.clear();
        renderer.clips.clear();
        renderer.outlines.clear();
        scene.render(renderer);
    }
};

} // namespace

TEST(WindowAnimation, MaximizeCarriesTheCornersShadowAndOutlineAcross)
{
    const base::Rect normal(100, 100, 400, 300), maximized(0, 0, 800, 600);
    ExteriorMorph morph(normal, maximized);
    const double radius = 12 * (1 - morph.progress);
    ASSERT_GT(morph.progress, .2);
    ASSERT_LT(morph.progress, .9);

    // The live content is clipped in the snapshot to corners that come out at
    // the morphed radius once the snapshot is scaled down to the sample.
    ASSERT_EQ(morph.renderer.clips.size(), 2);
    const auto snapshotClip = morph.renderer.clips[0];
    EXPECT_NEAR(snapshotClip.radius * snapshotClip.radiusScale.width() * morph.bounds.width()
                        / maximized.width(),
                radius, 1e-6);
    const auto bodyClip = morph.renderer.clips[1];
    EXPECT_NEAR(bodyClip.radius, radius, 1e-6);
    EXPECT_EQ(bodyClip.radiusScale, base::SizeF(1, 1));
    EXPECT_EQ(bodyClip.bounds, morph.bounds);

    // The old shadow fades out underneath, then the body is drawn inside the
    // morphed corners, and the old content dissolves over it.
    const auto &calls = morph.renderer.calls;
    ASSERT_EQ(calls.size(), 4);
    EXPECT_EQ(calls[1].texture, morph.oldTexture);
    EXPECT_NEAR(calls[1].alpha, 1 - morph.progress, 1e-6);
    EXPECT_NE(calls[2].texture, morph.oldTexture);
    EXPECT_FLOAT_EQ(calls[2].alpha, 1);
    EXPECT_EQ(calls[3].texture, morph.oldTexture);
    ASSERT_EQ(morph.renderer.outlines.size(), 1);
    EXPECT_NEAR(morph.renderer.outlines[0], 1 - morph.progress, 1e-6);
}

TEST(WindowAnimation, RestoreFromMaximizedGrowsTheExteriorBack)
{
    const base::Rect maximized(0, 0, 800, 600), normal(100, 100, 400, 300);
    ExteriorMorph morph(maximized, normal);
    const double radius = 12 * morph.progress;

    ASSERT_EQ(morph.renderer.clips.size(), 2);
    const auto snapshotClip = morph.renderer.clips[0];
    EXPECT_NEAR(snapshotClip.radius * snapshotClip.radiusScale.width() * morph.bounds.width()
                        / normal.width(),
                radius, 1e-6);
    EXPECT_NEAR(morph.renderer.clips[1].radius, radius, 1e-6);

    // The new shadow fades in around a body drawn once at full opacity.
    const auto &calls = morph.renderer.calls;
    ASSERT_EQ(calls.size(), 4);
    EXPECT_NE(calls[1].texture, morph.oldTexture);
    EXPECT_NEAR(calls[1].alpha, morph.progress, 1e-6);
    EXPECT_EQ(calls[2].texture, calls[1].texture);
    EXPECT_FLOAT_EQ(calls[2].alpha, 1);
    EXPECT_EQ(calls[3].texture, morph.oldTexture);
    ASSERT_EQ(morph.renderer.outlines.size(), 1);
    EXPECT_NEAR(morph.renderer.outlines[0], morph.progress, 1e-6);
}

TEST(WindowAnimation, ADragCrossFadeChasesTheWindowItDissolvesInto)
{
    int64_t now = 0;
    scene::Scene scene;
    SnapshotRenderer renderer;
    auto *node = static_cast<Animation *>(scene.root()->addChild(std::make_unique<Animation>([&] {
        return now;
    })));
    const base::Rect maximized(0, 0, 800, 600);
    const base::Rect restored(200, 100, 400, 300);
    node->setGeometry(maximized);
    node->finish();
    scene.render(renderer);
    ASSERT_NE(nullptr, renderer.calls.back().texture);

    // The drag pulls the window out of the maximized state. The last draw of a
    // cross-fade is the outgoing snapshot, laid over the window's own content.
    node->setGeometry(restored);
    node->start(Type::DragBounds, maximized, restored);
    scene.render(renderer);
    EXPECT_NEAR(400, renderer.calls.back().rect.centerF().x(), 1);
    EXPECT_NEAR(300, renderer.calls.back().rect.centerF().y(), 1);

    // The drag keeps going while the cross-fade runs: the window moves 300px
    // right and 200px down, and the dissolve has to come with it.
    const base::Rect moved(500, 300, 400, 300);
    node->setGeometry(moved);
    node->setDragTarget(moved);
    now = 60;
    scene.render(renderer);
    const base::RectF halfway = renderer.calls.back().rect;
    EXPECT_NEAR(700, halfway.center().x(), 1);
    // Half way the window is still 600x450 on its way down from 800x600, and
    // it is the top left that stays anchored - as it does upstream, where the
    // scale is applied about the layer origin - so the centre sits low.
    EXPECT_NEAR(475, halfway.center().y(), 1);
    EXPECT_GT(halfway.width(), moved.width());
    EXPECT_LT(halfway.width(), maximized.width());

    now = 120;
    scene.render(renderer);
    // And it lands exactly on the window.
    EXPECT_NEAR(700, renderer.calls.back().rect.centerF().x(), 1);
    EXPECT_NEAR(450, renderer.calls.back().rect.centerF().y(), 1);
    // The content underneath is drawn at its natural size, plus the 64px halo
    // the snapshot coverage carries on each side.
    const base::RectF content = renderer.calls[renderer.calls.size() - 2].rect;
    EXPECT_NEAR(double(moved.width()), content.width() - 128, 1);
    EXPECT_NEAR(double(moved.height()), content.height() - 128, 1);
}

TEST(WindowAnimation, ADragCrossFadeIsShorterThanAStateChange)
{
    EXPECT_EQ(120, Animation::duration(Type::DragBounds, {0, 0, 800, 600}, {0, 0, 400, 300}));
    // Linear, so half the time is half the way.
    const auto value = Animation::sample(Type::DragBounds, {0, 0, 800, 600}, {0, 0, 400, 300}, .5);
    EXPECT_DOUBLE_EQ(600, value.bounds.width());
}

TEST(WindowAnimation, UnsupportedRendererDrawsNormally)
{
    Animation node;
    testsupport::FakeRenderer renderer;
    node.setGeometry({0, 0, 100, 100});
    auto color = std::make_unique<scene::ColorNode>(render::Color {1, 0, 0, 1});
    color->setGeometry({0, 0, 100, 100});
    node.addChild(std::move(color));
    node.render(renderer);
    ASSERT_EQ(renderer.calls.size(), 1);
    EXPECT_EQ(renderer.calls.back().kind, "fill");
}

TEST(WindowAnimation, ClosingBeforeResizePaintRetainsPreviousSnapshot)
{
    int64_t now = 0;
    scene::Scene scene;
    SnapshotRenderer renderer;
    auto *node = static_cast<Animation *>(scene.root()->addChild(std::make_unique<Animation>([&] {
        return now;
    })));
    const base::Rect from(100, 100, 400, 300), to(0, 0, 800, 600);
    node->setGeometry(from);
    node->finish();
    scene.render(renderer);
    const auto *texture = renderer.calls.back().texture;
    node->start(Type::Bounds, from, to);
    node->close();
    scene.render(renderer);
    ASSERT_EQ(scene.root()->children().size(), 1u);
    EXPECT_EQ(renderer.calls.back().texture, texture);
    now = Animation::duration(Type::Close, {}, {});
    scene.render(renderer);
    EXPECT_TRUE(scene.root()->children().empty());
}

TEST(WindowAnimation, FramesDamageOnlyTheAnimatedWindow)
{
    int64_t now = 0;
    scene::Scene scene;
    SnapshotRenderer renderer;
    auto *layer = scene.root()->addChild(std::make_unique<scene::Node>());
    auto *other = layer->addChild(std::make_unique<scene::ColorNode>(render::Color {1, 1, 1, 1}));
    other->setGeometry({700, 500, 200, 200});
    auto *node = static_cast<Animation *>(layer->addChild(std::make_unique<Animation>([&] {
        return now;
    })));
    node->setGeometry({100, 100, 300, 200});
    const base::Rect viewport(0, 0, 1000, 800);
    scene::DamageTracker tracker;
    tracker.update(*scene.root(), viewport);
    do {
        scene.render(renderer);
        now += 16;
        // Neither the window layer nor the whole output: the open grows from
        // the window's centre, inside its shadow margin.
        const base::Region damage = tracker.update(*scene.root(), viewport);
        EXPECT_FALSE(damage.isEmpty()) << now;
        EXPECT_TRUE(damage.subtracted(base::Rect(0, 0, 500, 400)).isEmpty()) << now;
        EXPECT_FALSE(damage.intersects(other->geometry())) << now;
    } while (node->isRunning() && now < 2000);
    EXPECT_FALSE(node->isRunning());
    EXPECT_TRUE(tracker.update(*scene.root(), viewport).isEmpty());
}

TEST(WindowAnimation, AHiddenMinimizeIsDamagedUntilItsLastFrameIsCleared)
{
    int64_t now = 0;
    scene::Scene scene;
    SnapshotRenderer renderer;
    auto *node = static_cast<Animation *>(scene.root()->addChild(std::make_unique<Animation>([&] {
        return now;
    })));
    const base::Rect bounds(100, 100, 400, 300), target(500, 900, 48, 40);
    node->setGeometry(bounds);
    node->finish();
    scene.render(renderer);
    const base::Rect screen(0, 0, 1000, 1000), elsewhere(1000, 0, 1000, 1000);
    scene::DamageTracker tracker, beside;
    tracker.update(*scene.root(), screen);
    beside.update(*scene.root(), elsewhere);
    int requests = 0;
    scene.root()->setChangeCallback([&] {
        ++requests;
    });
    node->setVisible(false);
    node->start(Type::Minimize, bounds, target);
    EXPECT_EQ(requests, 2);
    const base::Rect path = node->absolutePaintBounds();
    EXPECT_EQ(tracker.update(*scene.root(), screen), base::Region(screen));
    EXPECT_TRUE(beside.update(*scene.root(), elsewhere).isEmpty());
    scene.render(renderer);
    now = Animation::duration(Type::Minimize, bounds, target);
    requests = 0;
    scene.render(renderer);
    EXPECT_FALSE(node->isRunning());
    EXPECT_EQ(requests, 1);
    EXPECT_EQ(tracker.update(*scene.root(), screen), base::Region(path));
    EXPECT_TRUE(beside.update(*scene.root(), elsewhere).isEmpty());
}

TEST(WindowAnimation, FinishedMinimizeStopsRequestingFramesAndShutdownReleasesSnapshots)
{
    int64_t now = 0;
    scene::Scene scene;
    SnapshotRenderer renderer;
    auto *node = static_cast<Animation *>(scene.root()->addChild(std::make_unique<Animation>([&] {
        return now;
    })));
    node->setGeometry({100, 100, 400, 300});
    node->finish();
    scene.render(renderer);
    node->setVisible(false);
    node->start(Type::Minimize, node->geometry(), {500, 900, 1, 1});
    scene.render(renderer);
    now = Animation::duration(Type::Minimize, node->geometry(), {500, 900, 1, 1});
    scene.render(renderer);
    int requests = 0;
    scene.root()->setChangeCallback([&] {
        ++requests;
    });
    renderer.calls.clear();
    scene.render(renderer);
    EXPECT_EQ(requests, 0);
    EXPECT_EQ(renderer.calls.size(), 1); // Scene clear only.
    node->releaseRenderResources();
    node->close();
    scene.render(renderer);
    EXPECT_TRUE(scene.root()->children().empty());
}

TEST(WindowAnimation, RestoreWaitsForResizedCommitBeforeCapturingOrStartingClock)
{
    int64_t now = 0;
    scene::Scene scene;
    SnapshotRenderer renderer;
    auto *node = static_cast<Animation *>(scene.root()->addChild(std::make_unique<Animation>([&] {
        return now;
    })));
    const base::Rect maximized(0, 0, 1920, 1080), restored(200, 150, 800, 600);
    node->setGeometry(maximized);
    auto *client = node->addChild(std::make_unique<scene::ColorNode>(render::Color {1, 1, 1, 1}));
    client->setGeometry({0, 0, 1920, 1080});
    node->finish();
    scene.render(renderer);
    const auto original = renderer.calls.back();
    node->setGeometry(restored);
    node->start(Type::Bounds, maximized, restored);
    node->setBoundsContentReady(false);
    int requests = 0;
    scene.root()->setChangeCallback([&] {
        ++requests;
    });
    for (now = 0; now <= 1000; now += 100) {
        scene.render(renderer);
        EXPECT_EQ(renderer.captures, 1);
        EXPECT_EQ(renderer.calls.back().texture, original.texture);
        EXPECT_EQ(renderer.calls.back().rect, original.rect);
        EXPECT_FLOAT_EQ(renderer.calls.back().alpha, 1);
    }
    EXPECT_EQ(requests, 0);
    client->setGeometry({0, 0, 800, 600});
    node->setBoundsContentReady(true);
    EXPECT_GT(requests, 0);
    scene.render(renderer);
    EXPECT_EQ(renderer.captures, 2);
    EXPECT_EQ(renderer.calls.back().rect, original.rect);
    EXPECT_FLOAT_EQ(renderer.calls.back().alpha, 1);
    now += Animation::duration(Type::Bounds, maximized, restored);
    scene.render(renderer);
    scene.render(renderer);
    EXPECT_EQ(renderer.calls.back().rect, restored.adjusted(-64, -64, 64, 64));
}

TEST(WindowAnimation, ReversingWhileWaitingPreservesPresentedBounds)
{
    Animation node;
    SnapshotRenderer renderer;
    const base::Rect maximized(0, 0, 1920, 1080), restored(200, 150, 800, 600);
    node.setGeometry(maximized);
    node.finish();
    node.render(renderer);
    const auto original = renderer.calls.back();
    node.setGeometry(restored);
    node.start(Type::Bounds, maximized, restored);
    node.setBoundsContentReady(false);
    node.render(renderer);
    node.setGeometry(maximized);
    node.start(Type::Bounds, restored, maximized);
    node.setBoundsContentReady(false);
    node.render(renderer);
    EXPECT_EQ(renderer.captures, 1);
    EXPECT_EQ(renderer.calls.back().rect, original.rect);
    EXPECT_EQ(renderer.calls.back().texture, original.texture);
}

TEST(WindowAnimation, InterruptionsContinueFromLastDrawnFrame)
{
    for (const auto type : {Type::Open, Type::Close, Type::Minimize, Type::Restore, Type::Bounds}) {
        int64_t now = 0;
        Animation node([&] {
            return now;
        });
        SnapshotRenderer renderer;
        const base::Rect bounds(100, 100, 400, 300);
        node.setGeometry(bounds);
        node.render(renderer);
        now = 45;
        node.render(renderer);
        const auto previous = renderer.calls.back();
        now += 50;
        node.start(type, bounds, bounds);
        renderer.calls.clear();
        node.render(renderer);
        ASSERT_FALSE(renderer.calls.empty());
        EXPECT_EQ(renderer.calls.front().rect, previous.rect);
        EXPECT_FLOAT_EQ(renderer.calls.front().alpha, previous.alpha);
    }
}

TEST(WindowAnimation, ResizeWaitKeepsInterruptedOpeningOpacity)
{
    int64_t now = 0;
    Animation node([&] {
        return now;
    });
    SnapshotRenderer renderer;
    const base::Rect bounds(100, 100, 400, 300), target(0, 0, 800, 600);
    node.setGeometry(bounds);
    node.render(renderer);
    now = 45;
    node.render(renderer);
    const auto previous = renderer.calls.back();
    node.setGeometry(target);
    node.start(Type::Bounds, bounds, target);
    node.setBoundsContentReady(false);
    now = 1000;
    node.render(renderer);
    EXPECT_EQ(renderer.calls.back().rect, previous.rect);
    EXPECT_FLOAT_EQ(renderer.calls.back().alpha, previous.alpha);
}

TEST(WindowAnimation, LauncherFadesThroughASlightZoomAndLandsSoftly)
{
    const base::RectF screen(0, 0, 1000, 800), zoomed(-25, -20, 1050, 840);
    const auto style = Animation::Style::Launcher;
    EXPECT_EQ(Animation::duration(Type::Open, screen, screen, style), 360);
    EXPECT_EQ(Animation::duration(Type::Close, screen, screen, style), 160);
    for (const auto type : {Type::Open, Type::Close}) {
        const base::RectF from = type == Type::Open ? zoomed : screen;
        const base::RectF to = type == Type::Open ? screen : zoomed;
        EXPECT_EQ(Animation::sample(type, from, to, 0, style).bounds, from);
        EXPECT_EQ(Animation::sample(type, from, to, 1, style).bounds, to);
        EXPECT_FLOAT_EQ(Animation::sample(type, from, to, 1, style).opacity,
                        type == Type::Open ? 1 : 0);
        double lastWidth = from.width();
        float lastOpacity = type == Type::Open ? 0 : 1;
        for (int frame = 1; frame <= 240; ++frame) {
            const auto value = Animation::sample(type, from, to, frame / 240.0, style);
            EXPECT_LE(std::abs(value.bounds.width() - to.width()),
                      std::abs(lastWidth - to.width()) + 1e-9);
            EXPECT_TRUE(base::fuzzyCompare(value.bounds.center(), screen.center()))
                    << value.bounds.center();
            if (type == Type::Open)
                EXPECT_GE(value.opacity, lastOpacity);
            else
                EXPECT_LE(value.opacity, lastOpacity);
            lastWidth = value.bounds.width();
            lastOpacity = value.opacity;
        }
        // Responds on the first frame at 60Hz instead of easing in.
        const double firstFrame = (1000 / 60.0) / Animation::duration(type, from, to, style);
        EXPECT_GT(std::abs(Animation::sample(type, from, to, firstFrame, style).opacity
                           - (type == Type::Open ? 0 : 1)),
                  .1);
    }
}

// A hide is drawn as it arrives, between refreshes; each refresh after it must
// still take the same share of the fade, down to the last one.
TEST(WindowAnimation, LauncherCloseFadesInEvenRefreshSteps)
{
    const base::Rect screen(0, 0, 1000, 800);
    scene::Scene scene;
    SnapshotRenderer renderer;
    auto *node = static_cast<Animation *>(scene.root()->addChild(std::make_unique<Animation>()));
    node->setStyle(Animation::Style::Launcher);
    node->setGeometry(screen);
    node->finish();
    scene.render(renderer);
    node->close();

    const double period = 1000.0 / 60;
    renderer.framePeriod = period;
    renderer.frameTime = 5000 - 4.4;
    scene.render(renderer);
    EXPECT_NEAR(renderer.calls.back().alpha, .9, 1e-4);
    for (int frame = 1; frame <= 8; ++frame) {
        renderer.frameTime = 5000 + frame * period;
        scene.render(renderer);
        EXPECT_NEAR(renderer.calls.back().alpha, .9 - frame * .1, 1e-4) << frame;
    }
    renderer.frameTime = 5000 + 9 * period;
    scene.render(renderer);
    EXPECT_EQ(renderer.calls.back().alpha, 0);
}

TEST(WindowAnimation, FadeOnlyChangesOpacity)
{
    const base::RectF desktop(0, 0, 1920, 1080);
    const auto style = Animation::Style::Fade;
    EXPECT_EQ(Animation::duration(Type::Open, desktop, desktop, style),
              Animation::duration(Type::Open, desktop, desktop, Animation::Style::Window));
    for (const auto type : {Type::Open, Type::Close}) {
        float lastOpacity = type == Type::Open ? 0 : 1;
        for (int frame = 0; frame <= 100; ++frame) {
            const auto value = Animation::sample(type, desktop, desktop, frame / 100.0, style);
            EXPECT_EQ(value.bounds, desktop);
            if (type == Type::Open)
                EXPECT_GE(value.opacity, lastOpacity);
            else
                EXPECT_LE(value.opacity, lastOpacity);
            lastOpacity = value.opacity;
        }
        EXPECT_FLOAT_EQ(lastOpacity, type == Type::Open ? 1 : 0);
    }

    int64_t now = 0;
    scene::Scene scene;
    SnapshotRenderer renderer;
    auto *node = static_cast<Animation *>(scene.root()->addChild(std::make_unique<Animation>([&] {
        return now;
    })));
    node->setStyle(style);
    node->setGeometry(desktop.toNearestRect());
    scene.render(renderer);
    now = 100;
    scene.render(renderer);
    EXPECT_TRUE(node->isRunning());
    // Drawn unscaled mid-fade: the 64 px overflow margin stays 64 px.
    EXPECT_EQ(renderer.calls.back().rect, desktop.toNearestRect().adjusted(-64, -64, 64, 64));
    EXPECT_GT(renderer.calls.back().alpha, 0);
    EXPECT_LT(renderer.calls.back().alpha, 1);
}

TEST(WindowAnimation, PopupFadesInPlaceAndFollowsItsLiveGeometry)
{
    const base::RectF menu(100, 100, 200, 300);
    const auto style = Animation::Style::Popup;
    EXPECT_EQ(Animation::duration(Type::Open, menu, menu, style), 100);
    EXPECT_EQ(Animation::duration(Type::Close, menu, menu, style), 80);
    for (const auto type : {Type::Open, Type::Close}) {
        float lastOpacity = type == Type::Open ? 0 : 1;
        for (int frame = 0; frame <= 100; ++frame) {
            const auto value = Animation::sample(type, menu, menu, frame / 100.0, style);
            EXPECT_EQ(value.bounds, menu);
            if (type == Type::Open)
                EXPECT_GE(value.opacity, lastOpacity);
            else
                EXPECT_LE(value.opacity, lastOpacity);
            lastOpacity = value.opacity;
        }
        EXPECT_FLOAT_EQ(lastOpacity, type == Type::Open ? 1 : 0);
        // Most of the change lands within the first 60Hz frame.
        const double firstFrame = (1000 / 60.0) / Animation::duration(type, menu, menu, style);
        EXPECT_GT(std::abs(Animation::sample(type, menu, menu, firstFrame, style).opacity
                           - (type == Type::Open ? 0 : 1)),
                  .25);
    }

    int64_t now = 0;
    scene::Scene scene;
    SnapshotRenderer renderer;
    auto *popup = static_cast<Animation *>(scene.root()->addChild(std::make_unique<Animation>([&] {
        return now;
    })));
    popup->setStyle(style);
    popup->setGeometry(menu.toNearestRect());
    scene.render(renderer);
    now = 50;
    scene.render(renderer);
    // The snapshot is drawn unscaled: its 64 px overflow margin stays 64 px.
    const auto drawn = [&](const base::Rect &geometry) {
        return geometry.adjusted(-64, -64, 64, 64);
    };
    EXPECT_EQ(renderer.calls.back().rect, drawn(menu.toNearestRect()));
    ASSERT_GT(renderer.calls.back().alpha, 0);
    ASSERT_LT(renderer.calls.back().alpha, 1);
    // The client moved the menu: the fade continues at the new place.
    const base::Rect moved(150, 120, 200, 300);
    popup->setGeometry(moved);
    now = 60;
    scene.render(renderer);
    EXPECT_EQ(renderer.calls.back().rect, drawn(moved));
    now = 100;
    scene.render(renderer);
    EXPECT_FLOAT_EQ(renderer.calls.back().alpha, 1);
    EXPECT_FALSE(popup->isRunning());

    // Closing fades out where the last frame was drawn and then retires.
    popup->close();
    scene.render(renderer);
    now = 140;
    scene.render(renderer);
    EXPECT_EQ(renderer.calls.back().rect, drawn(moved));
    EXPECT_LT(renderer.calls.back().alpha, 1);
    now = 200;
    scene.render(renderer);
    EXPECT_TRUE(scene.root()->children().empty());
}

TEST(WindowAnimation, StyleNoneShowsAndRemovesWithoutAnimating)
{
    int64_t now = 0;
    scene::Scene scene;
    SnapshotRenderer renderer;
    auto *node = static_cast<Animation *>(scene.root()->addChild(std::make_unique<Animation>([&] {
        return now;
    })));
    node->setStyle(Animation::Style::None);
    node->setGeometry(base::Rect(100, 100, 200, 300));
    scene.render(renderer);
    EXPECT_FALSE(node->isRunning());
    EXPECT_FLOAT_EQ(renderer.calls.back().alpha, 1);
    EXPECT_FALSE(node->paintsSubtree());

    node->close();
    EXPECT_TRUE(node->isRetired());
    renderer.calls.clear();
    scene.render(renderer);
    EXPECT_TRUE(std::none_of(renderer.calls.begin(), renderer.calls.end(), [](const auto &call) {
        return call.texture;
    }));
    EXPECT_TRUE(scene.root()->children().empty());
}

TEST(WindowAnimation, ReopeningALauncherReversesItsCloseWithoutStackingFades)
{
    int64_t now = 0;
    scene::Scene scene;
    SnapshotRenderer renderer;
    auto clock = [&] {
        return now;
    };
    const base::Rect screen(0, 0, 1000, 800);
    auto *closing =
            static_cast<Animation *>(scene.root()->addChild(std::make_unique<Animation>(clock)));
    closing->setStyle(Animation::Style::Launcher);
    closing->setGeometry(screen);
    closing->finish();
    scene.render(renderer);
    closing->close();
    scene.render(renderer);
    now = 120;
    scene.render(renderer);
    const auto presented = renderer.calls.back();
    ASSERT_GT(presented.alpha, 0);
    ASSERT_LT(presented.alpha, 1);

    auto *opening =
            static_cast<Animation *>(scene.root()->addChild(std::make_unique<Animation>(clock)));
    opening->setGeometry(screen);
    opening->setStyle(Animation::Style::Launcher);
    renderer.calls.clear();
    scene.render(renderer);
    ASSERT_EQ(scene.root()->children().size(), 1u);
    EXPECT_EQ(scene.root()->children().front(), opening);
    // Only the new launcher draws: the one it took over never overlaps it.
    EXPECT_EQ(std::count_if(renderer.calls.begin(), renderer.calls.end(),
                            [](const auto &call) {
                                return call.texture;
                            }),
              1);
    EXPECT_FLOAT_EQ(renderer.calls.back().alpha, presented.alpha);
    EXPECT_EQ(renderer.calls.back().rect, presented.rect);
    now += 360;
    scene.render(renderer);
    EXPECT_FLOAT_EQ(renderer.calls.back().alpha, 1);
}

TEST(WindowAnimation, ALauncherKeepsWhatLiesUnderItWhileItAnimates)
{
    int64_t now = 0;
    scene::Scene scene;
    SnapshotRenderer renderer;
    const base::Rect screen(0, 0, 1000, 800);
    scene.setSize(screen.size());
    scene::Node *surfaces =
            scene.root()->addChild(std::make_unique<scene::Node>("surfaces"));
    auto *desktop = static_cast<scene::ColorNode *>(surfaces->addChild(
            std::make_unique<scene::ColorNode>(render::Color {.2f, .3f, .4f, 1})));
    desktop->setGeometry(screen);
    auto *launcher = static_cast<Animation *>(surfaces->addChild(
            std::make_unique<Animation>([&] { return now; })));
    launcher->setStyle(Animation::Style::Launcher);
    launcher->setGeometry(screen);
    const auto fills = [&] {
        return countKind(renderer, "fill");
    };

    scene.render(renderer);
    EXPECT_EQ(fills(), 1);
    // Drawn from the texture while nothing under the launcher changes.
    for (now = 40; now < 200; now += 40) {
        renderer.calls.clear();
        scene.render(renderer);
        EXPECT_EQ(fills(), 0);
        EXPECT_GE(countKind(renderer, "texture"), 1);
    }
    // Opening, what changes under it waits: the launcher covers it by its end.
    auto *window = static_cast<scene::ColorNode *>(surfaces->addChild(
            std::make_unique<scene::ColorNode>(render::Color {1, 1, 1, 1})));
    window->setGeometry({100, 100, 200, 100});
    surfaces->reorderChildren({desktop, window, launcher});
    renderer.calls.clear();
    scene.render(renderer);
    EXPECT_EQ(fills(), 0);

    // Closing, it catches up, drawing only the part that changed.
    launcher->close();
    renderer.calls.clear();
    renderer.repaintRects.clear();
    scene.render(renderer);
    EXPECT_EQ(fills(), 2);
    EXPECT_TRUE(std::ranges::find(renderer.repaintRects, base::Rect(100, 100, 200, 100))
                != renderer.repaintRects.end());
    now += 40;
    renderer.calls.clear();
    scene.render(renderer);
    EXPECT_EQ(fills(), 0);

    // Once it is gone, the scene draws them again.
    now = 1000;
    scene.render(renderer);
    renderer.calls.clear();
    window->setColor({0, 0, 0, 1});
    scene.render(renderer);
    EXPECT_EQ(fills(), 2);
}
