/*
 * Copyright (C) 2017 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     zccrs <zccrs@live.com>
 *
 * Maintainer: zccrs <zhangjide@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "chameleonshadow.h"
#include "chameleontheme.h"

#include <QPainter>
#include <QDebug>

#include <cmath>

class _ChameleonShadow : public ChameleonShadow{};
Q_GLOBAL_STATIC(_ChameleonShadow, _global_cs)

ChameleonShadow *ChameleonShadow::instance()
{
    return _global_cs;
}

QString ChameleonShadow::buildShadowCacheKey(const ChameleonTheme::ThemeConfig *config, qreal scale)
{
    auto window_radius = config->radius;
    auto shadow_offset = config->shadowConfig.shadowOffset;
    QColor shadow_color = config->shadowConfig.shadowColor;
    int shadow_size = config->shadowConfig.shadowRadius;
    qreal border_width = config->borderConfig.borderWidth;
    QColor border_color = config->borderConfig.borderColor;

    const QPointF shadow_overlap(qMax(window_radius.x(), 3.0), qMax(window_radius.y(), 3.0));
    const QMargins &paddings = QMargins(shadow_size - shadow_offset.x() - shadow_overlap.x(),
                                        shadow_size - shadow_offset.y() - shadow_overlap.y(),
                                        shadow_size - shadow_overlap.x(),
                                        shadow_size - shadow_overlap.y());

    return QString("%1_%2.%3_%4_%5_%6.%7.%8.%9").arg(qRound(window_radius.x())).arg(qRound(window_radius.y()))
                                                .arg(paddings.left()).arg(paddings.top()).arg(paddings.right()).arg(paddings.bottom())
                                                .arg(shadow_color.name(QColor::HexArgb))
                                                .arg(border_width).arg(border_color.name());
}

QSharedPointer<KDecoration2::DecorationShadow> ChameleonShadow::getShadow(const ChameleonTheme::ThemeConfig *config, qreal scale)
{
    return m_emptyShadow;
    if ((config->shadowConfig.shadowColor.alpha() == 0 || qIsNull(config->shadowConfig.shadowRadius))
            && (config->borderConfig.borderColor.alpha() == 0 || qIsNull(config->borderConfig.borderWidth))) {
        return m_emptyShadow;
    }

    bool no_shadow = config->shadowConfig.shadowColor.alpha() == 0 || qIsNull(config->shadowConfig.shadowRadius);

    auto window_radius = config->radius;
    auto shadow_offset = config->shadowConfig.shadowOffset;
    QColor shadow_color = config->shadowConfig.shadowColor;
    // 因为阴影区域会抹除窗口圆角区域，所以阴影大小需要额外加上窗口圆角大小
    int shadow_size = config->shadowConfig.shadowRadius + window_radius.x() + window_radius.y();

    qreal border_width = config->borderConfig.borderWidth;
    QColor border_color = config->borderConfig.borderColor;

    const QPointF shadow_overlap(qMax(window_radius.x(), 3.0), qMax(window_radius.y(), 3.0));
    const QMargins &paddings = QMargins(shadow_size - shadow_offset.x() - shadow_overlap.x(),
                                        shadow_size - shadow_offset.y() - shadow_overlap.y(),
                                        shadow_size - shadow_overlap.x(),
                                        shadow_size - shadow_overlap.y());
    const QString key = buildShadowCacheKey(config, scale);
    auto shadow = m_shadowCache.value(key);

    if (!shadow) {
        // create image
        qreal shadowStrength = shadow_color.alpha();
        QImage image(2 * shadow_size, 2 * shadow_size, QImage::Format_ARGB32);
        image.fill(Qt::transparent);

        if (!no_shadow) {
            // create gradient
            // gaussian delta function
            auto alpha = [](qreal x) { return std::exp(-x * x / 0.15); };

            // color calculation delta function
            auto gradientStopColor = [] (QColor color, int alpha) {
                color.setAlpha(alpha);
                return color;
            };

            QRadialGradient radialGradient(shadow_size, shadow_size, shadow_size);

            for(int i = 0; i < 10; ++i) {
                const qreal x(qreal(i) / 9);
                radialGradient.setColorAt(x, gradientStopColor(shadow_color, alpha(x) * shadowStrength * 0.6));
            }

            radialGradient.setColorAt(1, gradientStopColor(shadow_color, 0));

            // fill
            QPainter painter(&image);
            painter.setRenderHint( QPainter::Antialiasing, true);
            painter.fillRect(image.rect(), radialGradient);
        }

        // contrast pixel
        QRectF innerRect = QRectF(shadow_size - shadow_offset.x() - shadow_overlap.x(),
                                  shadow_size - shadow_offset.y() - shadow_overlap.y(),
                                  shadow_offset.x() + 2 * shadow_overlap.x(),
                                  shadow_offset.y() + 2 * shadow_overlap.y());

        QPainter painter(&image);

        if (window_radius.x() > 0 && window_radius.y() > 0) {
            painter.setRenderHint(QPainter::Antialiasing, true);
        }

        if (border_width > 0 && border_color.alpha() != 0) {
            painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
            painter.setPen(QPen(border_color, border_width + 1));
            painter.setBrush(Qt::NoBrush);
            if (window_radius.x() > 0 && window_radius.y() > 0) {
                painter.drawRoundedRect(innerRect, window_radius.x() - 0.5, window_radius.y() - 0.5);
            } else {
                painter.drawRect(innerRect);
            }
        }

        if (!no_shadow) {
            painter.setPen(Qt::NoPen);
            painter.setBrush(Qt::black);
            painter.setCompositionMode(QPainter::CompositionMode_DestinationOut);
            if (window_radius.x() > 0 && window_radius.y() > 0) {
                painter.drawRoundedRect(innerRect, 0.5 + window_radius.x(), 0.5 + window_radius.y());
            } else {
                painter.drawRect(innerRect);
            }
        }

        shadow = QSharedPointer<KDecoration2::DecorationShadow>::create();
        shadow->setPadding(paddings);
        shadow->setInnerShadowRect(QRect(shadow_size, shadow_size, 1, 1));
        shadow->setShadow(image);

        m_shadowCache[key] = shadow;
    }

    return shadow;
}

void ChameleonShadow::clearCache()
{
    m_shadowCache.clear();
}

ChameleonShadow::ChameleonShadow()
{
    m_emptyShadow = QSharedPointer<KDecoration2::DecorationShadow>::create();
}
