namespace Lingmo::base { class EventLoop; }
namespace base = Lingmo::base;

// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/bitmap_painter.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "ui/bitmap_private.h"
#include "ui/path_private.h"

namespace Lingmo::ui {

struct BitmapPainter::Canvas
{
    SkBitmap bitmap;
    std::unique_ptr<SkCanvas> canvas;
};

static SkMatrix toSkMatrix(const base::AffineTransform& t)
{
    return SkMatrix::MakeAll(SkScalar(t.m11()), SkScalar(t.m21()), SkScalar(t.dx()), SkScalar(t.m12()),
                             SkScalar(t.m22()), SkScalar(t.dy()), 0, 0, 1);
}

static SkBlendMode toSkBlendMode(BitmapPainter::CompositionMode mode)
{
    switch (mode) {
    case BitmapPainter::CompositionMode::SourceOver:
        return SkBlendMode::kSrcOver;
    case BitmapPainter::CompositionMode::Source:
        return SkBlendMode::kSrc;
    case BitmapPainter::CompositionMode::SourceIn:
        return SkBlendMode::kSrcIn;
    case BitmapPainter::CompositionMode::DestinationIn:
        return SkBlendMode::kDstIn;
    case BitmapPainter::CompositionMode::DestinationOut:
        return SkBlendMode::kDstOut;
    case BitmapPainter::CompositionMode::Clear:
        return SkBlendMode::kClear;
    case BitmapPainter::CompositionMode::Plus:
        return SkBlendMode::kPlus;
    }
    return SkBlendMode::kSrcOver;
}

BitmapPainter::BitmapPainter(Bitmap& target)
{
    if (target.isNull())
        return;
    auto canvas = std::make_unique<Canvas>();
    const SkImageInfo info = SkImageInfo::Make(target.width(), target.height(), skColorType(target.format()),
                                               kPremul_SkAlphaType);
    if (!canvas->bitmap.installPixels(info, target.bits(), target.stride()))
        return;
    canvas->canvas = std::make_unique<SkCanvas>(canvas->bitmap);
    m_canvas = std::move(canvas);
}

BitmapPainter::~BitmapPainter() = default;

void BitmapPainter::save()
{
    m_saved.push_back(m_state);
    if (m_canvas)
        m_canvas->canvas->save();
}

void BitmapPainter::restore()
{
    if (m_saved.empty())
        return;
    m_state = m_saved.back();
    m_saved.pop_back();
    if (m_canvas)
        m_canvas->canvas->restore();
}

static SkPaint makePaint(Color color, double opacity, bool antialiasing, BitmapPainter::CompositionMode mode)
{
    SkPaint paint;
    paint.setColor(color.withAlpha(color.a / 255.0 * std::clamp(opacity, 0.0, 1.0)).argb());
    paint.setAntiAlias(antialiasing);
    paint.setBlendMode(toSkBlendMode(mode));
    return paint;
}

void BitmapPainter::clipRect(const base::RectF& rect)
{
    Path path;
    path.addRect({rect.x(), rect.y(), rect.width(), rect.height()});
    clipPath(path);
}

void BitmapPainter::clipPath(const Path& path)
{
    if (!m_canvas)
        return;
    m_canvas->canvas->clipPath(PathAccess::toSkPath(path, m_state.transform), m_state.antialiasing);
}

void BitmapPainter::fill(Color color)
{
    if (!m_canvas)
        return;
    SkCanvas* canvas = m_canvas->canvas.get();
    canvas->drawPaint(makePaint(color, m_state.opacity, false, m_state.mode));
}

void BitmapPainter::fillRect(const base::RectF& rect, Color color)
{
    Path path;
    path.addRect({rect.x(), rect.y(), rect.width(), rect.height()});
    fillPath(path, color);
}

void BitmapPainter::fillEllipse(const base::RectF& rect, Color color)
{
    Path path;
    path.addEllipse({rect.x(), rect.y(), rect.width(), rect.height()});
    fillPath(path, color);
}

void BitmapPainter::fillPath(const Path& path, Color color)
{
    if (!m_canvas || path.isEmpty())
        return;
    m_canvas->canvas->drawPath(PathAccess::toSkPath(path, m_state.transform),
                                 makePaint(color, m_state.opacity, m_state.antialiasing, m_state.mode));
}

void BitmapPainter::strokePath(const Path& path, Color color, double width, LineCap cap, LineJoin join,
                               double miterLimit)
{
    if (!m_canvas || path.isEmpty())
        return;
    SkPaint paint = makePaint(color, m_state.opacity, m_state.antialiasing, m_state.mode);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(SkScalar(width));
    paint.setStrokeMiter(SkScalar(miterLimit));
    paint.setStrokeCap(cap == LineCap::Butt ? SkPaint::kButt_Cap
                               : cap == LineCap::Round ? SkPaint::kRound_Cap
                                                       : SkPaint::kSquare_Cap);
    paint.setStrokeJoin(join == LineJoin::Miter ? SkPaint::kMiter_Join
                                : join == LineJoin::Round ? SkPaint::kRound_Join
                                                          : SkPaint::kBevel_Join);
    SkCanvas* canvas = m_canvas->canvas.get();
    canvas->save();
    canvas->concat(toSkMatrix(m_state.transform));
    canvas->drawPath(PathAccess::toSkPath(path), paint);
    canvas->restore();
}

void BitmapPainter::drawLine(base::PointF from, base::PointF to, Color color, double width, LineCap cap)
{
    Path path;
    path.moveTo(from.x(), from.y());
    path.lineTo(to.x(), to.y());
    strokePath(path, color, width, cap);
}

void BitmapPainter::drawPolygon(const std::vector<base::PointF>& points, std::optional<Color> fill,
                                std::optional<Color> stroke, double strokeWidth, LineJoin join)
{
    if (points.size() < 2)
        return;
    Path path;
    path.moveTo(points.front().x(), points.front().y());
    for (size_t i = 1; i < points.size(); ++i)
        path.lineTo(points[i].x(), points[i].y());
    path.close();
    if (fill)
        fillPath(path, *fill);
    if (stroke)
        strokePath(path, *stroke, strokeWidth, LineCap::Square, join);
}

void BitmapPainter::drawBitmap(base::PointF topLeft, const Bitmap& bitmap)
{
    drawBitmap(base::RectF(topLeft, base::SizeF(bitmap.size())), bitmap, false);
}

void BitmapPainter::drawBitmap(const base::RectF& target, const Bitmap& bitmap, bool smooth)
{
    if (!m_canvas || bitmap.isNull() || target.isEmpty())
        return;
    const sk_sp<SkImage> image = SkImages::RasterFromPixmapCopy(skPixmap(bitmap));
    if (!image)
        return;
    SkPaint paint;
    paint.setAlphaf(float(std::clamp(m_state.opacity, 0.0, 1.0)));
    paint.setBlendMode(toSkBlendMode(m_state.mode));
    paint.setAntiAlias(m_state.antialiasing);
    const SkSamplingOptions sampling = smooth ? SkSamplingOptions(SkFilterMode::kLinear) : SkSamplingOptions();
    SkCanvas* canvas = m_canvas->canvas.get();
    canvas->save();
    canvas->concat(toSkMatrix(m_state.transform));
    canvas->drawImageRect(image, SkRect::MakeXYWH(SkScalar(target.x()), SkScalar(target.y()), SkScalar(target.width()),
                                                  SkScalar(target.height())),
                          sampling, &paint);
    canvas->restore();
}

} // namespace Lingmo::ui
