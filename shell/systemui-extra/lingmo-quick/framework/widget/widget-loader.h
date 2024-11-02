/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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

#ifndef LINGMO_QUICK_WIDGET_LOADER_H
#define LINGMO_QUICK_WIDGET_LOADER_H

#include <QObject>
#include <QString>

#include "widget-metadata.h"

namespace LingmoUIQuick {

class Widget;
class WidgetLoaderPrivate;

class WidgetLoader : public QObject
{
    Q_OBJECT
public:
    explicit WidgetLoader(QObject *parent = nullptr);
    ~WidgetLoader() override;

    Widget *loadWidget(const QString &id);
    WidgetMetadata loadMetadata(const QString &id);

    void addWidgetSearchPath(const QString &path);
    void setShowInFilter(WidgetMetadata::Hosts hosts);

    static WidgetLoader &globalLoader();

private:
    WidgetLoaderPrivate *d {nullptr};
};

} // LingmoUIQuick

#endif //LINGMO_QUICK_WIDGET_LOADER_H
