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
 * Authors: Yan Wang <wangyan@kylinos.cn>
 *
 */

#include "kystylehelper.h"

#include <QQuickWindow>
#include <QGSettings/QGSettings>

#include <QWidget>

KyStyleHelper::KyStyleHelper(QQuickItem *parent)
    : QQuickItem(parent)
{
    if (QGSettings::isSchemaInstalled("org.lingmo.style")) {
        QGSettings* styleSettings = new QGSettings("org.lingmo.style", QByteArray(), this);
        connect(styleSettings, &QGSettings::changed, this, [&](const QString &key){
            if (key == "styleName" || key == "themeColor") {
                emit paletteChanged();
                emit qcolorChanged();
            }
            if (key == "systemFontSize" || key == "systemFont") {
                emit fontChanged();
            }
        });
    }
}

KyStyleHelper::~KyStyleHelper() {}

KyStyleHelper* KyStyleHelper::qmlAttachedProperties(QObject *parent)
{
    auto p = qobject_cast<QQuickItem*>(parent);
    return new KyStyleHelper(p);
}
