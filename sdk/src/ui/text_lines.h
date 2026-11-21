// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "include/core/SkColor.h"
#include "include/core/SkRefCnt.h"

#include "ui/geometry.h"

class SkCanvas;
class SkPaint;
class SkTextBlob;
class SkTypeface;

namespace skia::textlayout {
class Paragraph;
}

namespace Lingmo::ui {

// A line of a paragraph as the text layout places it, in logical pixels from
// the paragraph's top: the greatest ascent and descent of the fonts its
// glyphs use, fallback fonts included, and as tall as both rounded up.
struct TextLineBox
{
    double top = 0;
    double height = 0;
    double ascent = 0;
};

// How glyph advances are taken: rounded to whole pixels when metrics are
// hinted, and absolute letter spacing added to each glyph's (the last's too).
// A bool converts, for no spacing.
struct AdvanceRule
{
    AdvanceRule(bool rounds, double spacing = 0)
        : round(rounds)
        , letterSpacing(spacing)
    {
    }
    bool round = false;
    double letterSpacing = 0;
};

// The paragraph is laid out at the logical pixel size, in logical units.
std::vector<TextLineBox> textLineBoxes(skia::textlayout::Paragraph* paragraph, const SkTypeface* typeface,
                                       double pixelSize, double scale, bool hintedLayout);
double textHeight(const std::vector<TextLineBox>& lines);

// A paragraph's glyphs line by line at their truncated advances, as the
// functions below measure and draw them, for a paragraph whose layout does
// not change.
struct LineGlyphs;
std::shared_ptr<const LineGlyphs> lineGlyphs(skia::textlayout::Paragraph* paragraph, const AdvanceRule& roundAdvances);

// The widest line's advance: the sum of each glyph's design advance
// truncated to 26.6 fixed point, kerning included, and rounded to whole
// pixels when metrics are hinted (TextContext::roundsMetrics).
double naturalWidth(skia::textlayout::Paragraph* paragraph, const AdvanceRule& roundAdvances);
double naturalWidth(const LineGlyphs& glyphs);
// Each line's advance, as naturalWidth measures it.
std::vector<double> lineWidths(skia::textlayout::Paragraph* paragraph, const AdvanceRule& roundAdvances);

// Wrap modes: none, at word boundaries, at any character, or at word
// boundaries and at any character where a word does not fit.
enum class LineWrap {
    None,
    Word,
    Anywhere,
    WordOrAnywhere,
};

// Where each line of the text starts at the width, on the truncated
// advances and the line break opportunities: UTF-8 offsets, the first 0. The
// paragraph holds the text laid out at an unbounded width.
std::vector<size_t> lineStarts(skia::textlayout::Paragraph* unbounded, const std::string& text, double width,
                                 LineWrap wrap, const AdvanceRule& roundAdvances);

// Draws the paragraph's glyphs on the boxes' baselines and at the
// truncated advances, offset in logical
// pixels on a canvas that maps logical pixels to device ones. The colour
// comes from the UTF-8 offset of each run.
// Lines start at the left of a paragraph laid out left aligned; `align` 0,
// 0.5 or 1 places each within the width by its advance, overhang left out.
// Greyscale glyphs without `subpixel`, whatever fontconfig asks for.
void paintTextLines(SkCanvas* canvas, skia::textlayout::Paragraph* paragraph, const std::vector<TextLineBox>& lines,
                    PointF offset, double width, double align, const AdvanceRule& roundAdvances,
                    const std::function<SkColor(size_t utf8Offset)>& color, bool subpixel = true);
// The same, with the glyphs made by lineGlyphs() for the paragraph.
void paintTextLines(SkCanvas* canvas, const LineGlyphs& glyphs, const std::vector<TextLineBox>& lines, PointF offset,
                    double width, double align, const std::function<SkColor(size_t utf8Offset)>& color,
                    bool subpixel = true);

// Paragraph::paint, its glyphs drawn as paintTextLines draws them on
// translucent targets (per-channel coverage blended over the target).
void paintParagraph(SkCanvas* canvas, skia::textlayout::Paragraph* paragraph, double x, double y);

// Forgets the coverage kept for subpixel text over translucent targets, as
// after a change of theme, font or scale.
void clearSubpixelCoverage();

// Where a paragraph's text went, for carets, selections and hit tests:
// every line's UTF-8 range and the x of each cluster as drawn.
struct TextGeometry
{
    struct Line
    {
        size_t start = 0;
        // Before a line break.
        size_t end = 0;
        // Cluster starts and the line's end, by offset, with their x.
        std::vector<std::pair<size_t, double>> stops;
    };

    std::vector<Line> lines;

    size_t lineOf(size_t offset) const;
    // The caret's x at the offset, on its line.
    double x(size_t offset) const;
    // The cluster boundary nearest to x on the line.
    size_t offsetAt(size_t line, double x) const;
};

// The paragraph's text is `text`, in UTF-8.
TextGeometry textGeometry(skia::textlayout::Paragraph* paragraph, const std::string& text, double width,
                          double align, const AdvanceRule& roundAdvances);
TextGeometry textGeometry(skia::textlayout::Paragraph* paragraph, const LineGlyphs& glyphs, const std::string& text,
                          double width, double align);

// Draws a blob of positioned runs as paintTextLines draws glyphs: subpixel
// runs on a translucent target through the subpixel blend when the window
// allows it, else greyscale.
void drawTextBlobForTarget(SkCanvas* canvas, const sk_sp<SkTextBlob>& blob, float x, float y, const SkPaint& paint);

// The ink bounds of the paragraph's first line, from the origin of its
// baseline: whole-pixel glyph boxes at the truncated advances.
RectF tightBoundingRect(skia::textlayout::Paragraph* paragraph, double pixelSize, double scale, bool hintedLayout);
// The ink of the paragraph's first line, from the origin of its baseline in
// logical pixels: the glyphs' outlines at the device size, hinted only when
// metrics are hinted, unrounded, at the truncated advances.
RectF inkBoundingRect(skia::textlayout::Paragraph* paragraph, double pixelSize, double scale, bool hintedLayout);

} // namespace Lingmo::ui
