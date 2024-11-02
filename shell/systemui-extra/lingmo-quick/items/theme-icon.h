/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

#ifndef LINGMO_QUICK_ITEMS_THEME_ICON_H
#define LINGMO_QUICK_ITEMS_THEME_ICON_H

#include <QQuickPaintedItem>
#include <QIcon>
#include "icon.h"

namespace LingmoUIQuick {

class ThemeIcon : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(QVariant source READ getSource WRITE setSource)
    Q_PROPERTY(QString fallback READ getFallBack WRITE setFallBack)
    Q_PROPERTY(int radius READ radius WRITE setRadius)
    Q_PROPERTY(LingmoUIQuick::Icon::Mode mode READ mode WRITE setMode)
    Q_PROPERTY(bool keepAspectRatio READ getKeepAspectRatio WRITE setKeepAspectRatio)
public:
    explicit ThemeIcon(QQuickItem *parent = nullptr);
    static bool isPixmapPureColor(const QPixmap &pixmap);
    static QColor getSymbolicColor();

    void paint(QPainter *painter) override;

    QVariant getSource();
    void setSource(const QVariant& source);

    QString getFallBack();
    void setFallBack(const QString &fallback);

    Icon::Mode mode() const;
    void setMode(Icon::Mode mode);

    int radius();
    void setRadius(int radius);

    bool getKeepAspectRatio() const;
    void setKeepAspectRatio(bool keepAspectRatio);

private Q_SLOTS:
    void updateMode();

private:
    void updateRawIcon();

private:
    int        m_radius = 0;
    Icon::Mode m_mode;
    QIcon      m_rawIcon;
    QVariant   m_source;
    QString    m_fallback;
    bool       m_keepAspectRatio;
    QSize      m_sourceSize;

    static QColor symbolicColor;
};

}
#endif //LINGMO_QUICK_ITEMS_THEME_ICON_H
