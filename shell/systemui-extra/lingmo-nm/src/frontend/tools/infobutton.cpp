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
#include "infobutton.h"
#include <QEvent>
#include <QPainter>
#include <QPainterPath>
#include <QApplication>
#include <QGSettings>
#include "themepalette.h"

#define BUTTON_SIZE 36,36
#define ICON_SIZE 16,16
#define BACKGROUND_COLOR QColor(0,0,0,0)
#define FOREGROUND_COLOR_NORMAL this->palette().text().color()
#define FOREGROUND_COLOR_BRIGHTTEXT this->palette().brightText().color()
#define FOREGROUND_COLOR_HIGHLIGHT this->palette().highlight().color()
#define OUTER_PATH 8,10,16,16
#define INNER_PATH 9,11,14,14
#define TEXT_POS 14,7,16,16,0

#define BUTTON_SIZE 36,36
#define THEME_SCHAME "org.lingmo.style"
#define COLOR_THEME "styleName"

QColor mixColor(const QColor &c1, const QColor &c2, qreal bias);

InfoButton::InfoButton(QWidget *parent) : QPushButton(parent)
{
    this->setFixedSize(BUTTON_SIZE);
    initUI();
    connect(qApp, &QApplication::paletteChanged, this, &InfoButton::onPaletteChanged);

    const QByteArray id(THEME_SCHAME);
    if (QGSettings::isSchemaInstalled(id)) {
        QGSettings * styleGsettings = new QGSettings(id, QByteArray(), this);
        connect(styleGsettings, &QGSettings::changed, this, [=](QString key){
            if ("themeColor" == key) {
                onPaletteChanged();
            }
        });
    }
    onPaletteChanged();
}

void InfoButton::initUI()
{
    this->setFixedSize(BUTTON_SIZE);
    this->setIcon(QIcon::fromTheme("preferences-system-details-symbolic"));
    this->setProperty("useButtonPalette", true);
    this->setFlat(true);
    m_backgroundColor = BACKGROUND_COLOR;
    m_foregroundColor = FOREGROUND_COLOR_NORMAL;
}

void InfoButton::onPaletteChanged()
{
    QPalette pal = qApp->palette();
    QGSettings * styleGsettings = nullptr;
    const QByteArray styleId(THEME_SCHAME);
    if (QGSettings::isSchemaInstalled(styleId)) {
       styleGsettings = new QGSettings(styleId, QByteArray(), this);
       QString currentTheme = styleGsettings->get(COLOR_THEME).toString();
       if(currentTheme == "lingmo-default"){
           pal = themePalette(true, this);
       }
    }
    pal.setColor(QPalette::Base, pal.color(QPalette::Base));//解决Wayland环境this->setPalette(pal)不生效问题
    pal.setColor(QPalette::Text, pal.color(QPalette::Text));
    pal.setColor(QPalette::Highlight, pal.color(QPalette::Highlight));
    pal.setColor(QPalette::BrightText, pal.color(QPalette::BrightText));

    this->setPalette(pal);

    m_foregroundColor = FOREGROUND_COLOR_NORMAL;
    this->repaint();
}


void InfoButton::enterEvent(QEvent *event)
{
//    m_foregroundColor = FOREGROUND_COLOR_HOVER;
    m_foregroundColor = FOREGROUND_COLOR_HIGHLIGHT;
    this->update();
}

void InfoButton::leaveEvent(QEvent *event)
{
    m_foregroundColor = FOREGROUND_COLOR_NORMAL;
    this->update();
}

void InfoButton::mousePressEvent(QMouseEvent *event)
{
//    m_foregroundColor = FOREGROUND_COLOR_PRESS;
    m_foregroundColor = mixColor(FOREGROUND_COLOR_HIGHLIGHT, FOREGROUND_COLOR_BRIGHTTEXT, 0.2);
    this->update();
    return QPushButton::mousePressEvent(event);
}

void InfoButton::mouseReleaseEvent(QMouseEvent *event)
{
//    m_foregroundColor = FOREGROUND_COLOR_HOVER;
    m_foregroundColor = mixColor(FOREGROUND_COLOR_HIGHLIGHT, FOREGROUND_COLOR_BRIGHTTEXT, 0.2);
    this->update();
    return QPushButton::mouseReleaseEvent(event);
}
