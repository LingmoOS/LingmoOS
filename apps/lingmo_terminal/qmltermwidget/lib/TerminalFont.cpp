// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "TerminalFont.h"

#include "TerminalView.h"
#include "tools.h"

namespace ui = Lingmo::ui;

namespace Konsole
{

// TerminalDisplay's representative characters for the cell width.
static const char REPCHAR[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                              "abcdefgjijklmnopqrstuvwxyz"
                              "0123456789./+@";

// Shaped clusters a renderer keeps; output can name any of the million code
// points and endless combining sequences, so the cache starts over past this.
static constexpr size_t kMaximumCachedClusters = 8192;

TerminalFont::TerminalFont() = default;

TerminalFont::~TerminalFont() = default;

std::vector<std::string> TerminalFont::fixedPitchFamilies()
{
    std::vector<std::string> families;
    for (const ui::FontFamily& family : ui::fontFamilies()) {
        if (family.fixedPitch)
            families.push_back(family.name);
    }
    return families;
}

void TerminalFont::setFont(const std::string& family, double pointSize)
{
    if (family == _family && pointSize == _pointSize)
        return;
    _family = family;
    _pointSize = pointSize;
    for (auto& renderer : _renderers)
        renderer.reset();
}

ui::FontSpec TerminalFont::spec(int style) const
{
    return ui::FontSpec{_family, _pointSize, bool(style & Bold), bool(style & Italic)};
}

CellMetrics TerminalFont::metrics() const
{
    const ui::FontMetrics fm(spec(Regular), 1, true);
    const std::string_view repchar(REPCHAR, sizeof(REPCHAR) - 1);

    CellMetrics metrics;
    metrics.height = fm.height();
    metrics.width = kRound(double(fm.horizontalAdvance(repchar)) / double(repchar.size()));
    metrics.ascent = fm.ascent();
    metrics.descent = fm.descent();

    metrics.fixedPitch = true;
    const int first = fm.horizontalAdvance(repchar.substr(0, 1));
    for (size_t i = 1; i < repchar.size(); ++i) {
        if (fm.horizontalAdvance(repchar.substr(i, 1)) != first) {
            metrics.fixedPitch = false;
            break;
        }
    }

    // The GPU node drew decorations from QFontMetricsF.
    metrics.underlinePos = fm.underlinePosF();
    metrics.strikeOutPos = fm.strikeOutPosF();
    metrics.overlinePos = fm.overlinePosF();
    metrics.lineWidth = fm.lineWidthF();
    return metrics;
}

void TerminalFont::draw(ui::Painter& painter, int style, Color color, double scale, double baseline,
                        std::span<const ui::GlyphCluster> clusters, ui::SizeF stretch, ui::PointF stretchOrigin)
{
    std::unique_ptr<ui::GlyphRenderer>& renderer = _renderers[style & 3];
    if (!renderer || renderer->scale() != scale)
        renderer = std::make_unique<ui::GlyphRenderer>(spec(style), scale);
    else if (renderer->cachedClusters() > kMaximumCachedClusters)
        renderer->clearCache();
    renderer->draw(painter, color, baseline, clusters, stretch, stretchOrigin);
}

size_t TerminalFont::cachedClusters() const
{
    size_t count = 0;
    for (const auto& renderer : _renderers) {
        if (renderer)
            count += renderer->cachedClusters();
    }
    return count;
}

}
