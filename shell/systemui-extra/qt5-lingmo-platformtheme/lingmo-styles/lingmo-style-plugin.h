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

#ifndef QT5LINGMOSTYLEPLUGIN_H
#define QT5LINGMOSTYLEPLUGIN_H

#include <QStylePlugin>

#if (QT_VERSION >= QT_VERSION_CHECK(5,12,0))
#include<private/qfusionstyle_p.h>
#define Style QFusionStyle
#else
#define Style QProxyStyle
#endif

class LINGMOStylePlugin : public QStylePlugin
{
    Q_OBJECT
#if QT_VERSION >= 0x050000
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QStyleFactoryInterface" FILE "qt5-style-lingmo.json")
#endif // QT_VERSION >= 0x050000

public:
    LINGMOStylePlugin(QObject *parent = 0);

    QStyle *create(const QString &key) override;

    const QStringList blackList();

protected:
    void onSystemPaletteChanged();
    void tableModeChanged(bool isTableMode);

private:
    QString m_current_style_name;
    QString m_current_theme_name;

};

#endif // QT5LINGMOSTYLEPLUGIN_H
