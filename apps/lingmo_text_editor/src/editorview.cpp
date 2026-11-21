// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "editorview.h"
#include "unicode.h"

#include "base/strings.h"
#include "base/url.h"
#include "ui/clipboard.h"
#include "ui/drag.h"
#include "ui/keys.h"
#include "ui/painter.h"
#include "ui/scroll_bar.h"
#include "ui/window.h"

#include <algorithm>
#include <cmath>
#include <unordered_set>

using namespace Lingmo::ui;
using namespace std::chrono_literals;

// A line longer than this is laid out in pieces, and always wrapped.
static constexpr int segmentLength = 1024;
// The pieces' size: each layout has a fixed cost, which fewer, larger ones save.
static constexpr int pieceLength = 4096;
static constexpr double topPadding = 6;
static constexpr double bottomPadding = 12;
// The caret stops blinking after this long without typing, so an idle window
// does not keep waking up.
static constexpr int64_t blinkDuration = 10000;
// One blink: shown and hidden for half of it each.
static constexpr int cursorFlashTime = 1000;

void VisualLineIndex::build(const std::vector<int> &counts)
{
    m_size = int(counts.size());
    m_tree.assign(size_t(m_size) + 1, 0);
    m_total = 0;
    for (int i = 0; i < m_size; ++i) {
        m_total += counts[size_t(i)];
        m_tree[size_t(i) + 1] += counts[size_t(i)];
        const int parent = (i + 1) + ((i + 1) & -(i + 1));
        if (parent <= m_size)
            m_tree[size_t(parent)] += m_tree[size_t(i) + 1];
    }
}

void VisualLineIndex::add(int line, int delta)
{
    m_total += delta;
    for (int i = line + 1; i <= m_size; i += i & -i)
        m_tree[size_t(i)] += delta;
}

int64_t VisualLineIndex::before(int line) const
{
    int64_t sum = 0;
    for (int i = std::min(line, m_size); i > 0; i -= i & -i)
        sum += m_tree[size_t(i)];
    return sum;
}

int VisualLineIndex::lineAt(int64_t visual, int *offset) const
{
    if (m_size == 0) {
        *offset = 0;
        return 0;
    }
    if (visual >= m_total) {
        const int last = m_size - 1;
        *offset = int(std::max<int64_t>(0, m_total - before(last) - 1));
        return last;
    }
    visual = std::max<int64_t>(0, visual);
    int position = 0;
    int64_t remaining = visual;
    int step = 1;
    while (step * 2 <= m_size)
        step *= 2;
    for (; step > 0; step /= 2) {
        const int next = position + step;
        if (next <= m_size && m_tree[size_t(next)] <= remaining) {
            position = next;
            remaining -= m_tree[size_t(next)];
        }
    }
    *offset = int(remaining);
    return position;
}

static RectF united(const RectF &a, const RectF &b)
{
    const double left = std::min(a.x, b.x);
    const double top = std::min(a.y, b.y);
    return {left, top, std::max(a.right(), b.right()) - left, std::max(a.bottom(), b.bottom()) - top};
}

static int digitCount(int value)
{
    int digits = 1;
    while (value >= 10) {
        value /= 10;
        ++digits;
    }
    return digits;
}

// Connects the view to its scroller.
class EditorView::ScrollTarget final : public Scroller::Target
{
public:
    explicit ScrollTarget(EditorView *view)
        : m_view(view)
    {
    }

    Scroller::Axis horizontalAxis() const override
    {
        const double viewport = m_view->geometry().width;
        return {0, std::max(0.0, m_view->m_contentWidth - viewport), viewport,
                !m_view->m_wordWrap && m_view->m_contentWidth > viewport};
    }

    Scroller::Axis verticalAxis() const override
    {
        const double viewport = m_view->geometry().height;
        return {0, std::max(0.0, m_view->m_contentHeight - viewport), viewport, m_view->m_contentHeight > viewport};
    }

    PointF position() const override { return {m_view->m_contentX, m_view->m_contentY}; }

    void applyPosition(PointF position) override { m_view->setContentPosition(position.x, position.y); }

    void movingChanged(bool moving) override
    {
        m_view->m_verticalBar->setMoving(moving);
        m_view->m_horizontalBar->setMoving(moving);
    }

private:
    EditorView *m_view;
};

EditorView::EditorView()
    : m_scrollTarget(std::make_unique<ScrollTarget>(this))
    , m_verticalBar(addChild<ScrollBar>(ScrollBar::Orientation::Vertical))
    , m_horizontalBar(addChild<ScrollBar>(ScrollBar::Orientation::Horizontal))
{
    setName("editorView");
    setCursorShape(CursorShape::Text);
    m_verticalBar->onPositionRequested = [this](double y) {
        if (m_scroller)
            m_scroller->cancel(false);
        setContentPosition(m_contentX, y);
    };
    m_horizontalBar->onPositionRequested = [this](double x) {
        if (m_scroller)
            m_scroller->cancel(false);
        setContentPosition(x, m_contentY);
    };

    // Exact wrapped heights of lines out of view, a slice at a time, so the
    // scroll bar settles without a pause on a large file.
    m_exactTimer.setSingleShot(true);
    m_exactTimer.setInterval(0ms);
    m_exactTimer.onTimeout = [this] { computeExactHeights(); };

    m_primaryTimer.setSingleShot(true);
    m_primaryTimer.setInterval(150ms);
    m_primaryTimer.onTimeout = [this] { updatePrimarySelection(); };

    m_blinkTimer.setInterval(std::chrono::milliseconds(cursorFlashTime / 2));
    m_blinkTimer.onTimeout = [this] {
        if (m_blinkStarted.elapsedMs() > blinkDuration) {
            m_blinkTimer.stop();
            m_caretVisible = true;
        } else {
            m_caretVisible = !m_caretVisible;
        }
        requestPaint();
    };

    m_autoScrollTimer.setInterval(30ms);
    m_autoScrollTimer.onTimeout = [this] { autoScroll(); };

    m_font = FontSpec{"monospace", 10};
    updateTheme();
    updateMetrics();
}

EditorView::~EditorView() = default;

void EditorView::setDocument(Document *document)
{
    if (m_document == document)
        return;
    m_connections.clear();
    m_document = document;
    m_preedit.clear();
    m_preferredX = -1;
    if (m_document) {
        m_connections.push_back(m_document->linesChanged.connect(
                [this](int line, int removed, int added) { onLinesChanged(line, removed, added); }));
        m_connections.push_back(m_document->syntaxChanged.connect([this](int first, int last) { onSyntaxChanged(first, last); }));
        m_connections.push_back(m_document->cursorChanged.connect([this] { onCursorChanged(); }));
        m_connections.push_back(m_document->revealCursor.connect([this] { ensureCursorVisible(); }));
        m_connections.push_back(m_document->indentationChanged.connect([this] { invalidateLayouts(true); }));
        m_connections.push_back(m_document->languageChanged.connect([this] { invalidateLayouts(false); }));
        m_connections.push_back(m_document->matchesChanged.connect([this] { polish(); }));
        m_connections.push_back(m_document->searchChanged.connect([this] { polish(); }));
        m_connections.push_back(m_document->lockedChanged.connect([this] { polish(); }));
    }
    invalidateLayouts(true);
}

void EditorView::setFont(const FontSpec &font)
{
    if (m_font == font)
        return;
    m_font = font;
    updateMetrics();
}

void EditorView::setWordWrap(bool wrap)
{
    if (m_wordWrap == wrap)
        return;
    const double top = topLine();
    m_wordWrap = wrap;
    invalidateLayouts(true);
    scrollToLine(top);
    optionsChanged();
}

void EditorView::setLineNumbers(bool show)
{
    if (m_lineNumbers == show)
        return;
    m_lineNumbers = show;
    invalidateLayouts(true);
    metricsChanged();
    optionsChanged();
}

void EditorView::setHighlightCurrentLine(bool highlight)
{
    if (m_highlightCurrentLine == highlight)
        return;
    m_highlightCurrentLine = highlight;
    polish();
    optionsChanged();
}

void EditorView::setShowWhitespace(bool show)
{
    if (m_showWhitespace == show)
        return;
    m_showWhitespace = show;
    polish();
    optionsChanged();
}

void EditorView::setMatchBrackets(bool match)
{
    if (m_matchBrackets == match)
        return;
    m_matchBrackets = match;
    polish();
    optionsChanged();
}

void EditorView::setAutoIndent(bool indent)
{
    if (m_autoIndent == indent)
        return;
    m_autoIndent = indent;
    optionsChanged();
}

void EditorView::setOverwriteMode(bool overwrite)
{
    if (m_overwrite == overwrite)
        return;
    m_overwrite = overwrite;
    polish();
    overwriteModeChanged();
}

void EditorView::setDarkMode(bool dark)
{
    if (m_dark == dark)
        return;
    m_dark = dark;
    updateTheme();
}

