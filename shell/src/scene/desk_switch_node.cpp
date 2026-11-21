// Copyright 2019 The Chromium Authors
// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ash/wm/desks/root_window_desk_switch_animator.cc
//           chromium/src/ash/wm/desks/desks_animations.cc (PerformHitTheWallAnimation)
// Revision: 8abbac30b9cf2fe976422c58c000c94685648b65

#include "scene/desk_switch_node.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iterator>
#include <utility>
#include <vector>

#include "scene/motion.h"
#include "base/geometry.h"

namespace Lingmo::Scene {

namespace {

constexpr render::Color kBlack {0, 0, 0, 1};

// The solver's ends are a hair off 0 and 1, which would round a frame a pixel
// off the screen's edge.
double tween(double x1, double y1, double x2, double y2, double t)
{
    if (t <= 0)
        return 0;
    if (t >= 1)
        return 1;
    return motion::cubicBezier(x1, y1, x2, y2, t);
}

// gfx::Tween::ACCEL_20_DECEL_100.
double accel20Decel100(double t)
{
    return tween(0.2, 0, 0, 1, t);
}

// gfx::Tween::ACCEL_20_DECEL_60, the curve a minimize flies on.
double accel20Decel60(double t)
{
    return tween(0.2, 0, 0.4, 1, t);
}

int scaled(int durationMs)
{
    return std::max(1, int(std::lround(durationMs * motion::durationScale())));
}

// Frames land on whole device pixels, so they stay as sharp as the desk
// underneath instead of shimmering between two pixels while they move.
double snapped(double offset, double scale)
{
    return scale > 0 ? std::round(offset * scale) / scale : offset;
}

} // namespace

DeskSwitchNode::DeskSwitchNode(std::function<int64_t()> clock)
    : Node("desk-switch")
    , m_now(clock ? std::move(clock) : [] {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
                       std::chrono::steady_clock::now().time_since_epoch())
                .count();
    })
{
}

double DeskSwitchNode::slotDistance(int width) const
{
    return m_type == Type::Quick ? width * kQuickAnimationTranslationDistanceRatio
                                 : width + kDesksSpacing;
}

void DeskSwitchNode::start(Type type, int desks, int startingDesk, int endingDesk, Frames frames,
                           int width)
{
    stop();
    m_type = type;
    m_desks = desks;
    m_startingDesk = startingDesk;
    m_endingDesk = endingDesk;
    m_width = std::max(1, width);
    for (auto &[output, texture] : frames) {
        if (!texture)
            continue;
        // What paintCoveredContent() draws is on black, opaque throughout.
        texture->contentOpaque = true;
        m_frames[output][startingDesk] = std::move(texture);
    }
    m_strip = {startingDesk};
    notifyChanged();
}

void DeskSwitchNode::startRemoval(int desks, int removedDesk, int endingDesk, Frames stillFrames,
                                  std::set<std::string> outputs, int width)
{
    stop();
    m_type = Type::Quick;
    m_desks = desks;
    m_startingDesk = removedDesk;
    m_endingDesk = endingDesk;
    m_width = std::max(1, width);
    for (const std::string &output : outputs)
        m_frames[output];
    for (auto &[output, texture] : stillFrames) {
        if (texture)
            m_stillFrames[output] = std::move(texture);
    }
    m_strip = {removedDesk};
    notifyChanged();
}

void DeskSwitchNode::takeEndingScreenshot(int endingDesk)
{
    m_endingDesk = endingDesk;
    addDesk(endingDesk);
    for (auto &[output, frames] : m_frames)
        frames.erase(endingDesk);
    m_captureDesk = endingDesk;
    notifyChanged();
}

void DeskSwitchNode::addDesk(int desk)
{
    if (m_strip.count(desk))
        return;
    const auto index = std::distance(m_strip.begin(), m_strip.lower_bound(desk));
    m_strip.insert(desk);
    // A frame put in on the left moves everything already shown one place
    // right; the strip shifts with it so what is on screen stays put.
    const double now = m_now();
    if (index <= std::lround(position(now)))
        shiftStrip(1, now);
}

void DeskSwitchNode::shiftStrip(double by, double now)
{
    m_position += by;
    m_fingers.shift(by);
    m_animation.from += by;
    m_animation.to += by;
    if (m_motion == Motion::Spring)
        m_spring.reset(now, m_spring.value(now) + by, m_spring.target() + by,
                       m_spring.velocity(now));
    if (m_coast)
        m_coast->reset(m_coastStart, m_coast->value(m_coastStart) + by, m_coast->target() + by,
                       m_coast->velocity(m_coastStart));
}

