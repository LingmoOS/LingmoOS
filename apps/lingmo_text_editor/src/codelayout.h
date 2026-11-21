// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CODELAYOUT_H
#define CODELAYOUT_H

#include "ui/geometry.h"
#include "ui/glyph_run.h"
#include "ui/text_layout.h"

#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace Lingmo::ui {
class Painter;
}

// The editor's font at a window scale, with fractional metrics in logical
// pixels.
class CodeFont
{
public:
    CodeFont(const Lingmo::ui::FontSpec &font, double scale, bool hintedLayout);
    ~CodeFont();

    const Lingmo::ui::FontSpec &font() const { return m_font; }
    double scale() const { return m_scale; }
    bool hintedLayout() const { return m_hintedLayout; }

    double height() const { return m_height; }
    double ascent() const { return m_ascent; }
    // The text's advance in fractional logical pixels.
    double horizontalAdvance(std::string_view text) const;

private:
    Lingmo::ui::FontSpec m_font;
    double m_scale;
    bool m_hintedLayout;
    std::unique_ptr<Lingmo::ui::FontMetrics> m_metrics;
    double m_height = 0;
    double m_ascent = 0;
};

// One paragraph of code on the toolkit's TextLayout, wrapping at word
// boundaries or anywhere, with trailing spaces in the width. Offsets are
// UTF-8 bytes into the text.
class CodeLayout
{
public:
    struct Format
    {
        int start = 0;
        int length = 0;
        Lingmo::ui::Color color;
        bool bold = false;
        bool italic = false;
        bool underline = false;
        // The colour applies; otherwise only the style does.
        bool hasColor = true;
    };

    struct Line
    {
        int start = 0;
        int length = 0;
        // Where the line was put, relative to the layout.
        double x = 0;
        double y = 0;
        // The width of the line's text, trailing spaces included.
        double naturalWidth = 0;
    };

    CodeLayout(std::shared_ptr<CodeFont> font, std::string text);
    ~CodeLayout();

    const std::string &text() const { return m_layout.text(); }
    const std::shared_ptr<CodeFont> &font() const { return m_font; }

    void setTabStopDistance(double distance);
    void setWrapping(bool wraps);
    // Before beginLayout(); later ones win where they overlap, a format
    // without a colour keeps the colour under it.
    void setFormats(const std::vector<Format> &formats);

    void beginLayout();
    // The next line at this width and position; false when the text is
    // used up.
    bool createLine(double width, double x, double y);
    void endLayout();

    int lineCount() const { return int(m_lines.size()); }
    const Line &lineAt(int index) const { return m_lines[size_t(index)]; }
    int lineForTextPosition(int offset) const;
    double maximumWidth() const { return m_maximumWidth; }

    // Relative to the layout, the line's x included.
    double cursorToX(int line, int offset) const;
    // The cluster boundary nearest to x (relative to the layout) on the line;
    // the line's end past it.
    int xToCursor(int line, double x) const;

    // Every line at its position from the origin, the text in the default
    // colour where no format colours it.
    void draw(Lingmo::ui::Painter &painter, Lingmo::ui::PointF origin, Lingmo::ui::Color color) const;

private:
    std::shared_ptr<CodeFont> m_font;
    // Drawing caches glyphs in it.
    mutable Lingmo::ui::TextLayout m_layout;
    std::vector<Line> m_lines;
    double m_maximumWidth = 0;
    mutable Lingmo::ui::Color m_drawColor;
    mutable bool m_hasDrawColor = false;
};

#endif
