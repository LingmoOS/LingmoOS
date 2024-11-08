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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef PLATFORMTHEMEFONTDATA_H
#define PLATFORMTHEMEFONTDATA_H

#include <QObject>
#include <QFont>
#include <fontconfig/fontconfig.h>
#include <freetype2/ft2build.h>
#include <freetype2/freetype/freetype.h>


typedef struct _FontInformation
{
    QString path;             /* 路径 */
    QString family;           /* 系列 */
    QString style;            /* 样式 */
    //    QString name;             /* 名称 */
    //    QString type;             /* 种类 */
    //    QString version;          /* 版本 */
    //    QString copyright;        /* 版权 */
    //    QString manufacturer;     /* 商标 */
    //    QString description;      /* 描述 */
    //    QString designer;         /* 设计师 */
    //    QString license;          /* 许可证 */
} FontInformation;

class PlatformThemeFontData : public QObject
{
    Q_OBJECT
public:
    PlatformThemeFontData();
    ~PlatformThemeFontData();

    public:
    QMap<QString, QString> getAllFontInformation();
    bool chooseFontFile(QString path);

};

#endif // PLATFORMTHEMEFONTDATA_H