int DeskSwitchNode::slotOf(int desk) const
{
    const auto it = m_strip.find(desk);
    return it == m_strip.end() ? 0 : int(std::distance(m_strip.begin(), it));
}

double DeskSwitchNode::edgeInSlots() const
{
    return std::round(m_width * kEdgePaddingRatio) / slotDistance(m_width);
}

double DeskSwitchNode::rubberBand(double pulled) const
{
    const double last = double(m_strip.size()) - 1;
    const double edge = edgeInSlots();
    const auto give = [edge](double beyond) {
        return edge * (1 - 1 / (1 + beyond * kRubberBandCoefficient / edge));
    };
    if (edge <= 0)
        return std::clamp(pulled, 0.0, std::max(0.0, last));
    if (pulled < 0)
        return -give(-pulled);
    if (pulled > last)
        return last + give(pulled - last);
    return pulled;
}

double DeskSwitchNode::pulledPosition(double shown) const
{
    const double last = double(m_strip.size()) - 1;
    const double edge = edgeInSlots();
    const auto pull = [edge](double beyond) {
        const double given = std::min(beyond, edge * 0.999);
        return edge / kRubberBandCoefficient * (1 / (1 - given / edge) - 1);
    };
    if (edge <= 0)
        return shown;
    if (shown < 0)
        return -pull(-shown);
    if (shown > last)
        return last + pull(shown - last);
    return shown;
}

double DeskSwitchNode::fingerPosition(double timeMs) const
{
    return m_fingers.empty() ? m_position : m_fingers.position(timeMs);
}

void DeskSwitchNode::settle(double now, double from, double target, double velocity,
                            double response)
{
    m_spring = motion::settlingSpring(now, from, target, velocity, response);
    m_motion = Motion::Spring;
    m_fingers.clear();
    m_coast.reset();
    m_animationStart = now;
    m_finishNotified = false;
    notifyChanged();
}

double DeskSwitchNode::position(double now) const
{
    switch (m_motion) {
    case Motion::Still:
        return m_position;
    case Motion::Tracking:
        if (m_coast && now > m_coastStart)
            return m_coast->value(now);
        return rubberBand(fingerPosition(now - kResampleLatencyMs));
    case Motion::Tween: {
        const double t = std::clamp((now - m_animationStart) / m_animation.durationMs, 0.0, 1.0);
        const double eased = m_type == Type::Glide ? accel20Decel60(t) : accel20Decel100(t);
        return m_animation.from + (m_animation.to - m_animation.from) * eased;
    }
    case Motion::Spring:
        return m_spring.value(now);
    }
    return m_position;
}

bool DeskSwitchNode::hasSettled(double now, double distance, double scale) const
{
    if (m_motion == Motion::Tween)
        return now - m_animationStart >= m_animation.durationMs;
    if (m_motion != Motion::Spring)
        return false;
    // Within half a device pixel, and slower than a pixel a frame: the next
    // frame would be this one.
    const double pixels = distance * std::max(scale, 1.0);
    return std::abs(m_spring.value(now) - m_spring.target()) * pixels < 0.5
            && std::abs(m_spring.velocity(now)) * pixels < 60;
}

void DeskSwitchNode::startAnimation()
{
    const double now = m_now();
    const double from = position(now);
    const double to = slotOf(m_endingDesk);
    if (m_type == Type::Continuous) {
        settle(now, from, to, m_motion == Motion::Spring ? m_spring.velocity(now) : 0,
               kSwitchResponse);
        return;
    }
    m_animation = {from, to,
                   scaled(m_type == Type::Glide ? kGlideDurationMs : kQuickAnimationDurationMs)};
    m_motion = Motion::Tween;
    m_position = from;
    m_animationStart = now;
    m_finishNotified = false;
    notifyChanged();
}

bool DeskSwitchNode::replaceAnimation(int endingDesk)
{
    m_startingDesk = m_endingDesk;
    m_endingDesk = endingDesk;
    if (!m_strip.count(endingDesk))
        return true;
    return std::any_of(m_frames.begin(), m_frames.end(), [endingDesk](const auto &entry) {
        return !entry.second.count(endingDesk);
    });
}