void EditorView::setAccentColor(Color color)
{
    if (m_accent == color)
        return;
    m_accent = color;
    updateTheme();
}

void EditorView::setSidePadding(double padding)
{
    if (m_sidePadding == padding)
        return;
    m_sidePadding = padding;
    invalidateLayouts(true);
    optionsChanged();
}

void EditorView::setScrollBarDarkMode(bool dark)
{
    m_verticalBar->setDarkMode(dark);
    m_horizontalBar->setDarkMode(dark);
}

void EditorView::setOcclusion(double left, double height)
{
    m_occludedLeft = left;
    m_occludedHeight = height;
}

void EditorView::updateTheme()
{
    m_theme = SyntaxTheme::theme(m_dark, m_accent);
    invalidateLayouts(false);
}

void EditorView::updateMetrics()
{
    m_numberLayouts.clear();
    const Window *window = this->window();
    if (window)
        m_dpr = window->scale();
    m_codeFont = std::make_shared<CodeFont>(m_font, m_dpr, window ? window->hintedTextLayout() : true);
    const double height = m_codeFont->height();
    // Whole device pixels, so every line starts on the pixel grid.
    m_lineHeight = std::ceil(height * 1.4 * m_dpr) / m_dpr;
    m_lineInset = std::round((m_lineHeight - height) / 2 * m_dpr) / m_dpr;
    m_ascent = m_codeFont->ascent();
    m_charWidth = m_codeFont->horizontalAdvance("M");
    m_digitWidth = m_codeFont->horizontalAdvance("0");
    m_spaceWidth = m_codeFont->horizontalAdvance(" ");
    invalidateLayouts(true);
    metricsChanged();
}

double EditorView::gutterWidth() const
{
    if (!m_lineNumbers)
        return 0;
    const int lines = m_document ? m_document->lineCount() : 1;
    const double width = m_sidePadding + std::max(2, digitCount(lines)) * m_digitWidth + m_sidePadding;
    return std::ceil(width * m_dpr) / m_dpr;
}

// Rounds half up with some slack: lines' positions come from different sums and,
// scrolled to a half pixel, land a hair either side of it, one line up and the next down.
double EditorView::toPixel(double value) const
{
    return std::floor(value * m_dpr + 0.5 + 1e-4) / m_dpr;
}

double EditorView::textLeft() const
{
    return m_lineNumbers ? gutterWidth() : std::round(m_sidePadding * m_dpr) / m_dpr;
}

double EditorView::wrapWidth() const
{
    return std::max(m_charWidth * 4, geometry().width - textLeft() - m_sidePadding);
}

static bool isWide(char32_t c)
{
    // East Asian wide characters take two cells.
    return (c >= 0x1100 && c <= 0x115F) || (c >= 0x2E80 && c <= 0xA4CF) || (c >= 0xAC00 && c <= 0xD7A3)
            || (c >= 0xF900 && c <= 0xFAFF) || (c >= 0xFE30 && c <= 0xFE4F) || (c >= 0xFF00 && c <= 0xFF60)
            || (c >= 0xFFE0 && c <= 0xFFE6);
}

int EditorView::displayColumns(const std::string &text) const
{
    const int tabWidth = m_document ? m_document->tabWidth() : 4;
    int columns = 0;
    const size_t size = text.size();
    for (size_t i = 0; i < size;) {
        const auto byte = static_cast<unsigned char>(text[i]);
        if (byte < 0x80) {
            columns += byte == '\t' ? tabWidth - columns % tabWidth : 1;
            ++i;
            continue;
        }
        // Characters outside the BMP were two UTF-16 units, each a column.
        const char32_t c = Unicode::at(text, i);
        columns += c >= 0x10000 ? 2 : isWide(c) ? 2 : 1;
        i = Unicode::next(text, i);
    }
    return columns;
}

int EditorView::estimateVisualLines(const std::string &text, int columns) const
{
    const bool wraps = m_wordWrap || text.size() > size_t(segmentLength);
    if (!wraps)
        return 1;
    const double available = wrapWidth();
    return std::max(1, int(std::ceil(columns * m_charWidth / available)));
}

void EditorView::polish()
{
    m_planned = false;
    requestLayout();
    requestPaint();
}

void EditorView::invalidateLayouts(bool estimates)
{
    // The plan points into the caches; it is rebuilt in the next layout.
    m_plan.clear();
    m_planned = false;
    m_caches.clear();
    m_staleCaches.clear();
    m_widestLaidOut = 0;
    if (estimates) {
        m_lines.clear();
        if (m_document) {
            const TextBuffer &buffer = m_document->buffer();
            m_lines.resize(size_t(buffer.lineCount()));
            m_maxColumns = 0;
            for (int i = 0; i < buffer.lineCount(); ++i) {
                LineInfo &info = m_lines[size_t(i)];
                info.id = m_nextId++;
                info.columns = displayColumns(buffer.line(i));
                info.visualLines = estimateVisualLines(buffer.line(i), info.columns);
                info.exact = !m_wordWrap && buffer.line(i).size() <= size_t(segmentLength);
                m_maxColumns = std::max(m_maxColumns, info.columns);
            }
        }
        m_maxColumnsDirty = false;
        m_indexDirty = true;
        m_exactCursor = 0;
        if (m_wordWrap)
            m_exactTimer.start();
    }
    polish();
}

void EditorView::ensureIndex()
{
    if (!m_indexDirty)
        return;
    std::vector<int> counts(m_lines.size());
    for (size_t i = 0; i < m_lines.size(); ++i)
        counts[i] = m_lines[i].visualLines;
    m_index.build(counts);
    m_indexDirty = false;
}

void EditorView::setLineVisualCount(int line, int count)
{
    LineInfo &info = m_lines[size_t(line)];
    if (info.visualLines == count)
        return;
    if (!m_indexDirty)
        m_index.add(line, count - info.visualLines);
    info.visualLines = count;
}

void EditorView::recountWidth()
{
    if (!m_maxColumnsDirty)
        return;
    m_maxColumns = 0;
    for (const LineInfo &info : m_lines)
        m_maxColumns = std::max(m_maxColumns, info.columns);
    m_maxColumnsDirty = false;
}

void EditorView::computeExactHeights()
{
    if (!m_document || !m_wordWrap || geometry().width <= 0)
        return;
    Lingmo::base::ElapsedTimer timer;
    timer.start();
    ensureIndex();
    int anchorOffset = 0;
    const int anchor = m_index.lineAt(int64_t((m_contentY - topPadding) / m_lineHeight), &anchorOffset);
    int64_t shift = 0;
    const int count = int(m_lines.size());
    int checked = 0;
    while (m_exactCursor < count) {
        LineInfo &info = m_lines[size_t(m_exactCursor)];
        if (!info.exact) {
            // Laid out as when shown, then dropped; very long lines stay estimated.
            int lines = info.visualLines;
            if (m_document->buffer().line(m_exactCursor).size() <= size_t(segmentLength) * 8) {
                LineCache scratch;
                buildLayout(m_exactCursor, &scratch);
                lines = scratch.visualLines;
            }
            lines = std::max(1, lines);
            if (lines != info.visualLines) {
                if (m_exactCursor < anchor)
                    shift += lines - info.visualLines;
                setLineVisualCount(m_exactCursor, lines);
            }
            info.exact = true;
        }
        ++m_exactCursor;
        if ((++checked & 31) == 0 && timer.elapsedMs() >= 3)
            break;
    }
    if (shift != 0)
        applyScroll(m_contentX, m_contentY + double(shift) * m_lineHeight);
    if (m_exactCursor < count)
        m_exactTimer.start();
    polish();
}

