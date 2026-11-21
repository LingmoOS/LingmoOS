// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_SCENE_OVERVIEW_NODE_H_
#define LINGMO_SCENE_OVERVIEW_NODE_H_

#include <functional>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <vector>

#include <string>

#include "scene/finger_track.h"
#include "scene/motion.h"
#include "scene/node.h"
#include "base/geometry.h"
#include "ui/bitmap.h"

namespace Lingmo::Scene {

class WindowAnimation;

// One line of text at `density` device pixels to a logical one, no wider than
// `maxWidth` logical pixels: premultiplied RGBA8888, or null.
using OverviewTextPainter = std::function<ui::Bitmap(const std::string &text, int pixelSize,
                                                     const render::Color &color, double density,
                                                     int maxWidth)>;

// What the overview draws around every window alike, whatever its own state:
// the corners of a window in its normal state, `radius` logical pixels scaled
// with the window, a shadow under the body and an outline over it. `uniform`
// goes from 0, where the window's own shadow, which the painter reads off
// `window`, is drawn, to 1, where only the overview's is. `moving` says the
// body is not yet where it rests.
struct OverviewWindowFrame {
    double radius = 0;
    std::function<void(render::Renderer &renderer, const WindowAnimation &window,
                       const render::RoundedShape &body, double uniform, float opacity,
                       bool moving)>
            shadow;
    std::function<void(render::Renderer &renderer, const render::RoundedShape &body,
                       float opacity)>
            outline;
};

// Textures of the overview's text and glyphs, made once per look and scale.
class OverviewChromeCache
{
public:
    render::Texture *label(render::Renderer &renderer, const OverviewTextPainter &painter,
                           const std::string &text, int pixelSize, const render::Color &color,
                           int maxWidth, base::SizeF *size);
    enum class Glyph {
        Close,
        Plus,
    };
    // The glyph filling `size` logical pixels, drawn in `color`.
    render::Texture *glyph(render::Renderer &renderer, Glyph glyph, const base::SizeF &size,
                           const render::Color &color);
    void clear() { m_entries.clear(); }

private:
    struct Entry {
        std::string key;
        std::unique_ptr<render::Texture> texture;
        base::SizeF size;
    };
    render::Texture *find(const std::string &key, base::SizeF *size);
    render::Texture *insert(render::Renderer &renderer, const std::string &key, const ui::Bitmap &image,
                            const base::SizeF &size);
    std::list<Entry> m_entries;
};

// The window overview's windows, over the desktop dimmed behind them. It sits
// above the active desk's windows and draws them itself, each moved from
// where it is towards its place in the overview by the overview's progress:
// 0 is the desk as it is, 1 the overview. Everything below it in its parent
// is covered while it is active; the dock above stays.
//
// The progress follows the fingers of a touchpad swipe, resampled at frame
// time as the desk swipe is, stops dead at 0 and 1 and lets go into a quick
// tween leaving at the fingers' speed; the shortcut and a click ride the same
// curve from rest. Upstream Ash
// interpolates cached transforms by the scroll ratio
// (OverviewGrid::PositionWindowsContinuously) and decides the release by
// distance alone.
class OverviewNode : public Node
{
public:
    struct Item {
        // Drawn together, bottom to top: a window and its transient children.
        // The first one's geometry is what `to` places.
        std::vector<const WindowAnimation *> windows;
        base::RectF to;
        std::string title;
    };

    // What the pointer or the keyboard is on.
    struct Chrome {
        int hovered = -1;
        int selected = -1;
    };

    // The whole way in or out, shorter for less of it.
    static constexpr double kTransitionDurationMs = 250;
    // Ash's kTransition, for a window moving to a new place.
    static constexpr double kLayoutDurationMs = 300;
    // Released faster than this, in progress per second, the overview goes on
    // the way the fingers went; slower, to the nearer end.
    static constexpr double kFlickVelocity = 0.5;
    static constexpr double kChromeFadeMs = 150;
    static constexpr float kDimOpacity = 0.35f;

    explicit OverviewNode(std::function<int64_t()> clock = {});

    void setTextPainter(OverviewTextPainter painter) { m_textPainter = std::move(painter); }
    void setWindowFrame(OverviewWindowFrame frame) { m_frame = std::move(frame); }

    // The desktop, drawn under the dim.
    void setBackdrop(std::vector<const Node *> nodes);
    // Drawn over the backdrop and faded out as the overview comes in: the
    // desktop's icons.
    void setFadingBackdrop(std::vector<const Node *> nodes);

    // `animate` moves windows already shown to their new places on a tween;
    // otherwise they are there at once.
    void setItems(std::vector<Item> items, bool animate);
    const std::vector<Item> &items() const { return m_items; }

    // The item drawn at `rect` as the pointer drags it; none for -1. It is not
    // drawn with the rest but by renderDragged(), over the desk bar.
    void setDragged(int index, const base::RectF &rect);
    void renderDragged(render::Renderer &renderer) const;

