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
#ifndef PICTURETOWHITE_H
#define PICTURETOWHITE_H

#include <QObject>
#include <QGSettings>
#include <QString>
#include <QVariant>
#include <QPixmap>
#include <QDebug>

#define ORG_LINGMO_STYLE            "org.lingmo.style"
#define STYLE_NAME                "styleName"
#define STYLE_NAME_KEY_DARK       "lingmo-dark"
#define STYLE_NAME_KEY_DEFAULT    "lingmo-default"
#define STYLE_NAME_KEY_BLACK       "lingmo-black"
#define STYLE_NAME_KEY_LIGHT       "lingmo-light"
#define STYLE_NAME_KEY_WHITE       "lingmo-white"
#define TRAY_ICON_COLOR_LOGHT      0
#define TRAY_ICON_COLOR_DRAK       255

class PictureToWhite : public QObject
{
    Q_OBJECT
public:
    explicit PictureToWhite(QObject *parent = nullptr);
    void initGsettingValue();
    QPixmap drawSymbolicColoredPixmap(const QPixmap &source);

public:
    QGSettings  *m_pgsettings;
    int         tray_icon_color;

};

#endif // PICTURETOWHITE_H