void EditorView::onLinesChanged(int line, int removed, int added)
{
    if (!m_document)
        return;
    m_plan.clear();
    m_planned = false;
    const TextBuffer &buffer = m_document->buffer();
    if (removed > int(m_lines.size()) - line) {
        invalidateLayouts(true);
        return;
    }
    const int oldMax = m_maxColumns;
    const int kept = std::min(removed, added);
    for (int i = line; i < line + removed; ++i) {
        auto cached = m_caches.find(m_lines[size_t(i)].id);
        if (cached != m_caches.end()) {
            if (i < line + kept && buffer.line(i).size() > size_t(segmentLength) && cached->second->plain)
                m_staleCaches[m_lines[size_t(i)].id] = std::move(cached->second);
            m_caches.erase(cached);
        }
        if (i >= line + kept)
            m_staleCaches.erase(m_lines[size_t(i)].id);
        // Only losing the widest line needs a recount.
        if (m_lines[size_t(i)].columns >= oldMax && (i >= line + kept || displayColumns(buffer.line(i)) < oldMax))
            m_maxColumnsDirty = true;
    }

    // Lines edited in place keep their id, so their layout cache is found again.
    for (int i = line; i < line + kept; ++i) {
        LineInfo &info = m_lines[size_t(i)];
        info.columns = displayColumns(buffer.line(i));
        setLineVisualCount(i, estimateVisualLines(buffer.line(i), info.columns));
        info.exact = !m_wordWrap && buffer.line(i).size() <= size_t(segmentLength);
        m_maxColumns = std::max(m_maxColumns, info.columns);
    }
    if (removed > added) {
        m_lines.erase(m_lines.begin() + line + added, m_lines.begin() + line + removed);
        m_indexDirty = true;
    } else if (added > removed) {
        std::vector<LineInfo> fresh(size_t(added - removed));
        for (int k = 0; k < int(fresh.size()); ++k) {
            const int i = line + removed + k;
            LineInfo &info = fresh[size_t(k)];
            info.id = m_nextId++;
            info.columns = displayColumns(buffer.line(i));
            info.visualLines = estimateVisualLines(buffer.line(i), info.columns);
            info.exact = !m_wordWrap && buffer.line(i).size() <= size_t(segmentLength);
            m_maxColumns = std::max(m_maxColumns, info.columns);
        }
        m_lines.insert(m_lines.begin() + line + removed, fresh.begin(), fresh.end());
        m_indexDirty = true;
    }
    if (m_wordWrap) {
        m_exactCursor = std::min(m_exactCursor, line);
        m_exactTimer.start();
    }
    // The gutter widens when the line count gains a digit.
    if (m_lineNumbers && digitCount(int(m_lines.size())) != digitCount(int(m_lines.size()) - added + removed)) {
        invalidateLayouts(true);
        metricsChanged();
    }
    polish();
}

void EditorView::onSyntaxChanged(int first, int last)
{
    m_plan.clear();
    m_planned = false;
    for (int i = std::max(0, first); i <= last && i < int(m_lines.size()); ++i)
        m_caches.erase(m_lines[size_t(i)].id);
    polish();
}

void EditorView::onCursorChanged()
{
    resetBlink();
    m_primaryTimer.start();
    polish();
}

EditorView::LineCache *EditorView::ensureLayout(int line)
{
    LineInfo &info = m_lines[size_t(line)];
    auto it = m_caches.find(info.id);
    if (it != m_caches.end())
        return it->second.get();
    auto cache = std::make_unique<LineCache>();
    auto stale = m_staleCaches.find(info.id);
    if (stale != m_staleCaches.end()) {
        buildLayout(line, cache.get(), stale->second.get());
        m_staleCaches.erase(stale);
    } else {
        buildLayout(line, cache.get());
    }
    LineCache *result = cache.get();
    m_caches.emplace(info.id, std::move(cache));
    info.exact = true;
    setLineVisualCount(line, result->visualLines);
    return result;
}

void EditorView::buildLayout(int line, LineCache *cache, LineCache *previous)
{
    const std::string &text = m_document->buffer().line(line);
    const bool wraps = m_wordWrap || text.size() > size_t(segmentLength);
    const double available = wrapWidth();

    std::vector<SyntaxSpan> spans;
    SyntaxStates &states = m_document->syntaxStates();
    if (m_document->isHighlighting() && !Document::isTooLongToHighlight(text)) {
        Syntax::highlight(states.language(), text, states.before(line), &spans);
        cache->provisionalSyntax = line > states.validLines();
    }

    const TextPosition cursor = m_document->cursor();
    const bool preeditHere = !m_preedit.empty() && cursor.line == line;
    cache->preeditColumn = preeditHere ? cursor.column : -1;
    cache->plain = spans.empty() && !preeditHere;
    // Segments of a long line that an edit left as they were keep their layouts.
    std::unordered_map<std::string_view, Segment *> reusable;
    if (previous && cache->plain && previous->plain) {
        for (Segment &old : previous->segments)
            reusable.emplace(old.layout->text(), &old);
    }

    const double tabStop = m_document->tabWidth() * m_spaceWidth;
    int start = 0;
    int visual = 0;
    double hanging = 0;
    cache->width = 0;
    cache->segments.clear();
    do {
        int length = int(text.size()) - start;
        if (length > pieceLength) {
            // Pieces end where the text itself says, not at a fixed offset, so
            // an edit early in a long line leaves the pieces after it alone:
            // after a separator whose preceding characters hash to a marker.
            length = pieceLength;
            int fallback = -1;
            for (int i = start + pieceLength / 4; i < start + pieceLength; ++i) {
                const char c = text[size_t(i)];
                if (c != ' ' && c != ';' && c != ',' && c != '}' && c != ')' && c != '>' && c != '\t')
                    continue;
                fallback = i;
                if (std::hash<std::string_view>{}(std::string_view(text).substr(size_t(i) - 6, 6)) % 8 == 0) {
                    length = i - start + 1;
                    fallback = -1;
                    break;
                }
            }
            if (fallback >= 0)
                length = fallback - start + 1;
            // Never inside a character.
            length = int(Unicode::boundary(text, size_t(start + length))) - start;
        }

        Segment segment;
        segment.start = start;
        segment.length = length;
        segment.firstVisual = visual;
        if (!reusable.empty()) {
            const auto old = reusable.find(std::string_view(text).substr(size_t(start), size_t(length)));
            if (old != reusable.end() && old->second->layout) {
                segment.layout = std::move(old->second->layout);
                visual += segment.layout->lineCount();
                cache->width = std::max(cache->width, segment.layout->maximumWidth());
                cache->segments.push_back(std::move(segment));
                start += length;
                continue;
            }
        }

        const bool preeditInSegment = preeditHere && cursor.column >= start
                && (cursor.column < start + length || start + length == int(text.size()));
        const int preeditAt = cursor.column - start;
        std::string segmentText = text.substr(size_t(start), size_t(length));
        if (preeditInSegment)
            segmentText.insert(size_t(preeditAt), m_preedit);
        auto shifted = [&](int position) {
            return preeditInSegment && position >= preeditAt ? position + int(m_preedit.size()) : position;
        };
        std::vector<CodeLayout::Format> formats;
        for (const SyntaxSpan &span : spans) {
            const int from = std::max(span.start, start);
            const int to = std::min(span.start + span.length, start + length);
            if (from >= to || span.style == SyntaxStyle::Normal)
                continue;
            CodeLayout::Format format;
            format.start = shifted(from - start);
            format.length = shifted(to - start) - format.start;
            format.color = m_theme.styles[int(span.style)];
            format.italic = span.style == SyntaxStyle::Comment || span.style == SyntaxStyle::Emphasis;
            format.bold = span.style == SyntaxStyle::Heading || span.style == SyntaxStyle::Strong
                    || span.style == SyntaxStyle::Section || span.style == SyntaxStyle::DiffHeader
                    || span.style == SyntaxStyle::Error;
            format.underline = span.style == SyntaxStyle::Link;
            formats.push_back(format);
        }
        if (preeditInSegment) {
            CodeLayout::Format underline;
            underline.start = preeditAt;
            underline.length = int(m_preedit.size());
            underline.underline = true;
            underline.hasColor = false;
            formats.push_back(underline);
        }

        segment.layout = std::make_unique<CodeLayout>(m_codeFont, std::move(segmentText));
        CodeLayout *layout = segment.layout.get();
        layout->setWrapping(wraps);
        layout->setTabStopDistance(tabStop);
        layout->setFormats(formats);
        layout->beginLayout();
        while (true) {
            // Wrapped lines continue at the line's own indentation.
            const double indent = visual > 0 ? hanging : 0;
            // Relative to the segment, so a reused one can move down the line.
            if (!layout->createLine(wraps ? available - indent : 1e7, indent,
                                    (visual - segment.firstVisual) * m_lineHeight + m_lineInset))
                break;
            const CodeLayout::Line &made = layout->lineAt(layout->lineCount() - 1);
            if (visual == 0 && wraps && m_wordWrap && made.length < int(layout->text().size())) {
                const std::string &laid = layout->text();
                size_t leading = 0;
                while (leading < size_t(made.length) && Unicode::isSpace(Unicode::at(laid, leading)))
                    leading = Unicode::next(laid, leading);
                hanging = std::min(layout->cursorToX(0, int(leading)), available / 2);
            }
            ++visual;
        }
        layout->endLayout();
        cache->width = std::max(cache->width, layout->maximumWidth());
        cache->segments.push_back(std::move(segment));
        start += length;
    } while (start < int(text.size()));

    cache->visualLines = std::max(1, visual);
    if (!wraps)
        m_widestLaidOut = std::max(m_widestLaidOut, cache->width);
}

void EditorView::dropFarCaches(int first, int last)
{
    const size_t keep = size_t(std::max(200, (last - first + 1) * 3));
    if (m_caches.size() + m_staleCaches.size() <= keep)
        return;
    const int margin = last - first + 1;
    std::unordered_set<uint32_t> wanted;
    for (int i = std::max(0, first - margin); i <= last + margin && i < int(m_lines.size()); ++i)
        wanted.insert(m_lines[size_t(i)].id);
    for (auto *caches : {&m_caches, &m_staleCaches}) {
        for (auto it = caches->begin(); it != caches->end();) {
            if (!wanted.count(it->first))
                it = caches->erase(it);
            else
                ++it;
        }
    }
}

