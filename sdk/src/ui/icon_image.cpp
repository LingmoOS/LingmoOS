// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/icon_image.h"

#include <cmath>
#include <utility>

#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"

#include "ui/icon_loader.h"
#include "ui/painter.h"

namespace Lingmo::ui {

IconImage::IconImage(std::string name, int pixels)
    : m_name(std::move(name))
    , m_pixels(pixels)
{
}

void IconImage::setName(std::string name)
{
    m_name = std::move(name);
    requestPaint();
}

void IconImage::setPixels(int pixels)
{
    m_pixels = pixels;
    requestPaint();
}

void IconImage::setFallbackName(std::string name)
{
    m_fallback = std::move(name);
    requestPaint();
}

bool IconImage::preload(const std::string& name, int pixels)
{
    return IconLoader::instance().pixmap(name, pixels) != nullptr;
}

void IconImage::paint(Painter& painter)
{
    IconLoader& loader = IconLoader::instance();
    sk_sp<SkImage> image = m_name.empty() ? nullptr : loader.pixmap(m_name, m_pixels);
    if (!image && !m_fallback.empty())
        image = loader.pixmap(m_fallback, m_pixels);
    if (!image)
        return;
    const double scale = painter.scale();
    const PointF origin = painter.offset();
    SkPaint paint;
    paint.setAlphaf(float(painter.opacity()));
    // At its own size the pixmap is copied to the rounded device position,
    // unfiltered; otherwise it scales it smoothly.
    const double width = geometry().width * scale;
    const double height = geometry().height * scale;
    if (std::abs(width - image->width()) < 1e-6 && std::abs(height - image->height()) < 1e-6) {
        painter.canvas()->drawImage(image, SkScalar(std::floor(origin.x * scale + 0.5)),
                                    SkScalar(std::floor(origin.y * scale + 0.5)), SkSamplingOptions(), &paint);
        return;
    }
    const SkRect target = SkRect::MakeXYWH(SkScalar(origin.x * scale), SkScalar(origin.y * scale),
                                           SkScalar(width), SkScalar(height));
    painter.canvas()->drawImageRect(image, target, SkSamplingOptions(SkFilterMode::kLinear), &paint);
}

} // namespace Lingmo::ui
