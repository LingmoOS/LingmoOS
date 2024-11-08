/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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

#ifndef LINGMO_QUICK_ITEMS_LINGMO_QUICK_VIEWS_PLUGIN_H
#define LINGMO_QUICK_ITEMS_LINGMO_QUICK_CORE_VIEWS_PLUGIN_H

#include <QQmlExtensionPlugin>

class LingmoUIQuickPlatformPlugin  : public QQmlExtensionPlugin
{
Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)
public:
    void registerTypes(const char *uri) override;
};

#endif //LINGMO_QUICK_ITEMS_LINGMO_QUICK_VIEWS_PLUGIN_H
