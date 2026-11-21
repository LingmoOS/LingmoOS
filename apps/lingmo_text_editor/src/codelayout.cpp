// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "codelayout.h"

#include "ui/painter.h"

#include <algorithm>

using namespace Lingmo::ui;

CodeFont::CodeFont(const FontSpec &font, double scale, bool hintedLayout)
    : m_font(font)
    , m_scale(scale)
    , m_hintedLayout(hintedLayout)
    , m_metrics(std::make_unique<FontMetrics>(font, scale, hintedLayout))
{
    m_height = m_metrics->heightF();
    m_ascent = m_metrics->ascentF();
}

CodeFont::~CodeFont() = default;

double CodeFont::horizontalAdvance(std::string_view text) const
{
    return m_metrics->horizontalAdvanceF(text);
}

CodeLayout::CodeLayout(std::shared_ptr<CodeFont> font, std::string text)
    : m_font(std::move(font))
    , m_layout(std::move(text), m_font->font(), m_font->scale(), m_font->hintedLayout())
{
}

CodeLayout::~CodeLayout() = default;

void CodeLayout::setTabStopDistance(double distance)
{
    m_layout.setTabStopDistance(distance);
}

void CodeLayout::setWrapping(bool wraps)
{
    m_layout.setWrap(wraps ? TextLayout::Wrap::WordOrAnywhere : TextLayout::Wrap::None);
}

void CodeLayout::setFormats(const std::vector<Format> &formats)
{
    // TextLayout takes the last range over a character whole; a range that
    // only styles (the preedit's underline) must keep the colour under it.
    std::vector<int> cuts;
    for (const Format &format : formats) {
        if (format.length <= 0)
            continue;
        cuts.push_back(format.start);
        cuts.push_back(format.start + format.length);
    }
    std::sort(cuts.begin(), cuts.end());
    cuts.erase(std::unique(cuts.begin(), cuts.end()), cuts.end());

    std::vector<TextFormatRange> ranges;
    for (size_t i = 0; i + 1 < cuts.size(); ++i) {
        const int from = cuts[i];
        const int to = cuts[i + 1];
        TextFormatRange range;
        range.start = size_t(from);
        range.length = size_t(to - from);
        bool covered = false;
        for (const Format &format : formats) {
            if (format.length <= 0 || format.start > from || format.start + format.length < to)
                continue;
            covered = true;
            if (format.hasColor)
                range.color = format.color;
            range.bold = range.bold || format.bold;
            range.italic = range.italic || format.italic;
            range.underline = range.underline || format.underline;
        }
        if (covered)
            ranges.push_back(range);
    }
    m_layout.setFormats(std::move(ranges));
}

void CodeLayout::beginLayout()
{
    m_lines.clear();
    m_layout.beginLayout();
}

bool CodeLayout::createLine(double width, double x, double y)
{
    const int index = m_layout.createLine(width, {x, y});
    if (index < 0)
        return false;
    const TextLayout::Line &made = m_layout.lineAt(index);
    Line line;
    line.start = int(made.textStart);
    line.length = int(made.textLength);
    line.x = made.position.x;
    line.y = made.position.y;
    line.naturalWidth = made.naturalTextWidth;
    m_lines.push_back(line);
    return true;
}

void CodeLayout::endLayout()
{
    m_layout.endLayout();
    m_maximumWidth = m_layout.maximumWidth();
}

int CodeLayout::lineForTextPosition(int offset) const
{
    return m_layout.lineForTextPosition(size_t(std::max(0, offset)));
}

double CodeLayout::cursorToX(int line, int offset) const
{
    return m_layout.cursorToX(line, size_t(std::max(0, offset)));
}

int CodeLayout::xToCursor(int line, double x) const
{
    return int(m_layout.xToCursor(line, x));
}

void CodeLayout::draw(Painter &painter, PointF origin, Color color) const
{
    // Setting it drops the layout's drawing cache.
    if (!m_hasDrawColor || m_drawColor.argb() != color.argb()) {
        m_layout.setDefaultColor(color);
        m_drawColor = color;
        m_hasDrawColor = true;
    }
    m_layout.draw(painter, origin);
}
