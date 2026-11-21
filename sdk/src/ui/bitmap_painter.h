// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

namespace Lingmo::base { class EventLoop; }
namespace base = Lingmo::base;

#include <memory>
#include <optional>
#include <vector>

#include "base/geometry.h"
#include "base/transform.h"
#include "ui/bitmap.h"
#include "ui/geometry.h"
#include "ui/path.h"

namespace Lingmo::ui {

// Draws into a Bitmap on the CPU, in coordinates the transform maps to the
// bitmap's pixels. Antialiasing starts off, as the common 2D painter has it.
// Fills map their outline in double precision; strokes and images go
// through a single-precision matrix.
class BitmapPainter
{
public:
    enum class CompositionMode {
        SourceOver,
        Source,
        SourceIn,
        DestinationIn,
        DestinationOut,
        Clear,
        Plus,
    };

    // `target` must outlive the painter and must not be copied from while
    // it paints.
    explicit BitmapPainter(Bitmap& target);
    ~BitmapPainter();

    BitmapPainter(const BitmapPainter&) = delete;
    BitmapPainter& operator=(const BitmapPainter&) = delete;

    bool isActive() const { return m_canvas != nullptr; }

    void setAntialiasing(bool antialiasing) { m_state.antialiasing = antialiasing; }
    bool antialiasing() const { return m_state.antialiasing; }

    const base::AffineTransform& transform() const { return m_state.transform; }
    void setTransform(const base::AffineTransform& transform) { m_state.transform = transform; }
    // Each applies before the current transform.
    void translate(double dx, double dy) { m_state.transform.translate(dx, dy); }
    void scale(double sx, double sy) { m_state.transform.scale(sx, sy); }
    void rotate(double degrees) { m_state.transform.rotate(degrees); }

    void setCompositionMode(CompositionMode mode) { m_state.mode = mode; }
    CompositionMode compositionMode() const { return m_state.mode; }
    // Multiplies every colour's and image's alpha.
    void setOpacity(double opacity) { m_state.opacity = opacity; }
    double opacity() const { return m_state.opacity; }

    // Transform, composition mode, antialiasing, opacity and clip.
    void save();
    void restore();
    // Intersects the clip with the rectangle, in current coordinates.
    void clipRect(const base::RectF& rect);
    void clipPath(const Path& path);

    void fill(Color color);
    void fillRect(const base::RectF& rect, Color color);
    void fillEllipse(const base::RectF& rect, Color color);
    void fillPath(const Path& path, Color color);
    void strokePath(const Path& path, Color color, double width, LineCap cap = LineCap::Square,
                    LineJoin join = LineJoin::Bevel, double miterLimit = 2);
    void drawLine(base::PointF from, base::PointF to, Color color, double width, LineCap cap = LineCap::Square);
    // A closed polygon filled and outlined, either optional.
    void drawPolygon(const std::vector<base::PointF>& points, std::optional<Color> fill,
                     std::optional<Color> stroke = std::nullopt, double strokeWidth = 1,
                     LineJoin join = LineJoin::Bevel);
    void drawBitmap(base::PointF topLeft, const Bitmap& bitmap);
    // Scaled into the rectangle; smooth filters, otherwise nearest pixel.
    void drawBitmap(const base::RectF& target, const Bitmap& bitmap, bool smooth = true);

private:
    struct State
    {
        base::AffineTransform transform;
        CompositionMode mode = CompositionMode::SourceOver;
        bool antialiasing = false;
        double opacity = 1;
    };
    struct Canvas;

    State m_state;
    std::vector<State> m_saved;
    std::unique_ptr<Canvas> m_canvas;
};

} // namespace Lingmo::ui
