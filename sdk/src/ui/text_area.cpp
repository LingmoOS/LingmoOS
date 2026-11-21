// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/text_area.h"

#include <algorithm>
#include <chrono>
#include <cmath>

#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "modules/skparagraph/include/Paragraph.h"
#include "modules/skparagraph/include/ParagraphBuilder.h"
#include "modules/skparagraph/include/ParagraphStyle.h"

#include "base/event_loop.h"
#include "base/i18n.h"
#include "ui/application.h"
#include "ui/clipboard.h"
#include "ui/editable_text.h"
#include "ui/keys.h"
#include "ui/label.h"
#include "ui/menu.h"
#include "ui/painter.h"
#include "ui/text_context.h"
#include "ui/text_lines.h"
#include "ui/theme.h"
#include "ui/translation.h"
#include "ui/window.h"
namespace base = Lingmo::base;
namespace i18n = Lingmo::I18n;

namespace textlayout = skia::textlayout;

namespace ui = Lingmo::ui;

namespace Lingmo::ui {

// A coarse timer of half the cursor flash time fires up to 5% early.
static constexpr auto kBlinkInterval = std::chrono::milliseconds(475);
// Wide enough that nothing wraps.
static constexpr double kUnboundedWidth = 1e6;
// The background: a field width when the area has no text.
static constexpr double kFrameWidth = 120;
// The surrounding text an input method gets around the cursor.
static constexpr size_t kSurroundingLimit = 4000;

struct TextArea::Private
{
    EditableText edit;
    // The text revision onTextChanged last told of.
    uint64_t reportedRevision = 0;
    std::string placeholder;
    bool readOnly = false;
    WrapMode wrap = WrapMode::WordWrap;
    Margins padding = Margins::all(6);
    bool frame = true;
    double fontSize = 0;
    int fontWeight = 400;
    std::string fontFamily;
    // 0 left, 0.5 centred, 1 right.
    double align = 0;
    bool verticallyCentered = false;
    std::optional<Color> textColor;
    std::optional<Color> selectionColor;
    std::optional<Color> selectedTextColor;
    bool persistentSelection = false;
    std::string preedit;
    int preeditCursor = -1;

    // What is laid out: the text with the preedit at the cursor and a newline
    // where each wrapped line starts.
    std::string display;
    // Display offsets of every text offset, and back.
    std::vector<size_t> toDisplay;
    std::vector<size_t> fromDisplay;
    // The display without wrapping, which the lines are measured on.
    std::unique_ptr<textlayout::Paragraph> unwrapped;
    std::vector<size_t> unwrappedFromDisplay;
    // What is drawn: the unwrapped paragraph when no line wraps.
    std::unique_ptr<textlayout::Paragraph> wrapped;
    textlayout::Paragraph* paragraph = nullptr;
    bool dirty = true;
    bool paragraphEnabled = true;
    uint64_t paragraphTheme = 0;
    double layoutWidth = -1;
    double layoutScale = 0;
    bool layoutHinted = true;
    SizeF unbounded;
    double unboundedScale = 0;
    bool unboundedHinted = true;
    std::vector<TextLineBox> lines;
    std::shared_ptr<const LineGlyphs> glyphs;
    TextGeometry geometry;

    // Logical pixels the text is scrolled up by.
    double scrollY = 0;
    // Where Up and Down aim for, from the left of the text.
    double goalX = -1;
    bool caretVisible = true;
    int blinkTimer = 0;
    bool selecting = false;
    int granularity = 1;
    std::pair<size_t, size_t> anchorRange;
    std::shared_ptr<TextArea*> self;
    std::unique_ptr<Menu> menu;
    Label* placeholderLabel = nullptr;

    double pixelSize() const { return TextContext::pixelSize(fontSize > 0 ? fontSize : Theme::current().fontSize()); }

    // With breaks, a newline before each of those text offsets: the lines
    // the layout made.
    void buildDisplay(const std::vector<size_t>& breaks = {})
    {
        const std::string& text = edit.text();
        const size_t cursor = edit.cursor();
        display.clear();
        toDisplay.assign(text.size() + 1, 0);
        fromDisplay.clear();
        const auto append = [this](std::string_view part, size_t textOffset) {
            for (size_t i = 0; i < part.size(); ++i)
                fromDisplay.push_back(textOffset);
            display += part;
        };
        size_t nextBreak = 0;
        for (size_t i = 0; i <= text.size(); ++i) {
            if (nextBreak < breaks.size() && breaks[nextBreak] == i) {
                append("\n", i);
                ++nextBreak;
            }
            if (i == cursor && !preedit.empty())
                append(preedit, i);
            toDisplay[i] = display.size();
            if (i == text.size())
                break;
            fromDisplay.push_back(i);
            display += text[i];
        }
        fromDisplay.push_back(text.size());
    }

