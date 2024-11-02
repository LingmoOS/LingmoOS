/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2022 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "divider.h"
#include <QPainter>
#include <QApplication>

#include "../netdetails/coninfo.h"
#include "themepalette.h"

#define THEME_SCHAME "org.lingmo.style"
#define COLOR_THEME "styleName"

Divider::Divider(bool useDarkPal, QWidget * parent)
    :m_useDarkPal(useDarkPal),
     QFrame(parent)
{
    this->setFixedHeight(1);
    connect(qApp, &QApplication::paletteChanged, this ,&Divider::onPaletteChanged);
    onPaletteChanged();
}

void Divider::onPaletteChanged()
{
    QPalette pal = qApp->palette();
    QGSettings * styleGsettings = nullptr;
    const QByteArray styleId(THEME_SCHAME);
    if (QGSettings::isSchemaInstalled(styleId) && m_useDarkPal) {
       styleGsettings = new QGSettings(styleId, QByteArray(), this);
       QString currentTheme = styleGsettings->get(COLOR_THEME).toString();
       if(currentTheme == "lingmo-default"){
           pal = themePalette(true, this);
       }
    }

    m_color = pal.color(QPalette::BrightText);
    m_color.setAlphaF(0.08);
}

void Divider::paintEvent(QPaintEvent * e)
{
    QPainter p(this);
    p.save();
    p.setBrush(m_color);
    p.setPen(Qt::transparent);
    p.drawRoundedRect(this->rect(), 6, 6);
    p.restore();
    return QFrame::paintEvent(e);
}
