// Copyright 2013 The Chromium Authors
// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ash/wm/window_animations.cc
//           chromium/src/ui/wm/core/window_animations.cc
//           chromium/src/ui/gfx/animation/tween.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a

#include "scene/window_animation.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <functional>
#include <utility>


#include "scene/motion.h"
#include "scene/surface_node.h"
#include "base/region.h"
#include "base/transform.h"

namespace Lingmo::Scene {

namespace {

base::RectF shrink(const base::RectF &bounds, double scale)
{
    const base::SizeF size = bounds.size() * scale;
    return base::RectF(bounds.center() - base::PointF(size.width(), size.height()) / 2, size);
}

double smoothStep(double t)
{
    t = std::clamp(t, 0.0, 1.0);
    return t * t * t * (10 + t * (-15 + 6 * t));
}

double easeOut(double t, double power)
{
    return 1 - std::pow(1 - std::clamp(t, 0.0, 1.0), power);
}

// How much larger than the output a launcher is when fully transparent.
constexpr double kLauncherZoom = 1.05;

// gfx::Tween::ACCEL_20_DECEL_60: moving from the first frame, with no dwell
// at either end, so the motion fills the whole duration and lands softly.
double accel20Decel60(double t)
{
    return motion::cubicBezier(.2, 0, .4, 1, t);
}

// Windows open and close on critically damped springs: response in seconds.
// The fade leads the scale, so the first frames already show most of it.
constexpr double kOpenMotion = .36;
constexpr double kOpenFade = .12;
constexpr double kCloseMotion = .26;
constexpr double kCloseFade = .16;
constexpr int kOpenDurationMs = 360;
constexpr int kCloseDurationMs = 220;
// A preview growing into the window it stands for, on the ease-out of a window
// opening, without the spring's overshoot.
constexpr int kScaleDurationMs = 250;

// A spring's step response from rest `elapsed` ms in. Early on it runs in real
// time; its tail is warped to end at `duration` with no speed left, instead of
// spending frames on the last fraction of a pixel.
double springStep(double response, double elapsed, double duration)
{
    const double t = elapsed / duration;
    if (t <= 0)
        return 0;
    if (t >= 1)
        return 1;
    motion::Spring spring({response, 1});
    spring.reset(0, 0, 1);
    return std::clamp(spring.value(elapsed / (1 - t * t)), 0.0, 1.0);
}

base::Rect subtreeBounds(const Node *node)
{
    base::Rect bounds = node->absolutePaintBounds();
    for (const Node *child : node->children()) {
        if (child->isVisible())
            bounds = bounds.united(subtreeBounds(child));
    }
    return bounds;
}

// A surface on a hardware plane is left out of the texture for this frame only.
bool hasPromotedSurface(const Node &node)
{
    if (node.isPromotedToOverlay())
        return true;
    return std::any_of(node.children().begin(), node.children().end(), [](const Node *child) {
        return hasPromotedSurface(*child);
    });
}

// What SurfaceNode::paint records, for surfaces reaching the screen through a
// texture drawn from them earlier.
void recordPresentation(render::Renderer &renderer, const Node &node)
{
    if (!node.isVisible())
        return;
    if (const auto *surface = dynamic_cast<const SurfaceNode *>(&node);
        surface && surface->texture() && surface->alpha() > 0
        && !node.absoluteGeometry().isEmpty())
        renderer.recordPresentation(surface->presentationContent(), node.absoluteGeometry());
    for (const Node *child : node.children())
        recordPresentation(renderer, *child);
}

const Node &sceneRoot(const Node &node)
{
    const Node *root = &node;
    while (root->parent())
        root = root->parent();
    return *root;
}

// The viewport the scene's damage tracker is given for the output drawn now.
base::Rect outputViewport(const render::Renderer &renderer, const Node &node)
{
    const base::Rect viewport = renderer.outputContext().logicalViewport;
    return viewport.isEmpty() ? sceneRoot(node).geometry() : viewport;
}

} // namespace

WindowAnimation::WindowAnimation(std::function<double()> clock)
    : Node("window")
    , m_now(clock ? std::move(clock) : [] {
        return std::chrono::duration<double, std::milli>(
                       std::chrono::steady_clock::now().time_since_epoch())
                .count();
    })
{
}

int WindowAnimation::duration(Type type, const base::RectF &from, const base::RectF &to, Style style)
{
    if (style == Style::Launcher && type == Type::Open)
        return 360;
    if (style == Style::Launcher && type == Type::Close)
        return 160;
    // Menus open under the pointer and are read immediately: shorter than the
    // 150 ms ui/wm gives WINDOW_TYPE_MENU so the fade never reads as a delay.
    if (style == Style::Popup && type == Type::Open)
        return 100;
    if (style == Style::Popup && type == Type::Close)
        return 80;
    if (type == Type::Open)
        return base::roundToInt(kOpenDurationMs * motion::durationScale());
    if (type == Type::Close)
        return base::roundToInt(kCloseDurationMs * motion::durationScale());
    if (type == Type::Minimize)
        return 320;
    if (type == Type::Restore)
        return 320;
    // Ash's kCrossFadeDuration. A drag cross-fade is a flourish on top of a
    // gesture that is still running, so it is much shorter than a state change
    // the user only watches.
    if (type == Type::DragBounds)
        return 120;
    const double a = from.width() * from.height();
    const double b = to.width() * to.height();
    const double largest = std::max(a, b);
    return 200 + (largest > 0 ? base::roundToInt(200 * std::abs(a - b) / largest) : 0);
}

WindowAnimation::Sample WindowAnimation::sample(Type type, const base::RectF &from, const base::RectF &to,
                                                double progress, Style style)
{
    const double t = std::clamp(progress, 0.0, 1.0);
    if (style == Style::Launcher && (type == Type::Open || type == Type::Close)) {
        // Opening decelerates: the launcher responds on the first frame and
        // lands without a visible stop. Closing, it fades out at an even rate
        // (an eased fade lingers as a faint ghost) and is still zooming as it
        // vanishes, at 0.4 of its mean speed: a zoom that settles first leaves
        // a still image fading out.
        const double eased = type == Type::Open ? easeOut(t, 4) : t + .6 * t * (1 - t);
        const float opacity = type == Type::Open ? easeOut(t / .6, 3) : 1 - t;
        return {base::RectF(from.topLeft() + (to.topLeft() - from.topLeft()) * eased,
                       from.size() + (to.size() - from.size()) * eased),
                opacity, t};
    }
    if (style == Style::Popup && (type == Type::Open || type == Type::Close)) {
        // No motion: the popup is drawn where it is and only its opacity moves,
        // decelerating so the first frame already shows most of the change.
        const float opacity = type == Type::Open ? easeOut(t, 2) : 1 - easeOut(t, 2);
        return {to, opacity, t};
    }
    if (style == Style::Fade && (type == Type::Open || type == Type::Close)) {
        const float opacity = type == Type::Open ? smoothStep(t / .55) : 1 - smoothStep(t);
        return {to, opacity, t};
    }
    // Upstream passes gfx::Tween::LINEAR for the drag cross-fades. Bounds
    // changes land like ash's EASE_OUT, but ramp up over the first frames
    // instead of starting at full speed.
    double eased = type == Type::DragBounds ? t : accel20Decel60(t);
    float opacity = 1;
    // Minimize stays opaque: the window lands on the dock thumbnail that shows
    // the same image. Restore only fades in when it interrupts a fade.
    if (type == Type::Restore) {
        opacity = smoothStep(t / .3);
    } else if (type == Type::Open || type == Type::Close) {
        const double length = duration(type, from, to, style);
        const double elapsed = t * length;
        const bool open = type == Type::Open;
        eased = springStep(open ? kOpenMotion : kCloseMotion, elapsed, length);
        const double fade = springStep(open ? kOpenFade : kCloseFade, elapsed, length);
        opacity = open ? fade : 1 - fade;
    }
    return {base::RectF(from.topLeft() + (to.topLeft() - from.topLeft()) * eased,
                   from.size() + (to.size() - from.size()) * eased),
            opacity, t};
}

WindowAnimation::Sample WindowAnimation::current() const
{
    double length = duration(m_type, m_from, m_to, m_style);
    // A launcher close vanishes on a refresh: a fraction of a step left over
    // shows as one faint frame that lingers after the even steps before it.
    if (m_style == Style::Launcher && m_type == Type::Close && m_framePeriod > 0)
        length = std::max(1.0, std::round(length / m_framePeriod)) * m_framePeriod;
    const double t = (m_started >= 0) ? (now() - m_started) / length : 0;
    Sample result = sample(m_type, m_from, m_to, t, m_style);
    if (m_type == Type::Minimize && result.progress < 1) {
        // Replace each new endpoint's contribution with a smooth correction
        // over the remaining time, leaving the preceding trajectory intact.
        for (const auto &adjustment : m_minimizeAdjustments) {
            const double remaining = (result.progress - adjustment.progress)
                    / (1 - adjustment.progress);
            const double weight = smoothStep(remaining) - accel20Decel60(result.progress);
            result.bounds.translate(adjustment.position * weight);
            result.bounds.setSize(result.bounds.size() + adjustment.size * weight);
        }
    }
    if (m_type == Type::Close)
        result.opacity *= m_startOpacity;
    if (m_type == Type::Minimize)
        result.opacity = m_startOpacity + (1 - m_startOpacity) * smoothStep(result.progress / .6);
    if (m_type == Type::Restore || m_type == Type::Open)
        result.opacity = m_startOpacity + (1 - m_startOpacity) * result.opacity;
    if (isCrossFade(m_type))
        result.opacity = m_startOpacity + (1 - m_startOpacity) * smoothStep(result.progress);
    return result;
}

void WindowAnimation::finish()
{
    m_running = false;
    m_holding = false;
    m_firstFrame = false;
    m_boundsContentReady = true;
    m_oldTexture.reset();
    m_landingTexture.reset();
    m_snapshot = nullptr;
    m_landingSnapshot = nullptr;
    changed();
}

void WindowAnimation::skip(Type type)
{
    m_type = type;
    finish();
}

void WindowAnimation::changed() const
{
    notifyChanged();
    // A hidden node asks for no frame of its own, yet a minimize plays on.
    if (!isVisible() && parent())
        parent()->requestFrame();
}

void WindowAnimation::releaseRenderResources()
{
    finish();
    m_texture.reset();
    m_stashedTexture.reset();
    m_belowTextures.clear();
    m_textureReusable = false;
    Node::releaseRenderResources();
}

void WindowAnimation::start(Type type, const base::Rect &from, const base::Rect &to, bool requestFrame)
{
    if (m_style == Style::None) {
        skip(type);
        return;
    }
    const bool interrupted = m_running;
    const Sample previous = interrupted ? m_presented : Sample {from, 1, 0};
    m_from = interrupted ? previous.bounds : base::RectF(from);
    m_to = to;
    m_startOpacity =
            !interrupted && type == Type::Open ? 0 : previous.opacity;
    m_type = type;
    m_holding = false;
    m_minimizeAdjustments.clear();
    m_landingTexture.reset();
    m_snapshot = nullptr;
    m_landingSnapshot = nullptr;
    if (type == Type::Minimize) {
        m_dockWindow = from;
        m_dockSlot = to;
    } else if (type == Type::Restore) {
        m_dockWindow = to;
        m_dockSlot = from;
    }
    m_boundsContentReady = true;
    m_running = true;
    m_firstFrame = false;
    // Start on the next presented opportunity, so a delayed client/event loop
    // cannot consume an animation before its first frame is drawn.
    m_started = -1;
    const bool launcher = m_style == Style::Launcher;
    const bool inPlace = m_style == Style::Popup || m_style == Style::Fade;
    if (type == Type::Open && !interrupted)
        m_from = inPlace ? base::RectF(to) : shrink(to, launcher ? kLauncherZoom : .94);
    if (type == Type::Close)
        m_to = inPlace ? m_from : launcher ? shrink(from, kLauncherZoom) : shrink(m_from, .95);
    m_presented = {m_from, m_startOpacity, 0};
    if (type == Type::DragBounds && !base::RectF(to).isEmpty()) {
        m_dragOffset = base::RectF(from).topLeft() - base::RectF(to).topLeft();
        m_dragScale =
                base::SizeF(double(from.width()) / to.width(), double(from.height()) / to.height());
    }
    // Whatever m_texture holds after this, it was not drawn from the subtree's
    // current state.
    m_textureReusable = false;
    m_stashedTexture.reset();
    if (isCrossFade(type) && m_texture) {
        m_oldTexture = std::move(m_texture);
        m_oldCoverage = m_coverage;
        m_oldBounds = m_textureBounds;
        m_oldVisual = m_visual;
    }
    if (!isCrossFade(type)) {
        if (!m_texture && m_oldTexture) {
            m_texture = std::move(m_oldTexture);
            m_coverage = m_oldCoverage;
            m_textureBounds = m_oldBounds;
            m_visual = m_oldVisual;
        }
        m_oldTexture.reset();
    }
    if (requestFrame)
        changed();
}

void WindowAnimation::requestSnapshot(std::function<void(const ui::Bitmap &)> callback,
                                      int maximumPixels)
{
    m_snapshot = std::move(callback);
    m_landingSnapshot = m_snapshot;
    m_snapshotPixels = std::max(1, maximumPixels);
    changed();
}

void WindowAnimation::readSnapshot(render::Renderer &renderer, bool landing) const
{
    const auto callback = std::exchange(landing ? m_landingSnapshot : m_snapshot, nullptr);
    if (!callback || !m_texture || m_textureBounds.isEmpty())
        return;
    // A missing dock uses a one-pixel target; keep its useful preview for a later shell.
    if (landing && (m_to.width() <= 1 || m_to.height() <= 1))
        return;
    base::SizeF pixels = (landing ? m_to.size() : base::SizeF(m_textureBounds.size())) * renderer.rasterScale();
    if (pixels.width() > m_snapshotPixels || pixels.height() > m_snapshotPixels) {
        // Fitted inside the square, keeping the aspect ratio.
        const double width = m_snapshotPixels * pixels.width() / pixels.height();
        pixels = width <= m_snapshotPixels
                ? base::SizeF(width, m_snapshotPixels)
                : base::SizeF(m_snapshotPixels, m_snapshotPixels * pixels.height() / pixels.width());
    }
    const base::Size size(std::max(1, landing ? int(std::ceil(pixels.width())) : base::roundToInt(pixels.width())),
                     std::max(1, landing ? int(std::ceil(pixels.height())) : base::roundToInt(pixels.height())));
    const double sx = size.width() / double(m_textureBounds.width());
    const double sy = size.height() / double(m_textureBounds.height());
    // The shadow margins of the coverage land outside the image.
    const base::RectF target((m_coverage.x() - m_textureBounds.x()) * sx,
                        (m_coverage.y() - m_textureBounds.y()) * sy, m_coverage.width() * sx,
                        m_coverage.height() * sy);
    // Trilinear filtering, which the flight needs, blends in the next smaller
    // level and softens a still image; the dock shows this one texel per pixel.
    const double minification = std::min(m_texture->size().width() / target.width(),
                                          m_texture->size().height() / target.height());
    const int level = int(std::floor(std::log2(std::max(1.0, minification))));
    renderer.limitMipmapLevel(m_texture.get(), level);
    const ui::Bitmap image = renderer.renderToImage(size, [&] {
        renderer.pushRoundedClip(bodyShape(m_textureBounds, base::RectF(base::PointF(), size)));
        renderer.drawTextureF(m_texture.get(), target);
        renderer.popRoundedClip();
    });
    renderer.limitMipmapLevel(m_texture.get(), -1);
    if (!image.isNull()) {
        if (landing) {
            const ui::Bitmap rgba = image.converted(ui::Bitmap::Format::Rgba8888Premultiplied);
            m_landingTexture =
                    renderer.createTexture(rgba.size(), rgba.constBits(), int(rgba.stride()),
                                           render::PixelFormat::Rgba8888);
        }
        callback(image);
    }
}

void WindowAnimation::setMinimizeTarget(const base::Rect &target)
{
    if (m_type != Type::Minimize || (!m_running && !m_holding) || target.isEmpty()
        || base::fuzzyCompare(m_to, base::RectF(target)))
        return;
    if (m_running && m_started >= 0) {
        const double progress = current().progress;
        // Introduce each target change with zero velocity and acceleration,
        // preserving the flight already in progress and its landing deadline.
        if (progress < 1) {
            m_minimizeAdjustments.push_back({base::RectF(target).topLeft() - m_to.topLeft(),
                                            base::SizeF(target.size()) - m_to.size(), progress});
        }
    }
    m_to = target;
    m_dockSlot = target;
    changed();
}

void WindowAnimation::releaseHold()
{
    if (!m_holding)
        return;
    m_holding = false;
    // Nothing draws or reads the snapshots of a minimized window again: a
    // restore draws the live window, and the preview is the window's own image.
    m_texture.reset();
    m_stashedTexture.reset();
    m_oldTexture.reset();
    m_landingTexture.reset();
    m_textureReusable = false;
    changed();
}

float WindowAnimation::exteriorOpacity(const Sample &sample) const
{
    if (!isDockTransition() || isPresentedHidden())
        return 1;
    const double range = m_dockWindow.width() - m_dockSlot.width();
    if (range <= 0)
        return 1;
    return std::clamp((sample.bounds.width() - m_dockSlot.width()) / range, double(0), double(1));
}

render::RoundedShape WindowAnimation::bodyShape(const base::Rect &original, const base::RectF &bounds) const
{
    if (m_visual.shape.bounds.isEmpty())
        return render::RoundedShape {bounds};
    return transformedShape(m_visual.shape, original, Sample {bounds, 1, 1});
}

bool WindowAnimation::morphsExterior() const
{
    const auto plain = [](const WindowVisualState &visual) {
        return visual.mode != WindowVisualMode::PreserveClient && visual.outlinePath.isEmpty()
                && visual.outlineShape.bounds.isEmpty() && !visual.shape.bounds.isEmpty();
    };
    return m_running && m_type == Type::Bounds && m_oldTexture && plain(m_visual)
            && plain(m_oldVisual);
}

double WindowAnimation::morphedRadius(const Sample &sample) const
{
    const double from = m_oldVisual.shape.radius;
    return from + (m_visual.shape.radius - from) * accel20Decel60(sample.progress);
}

void WindowAnimation::drawExteriorMorph(render::Renderer &renderer, const Sample &value,
                                        float blend) const
{
    const float progress = accel20Decel60(value.progress);
    // Only a window in its normal state has a shadow and an outline.
    const bool oldExterior = m_oldVisual.drawOutline;
    const bool newExterior = m_visual.drawOutline;
    render::RoundedShape body = transformedShape(m_visual.shape, m_textureBounds, value);
    body.radius = morphedRadius(value);
    body.radiusScale = base::SizeF(1, 1);

    // The old shadow fades out under the window growing out of it; its corners
    // are never tighter than the body's, so none of its content shows.
    if (oldExterior && !newExterior)
        draw(renderer, m_oldTexture.get(), m_oldCoverage, m_oldBounds, value, 1 - progress);
    const float exterior = newExterior ? (oldExterior ? 1 : progress) : 0;
    if (exterior > 0)
        draw(renderer, m_texture.get(), m_coverage, m_textureBounds, value, exterior);
    renderer.pushRoundedClip(body);
    if (exterior < 1)
        draw(renderer, m_texture.get(), m_coverage, m_textureBounds, value);
    draw(renderer, m_oldTexture.get(), m_oldCoverage, m_oldBounds, value, 1 - blend);
    renderer.popRoundedClip();

    const float outline = (oldExterior ? 1 - progress : 0) + (newExterior ? progress : 0);
    if (outline > 0 && !renderer.suppressWindowOutlines)
        paintWindowOutline(renderer, body, newExterior ? m_visual : m_oldVisual,
                           outline * value.opacity);
}

void WindowAnimation::setDragTarget(const base::Rect &bounds)
{
    if (m_type != Type::DragBounds || !m_running || bounds.isEmpty()
        || base::fuzzyCompare(m_to, base::RectF(bounds)))
        return;
    m_to = bounds;
    m_from = base::RectF(
            m_to.topLeft() + m_dragOffset,
            base::SizeF(m_to.width() * m_dragScale.width(), m_to.height() * m_dragScale.height()));
    changed();
}

void WindowAnimation::setBoundsContentReady(bool ready)
{
    if (!isCrossFade(m_type) || m_boundsContentReady == ready)
        return;
    m_boundsContentReady = ready;
    changed();
}

void WindowAnimation::setStyle(Style style)
{
    m_style = style;
    if (style != Style::Launcher || !m_firstFrame || !parent())
        return;
    for (Node *sibling : parent()->children()) {
        auto *closing = dynamic_cast<WindowAnimation *>(sibling);
        if (!closing || closing == this || closing->m_style != Style::Launcher
            || !closing->m_closing || !closing->m_running || closing->m_textureBounds.isEmpty())
            continue;
        const base::RectF snapshot = closing->m_textureBounds;
        const base::RectF presented = closing->m_presented.bounds;
        m_handoff = Sample {base::RectF((presented.x() - snapshot.x()) / snapshot.width(),
                                   (presented.y() - snapshot.y()) / snapshot.height(),
                                   presented.width() / snapshot.width(),
                                   presented.height() / snapshot.height()),
                            closing->m_presented.opacity, 0};
        // Retired before either paints again, so no frame draws both.
        closing->m_running = false;
        closing->changed();
        return;
    }
}

void WindowAnimation::setOpacity(float opacity)
{
    opacity = std::clamp(opacity, 0.0f, 1.0f);
    if (m_opacity == opacity)
        return;
    m_opacity = opacity;
    changed();
}

void WindowAnimation::setPresentationScale(double scale, const base::PointF &anchor, bool animate)
{
    m_scaleFrom = animate ? presentationScale() : scale;
    m_scaleTo = scale;
    m_scaleStarted = -1;
    m_scaleAnchor = anchor;
    // Whether the snapshot has its blurs may change.
    m_textureReusable = false;
    changed();
}

double WindowAnimation::presentationScale() const
{
    if (m_scaleFrom == m_scaleTo || m_scaleStarted < 0)
        return m_scaleFrom;
    const double t = (now() - m_scaleStarted) / (kScaleDurationMs * motion::durationScale());
    return t >= 1 ? m_scaleTo : m_scaleFrom + (m_scaleTo - m_scaleFrom) * easeOut(t, 3);
}

void WindowAnimation::close()
{
    if (!isVisible() || (m_type == Type::Minimize && !m_running) || (!m_texture && !m_oldTexture)) {
        m_running = false;
        m_holding = false;
        m_closing = true;
        changed();
        return;
    }
    // The snapshot is scaled relative to where it was captured; the live
    // geometry may have changed since the last paint.
    base::Rect &bounds = m_texture ? m_textureBounds : m_oldBounds;
    base::Rect &coverage = m_texture ? m_coverage : m_oldCoverage;
    // Frames drawn directly leave the snapshot where it was captured, up to
    // kDirectRefresh frames back; a window dragged since closes where it was drawn.
    const base::RectF drawn = m_presented.bounds;
    if (!m_running && base::fuzzyCompare(drawn.size(), base::SizeF(bounds.size()))) {
        const base::Point moved = drawn.topLeft().toPoint() - bounds.topLeft();
        bounds.translate(moved);
        coverage.translate(moved);
    }
    const base::Rect snapshot = bounds;
    start(Type::Close, snapshot, snapshot);
    m_closing = true;
}

void WindowAnimation::draw(render::Renderer &renderer, render::Texture *texture,
                           const base::Rect &coverage, const base::Rect &original, const Sample &value,
                           float alpha) const
{
    if (!texture || original.isEmpty())
        return;
    const double sx = value.bounds.width() / original.width();
    const double sy = value.bounds.height() / original.height();
    const base::Point offset = coverage.topLeft() - original.topLeft();
    const base::RectF target(value.bounds.topLeft() + base::PointF(offset.x() * sx, offset.y() * sy),
                        base::SizeF(coverage.width() * sx, coverage.height() * sy));
    if (!renderer.drawsInto(target.toEnclosingRect()))
        return;
    renderer.drawTextureF(texture, target, value.opacity * alpha * renderer.opacityScale);
}

render::RoundedShape WindowAnimation::transformedShape(render::RoundedShape shape,
                                                       const base::Rect &original, const Sample &sample)
{
    if (original.isEmpty())
        return shape;
    const double sx = sample.bounds.width() / original.width();
    const double sy = sample.bounds.height() / original.height();
    shape.bounds =
            base::RectF(sample.bounds.topLeft() + base::PointF(shape.bounds.x() * sx, shape.bounds.y() * sy),
                   base::SizeF(shape.bounds.width() * sx, shape.bounds.height() * sy));
    shape.radiusScale = base::SizeF(sx, sy);
    return shape;
}

void WindowAnimation::outline(render::Renderer &renderer, const WindowVisualState &visual,
                              const base::Rect &original, const Sample &sample) const
{
    if (!visual.drawOutline || renderer.suppressWindowOutlines)
        return;
    // The snapshot holds the surface where it was snapped to physical pixels.
    const base::PointF snap = clientBodySnap(renderer, visual, original.topLeft());
    if (!visual.outlinePath.isEmpty() && !original.isEmpty()) {
        if (base::fuzzyCompare(sample.bounds.size(), base::SizeF(original.size()))) {
            // Rasterized where it lands, so it stays on physical pixels.
            auto placed = visual;
            const base::PointF origin = sample.bounds.topLeft() + snap;
            placed.outlinePath.translate(origin.x(), origin.y());
            paintShapeOutline(renderer, placed, base::AffineTransform(), sample.opacity);
            return;
        }
        base::AffineTransform transform;
        transform.translate(sample.bounds.x(), sample.bounds.y());
        transform.scale(sample.bounds.width() / original.width(),
                        sample.bounds.height() / original.height());
        transform.translate(snap.x(), snap.y());
        paintShapeOutline(renderer, visual, transform, sample.opacity);
        return;
    }
    auto outlined = visual.outlined();
    outlined.bounds.translate(snap);
    paintWindowOutline(renderer, transformedShape(outlined, original, sample), visual,
                       sample.opacity);
}

base::Rect WindowAnimation::absolutePaintBounds() const
{
    base::Rect bounds = absoluteGeometry();
    if (!paintsSubtree())
        return bounds;
    for (const auto *child : children()) {
        if (child->isVisible())
            bounds = bounds.united(subtreeBounds(child));
    }
    // A translucent window at rest blends its shadow too, and an opacity
    // change repaints it; the travel of a past animation is not drawn.
    if (!animating())
        return bounds.united(m_coverage);
    // Retired live nodes cannot describe the frozen shadow or its travel path.
    return bounds.united(m_coverage)
            .united(m_oldCoverage)
            .united(m_presented.bounds.toEnclosingRect())
            .united(m_from.toEnclosingRect())
            .united(m_to.toEnclosingRect());
}

base::Rect WindowAnimation::textureCoverage() const
{
    // Include outlying subsurfaces and the SSD shadow's paint overflow.
    base::Rect coverage = absoluteGeometry();
    for (const auto *child : children()) {
        if (child->isVisible())
            coverage = coverage.united(subtreeBounds(child));
    }
    return coverage.adjusted(-64, -64, 64, 64);
}

bool WindowAnimation::textureReusable(const base::Rect &coverage, const base::Rect &viewport,
                                      double scale) const
{
    if (!m_textureReusable || !m_texture || animating() || coverage.size() != m_coverage.size()
        || scale != m_textureScale || viewport != m_textureViewport
        || sceneRoot(*this).revision() != m_textureRootRevision || hasPromotedSurface(*this))
        return false;
    // A window that only moved keeps its texture, as viz keeps a render pass
    // whose content is unchanged (DirectRenderer::CanSkipRenderPass), when it
    // moved by whole device pixels: the content inside is rasterized at the
    // subpixel phase of the window's origin, and a fractional move changes it.
    const base::PointF moved = base::PointF(coverage.topLeft() - m_coverage.topLeft()) * scale;
    return base::fuzzyIsNull(moved.x() - std::round(moved.x()))
            && base::fuzzyIsNull(moved.y() - std::round(moved.y()));
}

bool WindowAnimation::redrawsSubtree(const base::Rect &viewport, double scale) const
{
    // Hidden, or drawing a snapshot, render() makes no texture pass at all.
    if (!isVisible() || m_closing || m_type == Type::Minimize)
        return false;
    const base::Rect coverage = textureCoverage();
    if (!animating() && !viewport.isEmpty() && !coverage.intersects(viewport))
        return false;
    if (textureReusable(coverage, viewport, scale))
        return false;
    // Drawn directly, its blurs draw on the output like any other. Only its own
    // content decides, as in render(): a blur's backdrop changing would draw
    // it to the texture again and pull the repaint wide.
    return !drawsDirectly(m_textureDamage.hasDamage(*this, coverage, scale));
}

bool WindowAnimation::settled() const
{
    return !animating() && m_type != Type::Minimize && m_opacity >= 1 && !presentedScaled()
            && !m_presentedAt;
}

bool WindowAnimation::drawsDirectly(bool contentChanged) const
{
    // A menu's highlight follows the pointer frame after frame and the menu
    // closes on the click that ends it: its close fades out the snapshot, which
    // has to be the frame that was on screen, not one up to kDirectRefresh back.
    if (m_style == Style::Popup)
        return false;
    const double period = m_framePeriod > 0 ? m_framePeriod : 1000.0 / 60;
    return contentChanged && m_contentChanged && m_directFrames < kDirectRefresh - 1
            && m_now() - m_textureDrawnAt < (kDirectRefresh - 0.5) * period && settled();
}

bool WindowAnimation::renderSiblingsBelow(render::Renderer &renderer, size_t begin,
                                          size_t end) const
{
    if (m_style != Style::Launcher || !animating() || (!isVisible() && !paintsWhenHidden())) {
        m_belowTextures.clear();
        return false;
    }
    // Captures draw everything live, and the overview draws blurs elsewhere.
    const base::Rect viewport = outputViewport(renderer, *this);
    if (renderer.capturing || renderer.suppressBackgroundBlur
        || !absolutePaintBounds().intersects(viewport))
        return false;
    // A launcher this one takes over leaves on its first frame.
    const auto &siblings = parent()->children();
    for (size_t i = begin; i < end; ++i) {
        const auto *animation = dynamic_cast<const WindowAnimation *>(siblings[i]);
        if ((animation && animation->style() == Style::Launcher)
            || hasPromotedSurface(*siblings[i]))
            return false;
    }
    const double scale = renderer.rasterScale();
    BelowTexture &below = m_belowTextures[renderer.outputContext().outputId];
    below.damage.setChildRange(begin, end);
    const bool reusable = below.texture && below.scale == scale && below.viewport == viewport
            && below.rootRevision == sceneRoot(*this).revision();
    // Opening, the launcher covers what changes under it by its end: focus
    // leaving the windows there would otherwise redraw their blurs each frame.
    // The history stays put, so a close from here catches up.
    if (reusable && m_type == Type::Open && !m_closing) {
        if (!below.damage.hasDamage(*parent(), viewport, scale)) {
            for (size_t i = begin; i < end; ++i)
                recordPresentation(renderer, *siblings[i]);
        }
        renderer.drawTexture(below.texture.get(), viewport);
        return true;
    }
    const base::Region damage = below.damage.update(*parent(), viewport, scale);
    if (reusable && damage.isEmpty()) {
        for (size_t i = begin; i < end; ++i)
            recordPresentation(renderer, *siblings[i]);
    } else {
        // The pass suspends presentation recording; what it draws shows this frame.
        render::PresentationSubmission *submission = renderer.presentationSubmission;
        const auto scope = uint64_t(reinterpret_cast<uintptr_t>(&below));
        const auto kept = [&](uint64_t owner) {
            return reusable && renderer.keepsBackdrop(scope, owner);
        };
        const base::Rect repaint = reusable
                ? render::coverSamples(damage.boundingRect(), below.damage.samples(kept))
                : base::Rect();
        const std::vector<uint64_t> fresh = below.damage.freshBackdrops(kept);
        if (!renderer.renderToTexture(below.texture, viewport, repaint, [&] {
                renderer.presentationSubmission = submission;
                const render::BackdropPass backdrops(renderer, scope, fresh);
                for (size_t i = begin; i < end; ++i)
                    siblings[i]->render(renderer);
            })) {
            m_belowTextures.clear();
            return false;
        }
        below.scale = scale;
        below.viewport = viewport;
        below.rootRevision = sceneRoot(*this).revision();
    }
    renderer.drawTexture(below.texture.get(), viewport);
    return true;
}

bool WindowAnimation::textureIsCurrent(render::Renderer &renderer, const base::Rect &coverage,
                                       base::Rect *repaint) const
{
    // A texture drawn without its blurs, or with them, is only kept for a draw
    // that wants the same.
    const bool blurSuppressed =
            renderer.suppressBackgroundBlur || presentedScaled() || m_presentedAt;
    // The blurs in the texture read the output behind the window, which may
    // have changed wherever this frame draws it again, and nowhere else. One
    // drawn without them reads nothing behind it: the overview's full repaints
    // would otherwise draw every blurred window's texture again each frame.
    const base::Region damage = blurSuppressed
            ? m_textureDamage.update(*this, coverage, renderer.rasterScale())
            : m_textureDamage.update(*this, coverage, renderer.rasterScale(), 0,
                                     [&](const base::Rect &sampled) {
                                         return renderer.repaints(sampled);
                                     });
    const bool valid =
            textureReusable(coverage, outputViewport(renderer, *this), renderer.rasterScale());
    *repaint = !valid ? base::Rect()
            : blurSuppressed
            ? damage.boundingRect()
            : render::coverSamples(damage.boundingRect(), m_textureDamage.samples());
    // Without blurs the two are the same texture: entering the overview does
    // not draw every window again.
    return valid && damage.isEmpty()
            && (m_textureBlurSuppressed == blurSuppressed || !hasBackgroundBlurs());
}

void WindowAnimation::swapStashedTexture(bool blurSuppressed) const
{
    const bool wanted = m_stashedTexture && m_stashedTexture->blurSuppressed == blurSuppressed;
    const bool other = m_texture && m_textureBlurSuppressed != blurSuppressed;
    if (!wanted && !(other && hasBackgroundBlurs()))
        return;
    if (!m_stashedTexture)
        m_stashedTexture = std::make_unique<StashedTexture>();
    StashedTexture &stash = *m_stashedTexture;
    std::swap(m_texture, stash.texture);
    std::swap(m_textureBlurSuppressed, stash.blurSuppressed);
    std::swap(m_textureMipmapped, stash.mipmapped);
    std::swap(m_textureDamage, stash.damage);
    std::swap(m_textureReusable, stash.reusable);
    std::swap(m_textureScale, stash.scale);
    std::swap(m_textureViewport, stash.viewport);
    std::swap(m_textureRootRevision, stash.rootRevision);
    std::swap(m_coverage, stash.coverage);
    std::swap(m_textureBounds, stash.bounds);
    if (!stash.texture)
        m_stashedTexture.reset();
}

bool WindowAnimation::hasBackgroundBlurs() const
{
    const std::function<bool(const Node &)> visit = [&](const Node &node) {
        for (const Node *child : node.children()) {
            const auto *surface = dynamic_cast<const SurfaceNode *>(child);
            if ((surface && surface->hasBackgroundBlur()) || visit(*child))
                return true;
        }
        return false;
    };
    return visit(*this);
}

void WindowAnimation::renderPresented(render::Renderer &renderer,
                                      const Presentation &presentation) const
{
    m_presentedAt = presentation;
    render(renderer);
    m_presentedAt.reset();
}

render::RoundedShape WindowAnimation::presentedBody(const Presentation &presentation) const
{
    return presentedBody(presentation, presentation.bounds);
}

render::RoundedShape WindowAnimation::presentedBody(const Presentation &presentation,
                                                    const base::RectF &bounds) const
{
    const base::Rect original = m_textureBounds.isEmpty() ? absoluteGeometry() : m_textureBounds;
    render::RoundedShape body = bodyShape(original, bounds);
    const double own = body.radius * std::min(body.radiusScale.width(), body.radiusScale.height());
    const double uniform = std::clamp(presentation.radius, double(0),
                                     std::min(body.bounds.width(), body.bounds.height()) / 2);
    body.radius = own + (uniform - own) * std::clamp(presentation.uniform, 0.0, 1.0);
    body.radiusScale = base::SizeF(1, 1);
    return body;
}

void WindowAnimation::drawPresented(render::Renderer &renderer, const Sample &value) const
{
    renderer.pushRoundedClip(presentedBody(*m_presentedAt, value.bounds));
    drawPresentedBlurs(renderer, value);
    draw(renderer, m_texture.get(), m_coverage, m_textureBounds, value);
    renderer.popRoundedClip();
    Sample outlined = value;
    outlined.opacity *= float(1 - std::clamp(m_presentedAt->uniform, 0.0, 1.0));
    if (outlined.opacity > 0)
        outline(renderer, m_visual, m_textureBounds, outlined);
}

void WindowAnimation::drawPresentedBlurs(render::Renderer &renderer, const Sample &value) const
{
    if (renderer.suppressBackgroundBlur || m_textureBounds.isEmpty())
        return;
    const double sx = value.bounds.width() / m_textureBounds.width();
    const double sy = value.bounds.height() / m_textureBounds.height();
    const double scale = std::min(sx, sy);
    base::AffineTransform transform;
    transform.translate(value.bounds.x(), value.bounds.y());
    transform.scale(sx, sy);
    transform.translate(-m_textureBounds.x(), -m_textureBounds.y());
    const bool reveal = isPresentedHidden();
    const std::function<void(const Node &)> visit = [&](const Node &node) {
        for (const Node *child : node.children()) {
            const auto *surface = dynamic_cast<const SurfaceNode *>(child);
            if (!child->isVisible() && !(reveal && surface && surface->isHiddenWithWindow()))
                continue;
            if (surface && surface->hasBackgroundBlur() && surface->texture()) {
                const base::Rect geometry = surface->absoluteGeometry();
                render::GlassParams glass = surface->glass();
                // Its lengths shrink with the window, or the lip would swallow it.
                glass.blurSigma *= float(scale);
                glass.bevel *= float(scale);
                glass.depth *= float(scale);
                glass.displacement *= float(scale);
                glass.ambientOcclusionRadius *= float(scale);
                glass.plainSigma *= float(scale);
                glass.opacity *= value.opacity;
                const ui::Path &shape = surface->backgroundBlurShape();
                renderer.drawBackgroundBlur(
                        transform.mapRect(base::RectF(surface->backgroundBlurRect().translated(
                                                  geometry.topLeft())))
                                .toEnclosingRect(),
                        base::roundToInt(surface->backgroundBlurRadius() * scale),
                        transform.mapRect(base::RectF(geometry)).toEnclosingRect(), glass,
                        shape.isEmpty() ? ui::Path()
                                        : shape.translated(geometry.x(), geometry.y()).transformed(transform));
            }
            visit(*child);
        }
    };
    visit(*this);
}

void WindowAnimation::render(render::Renderer &renderer) const
{
    if ((!isVisible() && m_type != Type::Minimize && !m_closing && !m_presentedAt) || isRetired())
        return;
    // Sampled per frame, not per draw: every output frame advances the
    // animation by what the display showed since the last one, however late
    // the wakeup or the uploads before this draw were.
    m_frameTime = renderer.frameTime;
    m_framePeriod = renderer.framePeriod;
    if (m_firstFrame) {
        auto *self = const_cast<WindowAnimation *>(this);
        const base::Rect geometry = absoluteGeometry();
        self->start(Type::Open, geometry, geometry, false);
        if (m_handoff) {
            const base::RectF &relative = m_handoff->bounds;
            self->m_from = base::RectF(geometry.x() + relative.x() * geometry.width(),
                                  geometry.y() + relative.y() * geometry.height(),
                                  relative.width() * geometry.width(),
                                  relative.height() * geometry.height());
            self->m_startOpacity = m_handoff->opacity;
            self->m_presented = {m_from, m_startOpacity, 0};
            self->m_handoff.reset();
        }
    }
    // A configure acknowledgement alone does not replace the old client buffer.
    // Keep its snapshot until the corresponding surface commit is applied.
    if (m_running && isCrossFade(m_type) && !m_boundsContentReady && m_oldTexture) {
        Sample held = m_presented;
        held.opacity *= m_opacity;
        draw(renderer, m_oldTexture.get(), m_oldCoverage, m_oldBounds, held);
        outline(renderer, m_oldVisual, m_oldBounds, held);
        return;
    }
    if (m_running && m_started < 0) {
        m_firstStepAt = now();
        // A window opening or closing already moves in its first frame: one
        // showing where it starts would only add a frame of latency.
        const bool leadIn =
                (m_style == Style::Window && (m_type == Type::Open || m_type == Type::Close))
                || (m_style == Style::Launcher && m_type == Type::Close);
        m_started = leadIn ? m_firstStepAt - renderer.framePeriod : m_firstStepAt;
        m_firstStep = true;
    } else if (m_running && std::exchange(m_firstStep, false) && renderer.framePeriod > 0
               && now() - m_firstStepAt > renderer.framePeriod) {
        // A slow first frame (snapshot pass, readback, buffer import) delays
        // the animation by the slots it missed instead of opening with a jump.
        // One drawn at once mid-refresh is sampled early, yet shows a refresh
        // before the next, so the second step is one refresh too.
        m_started += now() - m_firstStepAt - renderer.framePeriod;
    }

    // A minimized window's snapshot goes to the dock; presented, it is drawn live.
    const bool frozen = (m_closing || m_type == Type::Minimize) && !isPresentedHidden();
    if (frozen && !m_texture) {
        m_snapshot = nullptr;
        m_landingSnapshot = nullptr;
        m_holding = false;
        if (std::exchange(m_running, false) && m_finished)
            m_finished();
        return;
    }
    if (!frozen) {
        for (const Node *child : children()) {
            if (const auto *visual = dynamic_cast<const WindowVisualNode *>(child))
                m_visual = visual->state();
        }
        const base::Rect coverage = textureCoverage();
        // An idle window on another output is not drawn here, so its snapshot
        // keeps the scale of an output that shows it.
        const base::Rect viewport = renderer.outputContext().logicalViewport;
        if (!animating() && !m_presentedAt && !viewport.isEmpty() && !coverage.intersects(viewport))
            return;
        // Corners in texture space that the draw below scales to morphedRadius().
        std::optional<render::Renderer::WindowCorners> corners;
        if (morphsExterior()) {
            const Sample sample = current();
            const base::Rect geometry = absoluteGeometry();
            if (!sample.bounds.isEmpty()) {
                corners = render::Renderer::WindowCorners {
                        morphedRadius(sample),
                        base::SizeF(geometry.width() / sample.bounds.width(),
                               geometry.height() / sample.bounds.height())};
            }
        }
        // The pass suspends presentation recording, which is right for a
        // snapshot kept for later. This one is drawn in the same frame, so the
        // live content in it is presented like any layer cc draws through a
        // render surface.
        render::PresentationSubmission *submission = renderer.presentationSubmission;
        // The overview draws a window's blurs where it shows it.
        const bool blurSuppressed =
                renderer.suppressBackgroundBlur || presentedScaled() || m_presentedAt;
        swapStashedTexture(blurSuppressed);
        const bool contentChanged =
                m_textureDamage.hasDamage(*this, coverage, renderer.rasterScale());
        const bool direct = !renderer.suppressBackgroundBlur && drawsDirectly(contentChanged);
        if (!renderer.capturing
            && (renderer.frameTime < 0 || renderer.frameTime != m_directFrameTime)) {
            m_directFrameTime = renderer.frameTime;
            m_directFrames = direct ? m_directFrames + 1 : 0;
            if (!direct)
                m_textureDrawnAt = m_now();
            // A first texture, or an animation's own frames, start no run.
            m_contentChanged = contentChanged && settled() && m_texture;
        }
        base::Rect repaint;
        if (textureIsCurrent(renderer, coverage, &repaint)) {
            recordPresentation(renderer, *this);
        } else if (direct) {
            // The texture keeps its last image, bounds and coverage, and is
            // drawn whole again by the next frame that does not draw directly.
            // Asking for that frame here damaged the whole window and its
            // shadow on every frame, which cost more than the pass it saved.
            m_textureReusable = false;
            Node::render(renderer);
            m_presented = {base::RectF(absoluteGeometry()), 1, 1};
            return;
        } else {
            // Mid-animation passes and plane promotions draw what only this
            // frame shows. A capture of the same output draws what the screen
            // does, so the two share the snapshot; one at another scale or
            // viewport fails textureReusable() anyway.
            const bool reusable = !animating() && !hasPromotedSurface(*this);
            // The blurs in it read the target this texture is drawn on.
            const uint64_t backdropScope = renderer.backdropScope;
            if (!renderer.renderToTexture(m_texture, coverage, repaint, [&] {
                    renderer.backdropScope = backdropScope;
                    const bool suppressed = renderer.suppressWindowOutlines;
                    const bool wasBlurSuppressed = renderer.suppressBackgroundBlur;
                    const bool revealed = renderer.revealHiddenSurfaces;
                    renderer.suppressWindowOutlines = true;
                    renderer.suppressBackgroundBlur = blurSuppressed;
                    renderer.revealHiddenSurfaces = isPresentedHidden();
                    renderer.presentationSubmission = submission;
                    renderer.windowCorners = corners;
                    renderSubtree(renderer);
                    renderer.windowCorners.reset();
                    renderer.revealHiddenSurfaces = revealed;
                    renderer.suppressBackgroundBlur = wasBlurSuppressed;
                    renderer.suppressWindowOutlines = suppressed;
                })) {
                m_textureReusable = false;
                Node::render(renderer);
                // Whoever waits for the end, e.g. to unmap a closed window, still hears of it.
                if (std::exchange(m_running, false) && m_finished)
                    m_finished();
                return;
            }
            // A blur drawn from what the last frame left behind the window is
            // not on screen now, but would be wrong later. The scene repaints
            // all a blur reads when the whole texture is drawn again next frame.
            const base::Rect drawn = repaint.isEmpty() ? coverage : repaint;
            const base::Rect output = outputViewport(renderer, *this);
            const auto &samples = m_textureDamage.samples();
            const bool backdropCurrent = blurSuppressed
                    || std::all_of(samples.begin(), samples.end(), [&](const base::Rect &sampled) {
                        return !sampled.intersects(drawn) || !sampled.intersects(output)
                                || renderer.repaints(sampled);
                    });
            m_textureReusable = reusable && backdropCurrent;
            m_textureBlurSuppressed = blurSuppressed;
            m_textureMipmapped = false;
            m_textureScale = renderer.rasterScale();
            m_textureViewport = output;
            m_textureRootRevision = sceneRoot(*this).revision();
        }
        m_coverage = coverage;
        m_textureBounds = absoluteGeometry();
        // A popup repositioned by its client mid-fade is drawn where it now is.
        if (m_style == Style::Popup && m_running) {
            auto *self = const_cast<WindowAnimation *>(this);
            self->m_from = self->m_to = base::RectF(m_textureBounds);
        }
    }

    // Shrunk to a dock slot, a bilinear snapshot shimmers; the dock images
    // are read through the same filtering, so the landing still matches.
    if (m_running && m_type == Type::Minimize)
        renderer.enableTrilinearFiltering(m_texture.get());
    if (m_snapshot && m_type == Type::Minimize && !m_presentedAt)
        readSnapshot(renderer);

    Sample value = (m_running || m_holding) && !isPresentedHidden()
            ? current()
            : Sample {absoluteGeometry(), 1, 1};
    // What was presented stays the animation's own; each draw adds the client's.
    if (!m_presentedAt)
        m_presented = value;
    value.opacity *= m_opacity;
    if (m_scaleFrom != m_scaleTo && m_scaleStarted < 0)
        m_scaleStarted = now();
    if (const double scale = presentationScale(); scale != 1) {
        // Kept inside, so a shrunk window never draws outside its own bounds.
        const base::Rect geometry = absoluteGeometry();
        const base::PointF anchor(geometry.x() + std::clamp<double>(m_scaleAnchor.x(), 0, geometry.width()),
                             geometry.y() + std::clamp<double>(m_scaleAnchor.y(), 0, geometry.height()));
        value.bounds = base::RectF(anchor + (value.bounds.topLeft() - anchor) * scale,
                              value.bounds.size() * scale);
    }
    if (m_presentedAt) {
        const base::Rect geometry = absoluteGeometry();
        if (!geometry.isEmpty()) {
            const base::RectF &to = m_presentedAt->bounds;
            const double sx = to.width() / geometry.width();
            const double sy = to.height() / geometry.height();
            const base::PointF from = value.bounds.topLeft() - base::PointF(geometry.topLeft());
            value.bounds = base::RectF(to.topLeft() + base::PointF(from.x() * sx, from.y() * sy),
                                  base::SizeF(value.bounds.width() * sx, value.bounds.height() * sy));
            // Far below its size, a bilinear texture shimmers as it moves.
            if (m_texture && !m_textureMipmapped && std::min(sx, sy) < 0.75) {
                renderer.enableTrilinearFiltering(m_texture.get());
                m_textureMipmapped = true;
            }
        }
        value.opacity *= m_presentedAt->opacity;
    }
    if (m_running && m_type == Type::Minimize && value.progress >= 1 && m_landingSnapshot)
        readSnapshot(renderer, true);

    if ((m_running || m_holding) && m_type == Type::Minimize && value.progress >= 1
        && m_landingTexture && !isPresentedHidden()) {
        // The clip and resampling are already baked into the image sent to the dock.
        renderer.drawTextureF(m_landingTexture.get(), value.bounds, value.opacity);
    } else if (!frozen || m_running || m_holding) {
        const float blend =
                m_running && isCrossFade(m_type) ? 1 - std::pow(1 - value.progress, 2) : 1;
        // The current snapshot supplies the exterior once. Old content fades
        // only inside its silhouette, so a square old buffer cannot leak out.
        const float exterior = exteriorOpacity(value);
        if (m_presentedAt) {
            drawPresented(renderer, value);
        } else if (morphsExterior()) {
            drawExteriorMorph(renderer, value, blend);
        } else {
            if (exterior < 1) {
                if (exterior > 0)
                    draw(renderer, m_texture.get(), m_coverage, m_textureBounds, value, exterior);
                renderer.pushRoundedClip(bodyShape(m_textureBounds, value.bounds));
                draw(renderer, m_texture.get(), m_coverage, m_textureBounds, value);
                renderer.popRoundedClip();
            } else {
                draw(renderer, m_texture.get(), m_coverage, m_textureBounds, value);
            }
            if (m_oldTexture) {
                if (m_visual.clipContent)
                    renderer.pushRoundedClip(
                            transformedShape(m_visual.shape, m_textureBounds, value));
                draw(renderer, m_oldTexture.get(), m_oldCoverage, m_oldBounds, value, 1 - blend);
                if (m_visual.clipContent)
                    renderer.popRoundedClip();
            }
            Sample outlined = value;
            outlined.opacity *= exterior;
            outline(renderer, m_visual, m_textureBounds, outlined);
        }
    }
    if (m_scaleFrom != m_scaleTo) {
        if (presentationScale() == m_scaleTo)
            m_scaleFrom = m_scaleTo;
        changed();
    }
    if (m_running) {
        if (value.progress >= 1) {
            m_running = false;
            m_holding = m_holdAtEnd && m_type == Type::Minimize;
            m_oldTexture.reset();
            if (m_finished)
                m_finished();
        }
        // Also repaint the final removal/hide. OutputScheduler coalesces these
        // requests with KMS presentation; no independent animation timer runs.
        changed();
    }
}

} // namespace Lingmo::Scene