double EditorView::lineTop(int line) const
{
    return topPadding + double(m_index.before(line)) * m_lineHeight;
}

double EditorView::topLine() const
{
    if (m_lines.empty() || m_indexDirty)
        return 0;
    int offset = 0;
    const double visual = (m_contentY - topPadding) / m_lineHeight;
    const int line = m_index.lineAt(int64_t(std::max(0.0, visual)), &offset);
    return line + std::clamp((visual - double(m_index.before(line))) / m_lines[size_t(line)].visualLines, 0.0, 0.999);
}

void EditorView::scrollToLine(double line)
{
    if (m_lines.empty())
        return;
    ensureIndex();
    const int index = std::clamp(int(line), 0, int(m_lines.size()) - 1);
    const double fraction = line - index;
    const double y = lineTop(index) + fraction * m_lines[size_t(index)].visualLines * m_lineHeight - topPadding;
    applyScroll(m_contentX, std::max(0.0, y));
}

void EditorView::applyScroll(double x, double y)
{
    if (m_scroller)
        m_scroller->cancel(false);
    const double maxY = std::max(0.0, m_contentHeight - geometry().height);
    const double maxX = std::max(0.0, m_contentWidth - geometry().width);
    x = std::clamp(x, 0.0, maxX);
    y = std::clamp(y, -m_occludedHeight, std::max(maxY, y > 0 ? y : 0));
    setContentPosition(x, y);
}

void EditorView::setContentPosition(double x, double y)
{
    if (x == m_contentX && y == m_contentY)
        return;
    m_contentX = x;
    m_contentY = y;
    updateScrollBars();
    polish();
    scrolled();
}

void EditorView::updateScrollBars()
{
    m_verticalBar->setRange(geometry().height, m_contentHeight, m_contentY);
    m_horizontalBar->setRange(geometry().width, m_wordWrap ? 0 : m_contentWidth, m_contentX);
}

int EditorView::columnInLayout(const LineCache *cache, const Segment &segment, const TextPosition &position) const
{
    int column = position.column - segment.start;
    if (cache->preeditColumn >= 0 && position.column > cache->preeditColumn)
        column += int(m_preedit.size());
    return column;
}

bool EditorView::locate(const TextPosition &position, LineCache **cacheOut, const Segment **segmentOut, int *textLineOut,
                        int *visualOut)
{
    if (!m_document || m_lines.empty())
        return false;
    const TextPosition p = m_document->buffer().clamp(position);
    LineCache *cache = ensureLayout(p.line);
    const Segment *segment = &cache->segments.front();
    for (const Segment &s : cache->segments) {
        if (p.column >= s.start)
            segment = &s;
    }
    const int textLine = segment->layout->lineForTextPosition(columnInLayout(cache, *segment, p));
    *cacheOut = cache;
    *segmentOut = segment;
    *textLineOut = textLine;
    *visualOut = segment->firstVisual + textLine;
    return textLine >= 0;
}

RectF EditorView::caretRect(const TextPosition &position)
{
    LineCache *cache;
    const Segment *segment;
    int textLine;
    int visual;
    if (!locate(position, &cache, &segment, &textLine, &visual))
        return {};
    const TextPosition p = m_document->buffer().clamp(position);
    int column = p.column - segment->start;
    if (cache->preeditColumn >= 0 && p.column >= cache->preeditColumn)
        column += p.column == cache->preeditColumn ? std::max(0, m_preeditCursor) : int(m_preedit.size());
    const double x = segment->layout->cursorToX(textLine, column);
    const double width = std::ceil(1.5 * m_dpr) / m_dpr;
    return {x, lineTop(p.line) + visual * m_lineHeight, width, m_lineHeight};
}

TextPosition EditorView::hitTest(PointF point)
{
    if (!m_document || m_lines.empty())
        return {};
    ensureIndex();
    const double y = point.y + m_contentY - topPadding;
    if (y < 0)
        return {0, 0};
    int offset = 0;
    const int line = m_index.lineAt(int64_t(y / m_lineHeight), &offset);
    if (y >= double(m_index.total()) * m_lineHeight)
        return m_document->buffer().end();
    LineCache *cache = ensureLayout(line);
    for (const Segment &segment : cache->segments) {
        const int lines = segment.layout->lineCount();
        if (offset >= segment.firstVisual + lines && &segment != &cache->segments.back())
            continue;
        const int textLine = std::clamp(offset - segment.firstVisual, 0, lines - 1);
        const double x = point.x - textLeft() + m_contentX;
        int column = segment.layout->xToCursor(textLine, x);
        // A click past the end of a wrapped line lands before its last space, on that line.
        const CodeLayout::Line &laid = segment.layout->lineAt(textLine);
        const int lineEnd = laid.start + laid.length;
        if (lineEnd < segment.length && column >= lineEnd)
            column = int(Unicode::previous(segment.layout->text(), size_t(lineEnd)));
        return m_document->buffer().clamp({line, segment.start + column});
    }
    return {line, 0};
}

std::vector<RectF> EditorView::rangeRects(int line, LineCache *cache, int from, int to, bool toLineEnd)
{
    std::vector<RectF> rects;
    const double top = lineTop(line);
    const double left = textLeft() - m_contentX;
    const double right = geometry().width + m_contentX;
    for (const Segment &segment : cache->segments) {
        const int segmentEnd = segment.start + segment.length;
        if (to < segment.start || from > segmentEnd)
            continue;
        const bool lastSegment = &segment == &cache->segments.back();
        for (int i = 0; i < segment.layout->lineCount(); ++i) {
            const CodeLayout::Line &laid = segment.layout->lineAt(i);
            const int lineStart = segment.start + laid.start;
            const int lineEnd = lineStart + laid.length;
            const bool lastLine = lastSegment && i == segment.layout->lineCount() - 1;
            if (to < lineStart || from > lineEnd || (from == lineEnd && !lastLine && from != to))
                continue;
            const int a = std::max(from, lineStart) - segment.start;
            const int b = std::min(to, lineEnd) - segment.start;
            double x1 = segment.layout->cursorToX(i, a);
            double x2 = segment.layout->cursorToX(i, b);
            if (x2 < x1)
                std::swap(x1, x2);
            // A selection running on to the next line reaches the right edge.
            if ((toLineEnd && lastLine) || (to > lineEnd && !lastLine))
                x2 = right;
            else if (toLineEnd && to >= lineEnd && lastLine)
                x2 = right;
            const double y = top + (segment.firstVisual + i) * m_lineHeight - m_contentY;
            rects.push_back({left + x1, y, std::max(x2 - x1, 0.0), m_lineHeight});
        }
    }
    return rects;
}

void EditorView::addWhitespaceMarks(int line, LineCache *cache)
{
    // A dot for each space and a line for each tab, on top of the text.
    static constexpr size_t limit = 20000;
    const std::string &text = m_document->buffer().line(line);
    const double top = lineTop(line) - m_contentY;
    const double left = textLeft() - m_contentX;
    const double dot = std::max(1.0, std::round(2 * m_dpr)) / m_dpr;
    const double hairline = std::max(1.0, std::round(m_dpr)) / m_dpr;
    for (const Segment &segment : cache->segments) {
        for (int l = 0; l < segment.layout->lineCount() && m_underlay.size() < limit; ++l) {
            const CodeLayout::Line &laid = segment.layout->lineAt(l);
            const double y = top + (segment.firstVisual + l) * m_lineHeight;
            if (y + m_lineHeight < 0 || y > geometry().height)
                continue;
            const double middle = y + m_lineHeight / 2;
            const int end = laid.start + laid.length;
            for (int i = laid.start; i < end && segment.start + i < int(text.size()); ++i) {
                const char c = text[size_t(segment.start + i)];
                if (c != ' ' && c != '\t')
                    continue;
                const double x1 = left + segment.layout->cursorToX(l, i);
                const double x2 = left + segment.layout->cursorToX(l, i + 1);
                if (c == ' ')
                    m_underlay.push_back({{(x1 + x2 - dot) / 2, middle - dot / 2, dot, dot}, m_theme.whitespace});
                else
                    m_underlay.push_back({{x1 + m_charWidth / 4, middle, std::max(0.0, x2 - x1 - m_charWidth / 2), hairline},
                                          m_theme.whitespace});
            }
        }
    }
}

void EditorView::layout()
{
    const RectF bounds = localRect();
    m_verticalBar->setGeometry({bounds.width - ScrollBar::thickness, 0, ScrollBar::thickness, bounds.height});
    m_horizontalBar->setGeometry({0, bounds.height - ScrollBar::thickness, bounds.width, ScrollBar::thickness});
    m_horizontalBar->setVisible(!m_wordWrap);

    if (const Window *window = this->window(); window && window->scale() != m_dpr)
        updateMetrics();
    if (bounds.width != m_laidOutWidth) {
        // A new width wraps the lines anew; the top line stays where it is.
        const bool first = m_laidOutWidth < 0;
        m_laidOutWidth = bounds.width;
        if (m_wordWrap) {
            const double top = topLine();
            invalidateLayouts(true);
            if (!first)
                scrollToLine(top);
        }
    }
    m_planned = false;
    plan();
}

