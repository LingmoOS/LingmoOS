// Copyright 2019 The Chromium Authors
// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ash/wm/desks/root_window_desk_switch_animator.cc
//           chromium/src/ash/wm/desks/desks_animations.cc (PerformHitTheWallAnimation)
// Revision: 8abbac30b9cf2fe976422c58c000c94685648b65

#ifndef LINGMO_SCENE_DESK_SWITCH_NODE_H_
#define LINGMO_SCENE_DESK_SWITCH_NODE_H_

#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <set>

#include <string>

#include "scene/finger_track.h"
#include "scene/motion.h"
#include "scene/node.h"
#include "base/geometry.h"

namespace Lingmo::Scene {

// RootWindowDeskSwitchAnimator for every output at once. Upstream screenshots
// the whole root; here only what lies below this node in its parent - the
// desktop and the desks' windows - slides, while the panels and the dock stay
// put above it.
//
// The phases are upstream's:
// (1) start() gets the starting desk's frame for each output, taken before the
//     ending desk is activated, and covers the desk content with it.
// (2) Once the ending desk is active underneath, takeEndingScreenshot() has each
//     output copy the content at its next frame; the ending-screenshot callback
//     runs when every output has.
// (3) startAnimation() slides the strip of desk frames towards the ending desk;
//     the finished callback runs once it lands.
//
// Positions are in desk frames: the frame at index k of the strip is fully on
// screen at position k. Upstream keeps a translation per root instead, whose
// touchpad conversion scales with the root's width, which comes to the same.
//
// Unlike upstream, a swipe moves the strip with the fingers, resampled at each
// frame's time as ui::LinearResampling does, pulls against the strip's ends
// like a rubber band, and lets go into a spring that keeps the fingers' speed;
// keyboard switches ride the same spring. Upstream's fixed-length tweens left
// the desk behind the fingers and cut short or stretched every release.
class DeskSwitchNode : public Node
{
public:
    enum class Type {
        // Clicks and other one-off switches: the ending desk moves a quarter of
        // the screen and fades in over the starting one.
        Quick,
        // Keyboard and touchpad: the desks sit side by side, a black gap apart.
        Continuous,
        // A click on a desk: side by side as Continuous, gliding there over a
        // fixed time on the curve a minimize flies.
        Glide,
    };

    // Keyed by OutputRenderContext::outputId.
    using Frames = std::map<std::string, std::unique_ptr<render::Texture>>;

    static constexpr int kDesksSpacing = 50;
    static constexpr double kEdgePaddingRatio = 0.1;
    static constexpr int kMinDistanceBeforeScreenshot = 40;
    static constexpr int kQuickAnimationDurationMs = 150;
    static constexpr int kQuickFadeInAnimationDurationMs = 100;
    static constexpr int kGlideDurationMs = 320;
    // Critically damped springs, response in seconds: a keyboard switch from
    // rest, and the strip settling once the fingers let go.
    static constexpr double kSwitchResponse = 0.5;
    static constexpr double kSettleResponse = 0.4;
    // Released faster than this, in desks per second, the strip goes on to the
    // next desk that way; slower, it settles on the nearest.
    static constexpr double kFlickVelocity = 0.5;
    // UIScrollView's rubber band: how much of the fingers' pull past an end the
    // strip follows at first, never going further than the edge padding.
    static constexpr double kRubberBandCoefficient = 0.55;
    static constexpr double kResampleLatencyMs = motion::FingerTrack::kResampleLatencyMs;
    static constexpr double kResampleMaxPredictionMs =
            motion::FingerTrack::kResampleMaxPredictionMs;
    static constexpr double kQuickAnimationTranslationDistanceRatio = 0.25;
    static constexpr int kHitTheWallDurationMs = 150;

    explicit DeskSwitchNode(std::function<int64_t()> clock = {});

    // Phase (1). `frames` holds each output's starting desk frame; outputs with
    // none take no part. `desks` is how many desks there are, and `width` the
    // output width the edges and the touchpad are measured against.
    void start(Type type, int desks, int startingDesk, int endingDesk, Frames frames, int width);

    // Phase (1) of removing the active desk, upstream's `for_remove`: in place of
    // the removed desk's frame the strip shows black, and `stillFrames` - its
    // windows as they were - stay on top, unmoved, until the removal is done.
    void startRemoval(int desks, int removedDesk, int endingDesk, Frames stillFrames,
                      std::set<std::string> outputs, int width);

    // Phase (2): the ending desk is active, copy it.
    void takeEndingScreenshot(int endingDesk);
    bool endingScreenshotTaken() const { return m_captureDesk < 0; }

    // Phase (3): slides from wherever the strip is to the ending desk, keeping
    // the speed of a slide already under way.
    void startAnimation();