    // Lays the text out in a content width, in logical pixels.
    bool hintedLayout() const
    {
        return (*self)->textLayoutMode().hinted;
    }

    std::unique_ptr<textlayout::Paragraph> make(bool enabled) const
    {
        const TextContext& context = Application::instance()->text();
        const Theme& theme = Theme::current();
        const textlayout::TextStyle style =
            context.style(fontSize, fontWeight, enabled ? theme.text : theme.disabledText, fontFamily);
        textlayout::ParagraphStyle paragraphStyle;
        paragraphStyle.setTextStyle(style);
        paragraphStyle.setApplyRoundingHack(false);
        auto builder = textlayout::ParagraphBuilder::make(paragraphStyle, context.fontCollection(), context.unicode());
        builder->pushStyle(style);
        // An empty last line still takes a line.
        builder->addText(display.data(), display.size());
        builder->pop();
        std::unique_ptr<textlayout::Paragraph> built = builder->Build();
        built->layout(SkScalar(kUnboundedWidth));
        return built;
    }

    // The text without wrapping, shaped once per change.
    void shape(bool enabled)
    {
        if (unwrapped && !dirty && paragraphEnabled == enabled && paragraphTheme == Theme::generation())
            return;
        buildDisplay();
        unwrapped = make(enabled);
        unwrappedFromDisplay = fromDisplay;
        wrapped.reset();
        paragraph = nullptr;
        dirty = false;
        paragraphEnabled = enabled;
        paragraphTheme = Theme::generation();
        layoutWidth = -1;
        layoutScale = 0;
        unboundedScale = 0;
    }

    // The size of the text laid out at an unbounded width, without changing
    // the layout at the area's width.
    SizeF unboundedSize(double scale, bool enabled)
    {
        shape(enabled);
        const bool hinted = hintedLayout();
        if (unboundedScale == scale && unboundedHinted == hinted)
            return unbounded;
        const bool round = Application::instance()->text().roundsMetrics(scale, hinted);
        const double width = naturalWidth(unwrapped.get(), round);
        // A line near the unbounded width may wrap even there, by its overhang or spaces.
        if (wrap != WrapMode::NoWrap && width > kUnboundedWidth / 2) {
            layout(kUnboundedWidth, scale, enabled);
            return {naturalWidth(*glyphs), textHeight()};
        }
        const TextContext& context = Application::instance()->text();
        const std::vector<TextLineBox> boxes =
            textLineBoxes(unwrapped.get(), context.typeface(fontWeight, fontFamily).get(), pixelSize(), scale, hinted);
        unbounded = {width, ui::textHeight(boxes)};
        unboundedScale = scale;
        unboundedHinted = hinted;
        return unbounded;
    }

    void layout(double width, double scale, bool enabled)
    {
        shape(enabled);
        const bool hinted = hintedLayout();
        const bool round = Application::instance()->text().roundsMetrics(scale, hinted);
        if (layoutWidth == width && layoutScale == scale && layoutHinted == hinted)
            return;
        // The lines made on the truncated advances, as newlines in the text.
        std::vector<size_t> breaks;
        if (wrap != WrapMode::NoWrap) {
            buildDisplay();
            const LineWrap mode = wrap == WrapMode::WordWrap       ? LineWrap::Word
                                : wrap == WrapMode::WrapAnywhere ? LineWrap::Anywhere
                                                                 : LineWrap::WordOrAnywhere;
            const std::vector<size_t> starts = lineStarts(unwrapped.get(), display, width, mode, round);
            for (size_t i = 1; i < starts.size(); ++i) {
                const size_t offset = unwrappedFromDisplay[starts[i]];
                if (display[starts[i] - 1] != '\n' && (breaks.empty() || breaks.back() != offset))
                    breaks.push_back(offset);
            }
        }
        buildDisplay(breaks);
        if (breaks.empty()) {
            wrapped.reset();
            paragraph = unwrapped.get();
        } else {
            wrapped = make(enabled);
            paragraph = wrapped.get();
        }
        layoutWidth = width;
        layoutScale = scale;
        layoutHinted = hinted;
        const TextContext& context = Application::instance()->text();
        lines = textLineBoxes(paragraph, context.typeface(fontWeight, fontFamily).get(), pixelSize(), scale,
                              hinted);
        glyphs = lineGlyphs(paragraph, round);
        geometry = textGeometry(paragraph, *glyphs, display, width, align);
    }