void EditorView::updateNow()
{
    m_planned = false;
    plan();
}

void EditorView::plan()
{
    if (m_planned)
        return;
    m_planned = true;
    m_plan.clear();
    m_underlay.clear();
    m_caret = {};
    m_currentLineNumber = -1;

    const double width = geometry().width;
    const double height = geometry().height;
    if (!m_document || m_lines.empty() || width <= 0 || height <= 0)
        return;
    ensureIndex();

    // Lines laid out for the first time may wrap differently than estimated;
    // the line at the top keeps its place on screen.
    int anchorOffset = 0;
    const int anchor = m_index.lineAt(int64_t(std::max(0.0, m_contentY - topPadding) / m_lineHeight), &anchorOffset);
    double anchorTop = lineTop(anchor);
    for (int pass = 0; pass < 4; ++pass) {
        m_plan.clear();
        int offset = 0;
        int line = m_index.lineAt(int64_t(std::max(0.0, m_contentY - topPadding) / m_lineHeight), &offset);
        while (line < int(m_lines.size())) {
            LineCache *cache = ensureLayout(line);
            const double top = lineTop(line);
            if (top >= m_contentY + height)
                break;
            m_plan.push_back({line, m_lines[size_t(line)].id, top, cache});
            ++line;
        }
        const double newTop = lineTop(anchor);
        if (newTop == anchorTop)
            break;
        m_contentY += newTop - anchorTop;
        anchorTop = newTop;
        scrolled();
    }

    const int firstLine = m_plan.empty() ? 0 : m_plan.front().line;
    const int lastLine = m_plan.empty() ? 0 : m_plan.back().line;
    m_document->requestHighlighting(lastLine + 50);
    dropFarCaches(firstLine, lastLine);

    // Content size.
    recountWidth();
    m_contentHeight = topPadding + double(m_index.total()) * m_lineHeight + bottomPadding;
    m_contentWidth = m_wordWrap ? width
                                : textLeft() + std::max(m_maxColumns * m_charWidth, m_widestLaidOut) + m_sidePadding + m_charWidth;
    updateScrollBars();

    const TextBuffer &buffer = m_document->buffer();
    const TextRange selection = m_document->selection();
    const TextPosition cursor = m_document->cursor();

    // Painted in the order declared.
    std::vector<RectF> selectionRects;
    std::vector<RectF> matchRects;
    std::vector<RectF> currentMatchRects;
    std::vector<RectF> bracketRects;
    std::vector<Rect> whitespace;
    for (const PlannedLine &planned : m_plan) {
        const int line = planned.line;
        if (m_showWhitespace) {
            std::swap(whitespace, m_underlay);
            addWhitespaceMarks(line, planned.cache);
            std::swap(whitespace, m_underlay);
        }
        if (!selection.isEmpty() && line >= selection.start.line && line <= selection.end.line) {
            const int from = line == selection.start.line ? selection.start.column : 0;
            const int to = line == selection.end.line ? selection.end.column : int(buffer.line(line).size());
            const std::vector<RectF> rects = rangeRects(line, planned.cache, from, to, line < selection.end.line);
            selectionRects.insert(selectionRects.end(), rects.begin(), rects.end());
        }
        const Searcher &searcher = m_document->searcher();
        if (searcher.isValid() && buffer.line(line).size() < 100000) {
            const TextRange scope = m_document->searchScope();
            if (scope.isEmpty() || (line >= scope.start.line && line <= scope.end.line)) {
                const int lower = !scope.isEmpty() && line == scope.start.line ? scope.start.column : 0;
                const int upper = !scope.isEmpty() && line == scope.end.line ? scope.end.column : -1;
                for (const TextRange &match : searcher.matchesInLine(buffer.line(line), line, lower, upper)) {
                    const std::vector<RectF> rects = rangeRects(line, planned.cache, match.start.column, match.end.column, false);
                    std::vector<RectF> &target = match == selection ? currentMatchRects : matchRects;
                    target.insert(target.end(), rects.begin(), rects.end());
                }
            }
        }
    }

    if (m_matchBrackets && selection.isEmpty() && m_document->isHighlighting()) {
        const TextRange brackets = m_document->matchingBracket(cursor);
        if (brackets.start.line >= 0) {
            for (const TextPosition &p : {brackets.start, brackets.end}) {
                if (p.line < firstLine || p.line > lastLine)
                    continue;
                LineCache *cache = ensureLayout(p.line);
                const std::vector<RectF> rects = rangeRects(p.line, cache, p.column, p.column + 1, false);
                bracketRects.insert(bracketRects.end(), rects.begin(), rects.end());
            }
        }
    }

    if (cursor.line >= firstLine && cursor.line <= lastLine) {
        const RectF caret = caretRect(cursor);
        if (caret.width > 0) {
            m_caret = {caret.x + textLeft() - m_contentX, caret.y - m_contentY, caret.width, caret.height};
            if (m_overwrite && selection.isEmpty() && m_preedit.empty())
                m_caret.width = std::max(m_caret.width, m_charWidth);
        }
        if (m_highlightCurrentLine && selection.isEmpty()) {
            const double top = lineTop(cursor.line) - m_contentY;
            m_underlay.push_back({{0, top, width, m_lines[size_t(cursor.line)].visualLines * m_lineHeight},
                                  m_theme.currentLine});
        }
    }
    for (const RectF &r : matchRects)
        m_underlay.push_back({r, m_theme.match});
    for (const RectF &r : currentMatchRects)
        m_underlay.push_back({r, m_theme.currentMatch});
    for (const RectF &r : selectionRects)
        m_underlay.push_back({r, m_theme.selection});
    for (const RectF &r : bracketRects)
        m_underlay.push_back({r, m_theme.bracket});
    m_underlay.insert(m_underlay.end(), whitespace.begin(), whitespace.end());
    m_currentLineNumber = cursor.line;

    // One-line layouts of the numbers in view, in the editor font.
    if (m_lineNumbers) {
        std::unordered_map<int, std::unique_ptr<CodeLayout>> numbers;
        for (const PlannedLine &planned : m_plan) {
            auto existing = m_numberLayouts.find(planned.line);
            if (existing != m_numberLayouts.end()) {
                numbers.emplace(planned.line, std::move(existing->second));
                continue;
            }
            auto layout = std::make_unique<CodeLayout>(m_codeFont, std::to_string(planned.line + 1));
            layout->beginLayout();
            layout->createLine(1e6, 0, m_lineInset);
            layout->endLayout();
            numbers.emplace(planned.line, std::move(layout));
        }
        m_numberLayouts = std::move(numbers);
    } else {
        m_numberLayouts.clear();
    }
    if (hasFocus())
        updateTextInput();
}

void EditorView::paint(Painter &painter)
{
    plan();
    const double height = geometry().height;
    painter.save();
    painter.clip(localRect());

    // Edges, not sizes, so that rects meeting at a fractional pixel stay joined.
    auto snapped = [this](const RectF &r) {
        const double left = toPixel(r.x);
        const double top = toPixel(r.y);
        return RectF{left, top, toPixel(r.x + r.width) - left, toPixel(r.y + r.height) - top};
    };

    for (const Rect &r : m_underlay)
        painter.fillRect(snapped(r.rect), r.color);

    // Text: each segment's lines at their places.
    const double left = textLeft();
    const double textX = std::round((left - m_contentX) * m_dpr) / m_dpr;
    for (const PlannedLine &planned : m_plan) {
        for (const Segment &segment : planned.cache->segments) {
            const double top = planned.top + segment.firstVisual * m_lineHeight;
            const double bottom = top + segment.layout->lineCount() * m_lineHeight;
            if (bottom < m_contentY || top > m_contentY + height)
                continue;
            segment.layout->draw(painter, {textX, toPixel(top - m_contentY)}, m_theme.text);
        }
    }

    const Window *window = this->window();
    const bool showCaret = hasFocus() && m_caretVisible && m_caret.width > 0 && window && window->isActive();
    if (showCaret)
        painter.fillRect(snapped(m_caret), m_overwrite && m_caret.width > 3 ? m_theme.selection : m_theme.text);
    if (m_dropCaret.width > 0)
        painter.fillRect(snapped(m_dropCaret), m_theme.text);

    // Text scrolled sideways goes under the gutter, which covers it.
    if (!m_wordWrap && m_contentX > 0)
        painter.fillRect({0, 0, left, height}, m_theme.background);

    // Line numbers, right-aligned in the gutter.
    if (m_lineNumbers) {
        const double right = gutterWidth() - m_sidePadding;
        for (const PlannedLine &planned : m_plan) {
            const auto layout = m_numberLayouts.find(planned.line);
            if (layout == m_numberLayouts.end())
                continue;
            const bool current = planned.line == m_currentLineNumber;
            const double numberWidth = layout->second->lineAt(0).naturalWidth;
            layout->second->draw(painter,
                                 {std::round((right - numberWidth) * m_dpr) / m_dpr, toPixel(planned.top - m_contentY)},
                                 current ? m_theme.text : m_theme.gutterText);
        }
    }
    painter.restore();
}