std::optional<int> DeskSwitchNode::updateSwipeAnimation(double deskDelta, double timeMs)
{
    // The fingers catch the strip where it is shown: sliding on its own, or
    // coasting since they paused rather than lifted.
    const bool coasted = m_motion == Motion::Tracking && m_coast
            && timeMs > m_coastStart - kResampleLatencyMs + kResampleMaxPredictionMs;
    if (m_motion != Motion::Tracking || coasted) {
        m_position = pulledPosition(position(m_now()));
        m_motion = Motion::Tracking;
        m_fingers.clear();
        m_coast.reset();
        m_animationStart = -1;
        m_finishNotified = false;
    }
    m_position -= deskDelta;
    m_fingers.add(timeMs, m_position);
    updateCoast();

    const int last = int(m_strip.size()) - 1;
    m_reachedEdge = m_position < 0 || m_position > last;
    notifyChanged();

    // Close to the end of the strip, the desk beyond it needs a frame; one at
    // a time. Without a move there is no way to tell which end.
    if (deskDelta == 0 || !endingScreenshotTaken())
        return std::nullopt;
    const bool movingLeft = deskDelta < 0;
    const double threshold = kMinDistanceBeforeScreenshot / slotDistance(m_width);
    const bool goingOutOfBounds =
            movingLeft ? last - m_position < threshold : m_position < threshold;
    if (!goingOutOfBounds)
        return std::nullopt;

    const int shownIndex = std::clamp(int(std::lround(m_position)), 0, last);
    const int desk = *std::next(m_strip.begin(), shownIndex) + (movingLeft ? 1 : -1);
    if (desk < 0 || desk >= m_desks || m_strip.count(desk))
        return std::nullopt;
    return desk;
}

int DeskSwitchNode::flickTarget(double position, double velocity) const
{
    double target = std::round(position);
    if (velocity >= kFlickVelocity)
        target = std::floor(position) + 1;
    else if (velocity <= -kFlickVelocity)
        target = std::ceil(position) - 1;
    return std::clamp(int(target), 0, int(m_strip.size()) - 1);
}

void DeskSwitchNode::updateCoast()
{
    m_coast.reset();
    // Fingers that lift leave the pad at speed; ones that stop to hold slow
    // down first. The fit over 100 ms lags a stop, so the last events decide.
    const std::optional<double> velocity = m_fingers.recentVelocity();
    if (!velocity || std::abs(*velocity) < kFlickVelocity)
        return;
    // It takes over where the resampled fingers stop, past the newest event.
    const double from = m_fingers.predictionEnd();
    const double position = fingerPosition(from);
    // Pulled past an end, the fingers hold it until they let go.
    if (position < 0 || position > double(m_strip.size()) - 1)
        return;
    m_coastStart = from + kResampleLatencyMs;
    m_coast = motion::settlingSpring(m_coastStart, position, flickTarget(position, *velocity),
                                     *velocity, kSettleResponse);
}

int DeskSwitchNode::endSwipeAnimation(double timeMs)
{
    if (m_strip.empty())
        return m_endingDesk;
    const double now = m_now();
    if (m_motion == Motion::Tracking && m_coast) {
        // Already on its way since the fingers stopped reporting.
        m_spring = *m_coast;
        m_motion = Motion::Spring;
        m_fingers.clear();
        m_coast.reset();
        m_animationStart = m_coastStart;
        m_finishNotified = false;
        notifyChanged();
    } else {
        double velocity = m_motion == Motion::Tracking ? m_fingers.releaseVelocity(timeMs) : 0;
        const double current = position(now);
        const int last = int(m_strip.size()) - 1;
        // Flung from beyond an end, it comes straight back.
        if ((current < 0 && velocity < 0) || (current > last && velocity > 0))
            velocity = 0;
        settle(now, current, flickTarget(current, velocity), velocity, kSettleResponse);
    }

    int slot = int(std::lround(m_spring.target()));
    if (!endingScreenshotTaken() && *std::next(m_strip.begin(), slot) != m_captureDesk) {
        // The frame being copied will not be shown after all.
        const int pending = std::exchange(m_captureDesk, -1);
        for (auto &[output, frames] : m_frames)
            frames.erase(pending);
        const auto index = std::distance(m_strip.begin(), m_strip.find(pending));
        m_strip.erase(pending);
        if (index < slot) {
            shiftStrip(-1, now);
            slot -= 1;
        }
    }
    // A frame still being copied shows once it is taken, the strip already
    // on its way.
    m_endingDesk = *std::next(m_strip.begin(), slot);
    return m_endingDesk;
}

int DeskSwitchNode::mostVisibleDesk() const
{
    if (m_strip.empty())
        return m_endingDesk;
    const int last = int(m_strip.size()) - 1;
    const int index = std::clamp(int(std::lround(position(m_now()))), 0, last);
    return *std::next(m_strip.begin(), index);
}