    // The item drawn over the rest; none for -1.
    void setRaised(int index);

    void setChrome(const Chrome &chrome);

    // --- Progress ---

    // Where the overview is at `timeMs` on the frame clock, between 0 and 1.
    double progress(double timeMs) const;
    double progress() const { return progress(clockTime()); }
    // The node's clock, for times outside an output frame.
    double clockTime() const { return double(m_now()); }

    // Heads for `target`, from where it is shown and at the speed it moves.
    void animateTo(double target);
    // Stands at `value` at once.
    void jumpTo(double value);
    double target() const;

    // The fingers take the progress where it is shown; `delta` moves it.
    void startTracking();
    void updateTracking(double delta, double timeMs);
    // The fingers let go at `timeMs`; returns where the progress heads, 0 or 1.
    double endTracking(double timeMs);
    bool isTracking() const { return m_motion == Motion::Tracking; }
    // Standing still at target().
    bool isSettled() const { return m_motion == Motion::Still; }

    // Shown at all: moving, or standing anywhere but 0.
    bool isActive() const { return m_motion != Motion::Still || m_position > 0; }

    // Draws and covers nothing while the progress goes on moving the desk bar
    // and the panels: a desk switch's strip shows the desks meanwhile.
    void setContentHidden(bool hidden);
    bool covers() const { return isActive() && !m_contentHidden; }

    // Runs from inside render() when the progress lands on 0 or 1, with that
    // value: must not change the scene.
    void setFinishedCallback(std::function<void(double)> callback)
    {
        m_finished = std::move(callback);
    }

    // Where item `index` is drawn at `progress`.
    base::RectF presentedRect(int index, double progress, double timeMs) const;

    void render(render::Renderer &renderer) const override;
    Content content() const override { return covers() ? Content::Opaque : Content::None; }
    bool paintsSubtree() const override { return covers(); }
    bool requiresFullRepaint() const override { return covers(); }
    bool acceptsInput(const base::PointF &) const override { return false; }
    bool occludesOutput(const render::Renderer &) const override { return covers(); }
    void releaseRenderResources() override;

private:
    enum class Motion {
        Still,
        Tracking,
        Tween,
    };

    motion::Tween tweenTo(double now, double from, double target, double velocity) const;
    void settle(double now, double from, double target, double velocity);
    void updateCoast();
    double layoutProgress(double timeMs) const;
    // Whether the backdrop paints the whole viewport opaquely.
    bool backdropCovers(const base::Rect &viewport) const;
    // Whether the chrome shows, and how much, at `timeMs`.
    float chromeOpacity(double timeMs) const;
    void renderItem(render::Renderer &renderer, int index, double progress, double now) const;
    // The main window's body at `rect`, at rest in the overview.
    render::RoundedShape restingBody(int index, const base::RectF &rect) const;
    void renderChrome(render::Renderer &renderer, const base::Rect &viewport, double now) const;
    void landed(double now) const;
    // Lands the progress, and asks for the frames its motion needs.
    void advance(double now) const;

    std::function<int64_t()> m_now;
    OverviewTextPainter m_textPainter;
    OverviewWindowFrame m_frame;
    std::vector<const Node *> m_backdrop;
    std::vector<const Node *> m_fadingBackdrop;
    std::vector<Item> m_items;
    // Where each item was headed when the layout last changed.
    std::vector<base::RectF> m_layoutFrom;
    std::optional<motion::Tween> m_layout;
    int m_dragged = -1;
    base::RectF m_dragRect;
    int m_raised = -1;
    bool m_contentHidden = false;
    Chrome m_chrome;

    mutable Motion m_motion = Motion::Still;
    mutable double m_position = 0;
    motion::FingerTrack m_fingers;
    std::optional<motion::Tween> m_coast;
    double m_coastStart = 0;
    motion::Tween m_tween;
    // When the progress came to rest at 1, for the chrome to fade in from.
    mutable double m_shownAt = -1;
    std::function<void(double)> m_finished;
    mutable OverviewChromeCache m_chromeCache;
};

// The overview's desk bar along the top edge of each output: a preview
// of every desk and a new-desk button, even with one desk (Ash shows a strip
// of buttons until there are two). It slides in from the top edge with the
// overview's progress and stays still above a desk switch, which slides the
// overview's windows underneath.
class OverviewDeskBarNode : public Node
{
public:
    struct Desk {
        base::RectF bounds;
        std::string name;
        bool active = false;
        // A window dragged over it would drop there.
        bool highlighted = false;
        // Pressed, with the pointer still on it.
        bool pressed = false;
        // Dragged along the bar: follows the pointer, over the rest.
        bool dragged = false;
        // The pointer is on it and it can be removed: its close button shows,
        // lit while the pointer is on the button.
        bool closeShown = false;
        bool closeHovered = false;
        // Stays with the desk as the bar changes around it.
        uintptr_t id = 0;
    };

