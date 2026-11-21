// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TERMINALFONT_H
#define TERMINALFONT_H

#include <memory>
#include <span>
#include <string>
#include <vector>

#include "ui/geometry.h"
#include "ui/glyph_run.h"

#include "CharacterColor.h"

namespace Lingmo::ui {
class Painter;
}

namespace Konsole
{

struct CellMetrics;

// The terminal's font: its cell metrics as QFontMetrics gave the display,
// and its clusters drawn at device positions as the glyph atlas drew them.
class TerminalFont
{
public:
    enum Style
    {
        Regular = 0,
        Bold = 1,
        Italic = 2,
    };

    TerminalFont();
    ~TerminalFont();

    // The families of fixed pitch, as QFontDatabase lists them.
    static std::vector<std::string> fixedPitchFamilies();

    void setFont(const std::string& family, double pointSize);
    const std::string& family() const { return _family; }
    double pointSize() const { return _pointSize; }

    // TerminalDisplay::fontChange(), line spacing 0. QFontMetrics of a font
    // without a paint device does not depend on the window's scale.
    CellMetrics metrics() const;

    // Clusters of one style and colour with their origins at device x, on a
    // baseline in device pixels, stretched about stretchOrigin for
    // double-width and double-height lines.
    void draw(Lingmo::ui::Painter& painter, int style, Color color, double scale, double baseline,
              std::span<const Lingmo::ui::GlyphCluster> clusters, Lingmo::ui::SizeF stretch,
              Lingmo::ui::PointF stretchOrigin);

    // Shaped clusters kept by the renderers of every style.
    size_t cachedClusters() const;

private:
    Lingmo::ui::FontSpec spec(int style) const;

    std::string _family = "Monospace";
    double _pointSize = 12;
    // One per style, for the scale they were made at.
    std::unique_ptr<Lingmo::ui::GlyphRenderer> _renderers[4];
};

}

#endif // TERMINALFONT_H