void DeskSwitchNode::hitTheWall(bool goingLeft, Frames frames, int width)
{
    stop();
    m_width = std::max(1, width);
    for (auto &[output, texture] : frames) {
        if (!texture)
            continue;
        texture->contentOpaque = true;
        m_frames[output][0] = std::move(texture);
    }
    if (m_frames.empty())
        return;
    m_hitTheWall = goingLeft;
    m_animationStart = m_now();
    m_finishNotified = false;
    notifyChanged();
}

void DeskSwitchNode::stop()
{
    const bool wasActive = isActive();
    m_frames.clear();
    m_stillFrames.clear();
    m_strip.clear();
    m_captureDesk = -1;
    m_motion = Motion::Still;
    m_position = 0;
    m_fingers.clear();
    m_coast.reset();
    m_animation = {};
    m_animationStart = -1;
    m_finishNotified = false;
    m_reachedEdge = false;
    m_hitTheWall.reset();
    if (wasActive)
        notifyChanged();
}

void DeskSwitchNode::paintCoveredContent(render::Renderer &renderer) const
{
    renderer.clear(kBlack);
    if (!parent())
        return;
    const auto &siblings = parent()->children();
    const auto end = std::find(siblings.begin(), siblings.end(), this);
    // Nothing under a sibling that covers the whole output shows, so the
    // frame starts there: at the overview, while it is up.
    auto first = siblings.begin();
    for (auto it = siblings.begin(); it != end; ++it) {
        if ((*it)->isVisible() && (*it)->occludesOutput(renderer))
            first = it;
    }
    for (auto it = first; it != end; ++it)
        (*it)->render(renderer);
}

void DeskSwitchNode::takeFrame(render::Renderer &renderer, const base::Rect &viewport) const
{
    if (m_captureDesk < 0 || renderer.capturing)
        return;
    const auto output = m_frames.find(renderer.outputContext().outputId);
    if (output == m_frames.end() || output->second.count(m_captureDesk))
        return;
    std::unique_ptr<render::Texture> texture;
    if (!renderer.renderToTexture(texture, viewport, [&] {
            paintCoveredContent(renderer);
        }))
        texture.reset();
    else
        texture->contentOpaque = true;
    // A frame that could not be copied shows black rather than holding the rest up.
    output->second[m_captureDesk] = std::move(texture);

    const int desk = m_captureDesk;
    if (std::all_of(m_frames.begin(), m_frames.end(), [desk](const auto &entry) {
            return entry.second.count(desk) > 0;
        })) {
        m_captureDesk = -1;
        if (m_endingScreenshotTaken)
            m_endingScreenshotTaken();
    }
}

void DeskSwitchNode::renderStrip(render::Renderer &renderer, const base::Rect &viewport,
                                 double now) const
{
    const auto output = m_frames.find(renderer.outputContext().outputId);
    if (output == m_frames.end())
        return;

    const double distance = slotDistance(viewport.width());
    const double scale = renderer.outputContext().scale;
    const double current = position(now);
    float endingAlpha = 1;
    if (m_type == Type::Quick) {
        // The ending desk's frame starts transparent and fades in as the strip moves.
        endingAlpha = 0;
        if (m_motion == Motion::Tween) {
            const double t = std::clamp((now - m_animationStart)
                                                / scaled(kQuickFadeInAnimationDurationMs),
                                        0.0, 1.0);
            endingAlpha = float(accel20Decel100(t));
        }
    }

    struct Draw {
        render::Texture *frame = nullptr;
        base::RectF target;
        float alpha = 1;
    };
    std::vector<Draw> draws;
    auto place = [&](int desk, int slot) {
        const auto frame = output->second.find(desk);
        if (frame == output->second.end() || !frame->second)
            return;
        const base::RectF target(viewport.x() + snapped((slot - current) * distance, scale),
                            viewport.y(), viewport.width(), viewport.height());
        if (!target.intersects(viewport))
            return;
        const bool ending = m_type == Type::Quick && desk == m_endingDesk && desk != m_startingDesk;
        draws.push_back({frame->second.get(), target, ending ? endingAlpha : 1});
    };
    int slot = 0;
    int endingSlot = -1;
    for (int desk : m_strip) {
        if (desk == m_endingDesk)
            endingSlot = slot;
        else
            place(desk, slot);
        ++slot;
    }
    // The ending desk goes over the others, as its layer is added last upstream.
    if (endingSlot >= 0)
        place(m_endingDesk, endingSlot);

    // Black only where no frame covers, between the desks and past the ends:
    // a full-screen fill under full-screen frames is most of a frame's GPU time.
    std::vector<std::pair<double, double>> covered;
    for (const Draw &draw : draws) {
        if (draw.alpha >= 1)
            covered.push_back({draw.target.left(), draw.target.right()});
    }
    std::sort(covered.begin(), covered.end());
    double x = viewport.left();
    for (const auto &[left, right] : covered) {
        if (left > x)
            renderer.fillRectF(base::RectF(x, viewport.y(), left - x, viewport.height()), kBlack);
        x = std::max(x, right);
    }
    if (x < viewport.x() + viewport.width())
        renderer.fillRectF(base::RectF(x, viewport.y(), viewport.x() + viewport.width() - x,
                                  viewport.height()),
                           kBlack);

    for (const Draw &draw : draws)
        renderer.drawTextureF(draw.frame, draw.target, draw.alpha);

    const auto still = m_stillFrames.find(renderer.outputContext().outputId);
    if (still != m_stillFrames.end())
        renderer.drawTexture(still->second.get(), viewport);
}

