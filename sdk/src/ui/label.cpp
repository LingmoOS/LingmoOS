// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/label.h"

#include <algorithm>
#include <cmath>
#include <utility>

#include <unicode/ubrk.h>
#include <unicode/utext.h>

#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkImage.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypeface.h"
#include "modules/skparagraph/include/Paragraph.h"
#include "modules/skparagraph/include/ParagraphBuilder.h"
#include "modules/skparagraph/include/ParagraphStyle.h"
#include "modules/skparagraph/include/TextStyle.h"

#include "ui/application.h"
#include "ui/painter.h"
#include "ui/text_context.h"
#include "ui/text_lines.h"
#include "ui/theme.h"
#include "ui/window.h"
#include "base/i18n.h"
namespace base = Lingmo::base;
namespace i18n = Lingmo::I18n;

namespace textlayout = skia::textlayout;

namespace ui = Lingmo::ui;

namespace Lingmo::ui {

// Wide enough that nothing wraps while measuring.
static constexpr double kUnboundedWidth = 1e6;
static constexpr const char* kEllipsis = "…";
// Ganesh draws text from 162 device pixels as distance fields; a pixel of margin.
static constexpr double kDistanceFieldTextSize = 161;

struct Label::Wrapping
{
    std::unique_ptr<textlayout::Paragraph> paragraph;
    std::vector<size_t> breaks;
    std::string text;
    double width = -1;
    bool rounded = false;
    bool elided = false;
};

Label::Label(std::string text)
    : m_text(std::move(text))
{
}

Label::~Label() = default;

void Label::setText(std::string text)
{
    if (text == m_text && m_spans.empty())
        return;
    m_text = std::move(text);
    m_spans.clear();
    invalidate();
}

void Label::setSpans(std::vector<Span> spans)
{
    m_text.clear();
    for (const Span& span : spans)
        m_text += span.text;
    m_spans = std::move(spans);
    invalidate();
}

void Label::setFontSize(double size)
{
    if (size == m_fontSize)
        return;
    m_fontSize = size;
    invalidate();
}

void Label::setFontFeatures(std::vector<std::pair<std::string, int>> features)
{
    if (features == m_features)
        return;
    m_features = std::move(features);
    invalidate();
}

void Label::setFontFamily(std::string family)
{
    if (family == m_family)
        return;
    m_family = std::move(family);
    invalidate();
}

void Label::setFadesWhenDisabled(bool fades)
{
    m_fadesWhenDisabled = fades;
    requestPaint();
}

void Label::setWeight(int weight)
{
    if (weight == m_weight)
        return;
    m_weight = weight;
    invalidate();
}

void Label::setColor(std::optional<ColorRecipe> color)
{
    if (sameColor(color, m_color))
        return;
    m_color = std::move(color);
    requestPaint();
}

void Label::setSubpixelAntialiasing(bool subpixel)
{
    if (subpixel == m_subpixel)
        return;
    m_subpixel = subpixel;
    requestPaint();
}

void Label::setLetterSpacing(double spacing)
{
    if (spacing == m_letterSpacing)
        return;
    m_letterSpacing = spacing;
    m_naturalWidth = -1;
    invalidate();
    requestLayout();
    requestPaint();
}

void Label::setWrap(bool wrap)
{
    if (wrap == m_wrap)
        return;
    m_wrap = wrap;
    invalidate();
}

void Label::setWrapAnywhere(bool anywhere)
{
    if (anywhere == m_wrapAnywhere)
        return;
    m_wrapAnywhere = anywhere;
    invalidate();
    requestLayout();
}

void Label::setFixedLineHeight(double height)
{
    if (height == m_fixedLineHeight)
        return;
    m_fixedLineHeight = height;
    requestLayout();
    requestPaint();
}

// A fixed line height: each line box that tall, the text at its top.
std::vector<TextLineBox> Label::lineBoxes(textlayout::Paragraph* text, double scale) const
{
    const TextContext& context = Application::instance()->text();
    std::vector<TextLineBox> lines = textLineBoxes(text, context.typeface(m_weight, m_family).get(), pixelSize(), scale, hintedLayout());
    if (m_fixedLineHeight > 0) {
        for (size_t i = 0; i < lines.size(); ++i) {
            lines[i].top = double(i) * m_fixedLineHeight;
            lines[i].height = m_fixedLineHeight;
        }
    }
    return lines;
}

void Label::setElide(Elide elide)
{
    if (elide == m_elide)
        return;
    m_elide = elide;
    invalidate();
}

void Label::setMaximumLineCount(int count)
{
    if (count == m_maximumLineCount)
        return;
    m_maximumLineCount = count;
    invalidate();
}

void Label::setTextAlignment(TextAlignment alignment)
{
    if (alignment == m_alignment)
        return;
    m_alignment = alignment;
    requestPaint();
}

void Label::setVerticallyCentered(bool centered)
{
    m_verticallyCentered = centered;
    requestPaint();
}

void Label::setBaseline(double y)
{
    if (y == m_baseline)
        return;
    m_baseline = y;
    requestLayout();
}

double Label::currentScale() const
{
    return textLayoutMode().scale;
}

AdvanceRule Label::advanceRule() const
{
    return {roundsAdvances(), m_letterSpacing};
}

bool Label::hintedLayout() const
{
    return textLayoutMode().hinted;
}

double Label::pixelSize() const
{
    return TextContext::pixelSize(m_fontSize > 0 ? m_fontSize : Theme::current().fontSize());
}

// Onto the device grid.
double Label::snapped(double value) const
{
    const double scale = currentScale();
    return std::round(value * scale) / scale;
}

double Label::ascent() const
{
    const TextContext& context = Application::instance()->text();
    return context.metrics(context.typeface(m_weight, m_family).get(), pixelSize(), currentScale(), hintedLayout()).ascent;
}

double Label::descent() const
{
    const TextContext& context = Application::instance()->text();
    return context.metrics(context.typeface(m_weight, m_family).get(), pixelSize(), currentScale(), hintedLayout()).descent;
}

double Label::xHeight() const
{
    const TextContext& context = Application::instance()->text();
    return context.metrics(context.typeface(m_weight, m_family).get(), pixelSize(), currentScale(), hintedLayout()).xHeight;
}

double Label::baselinePadding() const
{
    if (m_baseline < 0)
        return 0;
    // The top margin that puts the first baseline at m_baseline.
    return std::max(0.0, snapped(m_baseline - ascent()));
}

double Label::textOffset(double textHeight) const
{
    if (m_baseline >= 0)
        return baselinePadding();
    // Centred vertically: half the space left, not snapped.
    if (m_verticallyCentered)
        return (geometry().height - textHeight) / 2;
    return 0;
}

bool Label::roundsAdvances() const
{
    return Application::instance()->text().roundsMetrics(currentScale(), hintedLayout());
}

double Label::slack() const
{
    textlayout::Paragraph* text = paragraph();
    const bool round = roundsAdvances();
    if (m_naturalWidth < 0 || m_naturalRounded != round) {
        if (m_layoutWidth != kUnboundedWidth) {
            text->layout(SkScalar(kUnboundedWidth));
            m_layoutWidth = kUnboundedWidth;
        }
        m_naturalWidth = naturalWidth(text, AdvanceRule(round, m_letterSpacing));
        m_intrinsicWidth = text->getMaxIntrinsicWidth();
        m_naturalRounded = round;
    }
    return m_intrinsicWidth - m_naturalWidth;
}

void Label::checkMeasurements() const
{
    const uint64_t generation = measurementGeneration();
    const uint64_t theme = Theme::generation();
    if (m_measuredGeneration == generation && m_measuredTheme == theme)
        return;
    m_measuredGeneration = generation;
    m_measuredTheme = theme;
    m_measuredSize.reset();
    m_measuredHeights.clear();
}

SizeF Label::sizeHint() const
{
    checkMeasurements();
    if (m_measuredSize)
        return *m_measuredSize;
    textlayout::Paragraph* text = layoutParagraph(kUnboundedWidth);
    const std::vector<TextLineBox> lines = lineBoxes(text, currentScale());
    slack();
    // The natural width, unrounded.
    m_measuredSize = SizeF{m_naturalWidth, textHeight(lines) + baselinePadding()};
    return *m_measuredSize;
}

double Label::heightForWidth(double width) const
{
    checkMeasurements();
    const double layoutWidth = m_wrap ? width : kUnboundedWidth;
    for (const auto& [measuredWidth, height] : m_measuredHeights) {
        if (measuredWidth == layoutWidth)
            return height;
    }
    textlayout::Paragraph* text = layoutParagraph(layoutWidth);
    const std::vector<TextLineBox> lines = lineBoxes(text, currentScale());
    const double height = textHeight(lines) + baselinePadding();
    m_measuredHeights.emplace_back(layoutWidth, height);
    return height;
}

double Label::baselineOffset() const
{
    textlayout::Paragraph* text = layoutParagraph(m_wrap ? geometry().width : kUnboundedWidth);
    const std::vector<TextLineBox> lines = lineBoxes(text, currentScale());
    return textOffset(textHeight(lines)) + lines.front().ascent;
}

double Label::capHeight() const
{
    const TextContext& context = Application::instance()->text();
    const textlayout::TextStyle style = context.style(m_fontSize, m_weight, Theme::current().text, m_family);
    textlayout::ParagraphStyle paragraphStyle;
    paragraphStyle.setTextStyle(style);
    auto builder = textlayout::ParagraphBuilder::make(paragraphStyle, context.fontCollection(), context.unicode());
    builder->pushStyle(style);
    builder->addText("H", 1);
    builder->pop();
    std::unique_ptr<textlayout::Paragraph> capital = builder->Build();
    capital->layout(SkScalar(kUnboundedWidth));
    return ui::tightBoundingRect(capital.get(), pixelSize(), currentScale(), hintedLayout()).height;
}

double Label::capCenter() const
{
    const double baseline = m_baseline >= 0 ? m_baseline : baselineOffset();
    return snapped(snapped(baseline) - capHeight() / 2);
}

RectF Label::tightBoundingRect() const
{
    if (m_text.empty())
        return {};
    return ui::tightBoundingRect(layoutParagraph(kUnboundedWidth), pixelSize(), currentScale(), hintedLayout());
}

RectF Label::inkBoundingRect() const
{
    if (m_text.empty())
        return {};
    return ui::inkBoundingRect(layoutParagraph(kUnboundedWidth), pixelSize(), currentScale(), hintedLayout());
}

bool Label::isElided() const
{
    if (m_wrap)
        return m_wrappedWidth >= 0 && m_wrappedElided;
    if (!m_paragraph || m_layoutWidth < 0 || m_elide == Elide::None)
        return false;
    if (m_elided && m_elidedWidth == m_layoutWidth)
        return true;
    return m_paragraph->didExceedMaxLines();
}

std::string Label::shownText() const
{
    if (m_wrap)
        return m_wrappedWidth >= 0 ? m_wrappedText : m_text;
    if (m_elided && m_elidedWidth == m_layoutWidth)
        return m_elidedText;
    return m_text;
}

void Label::paint(Painter& painter)
{
    paintText(painter, {}, std::nullopt);
}

void Label::paintText(Painter& painter, PointF shift, std::optional<Color> color)
{
    if (m_text.empty())
        return;
    const double scale = painter.scale();
    double width = geometry().width;
    if (!m_wrap && m_elide == Elide::None) {
        // Runs past the label; the intrinsic width needs a layout at any width.
        paragraph();
        if (m_layoutWidth < 0)
            layoutParagraph(width);
        width = std::max(width, naturalWidth(m_paragraph.get(), advanceRule()));
    }
    textlayout::Paragraph* text = layoutParagraph(width);
    const TextContext& context = Application::instance()->text();
    const std::vector<TextLineBox> lines = lineBoxes(text, scale);
    const double offset = textOffset(textHeight(lines));

    const Theme& theme = Theme::current();
    const Color textColor = color.value_or(m_color ? m_color->resolve() : theme.text);
    const bool wrapped = m_wrapped && text == m_wrapped.get() && !m_wrappedElided;
    const bool spans = !color && !m_spans.empty() && (text == m_paragraph.get() || wrapped);
    const auto colorAt = [&](size_t utf8Offset) -> SkColor {
        if (wrapped) {
            // Back from the text with the newlines put in.
            size_t inserted = 0;
            while (inserted < m_wrappedBreaks.size() && m_wrappedBreaks[inserted] + inserted < utf8Offset)
                ++inserted;
            utf8Offset -= inserted;
        }
        if (spans) {
            size_t start = 0;
            for (const Span& span : m_spans) {
                if (utf8Offset < start + span.text.size())
                    return (span.color ? span.color->resolve() : textColor).argb();
                start += span.text.size();
            }
        }
        return textColor.argb();
    };

    SkCanvas* canvas = painter.canvas();
    // A label keeps its colour and fades when disabled. A layer only
    // when faded: glyphs blend themselves otherwise.
    const double opacity = painter.opacity() * (isEnabled() || !m_fadesWhenDisabled ? 1 : 0.6);
    const bool faded = opacity < 1;
    if (faded) {
        // Bounded by the text, with room for overhangs: an unbounded layer
        // is the whole window for every faded label.
        const double margin = pixelSize();
        const double right = std::max(width, geometry().width);
        const double bottom = std::max(geometry().height, offset + textHeight(lines));
        const SkRect bounds = SkRect::MakeLTRB(SkScalar((painter.offset().x - margin) * scale),
                                               SkScalar((painter.offset().y - margin) * scale),
                                               SkScalar((painter.offset().x + right + margin) * scale),
                                               SkScalar((painter.offset().y + bottom + margin) * scale));
        canvas->saveLayerAlphaf(&bounds, float(opacity));
    }
    const double align = m_alignment == TextAlignment::Left ? 0 : m_alignment == TextAlignment::Center ? 0.5 : 1;
    // Glyphs go at the truncated advances, in logical pixels from the label's
    // unsnapped origin; their baselines land on device pixels.
    const auto paintText = [&](SkCanvas* target) {
        target->save();
        target->translate(SkScalar(painter.offset().x * scale), SkScalar(painter.offset().y * scale));
        target->scale(SkScalar(scale), SkScalar(scale));
        paintTextLines(target, text, lines, {shift.x, offset + shift.y}, geometry().width, align, advanceRule(),
                       colorAt, usesSubpixel());
        target->restore();
    };
    // Ganesh draws glyphs this large as unhinted distance fields; the raster
    // backend's hinted masks, drawn aside, keep GPU frames like CPU ones.
    // Underlines, at the font's underline position under each run.
    const auto paintUnderlines = [&](SkCanvas* target) {
        if (!spans)
            return;
        const std::vector<SpanBox> boxes = spanBoxes(text, scale);
        if (boxes.empty())
            return;
        SkFontMetrics metrics;
        SkFont font(context.typeface(m_weight, m_family), SkScalar(pixelSize()));
        font.getMetrics(&metrics);
        if (!metrics.hasUnderlinePosition(&metrics.fUnderlinePosition)
            || !metrics.hasUnderlineThickness(&metrics.fUnderlineThickness)) {
            metrics.fUnderlinePosition = SkScalar(pixelSize() / 10);
            metrics.fUnderlineThickness = SkScalar(pixelSize() / 20);
        }
        const double position = std::ceil(double(metrics.fUnderlinePosition) * scale) / scale;
        const double thickness = std::max(1.0, std::round(double(metrics.fUnderlineThickness) * scale)) / scale;
        for (const SpanBox& box : boxes) {
            const Span& span = m_spans[box.span];
            if (!span.underline && span.link.empty())
                continue;
            SkPaint paint;
            paint.setColor((span.color ? span.color->resolve() : textColor).argb());
            const double y = std::round((painter.offset().y + box.baseline + shift.y + position) * scale);
            target->drawRect(SkRect::MakeLTRB(SkScalar((painter.offset().x + box.left + shift.x) * scale), SkScalar(y),
                                              SkScalar((painter.offset().x + box.right + shift.x) * scale),
                                              SkScalar(y + thickness * scale)),
                             paint);
        }
    };
    if (canvas->recordingContext() && pixelSize() * scale >= kDistanceFieldTextSize) {
        const double margin = pixelSize();
        const double right = std::max(width, geometry().width);
        const double bottom = std::max(geometry().height, offset + textHeight(lines)) + std::abs(shift.y);
        const SkIRect area = SkRect::MakeLTRB(SkScalar((painter.offset().x - margin) * scale),
                                              SkScalar((painter.offset().y - margin) * scale),
                                              SkScalar((painter.offset().x + right + margin) * scale),
                                              SkScalar((painter.offset().y + bottom + margin) * scale))
                                 .roundOut();
        const SkSurfaceProps properties = canvas->getBaseProps();
        const sk_sp<SkSurface> raster = SkSurfaces::Raster(
            SkImageInfo::MakeN32Premul(area.width(), area.height()), &properties);
        if (raster) {
            raster->getCanvas()->clear(SK_ColorTRANSPARENT);
            raster->getCanvas()->translate(SkScalar(-area.x()), SkScalar(-area.y()));
            paintText(raster->getCanvas());
            canvas->drawImage(raster->makeImageSnapshot(), SkScalar(area.x()), SkScalar(area.y()));
        }
    } else {
        paintText(canvas);
    }
    paintUnderlines(canvas);
    if (faded)
        canvas->restore();
}

std::vector<Label::SpanBox> Label::spanBoxes(textlayout::Paragraph* text, double scale) const
{
    std::vector<SpanBox> boxes;
    const bool wrapped = m_wrapped && text == m_wrapped.get() && !m_wrappedElided;
    if (m_spans.empty() || (text != m_paragraph.get() && !wrapped))
        return boxes;
    const bool any = std::any_of(m_spans.begin(), m_spans.end(),
                                 [](const Span& span) { return span.underline || !span.link.empty(); });
    if (!any)
        return boxes;
    // The paragraph's text: a newline before each break of a wrapped one.
    std::string shown = m_text;
    if (wrapped) {
        for (size_t i = m_wrappedBreaks.size(); i-- > 0;)
            shown.insert(m_wrappedBreaks[i], 1, '\n');
    }
    const auto toShown = [&](size_t offset) {
        if (!wrapped)
            return offset;
        size_t inserted = 0;
        while (inserted < m_wrappedBreaks.size() && m_wrappedBreaks[inserted] < offset)
            ++inserted;
        return offset + inserted;
    };
    const double align = m_alignment == TextAlignment::Left ? 0 : m_alignment == TextAlignment::Center ? 0.5 : 1;
    const TextGeometry geometry = textGeometry(text, shown, this->geometry().width, align, advanceRule());
    const std::vector<TextLineBox> lines = lineBoxes(text, scale);
    const double offset = textOffset(textHeight(lines));
    size_t start = 0;
    for (size_t index = 0; index < m_spans.size(); ++index) {
        const Span& span = m_spans[index];
        const size_t end = start + span.text.size();
        if ((span.underline || !span.link.empty()) && end > start) {
            const size_t from = toShown(start);
            const size_t to = toShown(end);
            for (size_t l = 0; l < geometry.lines.size() && l < lines.size(); ++l) {
                const TextGeometry::Line& line = geometry.lines[l];
                const size_t a = std::max(from, line.start);
                const size_t b = std::min(to, line.end);
                if (a >= b)
                    continue;
                SpanBox box;
                box.span = index;
                box.left = geometry.x(a);
                box.right = geometry.x(b);
                box.top = offset + lines[l].top;
                box.baseline = box.top + lines[l].ascent;
                box.height = lines[l].height;
                boxes.push_back(box);
            }
        }
        start = end;
    }
    return boxes;
}

std::optional<Label::LineGeometry> Label::lineGeometry(size_t line) const
{
    double width = geometry().width;
    if (!m_wrap && m_elide == Elide::None) {
        paragraph();
        if (m_layoutWidth < 0)
            layoutParagraph(width);
        width = std::max(width, naturalWidth(m_paragraph.get(), advanceRule()));
    }
    textlayout::Paragraph* text = layoutParagraph(width);
    if (!text || m_text.empty())
        return std::nullopt;
    const std::vector<TextLineBox> lines = lineBoxes(text, currentScale());
    const std::vector<double> widths = lineWidths(text, advanceRule());
    if (line >= lines.size() || line >= widths.size())
        return std::nullopt;
    const double offset = textOffset(textHeight(lines));
    LineGeometry geometry;
    geometry.width = widths[line];
    geometry.top = offset + lines[line].top;
    geometry.baseline = geometry.top + lines[line].ascent;
    geometry.height = lines[line].height;
    return geometry;
}

size_t Label::lineCount() const
{
    double width = geometry().width;
    if (!m_wrap && m_elide == Elide::None) {
        paragraph();
        if (m_layoutWidth < 0)
            layoutParagraph(width);
        width = std::max(width, naturalWidth(m_paragraph.get(), advanceRule()));
    }
    textlayout::Paragraph* text = layoutParagraph(width);
    return text && !m_text.empty() ? text->lineNumber() : 0;
}

std::string Label::linkAt(PointF point) const
{
    if (m_spans.empty())
        return {};
    double width = geometry().width;
    if (!m_wrap && m_elide == Elide::None) {
        paragraph();
        if (m_layoutWidth < 0)
            layoutParagraph(width);
        width = std::max(width, naturalWidth(m_paragraph.get(), advanceRule()));
    }
    textlayout::Paragraph* text = layoutParagraph(width);
    for (const SpanBox& box : spanBoxes(text, currentScale())) {
        if (!m_spans[box.span].link.empty() && point.x >= box.left && point.x < box.right && point.y >= box.top
            && point.y < box.top + box.height)
            return m_spans[box.span].link;
    }
    return {};
}

void Label::invalidate()
{
    m_paragraph.reset();
    m_elided.reset();
    m_wrapped.reset();
    m_wrappedWidth = -1;
    m_previousWrapping.reset();
    requestLayout();
}

std::unique_ptr<textlayout::Paragraph> Label::build(const std::string* text, const std::vector<size_t>* breaks) const
{
    const TextContext& context = Application::instance()->text();
    const Theme& theme = Theme::current();
    const Color color = m_color ? m_color->resolve() : theme.text;
    textlayout::TextStyle style = context.style(m_fontSize, m_weight, color, m_family);
    for (const auto& [feature, value] : m_features)
        style.addFontFeature(SkString(feature.c_str()), value);

    textlayout::ParagraphStyle paragraphStyle;
    paragraphStyle.setTextStyle(style);
    // Widths stay exact, not rounded to hundredths.
    paragraphStyle.setApplyRoundingHack(false);
    // Aligned when painted, by the truncated advances.
    paragraphStyle.setTextAlign(textlayout::TextAlign::kLeft);
    if (!m_wrap) {
        paragraphStyle.setMaxLines(1);
        if (m_elide == Elide::Right)
            paragraphStyle.setEllipsis(SkString(kEllipsis));
    } else if (m_maximumLineCount > 0) {
        paragraphStyle.setMaxLines(size_t(m_maximumLineCount));
        paragraphStyle.setEllipsis(SkString(kEllipsis));
    }

    auto builder = textlayout::ParagraphBuilder::make(paragraphStyle, context.fontCollection(), context.unicode());
    // The part of the text from the offset, with the newlines that fall in it.
    size_t nextBreak = 0;
    const auto withBreaks = [&](const std::string& part, size_t offset) {
        if (!breaks)
            return part;
        std::string out;
        size_t from = 0;
        for (; nextBreak < breaks->size() && (*breaks)[nextBreak] < offset + part.size(); ++nextBreak) {
            const size_t at = (*breaks)[nextBreak] - offset;
            out.append(part, from, at - from);
            out += '\n';
            from = at;
        }
        out.append(part, from, std::string::npos);
        return out;
    };
    if (text || m_spans.empty()) {
        const std::string content = withBreaks(text ? *text : m_text, 0);
        builder->pushStyle(style);
        builder->addText(content.data(), content.size());
        builder->pop();
    } else {
        size_t offset = 0;
        for (const Span& span : m_spans) {
            textlayout::TextStyle spanStyle = style;
            if (span.weight > 0 || span.italic)
                spanStyle.setFontStyle(SkFontStyle(span.weight > 0 ? span.weight : m_weight, SkFontStyle::kNormal_Width,
                                                   span.italic ? SkFontStyle::kItalic_Slant
                                                               : SkFontStyle::kUpright_Slant));
            const std::string content = withBreaks(span.text, offset);
            builder->pushStyle(spanStyle);
            builder->addText(content.data(), content.size());
            builder->pop();
            offset += span.text.size();
        }
    }
    return builder->Build();
}

textlayout::Paragraph* Label::paragraph() const
{
    const bool enabled = isEnabled();
    if (m_paragraph && m_paragraphEnabled == enabled && m_paragraphTheme == Theme::generation())
        return m_paragraph.get();

    m_paragraph = build(nullptr);
    m_paragraphEnabled = enabled;
    m_paragraphTheme = Theme::generation();
    m_layoutWidth = -1;
    m_naturalWidth = -1;
    m_elided.reset();
    m_wrapped.reset();
    m_wrappedWidth = -1;
    m_previousWrapping.reset();
    return m_paragraph.get();
}

void Label::swapWrapping() const
{
    if (!m_previousWrapping)
        m_previousWrapping = std::make_unique<Wrapping>();
    Wrapping& other = *m_previousWrapping;
    std::swap(m_wrapped, other.paragraph);
    std::swap(m_wrappedBreaks, other.breaks);
    std::swap(m_wrappedText, other.text);
    std::swap(m_wrappedWidth, other.width);
    std::swap(m_wrappedRounded, other.rounded);
    std::swap(m_wrappedElided, other.elided);
}

textlayout::Paragraph* Label::wrappedParagraph(double width) const
{
    textlayout::Paragraph* text = paragraph();
    const bool round = roundsAdvances();
    if (m_wrappedWidth == width && m_wrappedRounded == round)
        return m_wrapped ? m_wrapped.get() : text;
    if (m_previousWrapping && m_previousWrapping->width == width && m_previousWrapping->rounded == round) {
        swapWrapping();
        return m_wrapped ? m_wrapped.get() : text;
    }
    // What is in use becomes the previous wrapping; all of it is made anew below.
    if (m_wrappedWidth >= 0)
        swapWrapping();
    if (m_layoutWidth != kUnboundedWidth) {
        text->layout(SkScalar(kUnboundedWidth));
        m_layoutWidth = kUnboundedWidth;
    }
    std::string content = m_text;
    if (!m_spans.empty()) {
        content.clear();
        for (const Span& span : m_spans)
            content += span.text;
    }
    std::vector<size_t> starts =
        lineStarts(text, content, width, m_wrapAnywhere ? LineWrap::Anywhere : LineWrap::WordOrAnywhere,
                     AdvanceRule(round, m_letterSpacing));
    std::optional<std::string> elided;
    if (m_maximumLineCount > 0 && starts.size() > size_t(m_maximumLineCount)) {
        // The last line shown is elided with the next one at the end of it,
        // at the label's width; the rest of the text goes.
        const size_t count = size_t(m_maximumLineCount);
        const size_t last = starts[count - 1];
        const size_t nextEnd = count + 1 < starts.size() ? starts[count + 1] : content.size();
        elided = content.substr(0, last) + elidedText(content, last, nextEnd, width, Elide::Right);
        starts.resize(count);
    }

    m_wrappedWidth = width;
    m_wrappedRounded = round;
    m_wrappedElided = elided.has_value();
    m_wrappedText = elided ? *elided : content;
    // A newline where each line starts, unless one already ends the line before.
    m_wrappedBreaks.clear();
    for (size_t i = 1; i < starts.size(); ++i) {
        if (content[starts[i] - 1] != '\n')
            m_wrappedBreaks.push_back(starts[i]);
    }
    if (m_wrappedBreaks.empty() && !elided) {
        m_wrapped.reset();
        return text;
    }
    m_wrapped = build(elided ? &*elided : nullptr, &m_wrappedBreaks);
    m_wrapped->layout(SkScalar(kUnboundedWidth));
    return m_wrapped.get();
}

// Grapheme boundaries of text[from, to), both ends included.
static std::vector<size_t> graphemeBoundaries(const std::string& text, size_t from, size_t to)
{
    std::vector<size_t> boundaries{from};
    UErrorCode status = U_ZERO_ERROR;
    UText* utext = utext_openUTF8(nullptr, text.data() + from, int64_t(to - from), &status);
    UBreakIterator* iterator = ubrk_open(UBRK_CHARACTER, nullptr, nullptr, 0, &status);
    if (U_SUCCESS(status)) {
        ubrk_setUText(iterator, utext, &status);
        for (int32_t b = ubrk_next(iterator); U_SUCCESS(status) && b != UBRK_DONE; b = ubrk_next(iterator))
            boundaries.push_back(from + size_t(b));
    }
    ubrk_close(iterator);
    utext_close(utext);
    if (boundaries.back() != to)
        boundaries.push_back(to);
    return boundaries;
}

std::string Label::elidedText(const std::string& content, size_t from, size_t to, double width, Elide mode) const
{
    const AdvanceRule rule(roundsAdvances(), m_letterSpacing);
    const TextGeometry geometry = textGeometry(paragraph(), content, kUnboundedWidth, 0, rule);
    std::vector<std::pair<size_t, double>> clusters;
    for (const TextGeometry::Line& line : geometry.lines) {
        for (const auto& stop : line.stops) {
            if (clusters.empty() || stop.first > clusters.back().first)
                clusters.push_back(stop);
        }
    }
    // A range starting inside a cluster (a ligature) skips it, and one ending
    // inside takes all of it, so a ligature counts once, with its first
    // grapheme.
    const auto widthOf = [&clusters](size_t start, size_t end) {
        double left = clusters.back().second;
        double right = clusters.back().second;
        for (const auto& [offset, x] : clusters) {
            if (offset >= start) {
                left = x;
                break;
            }
        }
        for (const auto& [offset, x] : clusters) {
            if (offset >= end) {
                right = x;
                break;
            }
        }
        return std::max(0.0, right - left);
    };
    const std::vector<size_t> graphemes = graphemeBoundaries(content, from, to);
    const size_t count = graphemes.size() - 1;
    if (mode == Elide::None || widthOf(from, content.size()) <= width || count <= 1)
        return content.substr(from, to - from);

    const std::string ellipsisText = kEllipsis;
    std::unique_ptr<textlayout::Paragraph> ellipsis = build(&ellipsisText);
    ellipsis->layout(SkScalar(kUnboundedWidth));
    const double available = width - naturalWidth(ellipsis.get(), rule);
    if (available < 0)
        return {};

    if (mode == Elide::Right) {
        // Graphemes from the start while their sum stays under the room.
        double current = 0;
        size_t pos = 0;
        size_t next = 0;
        do {
            pos = next;
            ++next;
            current += widthOf(graphemes[pos], graphemes[next]);
        } while (next < count && current < available);
        return content.substr(from, graphemes[pos] - from) + kEllipsis;
    }

    // As many graphemes at each end, added in step while both fit.
    double leftWidth = 0;
    double rightWidth = 0;
    size_t left = 0;
    size_t nextLeft = 0;
    size_t right = count;
    size_t nextRight = count;
    do {
        left = nextLeft;
        right = nextRight;
        ++nextLeft;
        --nextRight;
        leftWidth += widthOf(graphemes[left], graphemes[nextLeft]);
        rightWidth += widthOf(graphemes[nextRight], graphemes[right]);
    } while (nextLeft < count && nextRight > 0 && leftWidth + rightWidth < available);
    return content.substr(from, graphemes[left] - from) + kEllipsis
            + content.substr(graphemes[right], to - graphemes[right]);
}

textlayout::Paragraph* Label::layoutParagraph(double width) const
{
    if (m_wrap && width < kUnboundedWidth)
        return wrappedParagraph(width);
    textlayout::Paragraph* text = paragraph();
    const double extra = width < kUnboundedWidth ? slack() + 1.0 / 64 : 0;
    if (width != m_layoutWidth) {
        // Breaks and elides where the truncated advances do.
        text->layout(SkScalar(width + extra));
        m_layoutWidth = width;
    }
    if (m_elide == Elide::None || m_wrap || !m_spans.empty() || m_naturalWidth <= width)
        return text;

    if (m_elided && m_elidedWidth == width)
        return m_elided.get();

    text->layout(SkScalar(kUnboundedWidth));
    m_layoutWidth = kUnboundedWidth;
    const std::string candidate = elidedText(m_text, 0, m_text.size(), width, m_elide);
    text->layout(SkScalar(width + extra));
    m_layoutWidth = width;
    std::unique_ptr<textlayout::Paragraph> best = build(&candidate);
    m_elidedText = candidate;
    best->layout(SkScalar(width + extra));
    m_elided = std::move(best);
    m_elidedWidth = width;
    return m_elided.get();
}

} // namespace Lingmo::ui