void EditorView::resetBlink()
{
    m_caretVisible = true;
    m_blinkStarted.start();
    if (hasFocus())
        m_blinkTimer.start();
    else
        m_blinkTimer.stop();
    requestPaint();
}

void EditorView::autoScroll()
{
    // Dragging a selection past an edge scrolls, faster the further out.
    double dy = 0;
    double dx = 0;
    if (m_lastMouse.y < 0)
        dy = m_lastMouse.y;
    else if (m_lastMouse.y > geometry().height)
        dy = m_lastMouse.y - geometry().height;
    if (!m_wordWrap) {
        if (m_lastMouse.x < textLeft())
            dx = m_lastMouse.x - textLeft();
        else if (m_lastMouse.x > geometry().width)
            dx = m_lastMouse.x - geometry().width;
    }
    if (dx == 0 && dy == 0) {
        m_autoScrollTimer.stop();
        return;
    }
    applyScroll(m_contentX + dx / 2, m_contentY + dy / 2);
    selectByUnit(hitTest(m_lastMouse));
}

void EditorView::focusIn(FocusReason)
{
    resetBlink();
}

void EditorView::focusOut()
{
    m_blinkTimer.stop();
    requestPaint();
}

void EditorView::moveCursor(const TextPosition &position, bool select, bool keepPreferredX)
{
    if (!m_document)
        return;
    if (!keepPreferredX)
        m_preferredX = -1;
    m_document->setCursor(position, select ? m_document->anchor() : position);
    ensureCursorVisible();
}

TextPosition EditorView::moveVertically(const TextPosition &from, int lines)
{
    LineCache *cache;
    const Segment *segment;
    int textLine;
    int visual;
    if (!locate(from, &cache, &segment, &textLine, &visual))
        return from;
    if (m_preferredX < 0)
        m_preferredX = caretRect(from).x;
    ensureIndex();
    const int64_t target = m_index.before(from.line) + visual + lines;
    if (target < 0)
        return {0, 0};
    if (target >= m_index.total())
        return m_document->buffer().end();
    int offset = 0;
    const int line = m_index.lineAt(target, &offset);
    LineCache *targetCache = ensureLayout(line);
    for (const Segment &s : targetCache->segments) {
        const int count = s.layout->lineCount();
        if (offset >= s.firstVisual + count && &s != &targetCache->segments.back())
            continue;
        const int targetLine = std::clamp(offset - s.firstVisual, 0, count - 1);
        int column = s.layout->xToCursor(targetLine, m_preferredX);
        const CodeLayout::Line &laid = s.layout->lineAt(targetLine);
        const int lineEnd = laid.start + laid.length;
        if (column >= lineEnd && lineEnd < s.length)
            column = int(Unicode::previous(s.layout->text(), size_t(lineEnd)));
        return m_document->buffer().clamp({line, s.start + column});
    }
    return {line, 0};
}

TextPosition EditorView::visualLineEdge(const TextPosition &from, bool end)
{
    LineCache *cache;
    const Segment *segment;
    int textLine;
    int visual;
    if (!locate(from, &cache, &segment, &textLine, &visual))
        return from;
    const std::string &text = m_document->buffer().line(from.line);
    const CodeLayout::Line &laid = segment->layout->lineAt(textLine);
    const int lineStart = segment->start + laid.start;
    int lineEnd = lineStart + laid.length;
    const bool lastVisual = segment == &cache->segments.back() && textLine == segment->layout->lineCount() - 1;
    if (end) {
        // A wrapped line ends before the space it wrapped at.
        if (!lastVisual && lineEnd > lineStart && Unicode::isSpace(Unicode::before(text, size_t(lineEnd))))
            lineEnd = int(Unicode::previous(text, size_t(lineEnd)));
        return {from.line, lastVisual ? int(text.size()) : lineEnd};
    }
    if (lineStart > 0)
        return {from.line, lineStart};
    // Home goes to the first non-blank character, then to the start.
    size_t indent = 0;
    while (indent < text.size() && Unicode::isSpace(Unicode::at(text, indent)))
        indent = Unicode::next(text, indent);
    return {from.line, from.column == int(indent) ? 0 : int(indent)};
}

void EditorView::ensureCursorVisible(bool center)
{
    if (!m_document || m_lines.empty() || geometry().height <= 0)
        return;
    ensureIndex();
    const RectF caret = caretRect(m_document->cursor());
    if (caret.width <= 0)
        return;
    const double height = geometry().height;
    double y = m_contentY;
    double x = m_contentX;
    const double margin = std::min(m_lineHeight * 2, height / 4);
    // The cursor and, on its line, the rest of the selection; under the
    // floating panel they count as hidden.
    RectF target = caret;
    const TextPosition anchor = m_document->anchor();
    if (anchor.line == m_document->cursor().line && anchor != m_document->cursor())
        target = united(target, caretRect(anchor));
    const bool underPanel = m_occludedHeight > 0 && target.right() + textLeft() - m_contentX > m_occludedLeft;
    const double top = m_contentY + (underPanel ? m_occludedHeight : 0);
    if (center && (target.y < top || target.bottom() > m_contentY + height))
        y = target.y + target.height / 2 - height / 3;
    else if (target.y - margin < top)
        y = target.y - margin - (underPanel ? m_occludedHeight : 0);
    else if (target.bottom() + margin > m_contentY + height)
        y = target.bottom() + margin - height;
    if (underPanel)
        y = std::max(y, -m_occludedHeight);
    else
        y = std::max(y, 0.0);
    if (!m_wordWrap) {
        const double area = geometry().width - textLeft() - m_sidePadding;
        const double hMargin = std::min(m_charWidth * 4, area / 4);
        if (caret.x - hMargin < m_contentX)
            x = caret.x - hMargin;
        else if (caret.right() + hMargin > m_contentX + area)
            x = caret.right() + hMargin - area;
    }
    // The content may have grown with the edit that moved the cursor.
    const double contentHeight = topPadding + double(m_index.total()) * m_lineHeight + bottomPadding;
    if (contentHeight > m_contentHeight)
        m_contentHeight = contentHeight;
    if (!m_wordWrap && caret.right() + textLeft() + m_sidePadding + m_charWidth > m_contentWidth)
        m_contentWidth = caret.right() + textLeft() + m_sidePadding + m_charWidth;
    if (y != m_contentY || x != m_contentX)
        applyScroll(std::max(0.0, x), y);
}

TextPosition EditorView::positionAt(PointF point)
{
    return hitTest(point);
}

void EditorView::typeText(std::string_view text)
{
    if (!m_document)
        return;
    m_preferredX = -1;
    m_document->insertText(text, m_overwrite);
}

void EditorView::cut()
{
    if (!m_document || m_document->isLocked())
        return;
    copy();
    if (m_document->hasSelection())
        m_document->deleteSelection();
    else
        m_document->deleteLines();
}

void EditorView::copy()
{
    if (!m_document)
        return;
    // Without a selection, the whole line.
    const std::string text = m_document->hasSelection() ? m_document->selectedText()
                                                        : m_document->buffer().line(m_document->cursor().line) + "\n";
    Clipboard::setText(text);
}

void EditorView::paste()
{
    if (!m_document)
        return;
    Document *document = m_document;
    Clipboard::requestText([this, document, alive = std::weak_ptr<bool>(m_alive)](std::string text) {
        if (!alive.expired() && m_document == document && !text.empty())
            typeText(text);
    });
}

void EditorView::updatePrimarySelection()
{
    if (!m_document || !m_document->hasSelection() || !Clipboard::supportsSelection())
        return;
    // A whole large file is not copied on every selection change.
    if (m_document->selectionLength() > 4 * 1024 * 1024)
        return;
    Clipboard::setText(m_document->selectedText(), ClipboardMode::Selection);
}

