// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "widgets/picture.h"

#include "ui/painter.h"
#include "widgets/card.h"

using namespace Lingmo::ui;

namespace settings {

static std::string &dataDirectory()
{
    static std::string directory = SETTINGS_DATA_DIR;
    return directory;
}

std::string dataPath(const std::string& name)
{
    return dataDirectory() + "/" + name;
}

void setDataDirectory(std::string directory)
{
    dataDirectory() = std::move(directory);
}

Picture::Picture(std::string path)
    : m_path(std::move(path))
{
}

void Picture::setPath(std::string path)
{
    m_path = std::move(path);
    requestPaint();
}

void Picture::setTint(std::optional<ColorRecipe> tint)
{
    m_tint = std::move(tint);
    requestPaint();
}

void Picture::setCornerRadius(double radius)
{
    m_radius = radius;
    requestPaint();
}

// The toolkit fits the picture centred with its shape kept, as Qt's SVG
// image reader does at another aspect.
void Picture::paint(Painter& painter)
{
    if (m_path.empty())
        return;
    const RectF bounds = localRect();
    // On the pixels a rounded fill or clip of the same rect snaps to, so a
    // glyph stays centred on its tile at fractional scales.
    const Rect device = painter.snappedRect(bounds);
    const double scale = painter.scale();
    const PointF offset = painter.offset();
    painter.save();
    if (m_radius > 0)
        painter.clipRoundedRect(bounds, m_radius, themeCorners());
    painter.drawSvg(m_path,
                    {device.x / scale - offset.x, device.y / scale - offset.y, device.width / scale,
                     device.height / scale},
                    m_tint ? std::optional<Color>(m_tint->resolve()) : std::nullopt);
    painter.restore();
}

} // namespace settings