    double textHeight() const { return ui::textHeight(lines); }
    // Down by this when the text is shorter than the area and centred in it.
    double centring(double contentHeight) const
    {
        return verticallyCentered ? std::max(0.0, (contentHeight - textHeight()) / 2) : 0;
    }

    size_t displayOffset(size_t offset) const { return toDisplay[std::min(offset, toDisplay.size() - 1)]; }
    size_t textOffset(size_t offset) const { return fromDisplay[std::min(offset, fromDisplay.size() - 1)]; }

    // The display offset where the caret shows.
    size_t caretDisplayOffset() const
    {
        const size_t cursor = displayOffset(edit.cursor());
        if (!preedit.empty() && preeditCursor >= 0)
            return cursor - preedit.size() + std::min(size_t(preeditCursor), preedit.size());
        return cursor;
    }

    const TextLineBox& lineBox(size_t line) const { return lines[std::min(line, lines.size() - 1)]; }
};

TextArea::TextArea(std::string text)
    : d(std::make_unique<Private>())
{
    d->self = std::make_shared<TextArea*>(this);
    d->edit.setText(std::move(text));
    d->reportedRevision = d->edit.revision();
    d->placeholderLabel = addChild<Label>();
    d->placeholderLabel->setVisible(false);
}

TextArea::~TextArea()
{
    if (d->blinkTimer)
        Application::instance()->loop().stopTimer(d->blinkTimer);
}

const std::string& TextArea::text() const
{
    return d->edit.text();
}

void TextArea::setText(std::string text)
{
    if (text == d->edit.text())
        return;
    d->edit.setText(std::move(text));
    d->preedit.clear();
    d->dirty = true;
    requestLayout();
    edited();
}

void TextArea::setPlaceholder(std::string placeholder)
{
    if (placeholder == d->placeholder)
        return;
    d->placeholder = std::move(placeholder);
    d->placeholderLabel->setText(d->placeholder);
    d->placeholderLabel->setVisible(!d->placeholder.empty() && d->edit.text().empty() && d->preedit.empty());
    requestLayout();
}

void TextArea::layout()
{
    const RectF bounds = localRect();
    d->placeholderLabel->setColor(Theme::current().disabledText);
    d->placeholderLabel->setGeometry({d->padding.left, d->padding.top,
                                      std::max(0.0, bounds.width - d->padding.horizontal()),
                                      std::max(0.0, bounds.height - d->padding.vertical())});
}

void TextArea::setReadOnly(bool readOnly)
{
    d->readOnly = readOnly;
    updateTextInput();
    requestPaint();
}

bool TextArea::isReadOnly() const
{
    return d->readOnly;
}

void TextArea::setWrapMode(WrapMode mode)
{
    if (mode == d->wrap)
        return;
    d->wrap = mode;
    d->dirty = true;
    requestLayout();
}

void TextArea::setPadding(const Margins& padding)
{
    if (padding == d->padding)
        return;
    d->padding = padding;
    requestLayout();
}

void TextArea::setFrameVisible(bool visible)
{
    if (visible == d->frame)
        return;
    d->frame = visible;
    requestLayout();
}

void TextArea::setFontSize(double size)
{
    if (size == d->fontSize)
        return;
    d->fontSize = size;
    d->placeholderLabel->setFontSize(size);
    d->dirty = true;
    requestLayout();
}

void TextArea::setFont(const std::string& family, double pixelSize, int weight)
{
    if (family == d->fontFamily && pixelSize == d->fontSize && weight == d->fontWeight)
        return;
    d->fontFamily = family;
    d->fontSize = pixelSize;
    d->fontWeight = weight;
    d->placeholderLabel->setFontSize(pixelSize);
    d->placeholderLabel->setFontFamily(family);
    d->placeholderLabel->setWeight(weight);
    d->dirty = true;
    requestLayout();
}

void TextArea::setHorizontalAlignment(HorizontalAlignment alignment)
{
    d->align = alignment == HorizontalAlignment::Center ? 0.5 : alignment == HorizontalAlignment::Right ? 1 : 0;
    d->layoutWidth = -1;
    requestPaint();
}

void TextArea::setVerticallyCentered(bool centered)
{
    d->verticallyCentered = centered;
    requestPaint();
}

void TextArea::setTextColor(std::optional<Color> color)
{
    d->textColor = color;
    requestPaint();
}

void TextArea::setSelectionColors(std::optional<Color> background, std::optional<Color> text)
{
    d->selectionColor = background;
    d->selectedTextColor = text;
    requestPaint();
}

void TextArea::setPersistentSelection(bool persistent)
{
    d->persistentSelection = persistent;
    requestPaint();
}

size_t TextArea::cursorPosition() const
{
    return d->edit.cursor();
}

void TextArea::setCursorPosition(size_t position)
{
    d->edit.setCursor(position);
    d->goalX = -1;
    moved();
}

void TextArea::select(size_t anchor, size_t cursor)
{
    d->edit.select(anchor, cursor);
    d->goalX = -1;
    moved();
}

void TextArea::selectAll()
{
    d->edit.selectAll();
    moved();
}

bool TextArea::hasSelection() const
{
    return d->edit.hasSelection();
}

std::string TextArea::selectedText() const
{
    return d->edit.selectedText();
}

const std::string& TextArea::preedit() const
{
    return d->preedit;
}

void TextArea::cut()
{
    if (d->readOnly || !d->edit.hasSelection())
        return;
    copy();
    d->edit.deleteBackward(false);
    edited();
}

void TextArea::copy()
{
    if (d->edit.hasSelection())
        Clipboard::setText(d->edit.selectedText());
}

void TextArea::paste()
{
    if (d->readOnly)
        return;
    Clipboard::requestText([self = std::weak_ptr<TextArea*>(d->self)](std::string text) {
        const auto area = self.lock();
        if (!area || text.empty())
            return;
        std::erase(text, '\r');
        (*area)->d->edit.insert(text);
        (*area)->edited();
    });
}

void TextArea::undo()
{
    if (d->readOnly || !d->edit.canUndo())
        return;
    d->edit.undo();
    edited();
}

void TextArea::redo()
{
    if (d->readOnly || !d->edit.canRedo())
        return;
    d->edit.redo();
    edited();
}

static double currentScale(const View* view)
{
    return view->textLayoutMode().scale;
}

SizeF TextArea::sizeHint() const
{
    // T.TextArea: the content, the background's width and the placeholder,
    // each with the padding.
    const SizeF text = d->unboundedSize(currentScale(this), isEnabled());
    double width = text.width + d->padding.horizontal();
    if (d->frame)
        width = std::max(width, kFrameWidth);
    double height = text.height;
    if (!d->placeholder.empty()) {
        const SizeF placeholder = d->placeholderLabel->sizeHint();
        width = std::max(width, placeholder.width + d->padding.horizontal());
        height = std::max(height, placeholder.height);
    }
    return {width, height + d->padding.vertical()};
}

double TextArea::heightForWidth(double width) const
{
    d->layout(std::max(0.0, width - d->padding.horizontal()), currentScale(this), isEnabled());
    return d->textHeight() + d->padding.vertical();
}

void TextArea::paint(Painter& painter)
{
    const Theme& theme = Theme::current();
    const double scale = painter.scale();
    const RectF bounds = localRect();
    const RectF content = {d->padding.left, d->padding.top, std::max(0.0, bounds.width - d->padding.horizontal()),
                           std::max(0.0, bounds.height - d->padding.vertical())};
    const bool focused = hasFocus();
    const bool enabled = isEnabled();

    if (d->frame) {
        // The background: a line half the bottom padding above the
        // bottom, thicker with focus.
        const double height = focused ? 2 : 1;
        painter.fillRoundedRect({0, bounds.height - height - d->padding.bottom / 2, bounds.width, height}, 0,
                                theme.background);
    }

    d->layout(content.width, scale, enabled);
    const size_t caret = d->caretDisplayOffset();
    const size_t caretLine = d->geometry.lineOf(caret);
    // Keep the caret's line in sight.
    const double maximum = std::max(0.0, d->textHeight() - content.height);
    if (focused) {
        const TextLineBox& box = d->lineBox(caretLine);
        if (box.top - d->scrollY < 0)
            d->scrollY = box.top;
        if (box.top + box.height - d->scrollY > content.height)
            d->scrollY = box.top + box.height - content.height;
    }
    d->scrollY = std::clamp(d->scrollY, 0.0, maximum);

    const PointF offset = painter.offset();
    const PointF origin = {content.x, content.y - d->scrollY + d->centring(content.height)};
    const auto device = [scale, offset, origin](double x, double y) {
        return SkPoint::Make(SkScalar(std::round((offset.x + origin.x + x) * scale)),
                             SkScalar(std::round((offset.y + origin.y + y) * scale)));
    };
    SkCanvas* canvas = painter.canvas();
    painter.save();
    painter.clip(bounds);

    // The selection's rects, line by line.
    std::vector<SkRect> selection;
    if ((focused || d->persistentSelection) && d->preedit.empty() && d->edit.hasSelection()) {
        const size_t start = d->displayOffset(d->edit.selectionStart());
        const size_t end = d->displayOffset(d->edit.selectionEnd());
        for (size_t l = 0; l < d->geometry.lines.size() && l < d->lines.size(); ++l) {
            const TextGeometry::Line& line = d->geometry.lines[l];
            const size_t from = std::max(start, line.start);
            const size_t to = std::min(end, line.end);
            if (from > to || (from == to && end <= line.end))
                continue;
            const double left = d->geometry.x(from);
            double right = to == line.end ? line.stops.back().second : d->geometry.x(to);
            // A selected line break shows as a sliver.
            if (end > line.end && to == line.end)
                right += std::max(1.0, d->pixelSize() / 4);
            const SkPoint topLeft = device(left, d->lines[l].top);
            const SkPoint bottomRight = device(right, d->lines[l].top + d->lines[l].height);
            selection.push_back(SkRect::MakeLTRB(topLeft.fX, topLeft.fY, bottomRight.fX, bottomRight.fY));
        }
        SkPaint paint;
        paint.setColor(d->selectionColor.value_or(theme.highlight).argb());
        for (const SkRect& rect : selection)
            canvas->drawRect(rect, paint);
    }

    // The text, and again in the highlighted text colour over the selection.
    const Color color = enabled ? d->textColor.value_or(theme.text) : theme.disabledText;
    const auto draw = [&](Color textColor) {
        canvas->save();
        canvas->translate(SkScalar(offset.x * scale), SkScalar(offset.y * scale));
        canvas->scale(SkScalar(scale), SkScalar(scale));
        paintTextLines(canvas, *d->glyphs, d->lines, origin, content.width, d->align,
                       [textColor](size_t) { return textColor.argb(); });
        canvas->restore();
    };
    draw(color);
    if (!selection.empty()) {
        canvas->save();
        for (const SkRect& rect : selection) {
            canvas->save();
            canvas->clipRect(rect);
            draw(d->selectedTextColor.value_or(theme.highlightedText));
            canvas->restore();
        }
        canvas->restore();
    }

    // The input method's text is underlined.
    if (!d->preedit.empty()) {
        const size_t start = d->displayOffset(d->edit.cursor()) - d->preedit.size();
        const size_t end = start + d->preedit.size();
        const size_t line = d->geometry.lineOf(start);
        const TextLineBox& box = d->lineBox(line);
        const SkPoint left = device(d->geometry.x(start), box.top + box.ascent);
        const SkPoint right = device(d->geometry.x(std::min(end, d->geometry.lines[line].end)), box.top + box.ascent);
        SkPaint paint;
        paint.setColor(color.argb());
        canvas->drawRect(SkRect::MakeLTRB(left.fX, left.fY + 1, right.fX, left.fY + 1 + std::max(1.0f, SkScalar(std::round(scale)))), paint);
    }

    const Window* window = this->window();
    if (focused && window && window->hasKeyboardFocus() && !d->readOnly && !d->blinkTimer)
        startBlinking();
    if (focused && window && window->hasKeyboardFocus() && d->caretVisible && !d->readOnly) {
        const TextLineBox& box = d->lineBox(caretLine);
        const SkPoint top = device(d->geometry.x(caret), box.top);
        const SkPoint bottom = device(d->geometry.x(caret), box.top + box.height);
        SkPaint paint;
        paint.setColor(d->textColor.value_or(theme.text).argb());
        canvas->drawRect(SkRect::MakeXYWH(top.fX, top.fY, SkScalar(std::max(1.0, std::round(scale))), bottom.fY - top.fY),
                         paint);
    }
    painter.restore();
}

size_t TextArea::offsetAt(PointF position) const
{
    const double scale = currentScale(this);
    const double width = std::max(0.0, geometry().width - d->padding.horizontal());
    d->layout(width, scale, isEnabled());
    const double contentHeight = std::max(0.0, geometry().height - d->padding.vertical());
    const double y = position.y - d->padding.top + d->scrollY - d->centring(contentHeight);
    size_t line = 0;
    while (line + 1 < d->lines.size() && y >= d->lines[line].top + d->lines[line].height)
        ++line;
    const size_t offset = d->geometry.offsetAt(line, position.x - d->padding.left);
    return d->textOffset(offset);
}

void TextArea::pointerPress(const PointerEvent& event)
{
    if (event.button == MouseButton::Right) {
        showContextMenu(event.position);
        return;
    }
    if (event.button != MouseButton::Left)
        return;
    if (!d->preedit.empty()) {
        d->preedit.clear();
        d->dirty = true;
    }
    const size_t offset = offsetAt(event.position);
    d->selecting = true;
    d->granularity = std::clamp(event.clickCount, 1, 3);
    if (d->granularity == 3) {
        // The paragraph between line breaks.
        const std::string& text = d->edit.text();
        const size_t start = offset == 0 ? 0 : text.rfind('\n', offset - 1);
        const size_t end = text.find('\n', offset);
        d->anchorRange = {start == std::string::npos || offset == 0 ? 0 : start + 1,
                          end == std::string::npos ? text.size() : end};
        d->edit.select(d->anchorRange.first, d->anchorRange.second);
    } else if (d->granularity == 2) {
        d->anchorRange = d->edit.wordAt(offset);
        d->edit.select(d->anchorRange.first, d->anchorRange.second);
    } else {
        d->edit.setCursor(offset, event.modifiers & ShiftModifier);
    }
    d->goalX = -1;
    moved();
}

void TextArea::pointerMove(const PointerEvent& event)
{
    if (!d->selecting)
        return;
    const size_t offset = offsetAt(event.position);
    if (d->granularity == 1) {
        d->edit.setCursor(offset, true);
    } else {
        const auto range = d->granularity == 2 ? d->edit.wordAt(offset) : std::make_pair(offset, offset);
        if (offset < d->anchorRange.first)
            d->edit.select(d->anchorRange.second, range.first);
        else
            d->edit.select(d->anchorRange.first, std::max(range.second, d->anchorRange.second));
    }
    moved();
}

void TextArea::pointerRelease(const PointerEvent& event)
{
    const bool selected = d->selecting;
    d->selecting = false;
    // A mouse selection goes to the primary selection, and a
    // middle click pastes it where it lands.
    if (event.button == MouseButton::Left && selected) {
        if (d->edit.hasSelection())
            Clipboard::setText(d->edit.selectedText(), ClipboardMode::Selection);
    } else if (event.button == MouseButton::Middle && !d->readOnly) {
        d->edit.setCursor(offsetAt(event.position));
        moved();
        Clipboard::requestText([self = std::weak_ptr<TextArea*>(d->self)](std::string text) {
            const auto area = self.lock();
            if (!area || text.empty())
                return;
            std::erase(text, '\r');
            (*area)->d->edit.insert(text);
            (*area)->edited();
        }, ClipboardMode::Selection);
    }
}

void TextArea::focusIn(FocusReason)
{
    moved();
}

void TextArea::focusOut()
{
    if (d->blinkTimer) {
        Application::instance()->loop().stopTimer(d->blinkTimer);
        d->blinkTimer = 0;
    }
    d->edit.setCursor(d->edit.cursor());
    d->preedit.clear();
    d->dirty = true;
    requestPaint();
}

bool TextArea::keyPress(const KeyEvent& event)
{
    const uint32_t modifiers = event.modifiers;
    const bool shift = modifiers & ShiftModifier;
    const bool control = modifiers & ControlModifier;
    const bool command = modifiers & (ControlModifier | AltModifier | SuperModifier);
    EditableText& edit = d->edit;

    const auto move = [this, &edit, shift](size_t position, bool keepGoal = false) {
        edit.setCursor(position, shift);
        if (!keepGoal)
            d->goalX = -1;
        moved();
        return true;
    };
    // The offset on another line nearest to where Up and Down aim.
    const auto vertical = [this, &move](int lines) {
        const double width = std::max(0.0, geometry().width - d->padding.horizontal());
        d->layout(width, currentScale(this), isEnabled());
        const size_t caret = d->displayOffset(d->edit.cursor());
        const size_t line = d->geometry.lineOf(caret);
        if (d->goalX < 0)
            d->goalX = d->geometry.x(caret);
        const long target = long(line) + lines;
        if (target < 0)
            return move(0, true);
        if (size_t(target) >= d->geometry.lines.size())
            return move(d->edit.text().size(), true);
        return move(d->textOffset(d->geometry.offsetAt(size_t(target), d->goalX)), true);
    };
    const auto lineEdge = [this](bool end) {
        const double width = std::max(0.0, geometry().width - d->padding.horizontal());
        d->layout(width, currentScale(this), isEnabled());
        const TextGeometry::Line& line = d->geometry.lines[d->geometry.lineOf(d->displayOffset(d->edit.cursor()))];
        return d->textOffset(end ? line.end : line.start);
    };

    if (control && !(modifiers & (AltModifier | SuperModifier))) {
        switch (event.key) {
        case key::letter('a'):
            selectAll();
            return true;
        case key::letter('c'):
            copy();
            return true;
        case key::letter('x'):
            cut();
            return true;
        case key::letter('v'):
            paste();
            return true;
        case key::letter('z'):
            shift ? redo() : undo();
            return true;
        case key::letter('y'):
            redo();
            return true;
        case key::Home:
            return move(0);
        case key::End:
            return move(edit.text().size());
        default:
            break;
        }
    }

    switch (event.key) {
    case key::Left:
        if (!shift && edit.hasSelection() && !control)
            return move(edit.selectionStart());
        return move(control ? edit.previousWordStart(edit.cursor()) : edit.previousGrapheme(edit.cursor()));
    case key::Right:
        if (!shift && edit.hasSelection() && !control)
            return move(edit.selectionEnd());
        return move(control ? edit.nextWordEnd(edit.cursor()) : edit.nextGrapheme(edit.cursor()));
    case key::Up:
        return vertical(-1);
    case key::Down:
        return vertical(1);
    case key::PageUp:
    case key::PageDown: {
        const double height = std::max(1.0, geometry().height - d->padding.vertical());
        const double lineHeight = d->lines.empty() ? 1 : d->lines.front().height;
        const int lines = std::max(1, int(height / lineHeight));
        return vertical(event.key == key::PageUp ? -lines : lines);
    }
    case key::Home:
        return move(lineEdge(false));
    case key::End:
        return move(lineEdge(true));
    case key::BackSpace:
        if (d->readOnly)
            return true;
        edit.deleteBackward(control);
        edited();
        return true;
    case key::Delete:
        if (d->readOnly)
            return true;
        edit.deleteForward(control);
        edited();
        return true;
    case key::Return:
    case key::KeypadEnter:
        if (d->readOnly)
            return false;
        edit.insert("\n", true);
        edited();
        return true;
    case key::Tab:
        // Moves the focus, as areas in forms do.
        return false;
    default:
        break;
    }

    if (!event.text.empty() && !command) {
        if (!d->readOnly) {
            edit.insert(event.text, true);
            edited();
        }
        return true;
    }
    return false;
}

bool TextArea::scroll(const ScrollEvent& event)
{
    const double height = std::max(0.0, geometry().height - d->padding.vertical());
    const double maximum = std::max(0.0, d->textHeight() - height);
    if (maximum <= 0 || event.deltaY == 0)
        return false;
    const double scrolled = std::clamp(d->scrollY + event.deltaY, 0.0, maximum);
    if (scrolled == d->scrollY)
        return false;
    d->scrollY = scrolled;
    requestPaint();
    return true;
}

bool TextArea::acceptsTextInput() const
{
    return !d->readOnly && isEnabled();
}

TextInputState TextArea::textInputState() const
{
    TextInputState state;
    const std::string& text = d->edit.text();
    // A window of the text around the cursor, cut on character boundaries.
    size_t start = d->edit.selectionStart() > kSurroundingLimit / 2 ? d->edit.selectionStart() - kSurroundingLimit / 2 : 0;
    while (start > 0 && (static_cast<unsigned char>(text[start]) & 0xc0) == 0x80)
        --start;
    size_t end = std::min(text.size(), start + kSurroundingLimit);
    while (end < text.size() && (static_cast<unsigned char>(text[end]) & 0xc0) == 0x80)
        --end;
    state.surroundingText = text.substr(start, end - start);
    state.cursor = d->edit.cursor() - start;
    state.anchor = d->edit.anchor() - start;

    const double width = std::max(0.0, geometry().width - d->padding.horizontal());
    d->layout(width, currentScale(this), isEnabled());
    const size_t caret = d->caretDisplayOffset();
    const TextLineBox& box = d->lineBox(d->geometry.lineOf(caret));
    const PointF topLeft = mapToWindow({d->padding.left + d->geometry.x(caret), d->padding.top + box.top - d->scrollY});
    state.cursorRectangle = {topLeft.x, topLeft.y, 1, box.height};
    return state;
}

void TextArea::textInput(const TextInputEvent& event)
{
    EditableText& edit = d->edit;
    d->preedit.clear();
    if (event.deleteBefore || event.deleteAfter) {
        const size_t cursor = edit.cursor();
        const size_t start = cursor > event.deleteBefore ? cursor - event.deleteBefore : 0;
        edit.erase(start, cursor + event.deleteAfter);
    }
    if (!event.commit.empty())
        edit.insert(event.commit, true);
    d->preedit = event.preedit;
    d->preeditCursor = event.preeditCursorBegin;
    edited();
}

void TextArea::showContextMenu(PointF position)
{
    // The text menu, without the rows a read-only area has no use for.
    const bool editable = !d->readOnly;
    const bool selection = d->edit.hasSelection();
    d->menu = std::make_unique<Menu>();
    Menu& menu = *d->menu;
    const auto addEditing = [&menu, editable](std::string text, std::function<void()> action, bool enabled) {
        const size_t index = menu.addItem(std::move(text), std::move(action));
        menu.setItemEnabled(index, enabled);
        menu.setItemVisible(index, editable);
    };
    addEditing(tr("Undo"), [this] { undo(); }, d->edit.canUndo());
    addEditing(tr("Redo"), [this] { redo(); }, d->edit.canRedo());
    if (editable)
        menu.addSeparator();
    addEditing(tr("Cut"), [this] { cut(); }, selection);
    const size_t copyIndex = menu.addItem(tr("Copy"), [this] { copy(); });
    menu.setItemEnabled(copyIndex, selection);
    addEditing(tr("Paste"), [this] { paste(); }, true);
    addEditing(tr("Delete"), [this] {
        d->edit.deleteBackward(false);
        edited();
    }, selection);
    menu.addSeparator();
    const size_t selectAllIndex = menu.addItem(tr("Select All"), [this] { selectAll(); });
    menu.setItemEnabled(selectAllIndex, !d->edit.text().empty());
    menu.popup(window(), mapToWindow(position));
}

void TextArea::edited()
{
    d->placeholderLabel->setVisible(!d->placeholder.empty() && d->edit.text().empty() && d->preedit.empty());
    d->dirty = true;
    d->goalX = -1;
    requestLayout();
    moved();
    // A key that deletes nothing, or the input method's composing alone.
    if (d->edit.revision() == d->reportedRevision)
        return;
    d->reportedRevision = d->edit.revision();
    if (onTextChanged)
        onTextChanged();
}

void TextArea::moved()
{
    // The preedit shows at the cursor; the text alone does not move with it.
    if (!d->preedit.empty())
        d->dirty = true;
    d->caretVisible = true;
    auto& loop = Application::instance()->loop();
    if (d->blinkTimer)
        loop.stopTimer(d->blinkTimer);
    d->blinkTimer = 0;
    if (hasFocus() && !d->readOnly)
        startBlinking();
    requestPaint();
    updateTextInput();
}

void TextArea::startBlinking()
{
    d->blinkTimer = Application::instance()->loop().startTimer(kBlinkInterval, true, [this] {
        // Without the keyboard no caret is drawn; the next paint with it
        // starts blinking again.
        const Window* window = this->window();
        if (!window || !window->hasKeyboardFocus()) {
            Application::instance()->loop().stopTimer(d->blinkTimer);
            d->blinkTimer = 0;
            d->caretVisible = true;
            return;
        }
        d->caretVisible = !d->caretVisible;
        requestPaint();
    });
}

} // namespace Lingmo::ui
