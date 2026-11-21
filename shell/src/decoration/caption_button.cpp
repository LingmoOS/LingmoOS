// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "decoration/caption_button.h"

#include <cmath>

#include "ui/bitmap.h"
#include "ui/bitmap_painter.h"
#include "ui/path.h"

namespace Lingmo::Decoration {

CaptionButton::CaptionButton(CaptionButtonType type)
    : m_type(type)
{
}

bool CaptionButton::setHovered(bool hovered)
{
    if (hovered == m_hovered)
        return false;
    m_hovered = hovered;
    return true;
}

bool CaptionButton::setPressed(bool pressed)
{
    if (pressed == m_pressed)
        return false;
    m_pressed = pressed;
    return true;
}

Image renderCaptionIcon(CaptionButtonType type, bool restore, int size,
                        const render::Color& color)
{
    Image image(base::Size(size, size));
    if (size <= 0)
        return image;

    // FishUI src/images/light/{close,minimize,maximize,restore}.svg, on its 24px grid.
    ui::Path path;
    path.setFillRule(ui::Path::FillRule::Winding);
    if (type == CaptionButtonType::Close) {
        path.moveTo(16.2241, 7.13313);
        path.cubicTo(16.4016, 6.95562, 16.6894, 6.95562, 16.8669, 7.13313);
        path.cubicTo(17.0444, 7.31065, 17.0444, 7.59838, 16.8669, 7.77589);
        path.lineTo(12.6428, 12);
        path.lineTo(16.8669, 16.2241);
        path.cubicTo(17.0444, 16.4016, 17.0444, 16.6894, 16.8669, 16.8669);
        path.cubicTo(16.6894, 17.0444, 16.4016, 17.0444, 16.2241, 16.8669);
        path.lineTo(12, 12.6428);
        path.lineTo(7.77589, 16.8669);
        path.cubicTo(7.59838, 17.0444, 7.31065, 17.0444, 7.13313, 16.8669);
        path.cubicTo(6.95562, 16.6894, 6.95562, 16.4016, 7.13313, 16.2241);
        path.lineTo(11.3572, 12);
        path.lineTo(7.13313, 7.77589);
        path.cubicTo(6.95562, 7.59838, 6.95562, 7.31065, 7.13313, 7.13313);
        path.cubicTo(7.31065, 6.95562, 7.59838, 6.95562, 7.77589, 7.13313);
        path.lineTo(12, 11.3572);
        path.lineTo(16.2241, 7.13313);
        path.close();
    } else if (type == CaptionButtonType::Minimize) {
        path.moveTo(16.529, 13);
        path.cubicTo(16.7891, 13, 17, 12.7891, 17, 12.529);
        path.lineTo(17, 12.471);
        path.cubicTo(17, 12.2109, 16.7891, 12, 16.529, 12);
        path.lineTo(7.471, 12);
        path.cubicTo(7.21087, 12, 7, 12.2109, 7, 12.471);
        path.lineTo(7, 12.529);
        path.cubicTo(7, 12.7891, 7.21087, 13, 7.471, 13);
        path.lineTo(16.529, 13);
        path.close();
    } else if (!restore) {
        path.moveTo(14.5, 16.0002);
        path.lineTo(14.5, 17);
        path.lineTo(9.5, 17);
        path.lineTo(9.5, 16.0002);
        path.lineTo(14.5, 16.0002);
        path.close();
        path.moveTo(16.0002, 14.5);
        path.lineTo(16.0002, 9.5);
        path.cubicTo(16.0002, 8.67157, 15.3284, 7.99976, 14.5, 7.99976);
        path.lineTo(9.5, 7.99976);
        path.cubicTo(8.67157, 7.99976, 7.99976, 8.67157, 7.99976, 9.5);
        path.lineTo(7.99976, 14.5);
        path.cubicTo(7.99976, 15.3284, 8.67157, 16.0002, 9.5, 16.0002);
        path.lineTo(9.5, 17);
        path.cubicTo(8.11929, 17, 7, 15.8807, 7, 14.5);
        path.lineTo(7, 9.5);
        path.cubicTo(7, 8.11929, 8.11929, 7, 9.5, 7);
        path.lineTo(14.5, 7);
        path.lineTo(14.7551, 7.01343);
        path.cubicTo(16.016, 7.14126, 17, 8.20544, 17, 9.5);
        path.lineTo(17, 14.5);
        path.lineTo(16.9866, 14.7551);
        path.cubicTo(16.8587, 16.016, 15.7946, 17, 14.5, 17);
        path.lineTo(14.5, 16.0002);
        path.cubicTo(15.3284, 16.0002, 16.0002, 15.3284, 16.0002, 14.5);
        path.close();
    } else {
        path.moveTo(8.53849, 8.67291);
        path.cubicTo(7.68616, 8.67291, 7, 9.41569, 7, 10.3383);
        path.lineTo(7, 15.3346);
        path.cubicTo(7, 16.2572, 7.68616, 17, 8.53849, 17);
        path.lineTo(13.9233, 17);
        path.cubicTo(14.7756, 17, 15.4618, 16.2572, 15.4618, 15.3346);
        path.lineTo(15.4618, 10.3383);
        path.cubicTo(15.4618, 9.41569, 14.7756, 8.67291, 13.9233, 8.67291);
        path.lineTo(8.53849, 8.67291);
        path.close();
        path.moveTo(8.53849, 10.3383);
        path.lineTo(13.9233, 10.3383);
        path.cubicTo(14.3494, 10.3383, 14.6925, 10.7098, 14.6925, 11.1711);
        path.lineTo(14.6925, 15.3347);
        path.cubicTo(14.6925, 15.796, 14.3494, 16.1673, 13.9233, 16.1673);
        path.lineTo(8.53849, 16.1673);
        path.cubicTo(8.11235, 16.1673, 7.76926, 15.796, 7.76926, 15.3347);
        path.lineTo(7.76926, 11.1711);
        path.cubicTo(7.76926, 10.7098, 8.11235, 10.3383, 8.53849, 10.3383);
        path.close();
        path.moveTo(10.0768, 7);
        path.cubicTo(9.50676, 7, 9.01606, 7.33654, 8.75018, 7.83266);
        path.lineTo(14.6923, 7.83266);
        path.cubicTo(15.5446, 7.83266, 16.2308, 8.57543, 16.2308, 9.49808);
        path.lineTo(16.2308, 15.0978);
        path.cubicTo(16.6891, 14.8102, 17, 14.2788, 17, 13.6618);
        path.lineTo(17, 9.49818);
        path.cubicTo(17, 8.11422, 15.9708, 7, 14.6923, 7);
        path.lineTo(10.0768, 7);
        path.close();
    }

    ui::Bitmap mask(size, size, ui::Bitmap::Format::Argb32Premultiplied);
    mask.fill(0);
    {
        ui::BitmapPainter painter(mask);
        painter.setAntialiasing(true);
        painter.scale(size / 24.0, size / 24.0);
        painter.fillPath(path, ui::Color::rgb(0xFFFFFF));
    }
    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x)
            image.blend(x, y, color, mask.alpha(x, y) / 255.0f);
    }
    return image;
}

Image renderButtonHighlight(int buttonSize, const render::Color& color)
{
    // FishUI's round image button rounds its 10% background margin.
    const int diameter = buttonSize - 2 * static_cast<int>(std::lround(buttonSize * 0.1));
    Image image(base::Size(diameter, diameter));
    if (diameter <= 0)
        return image;
    fillRoundedRect(image, base::RectF(0, 0, diameter, diameter), diameter / 2.0, color);
    return image;
}

} // namespace Lingmo::Decoration