bool EditorView::keyPress(const KeyEvent &event)
{
    if (!m_document)
        return false;
    const uint32_t mods = event.modifiers & (ShiftModifier | ControlModifier | AltModifier | SuperModifier);
    const bool shift = mods & ShiftModifier;
    const bool ctrl = mods & ControlModifier;
    const bool alt = mods & AltModifier;
    const uint32_t key = event.key;
    const TextPosition cursor = m_document->cursor();
    const TextRange selection = m_document->selection();
    const TextBuffer &buffer = m_document->buffer();

    // The standard editing keys of this platform, alternatives included.
    const bool undo = mods == ControlModifier && key == key::letter('z');
    const bool redo = (mods == (ControlModifier | ShiftModifier) && key == key::letter('z'))
            || (mods == ControlModifier && key == key::letter('y'));
    const bool cutKeys = (mods == ControlModifier && key == key::letter('x')) || (mods == ShiftModifier && key == key::Delete);
    const bool copyKeys = (mods == ControlModifier && key == key::letter('c')) || (mods == ControlModifier && key == key::Insert);
    const bool pasteKeys = (mods == ControlModifier && key == key::letter('v')) || (mods == ShiftModifier && key == key::Insert);
    const bool selectAll = mods == ControlModifier && key == key::letter('a');

    if (undo) {
        resetBlink();
        m_document->undo();
        return true;
    }
    if (redo) {
        resetBlink();
        m_document->redo();
        return true;
    }
    if (cutKeys) {
        resetBlink();
        cut();
        return true;
    }
    if (copyKeys) {
        copy();
        return true;
    }
    if (pasteKeys) {
        resetBlink();
        paste();
        return true;
    }
    if (selectAll) {
        m_document->selectAll();
        return true;
    }

    switch (key) {
    case key::Left:
    case key::Right: {
        const bool forward = key == key::Right;
        if (alt)
            return false;
        resetBlink();
        if (!shift && !selection.isEmpty() && !ctrl) {
            moveCursor(forward ? selection.end : selection.start, false);
            return true;
        }
        TextPosition target;
        if (ctrl) {
            target = m_document->wordBoundary(cursor, forward);
        } else if (forward) {
            const std::string &line = buffer.line(cursor.line);
            if (cursor.column < int(line.size()))
                target = {cursor.line, int(Unicode::next(line, size_t(cursor.column)))};
            else
                target = cursor.line + 1 < buffer.lineCount() ? TextPosition{cursor.line + 1, 0} : cursor;
        } else if (cursor.column > 0) {
            target = {cursor.line, int(Unicode::previous(buffer.line(cursor.line), size_t(cursor.column)))};
        } else {
            target = cursor.line > 0 ? TextPosition{cursor.line - 1, int(buffer.line(cursor.line - 1).size())} : cursor;
        }
        moveCursor(target, shift);
        return true;
    }
    case key::Up:
    case key::Down: {
        const bool up = key == key::Up;
        resetBlink();
        if (alt && !shift && !ctrl) {
            m_document->moveLines(up);
            return true;
        }
        if (ctrl) {
            // Scrolls by a line without moving the cursor.
            applyScroll(m_contentX, m_contentY + (up ? -m_lineHeight : m_lineHeight));
            return true;
        }
        if (!shift && !selection.isEmpty()) {
            moveCursor(up ? selection.start : selection.end, false);
            return true;
        }
        moveCursor(moveVertically(cursor, up ? -1 : 1), shift, true);
        return true;
    }
    case key::PageUp:
    case key::PageDown: {
        if (ctrl)
            return false;
        resetBlink();
        const int lines = std::max(1, int(geometry().height / m_lineHeight) - 1);
        const bool up = key == key::PageUp;
        const TextPosition target = moveVertically(cursor, up ? -lines : lines);
        applyScroll(m_contentX, m_contentY + (up ? -lines : lines) * m_lineHeight);
        moveCursor(target, shift, true);
        return true;
    }
    case key::Home:
        resetBlink();
        moveCursor(ctrl ? TextPosition{0, 0} : visualLineEdge(cursor, false), shift);
        return true;
    case key::End:
        resetBlink();
        moveCursor(ctrl ? buffer.end() : visualLineEdge(cursor, true), shift);
        return true;
    case key::BackSpace:
        resetBlink();
        m_preferredX = -1;
        m_document->deleteBackward(ctrl);
        return true;
    case key::Delete:
        resetBlink();
        m_preferredX = -1;
        m_document->deleteForward(ctrl);
        return true;
    case key::Return:
    case key::KeypadEnter:
        resetBlink();
        m_preferredX = -1;
        m_document->insertNewline(m_autoIndent);
        return true;
    case key::Tab: {
        if (ctrl)
            return false;
        resetBlink();
        if (shift || selection.start.line != selection.end.line) {
            m_document->indentSelection(shift);
        } else if (m_document->insertSpaces()) {
            const int width = m_document->indentWidth();
            const int column = m_document->visualColumn() - 1;
            typeText(std::string(size_t(width - column % width), ' '));
        } else {
            typeText("\t");
        }
        return true;
    }
    case key::Insert:
        if (mods != NoModifier)
            return false;
        setOverwriteMode(!m_overwrite);
        return true;
    default:
        break;
    }

    const std::string &text = event.text;
    if (!ctrl && !alt && !text.empty() && Unicode::isPrint(Unicode::at(text, 0))) {
        resetBlink();
        typeText(text);
        return true;
    }
    return false;
}

bool EditorView::acceptsTextInput() const
{
    return m_document && !m_document->isLocked();
}

TextInputState EditorView::textInputState() const
{
    TextInputState state;
    if (!m_document)
        return state;
    const TextPosition cursor = m_document->cursor();
    const TextPosition anchor = m_document->anchor();
    state.surroundingText = m_document->buffer().line(cursor.line);
    state.cursor = size_t(cursor.column);
    state.anchor = size_t(anchor.line == cursor.line ? anchor.column : cursor.column);
    const PointF origin = mapToWindow({m_caret.x, m_caret.y});
    state.cursorRectangle = {origin.x, origin.y, m_caret.width, m_caret.height};
    state.hints = MultilineHint;
    return state;
}

void EditorView::textInput(const TextInputEvent &event)
{
    if (!m_document || m_document->isLocked())
        return;
    if (event.deleteBefore > 0 || event.deleteAfter > 0) {
        const TextPosition cursor = m_document->cursor();
        const std::string &line = m_document->buffer().line(cursor.line);
        const int start = int(Unicode::boundary(line, size_t(std::max(0, cursor.column - int(event.deleteBefore)))));
        const int end = int(Unicode::boundary(line, std::min(line.size(), size_t(cursor.column) + event.deleteAfter)));
        if (!m_document->hasSelection())
            m_document->setCursor({cursor.line, end}, {cursor.line, start});
        if (event.commit.empty())
            m_document->deleteSelection();
    }
    if (!event.commit.empty())
        typeText(event.commit);
    const std::string &preedit = event.preedit;
    m_preeditCursor = event.preeditCursorBegin;
    if (preedit != m_preedit || !preedit.empty()) {
        // Typing over a selection with an input method replaces it first.
        if (!preedit.empty() && m_document->hasSelection())
            m_document->deleteSelection();
        m_preedit = preedit;
        const int line = m_document->cursor().line;
        if (line < int(m_lines.size())) {
            m_plan.clear();
            m_caches.erase(m_lines[size_t(line)].id);
        }
        polish();
    }
}

TextRange EditorView::unitAt(const TextPosition &position) const
{
    if (m_selectUnit == 1)
        return m_document->wordAt(position);
    const int last = m_document->lineCount() - 1;
    return position.line < last ? TextRange{{position.line, 0}, {position.line + 1, 0}}
                                : TextRange{{position.line, 0}, m_document->buffer().end()};
}

void EditorView::selectByUnit(const TextPosition &position)
{
    if (!m_document)
        return;
    if (m_selectUnit == 0) {
        m_document->setCursor(position, m_document->anchor());
        return;
    }
    const TextRange unit = unitAt(position);
    if (unit.start < m_unitAnchor.start)
        m_document->setCursor(unit.start, m_unitAnchor.end);
    else
        m_document->setCursor(std::max(unit.end, m_unitAnchor.end), m_unitAnchor.start);
}

CursorShape EditorView::cursorShape() const
{
    return m_lineNumbers && m_lastMouse.x < gutterWidth() ? CursorShape::Default : CursorShape::Text;
}

void EditorView::hoverMove(const PointerEvent &event)
{
    m_lastMouse = event.position;
}

void EditorView::pointerPress(const PointerEvent &event)
{
    setFocus(FocusReason::Mouse);
    if (!m_document)
        return;
    const PointF pos = event.position;
    const TextPosition hit = hitTest(pos);
    m_preferredX = -1;

    if (event.button == MouseButton::Right) {
        const TextRange selection = m_document->selection();
        if (selection.isEmpty() || hit < selection.start || hit > selection.end)
            m_document->setCursor(hit);
        if (onContextMenuRequested)
            onContextMenuRequested(pos);
        return;
    }
    if (event.button == MouseButton::Middle) {
        if (Clipboard::supportsSelection() && !m_document->isLocked()) {
            Document *document = m_document;
            Clipboard::requestText(
                    [this, document, hit, alive = std::weak_ptr<bool>(m_alive)](std::string text) {
                        if (alive.expired() || m_document != document || text.empty())
                            return;
                        m_document->setCursor(hit);
                        typeText(text);
                    },
                    ClipboardMode::Selection);
        }
        return;
    }

    const int clickCount = (std::max(1, event.clickCount) - 1) % 3 + 1;
    m_pressPos = pos;
    m_lastMouse = pos;

    const bool shift = event.modifiers & ShiftModifier;
    const TextRange selection = m_document->selection();
    const bool inGutter = m_lineNumbers && pos.x < gutterWidth();

    if (inGutter) {
        m_selectUnit = 2;
        m_unitAnchor = unitAt(hit);
        if (shift && !selection.isEmpty())
            m_unitAnchor = {selection.start, selection.start};
        selectByUnit(hit);
        m_selecting = true;
    } else if (clickCount == 1 && !shift && !selection.isEmpty() && hit >= selection.start && hit <= selection.end
               && !m_document->isLocked()) {
        // Pressing on the selection may start dragging it.
        m_maybeDrag = true;
    } else if (clickCount == 1) {
        m_selectUnit = 0;
        m_document->setCursor(hit, shift ? m_document->anchor() : hit);
        m_selecting = true;
    } else {
        m_selectUnit = clickCount == 2 ? 1 : 2;
        m_unitAnchor = unitAt(hit);
        selectByUnit(hit);
        m_selecting = true;
    }
}

