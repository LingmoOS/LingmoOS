/*
* Copyright (C) 2020 Tianjin LINGMO Information Technology Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/
#include "picturetowhite.h"

PictureToWhite::PictureToWhite(QObject *parent) : QObject(parent)
{
    initGsettingValue();
}

void PictureToWhite::initGsettingValue()
{
    const QByteArray id(ORG_LINGMO_STYLE);
    QStringList stylelist;
    stylelist << STYLE_NAME_KEY_DARK << STYLE_NAME_KEY_LIGHT << STYLE_NAME_KEY_DEFAULT;
    if (QGSettings::isSchemaInstalled(id)) {
        m_pgsettings = new QGSettings(id);
        if (stylelist.contains(m_pgsettings->get(STYLE_NAME).toString()) && m_pgsettings->get(STYLE_NAME).toString() == STYLE_NAME_KEY_LIGHT)
            tray_icon_color = TRAY_ICON_COLOR_LOGHT;
        else
            tray_icon_color = TRAY_ICON_COLOR_DRAK;
    }
    connect(m_pgsettings, &QGSettings::changed, this, [=] (const QString &key) {
        if (key==STYLE_NAME) {
            if (stylelist.contains(m_pgsettings->get(STYLE_NAME).toString()) && m_pgsettings->get(STYLE_NAME).toString() == STYLE_NAME_KEY_LIGHT)
                tray_icon_color = TRAY_ICON_COLOR_LOGHT;
            else
                tray_icon_color = TRAY_ICON_COLOR_DRAK;

        }
    });
}

QPixmap PictureToWhite::drawSymbolicColoredPixmap(const QPixmap &source)
{
    QColor gray(128,128,128);
    QColor standard (31,32,34);
    QImage img = source.toImage();
    for (int x = 0; x < img.width(); x++) {
        for (int y = 0; y < img.height(); y++) {
            auto color = img.pixelColor(x, y);
            if (color.alpha() > 0) {
                if (qAbs(color.red()-gray.red()) < 255 && qAbs(color.green()-gray.green()) < 255 && qAbs(color.blue()-gray.blue()) < 255) {
                    color.setRed(tray_icon_color);
                    color.setGreen(tray_icon_color);
                    color.setBlue(tray_icon_color);
                    img.setPixelColor(x, y, color);
                } else if (qAbs(color.red()-standard.red()) < 255 && qAbs(color.green()-standard.green()) < 255 && qAbs(color.blue()-standard.blue()) < 255) {
                    color.setRed(tray_icon_color);
                    color.setGreen(tray_icon_color);
                    color.setBlue(tray_icon_color);
                    img.setPixelColor(x, y, color);
                } else
                    img.setPixelColor(x, y, color);
            }
        }
    }
    return QPixmap::fromImage(img);
}