    struct Bar {
        // The output it is on, and where it sits once shown.
        base::Rect output;
        base::RectF bounds;
        std::vector<Desk> desks;
        base::RectF newDesk;
        bool newDeskHighlighted = false;
    };

    // What a desk preview shows: the desk's windows bottom to top, over the backdrop.
    struct DeskContent {
        // The Desk::id it is for.
        uintptr_t id = 0;
        std::vector<const WindowAnimation *> windows;
    };

    // Ash's DeskActionView close button, in the preview's top right corner.
    static constexpr double kCloseButtonSize = 24;
    static constexpr double kCloseButtonMargin = 4;
    static base::RectF closeButtonRect(const base::RectF &preview);

    static constexpr double kPreviewRadius = 8;
    static constexpr int kLabelPixelSize = 13;
    // A live desk preview is drawn again at most this often, and no more
    // previews than this in one frame.
    static constexpr double kPreviewRefreshMs = 500;
    static constexpr int kPreviewsPerFrame = 1;

    // Ash's desk_mini_view_animations: the rest move over to their new
    // places, a new desk scales up from its top centre and fades in, a removed
    // one fades out where it was. The new-desk button moves, comes and goes
    // the same way.
    static constexpr double kShiftDurationMs = 150;
    static constexpr double kAddScaleDelayMs = 50;
    static constexpr double kAddScaleDurationMs = 150;
    static constexpr double kAddInitialScale = 0.1;
    static constexpr double kAddFadeDelayMs = 100;
    static constexpr double kAddFadeDurationMs = 100;
    static constexpr double kRemoveFadeDurationMs = 200;

    explicit OverviewDeskBarNode(const OverviewNode *overview);

    void setTextPainter(OverviewTextPainter painter) { m_textPainter = std::move(painter); }
    void setBars(std::vector<Bar> bars);
    void setContent(std::vector<const Node *> backdrop, std::vector<DeskContent> desks);
    // Draws every preview again at the next frame.
    void invalidatePreviews();

    // How far the bar on an output is slid up out of view at `progress`.
    static double slideOffset(const Bar &bar, double progress);

    void render(render::Renderer &renderer) const override;
    Content content() const override
    {
        return m_overview->isActive() ? Content::Translucent : Content::None;
    }
    bool paintsSubtree() const override { return m_overview->isActive(); }
    bool requiresFullRepaint() const override { return m_overview->isActive(); }
    bool acceptsInput(const base::PointF &) const override { return false; }
    void releaseRenderResources() override;

private:
    struct Preview {
        std::unique_ptr<render::Texture> texture;
        base::RectF bounds;
        double scale = 0;
        double drawnAt = -1;
        bool stale = true;
    };

    // How a preview or the new-desk button heads for where the bar puts it.
    struct Placement {
        base::RectF from;
        std::optional<motion::Tween> shift;
        std::optional<motion::Tween> scale;
        std::optional<motion::Tween> fade;

        static Placement appearing(double now);
        base::RectF rect(const base::RectF &to, double now) const;
        double scaleAt(double now) const { return scale ? scale->value(now) : 1; }
        float opacity(double now) const { return fade ? float(fade->value(now)) : 1.0f; }
        bool isMoving(double now) const;
        void moveTo(const base::RectF &oldTo, const base::RectF &to, double now);
    };

    struct BarMotion {
        std::map<uintptr_t, Placement> desks;
        Placement newDesk;
    };

    // A removed desk, or the new-desk button gone, fading out where it was.
    struct Leaving {
        Bar bar;
        std::optional<Desk> desk;
        base::RectF rect;
        double scale = 1;
        std::unique_ptr<render::Texture> preview;
        motion::Tween fade;
    };

    void renderBar(render::Renderer &renderer, const Bar &bar, const BarMotion *motion,
                   double progress, double now) const;
    // `rect` is where the desk sits, its preview scaled by `scale` from the
    // top centre.
    void renderDesk(render::Renderer &renderer, const Desk &desk, const base::RectF &rect,
                    double scale, render::Texture *preview, float opacity) const;
    void renderNewDesk(render::Renderer &renderer, const base::RectF &rect, bool highlighted,
                       float opacity) const;
    void updatePreview(render::Renderer &renderer, const Bar &bar, int desk, double now) const;

    const OverviewNode *m_overview = nullptr;
    OverviewTextPainter m_textPainter;
    std::vector<Bar> m_bars;
    // One for each bar.
    std::vector<BarMotion> m_motions;
    mutable std::vector<Leaving> m_leaving;
    std::vector<const Node *> m_backdrop;
    std::vector<DeskContent> m_desks;
    // Keyed by the output's bounds and the desk's id.
    mutable std::map<std::pair<std::string, uintptr_t>, Preview> m_previews;
    mutable int m_previewsLeft = 0;
    mutable OverviewChromeCache m_chromeCache;
};

} // namespace Lingmo::Scene

#endif // LINGMO_SCENE_OVERVIEW_NODE_H_