    // Upstream ReplaceAnimation: the ending desk becomes `endingDesk`. True when
    // it has no frame yet, so the caller has to activate it and take one.
    bool replaceAnimation(int endingDesk);

    // Moves the strip with the fingers, `deskDelta` desks to the right (the
    // desk on the left coming in) at `timeMs` on the steady clock, taking it
    // over from wherever it was sliding. Returns the desk to activate and copy
    // when the strip nears its end and another desk lies beyond it.
    std::optional<int> updateSwipeAnimation(double deskDelta, double timeMs);

    // The fingers left at `timeMs`: flung, the strip goes on to the next desk
    // that way, otherwise back to the nearest. Returns that desk.
    int endSwipeAnimation(double timeMs);

    int mostVisibleDesk() const;
    bool reachedEdge() const { return m_reachedEdge; }
    int endingDesk() const { return m_endingDesk; }

    // PerformHitTheWallAnimation: the desk content nudges towards the edge it hit
    // and back.
    void hitTheWall(bool goingLeft, Frames frames, int width);

    // Drops every frame; the live content shows again.
    void stop();

    bool isActive() const { return !m_frames.empty() || !m_stillFrames.empty(); }
    bool isTracking() const { return m_motion == Motion::Tracking; }
    bool isAnimating() const
    {
        return (m_motion == Motion::Tween || m_motion == Motion::Spring) && !m_finishNotified;
    }

    // Runs from inside render(): must not change the scene.
    void setEndingScreenshotCallback(std::function<void()> callback)
    {
        m_endingScreenshotTaken = std::move(callback);
    }
    void setFinishedCallback(std::function<void()> callback) { m_finished = std::move(callback); }

    // What this node covers: everything below it in its parent, on black.
    void paintCoveredContent(render::Renderer &renderer) const;

    void render(render::Renderer &renderer) const override;
    Content content() const override { return isActive() ? Content::Opaque : Content::None; }
    bool paintsSubtree() const override { return isActive(); }
    bool acceptsInput(const base::PointF &) const override { return false; }
    // Every output the animation started on is covered in black and frames;
    // the live desk content below need not be drawn there.
    bool occludesOutput(const render::Renderer &renderer) const override;
    void releaseRenderResources() override;

    // The strip's layout on an output `width` wide.
    double slotDistance(int width) const;

private:
    enum class Motion {
        Still,
        // With the fingers: m_position is where they have pulled the strip,
        // past its ends too, and m_fingers where it was at each event.
        Tracking,
        Tween,
        Spring,
    };

    struct Animation {
        double from = 0;
        double to = 0;
        int durationMs = 0;
    };

    int slotOf(int desk) const;
    void addDesk(int desk);
    // Moves every position the strip keeps by `by` slots, as a frame comes in
    // or goes on its left.
    void shiftStrip(double by, double now);
    double edgeInSlots() const;
    double rubberBand(double pulled) const;
    double pulledPosition(double shown) const;
    double fingerPosition(double timeMs) const;
    // The slot a strip let go at `position` and `velocity` settles on.
    int flickTarget(double position, double velocity) const;
    void updateCoast();
    void settle(double now, double from, double target, double velocity, double response);
    double position(double now) const;
    bool hasSettled(double now, double distance, double scale) const;
    void takeFrame(render::Renderer &renderer, const base::Rect &viewport) const;
    void renderStrip(render::Renderer &renderer, const base::Rect &viewport, double now) const;
    void renderHitTheWall(render::Renderer &renderer, const base::Rect &viewport, double now) const;

    std::function<int64_t()> m_now;
    Type m_type = Type::Continuous;
    int m_desks = 0;
    int m_startingDesk = -1;
    int m_endingDesk = -1;
    int m_width = 0;
    // The desks with a place in the strip, in desk order.
    std::set<int> m_strip;
    mutable std::map<std::string, std::map<int, std::unique_ptr<render::Texture>>> m_frames;
    std::map<std::string, std::unique_ptr<render::Texture>> m_stillFrames;
    mutable int m_captureDesk = -1;
    Motion m_motion = Motion::Still;
    double m_position = 0;
    motion::FingerTrack m_fingers;
    // Fingers moving at a flick's speed that stop reporting have lifted, tens
    // of milliseconds before libinput ends the gesture: from m_coastStart the
    // strip settles on this spring rather than standing still meanwhile.
    std::optional<motion::Spring> m_coast;
    double m_coastStart = 0;
    Animation m_animation;
    motion::Spring m_spring;
    mutable double m_animationStart = -1;
    mutable bool m_finishNotified = false;
    bool m_reachedEdge = false;
    std::optional<bool> m_hitTheWall;
    std::function<void()> m_endingScreenshotTaken;
    std::function<void()> m_finished;
};

} // namespace Lingmo::Scene

#endif // LINGMO_SCENE_DESK_SWITCH_NODE_H_
