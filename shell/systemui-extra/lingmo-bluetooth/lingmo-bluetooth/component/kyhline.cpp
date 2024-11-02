/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/

#include "kyhline.h"

KyHLine::KyHLine(QWidget *parent) : QFrame(parent)
{
    this->setFixedHeight(1);
    if(QGSettings::isSchemaInstalled("org.lingmo.style")) {
        StyleSettings = new QGSettings("org.lingmo.style");
        connect(StyleSettings,&QGSettings::changed,this,[=](const QString &key) {
            if(key == "styleName")
                this->update();
        });
    }
}

void KyHLine::paintEvent(QPaintEvent * e)
{
    QPainter p(this);
    QColor color;
    if(StyleSettings->get("style-name").toString() == "lingmo-default"){
        color = QColor(217, 217, 217);
    } else
        color = qApp->palette().color(QPalette::Active, QPalette::BrightText);
    color.setAlphaF(0.08);
    p.save();
    p.setBrush(color);
    p.setPen(Qt::transparent);
    p.drawRoundedRect(this->rect(), 6, 6);
    p.restore();
    return QFrame::paintEvent(e);
}
