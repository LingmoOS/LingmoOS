/*
 * Qt5-LINGMO's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * Authors: xibowen <xibowen@kylinos.cn>
 *
 */

#ifndef SHADOWHELPER_H
#define SHADOWHELPER_H

#include <QMap>
#include <QSharedPointer>
#include <QPixmap>

#include <KWindowShadow>
#include <KWindowShadowTile>
namespace LINGMOConfigStyleSpace {

class ConfigShadowHelper : public QObject
{
    Q_OBJECT
public:
    explicit ConfigShadowHelper(QObject *parent = nullptr);
    ~ConfigShadowHelper();

    bool isWidgetNeedDecoShadow(const QWidget *widget);

    void registerWidget(QWidget *widget);
    void unregisterWidget(const QWidget *widget);

    QPixmap getShadowPixmap(/*State state,*/
                            QColor color,
                            int shadow_border,
                            qreal darkness,
                            QWidget *widget,
                            int borderRadiusTopLeft = 0,
                            int borderRadiusTopRight = 0,
                            int borderRadiusBottomLeft = 0,
                            int borderRadiusBottomRight = 0);

    QPainterPath caculateRelativePainterPath(QWidget *widget,
                                             qreal borderRadiusTopLeft = 0,
                                             qreal borderRadiusTopRight = 0,
                                             qreal borderRadiusBottomLeft = 0,
                                             qreal borderRadiusBottomRight = 0);


    KWindowShadow *getShadow(QColor color, int shadow_border,
                             qreal darkness, QWidget* widget,
                             int borderRadiusTopLeft = 0,
                             int borderRadiusTopRight = 0,
                             int borderRadiusBottomLeft = 0,
                             int borderRadiusBottomRight = 0);

    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QMap<const QWidget *, KWindowShadow *> m_shadows;
    int m_maxRadius = 8;
};

}
#endif // SHADOWHELPER_H