void DeskSwitchNode::renderHitTheWall(render::Renderer &renderer, const base::Rect &viewport,
                                      double now) const
{
    const auto output = m_frames.find(renderer.outputContext().outputId);
    if (output == m_frames.end() || !output->second.count(0))
        return;
    // Out towards the edge with EASE_OUT, back with EASE_IN.
    const double half = scaled(kHitTheWallDurationMs);
    const double t = std::clamp((now - m_animationStart) / half, 0.0, 2.0);
    const double amount = t < 1 ? 1 - std::pow(1 - t, 2) : 1 - std::pow(t - 1, 2);
    const double displacement =
            kEdgePaddingRatio * viewport.width() * (*m_hitTheWall ? 1 : -1) * amount;
    const double offset = snapped(displacement, renderer.outputContext().scale);
    const auto &frame = output->second.at(0);
    if (!frame || offset != 0) {
        const base::RectF uncovered = !frame ? base::RectF(viewport)
                : offset > 0 ? base::RectF(viewport.x(), viewport.y(), offset, viewport.height())
                             : base::RectF(viewport.x() + viewport.width() + offset, viewport.y(),
                                      -offset, viewport.height());
        renderer.fillRectF(uncovered, kBlack);
    }
    if (frame) {
        renderer.drawTextureF(frame.get(),
                              base::RectF(viewport.x() + offset, viewport.y(), viewport.width(),
                                     viewport.height()));
    }
}

void DeskSwitchNode::render(render::Renderer &renderer) const
{
    if (!isActive() || !isVisible())
        return;
    // Motion steps on the frame's slot on the display, not on when it was
    // drawn, the way WindowAnimation does.
    const double now = renderer.frameTime >= 0 ? renderer.frameTime : double(m_now());
    const auto &context = renderer.outputContext();
    const base::Rect viewport =
            context.logicalViewport.isEmpty() ? absoluteGeometry() : context.logicalViewport;

    if (m_hitTheWall) {
        renderHitTheWall(renderer, viewport, now);
        if (renderer.capturing)
            return;
        if (now - m_animationStart >= 2 * scaled(kHitTheWallDurationMs)) {
            if (!std::exchange(m_finishNotified, true) && m_finished)
                m_finished();
        } else {
            notifyChanged();
        }
        return;
    }

    takeFrame(renderer, viewport);
    renderStrip(renderer, viewport, now);
    if (renderer.capturing)
        return;
    if (m_motion == Motion::Tracking) {
        // Until the resampled position catches up with the newest event, or
        // for as long as the strip coasts.
        const double pixels = slotDistance(viewport.width()) * std::max(context.scale, 1.0);
        const bool coasting = m_coast
                && (std::abs(m_coast->value(now) - m_coast->target()) * pixels >= 0.5
                    || std::abs(m_coast->velocity(now)) * pixels >= 60);
        if (coasting || m_fingers.isCatchingUp(now))
            notifyChanged();
        return;
    }
    if (!isAnimating())
        return;
    if (hasSettled(now, slotDistance(viewport.width()), context.scale)) {
        m_finishNotified = true;
        if (m_finished)
            m_finished();
    } else {
        notifyChanged();
    }
}

bool DeskSwitchNode::occludesOutput(const render::Renderer &renderer) const
{
    const auto output = m_frames.find(renderer.outputContext().outputId);
    if (output == m_frames.end())
        return false;
    return !m_hitTheWall || output->second.count(0);
}

void DeskSwitchNode::releaseRenderResources()
{
    m_frames.clear();
    m_stillFrames.clear();
    Node::releaseRenderResources();
}

} // namespace Lingmo::Scene
