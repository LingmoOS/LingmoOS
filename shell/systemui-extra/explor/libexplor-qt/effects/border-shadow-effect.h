/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef BORDERSHADOWEFFECT_H
#define BORDERSHADOWEFFECT_H

#include <QGraphicsEffect>
#include <explor-core_global.h>

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
#include <QPainterPath>
#endif

/*!
 * \brief The BorderShadowEffect class
 * \details
 * This class is used to decorate a frameless window.
 * It provides a border shadow which can be adjusted.
 *
 * The effect is similar to QGraphicsDropShadowEffects,
 * but it doesn't blur allow the window. It just render
 * a border for toplevel window, whatever if the window
 * is transparent and whatever element on the window.
 *
 * \note
 * To let the effect works,
 * You have to use it on toplevel window, and let the
 * window has an invisible contents margins.
 *
 * If your window has a border radius, use setBorderRadius()
 * for matching your window border rendering.
 */
class PEONYCORESHARED_EXPORT BorderShadowEffect : public QGraphicsEffect
{
    Q_OBJECT
public:
    explicit BorderShadowEffect(QObject *parent = nullptr);
    void setBorderRadius(int radius);
    void setBorderRadius(int xradius, int yradius);
    void setBlurRadius(int radius);
    void setPadding(int padding);
    void setShadowColor(const QColor &color);

    void setWindowBackground(const QColor &color);

    void setTransParentPath(const QPainterPath &path);
    void setTransParentAreaBg(const QColor &transparentBg);

    void drawWindowShadowManually(QPainter *painter, const QRect &windowRect, bool fakeShadow = false);

protected:
    void draw(QPainter *painter) override;

private:
    int m_x_border_radius = 0;
    int m_y_border_radius = 0;
    int m_blur_radius = 0;
    int m_padding = 0;
    QColor m_shadow_color = QColor(63, 63, 63, 180); // dark gray
    QColor m_window_bg = Qt::transparent;

    QImage m_cache_shadow;
    bool m_force_update_cache = false;

    QPainterPath m_transparent_path;
    QColor m_transparent_bg = QColor(255, 255, 255, 127);
};

#endif // BORDERSHADOWEFFECT_H
