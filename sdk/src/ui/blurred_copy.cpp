// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/blurred_copy.h"

#include <cmath>

#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkSurface.h"

#include "ui/application.h"
#include "ui/multi_effect_blur.h"
#include "ui/painter.h"
#include "ui/text_context.h"

namespace Lingmo::ui {

struct BlurredCopy::Cache
{
    sk_sp<SkImage> image;
    double scale = 0;
    SizeF size;
    bool stale = true;
};

BlurredCopy::BlurredCopy(double blurMax, double blur)
    : m_blurMax(blurMax)
    , m_blur(blur)
    , m_cache(std::make_unique<Cache>())
{
}

BlurredCopy::~BlurredCopy() = default;

void BlurredCopy::setContentView(std::unique_ptr<View> content)
{
    m_content = std::move(content);
    contentChanged();
    requestLayout();
}

void BlurredCopy::contentChanged()
{
    m_cache->stale = true;
    // The content lies outside the window's tree: its requests stop at it.
    requestLayout();
}

// autoPaddingEnabled: the effect reaches blurMax past the item.
RectF BlurredCopy::visualRect() const
{
    return localRect().adjusted(-m_blurMax, -m_blurMax, m_blurMax, m_blurMax);
}

// The content lies outside the window's tree: its text lays out as the
// window's does only when told the window's scale and hinting.
bool BlurredCopy::adoptTextLayoutMode()
{
    const TextLayoutMode mode = textLayoutMode();
    if (!m_content || m_content->textLayoutMode() == mode)
        return false;
    m_content->setTextLayoutMode(mode);
    return true;
}

void BlurredCopy::layout()
{
    adoptTextLayoutMode();
    if (m_content) {
        m_content->setGeometry(localRect());
        m_content->layoutTree();
    }
    m_cache->stale = true;
}

void BlurredCopy::paint(Painter& painter)
{
    if (!m_content)
        return;
    if (adoptTextLayoutMode()) {
        m_content->setGeometry(localRect());
        m_content->layoutTree();
        m_cache->stale = true;
    }
    const double scale = painter.scale();
    const SizeF size = geometry().size();
    Cache& cache = *m_cache;
    if (cache.stale || !cache.image || scale != cache.scale || !(size == cache.size)) {
        cache.stale = false;
        cache.scale = scale;
        cache.size = size;
        cache.image.reset();
        // The source with its padding, then the blur levels: the
        // first half the source in multiples of eight, each next one half.
        const double width = std::ceil(size.width - 1e-6) + 2 * m_blurMax;
        const double height = std::ceil(size.height - 1e-6) + 2 * m_blurMax;
        const SkImageInfo info = SkImageInfo::MakeN32Premul(textureLength(width, scale), textureLength(height, scale));
        const SkSurfaceProps properties = Application::instance()->text().surfaceProps(false);
        const sk_sp<SkSurface> surface = SkSurfaces::Raster(info, &properties);
        if (!surface)
            return;
        surface->getCanvas()->clear(SK_ColorTRANSPARENT);
        Painter source(surface->getCanvas(), scale);
        source.translate({m_blurMax, m_blurMax});
        m_content->paintTree(source, nullptr);
        SkPixmap pixmap;
        if (!surface->peekPixels(&pixmap))
            return;
        const Texture texture = readTexture(pixmap);
        std::vector<Texture> blurs;
        double itemWidth = std::ceil(width / 16) * 8;
        double itemHeight = std::ceil(height / 16) * 8;
        for (int i = 0; i < multiEffectBlurLevels(m_blurMax); ++i) {
            blurs.push_back(blurPass(i == 0 ? texture : blurs.back(), itemWidth, itemHeight, scale));
            itemWidth /= 2;
            itemHeight /= 2;
        }
        cache.image = composite(texture, blurs, multiEffectBlurWeights(m_blurMax, m_blur), 0, info.height()).asImage();
    }
    if (!cache.image)
        return;
    const PointF origin = painter.offset();
    SkPaint paint;
    paint.setAlphaf(float(painter.opacity()));
    painter.canvas()->drawImage(cache.image, SkScalar(std::round((origin.x - m_blurMax) * scale)),
                                SkScalar(std::round((origin.y - m_blurMax) * scale)), SkSamplingOptions(), &paint);
}

} // namespace Lingmo::ui
