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
 */

#ifndef LINGMO_MENU_APP_SEARCH_PLUGIN_H
#define LINGMO_MENU_APP_SEARCH_PLUGIN_H

#include "data-provider-plugin-iface.h"

namespace LingmoUIMenu {

class AppSearchPluginPrivate;

class AppSearchPlugin : public DataProviderPluginIFace
{
    Q_OBJECT
public:
    AppSearchPlugin();
    ~AppSearchPlugin() override;

    int index() override;
    QString id() override;
    QString name() override;
    QString icon() override;
    QString title() override;
    PluginGroup::Group group() override;
    QVector<DataEntity> data() override;
    void forceUpdate() override;
    void forceUpdate(QString &key) override;

private Q_SLOTS:
    void onSearchedOne(const DataEntity &app);

private:
    AppSearchPluginPrivate *m_searchPluginPrivate{nullptr};
};

} // LingmoUIMenu

#endif //LINGMO_MENU_APP_SEARCH_PLUGIN_H