void EditorView::pointerMove(const PointerEvent &event)
{
    const PointF pos = event.position;
    m_lastMouse = pos;
    if (m_maybeDrag) {
        // The drag distance, in logical pixels.
        if (std::hypot(pos.x - m_pressPos.x, pos.y - m_pressPos.y) >= 10) {
            m_maybeDrag = false;
            startTextDrag();
        }
        return;
    }
    if (!m_selecting)
        return;
    selectByUnit(hitTest(pos));
    ensureCursorVisible();
    if (pos.y < 0 || pos.y > geometry().height || (!m_wordWrap && (pos.x < textLeft() || pos.x > geometry().width))) {
        if (!m_autoScrollTimer.isActive())
            m_autoScrollTimer.start();
    } else {
        m_autoScrollTimer.stop();
    }
}

void EditorView::pointerRelease(const PointerEvent &event)
{
    if (m_maybeDrag && m_document) {
        m_document->setCursor(hitTest(event.position));
        m_maybeDrag = false;
    }
    m_selecting = false;
    m_autoScrollTimer.stop();
    updatePrimarySelection();
}

bool EditorView::scroll(const ScrollEvent &event)
{
    if (event.modifiers & ControlModifier) {
        // Wheel clicks zoom; positive deltas scroll down, so they zoom out.
        const double delta = event.value120Y != 0 ? event.value120Y : event.deltaY;
        if (delta != 0 && onZoomRequested)
            onZoomRequested(delta < 0 ? 1 : -1);
        return true;
    }
    if (!m_scroller && window()) {
        m_scroller = std::make_unique<Scroller>(window(), m_scrollTarget.get());
        m_scroller->setElastic(false);
    }
    return m_scroller && m_scroller->handleScroll(event);
}

bool EditorView::gesture(const GestureEvent &event)
{
    if (event.type == GestureEvent::Type::PinchBegin) {
        m_pinch = 1;
        m_pinchAccumulated = 0;
        return true;
    }
    if (event.type == GestureEvent::Type::PinchUpdate) {
        // Zoom steps follow the changes of the pinch scale.
        m_pinchAccumulated += event.scale - m_pinch;
        m_pinch = event.scale;
        if (std::abs(m_pinchAccumulated) > 0.15) {
            if (onZoomRequested)
                onZoomRequested(m_pinchAccumulated > 0 ? 1 : -1);
            m_pinchAccumulated = 0;
        }
        return true;
    }
    return event.type == GestureEvent::Type::PinchEnd;
}

void EditorView::startTextDrag()
{
    if (!m_document)
        return;
    const TextRange range = m_document->selection();
    const std::string text = m_document->selectedText();
    DragSource source;
    source.mimeTypes = {"text/plain;charset=utf-8", "text/plain"};
    source.data = [text](const std::string &) { return text; };
    source.actions = CopyAction | MoveAction;
    source.context = this;
    source.onFinished = [this, range, alive = std::weak_ptr<bool>(m_alive)](DropAction action) {
        if (alive.expired())
            return;
        const bool moved = m_dragging && action == MoveAction;
        m_dragging = false;
        // Dropped into another view: this one gives the text up.
        if (moved && m_document && !m_document->isLocked()) {
            m_document->setCursor(range.end, range.start);
            m_document->deleteSelection();
        }
    };
    m_dragging = true;
    if (!startDrag(std::move(source))) {
        // Nothing can drag here: the press was a click.
        m_dragging = false;
        m_document->setCursor(hitTest(m_pressPos));
    }
}

bool EditorView::canDrop(const DropOffer &offer) const
{
    if (offer.hasMimeType("text/uri-list"))
        return true;
    const bool text = offer.hasMimeType("text/plain;charset=utf-8") || offer.hasMimeType("text/plain")
            || offer.hasMimeType("UTF8_STRING");
    return text && m_document && !m_document->isLocked();
}

void EditorView::dragEnter(const DragEvent &event)
{
    dragUpdate(event);
}

uint32_t EditorView::dragUpdate(const DragEvent &event)
{
    if (event.offer && event.offer->hasMimeType("text/uri-list"))
        return CopyAction;
    const TextPosition position = hitTest(event.position);
    const RectF caret = caretRect(position);
    m_dropCaret = caret.width > 0 ? RectF{caret.x + textLeft() - m_contentX, caret.y - m_contentY, caret.width, caret.height}
                                  : RectF{};
    requestPaint();
    const bool fromSelf = event.offer && event.offer->localSource() && event.offer->localSource()->context == this;
    const bool copy = (event.modifiers & ControlModifier) || !fromSelf;
    return copy ? CopyAction : MoveAction;
}

void EditorView::dragLeave()
{
    m_dropCaret = {};
    requestPaint();
}

uint32_t EditorView::drop(const DragEvent &event)
{
    m_dropCaret = {};
    requestPaint();
    if (!m_document || !event.offer)
        return NoAction;
    DropOffer *offer = event.offer;
    if (offer->hasMimeType("text/uri-list")) {
        offer->read("text/uri-list", [this, alive = std::weak_ptr<bool>(m_alive)](std::string data) {
            if (alive.expired())
                return;
            std::vector<std::string> paths;
            for (const std::string &line : Lingmo::base::strings::split(data, '\n')) {
                const std::string_view url = Lingmo::base::strings::trimmed(line);
                if (url.empty() || url[0] == '#')
                    continue;
                if (const std::optional<std::string> path = Lingmo::base::url::toLocalFile(url))
                    paths.push_back(*path);
            }
            if (!paths.empty() && onFilesDropped)
                onFilesDropped(paths);
        });
        return CopyAction;
    }
    if (m_document->isLocked())
        return NoAction;
    TextPosition position = hitTest(event.position);
    const bool fromSelf = offer->localSource() && offer->localSource()->context == this;
    const bool move = fromSelf && !(event.modifiers & ControlModifier);

    if (move) {
        const TextRange range = m_document->selection();
        const std::string text = m_document->selectedText();
        if (position >= range.start && position <= range.end) {
            m_dragging = false;
            return NoAction;
        }
        const TextBuffer &buffer = m_document->buffer();
        const int64_t at = buffer.offset(position);
        const int64_t start = buffer.offset(range.start);
        const int64_t end = buffer.offset(range.end);
        m_document->beginEdit();
        if (at > end) {
            m_document->replace(range, std::string(), UndoGroup::Other, {range.start, range.start});
            position = m_document->buffer().position(at - (end - start));
        } else {
            m_document->replace({position, position}, text, UndoGroup::Other, {position, position});
            const int64_t size = int64_t(text.size());
            const TextRange shifted{m_document->buffer().position(start + size), m_document->buffer().position(end + size)};
            m_document->replace(shifted, std::string(), UndoGroup::Other, {position, position});
            m_document->setCursor(TextBuffer::endOf(position, text), position);
            m_document->endEdit();
            // Handled here, so the drag source does not delete it again.
            m_dragging = false;
            return CopyAction;
        }
        const TextPosition endAt = TextBuffer::endOf(position, text);
        m_document->replace({position, position}, text, UndoGroup::Other, {endAt, position});
        m_document->endEdit();
        m_dragging = false;
        return CopyAction;
    }

    const std::string type = offer->hasMimeType("text/plain;charset=utf-8") ? "text/plain;charset=utf-8"
            : offer->hasMimeType("text/plain")                               ? "text/plain"
                                                                               : "UTF8_STRING";
    Document *document = m_document;
    offer->read(type, [this, document, position, alive = std::weak_ptr<bool>(m_alive)](std::string text) {
        if (alive.expired() || m_document != document || text.empty() || m_document->isLocked())
            return;
        const TextPosition at = m_document->buffer().clamp(position);
        const TextPosition endAt = TextBuffer::endOf(at, text);
        m_document->setCursor(at);
        m_document->replace({at, at}, text, UndoGroup::Other, {endAt, at});
        setFocus();
    });
    return CopyAction;
}
