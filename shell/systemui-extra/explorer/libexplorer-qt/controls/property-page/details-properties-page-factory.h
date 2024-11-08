/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2021, KylinSoft Co., Ltd.
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
 * Authors: Wenfei He <hewenfei@kylinos.cn>
 *
 */

#ifndef PEONY_DETAILS_PROPERTIES_PAGE_FACTORY_H
#define PEONY_DETAILS_PROPERTIES_PAGE_FACTORY_H

#include "properties-window-tab-page-plugin-iface.h"

namespace Peony {

class DetailsPropertiesPageFactory : public QObject, public PropertiesWindowTabPagePluginIface
{
Q_OBJECT
public:
    static DetailsPropertiesPageFactory *getInstance();

    //plugin iface
    const QString name() override {
        return QObject::tr("Details");
    }
    PluginType pluginType() override {
        return PluginType::PropertiesWindowPlugin;
    }
    const QString description() override {
        return QObject::tr("Details");
    }
    const QIcon icon() override {
        return QIcon::fromTheme("view-paged-symbolic", QIcon::fromTheme("folder"));
    }
    void setEnable(bool enable) override {
        Q_UNUSED(enable)
    }
    bool isEnable() override {
        return true;
    }

    //properties plugin iface
    int tabOrder() override {
        return 600;
    }
    bool supportUris(const QStringList &uris) override;

    PropertiesWindowTabIface *createTabPage(const QStringList &uris) override;

    void closeFactory() override;
private:
    explicit DetailsPropertiesPageFactory();

    ~DetailsPropertiesPageFactory();
};

}

#endif //PEONY_DETAILS_PROPERTIES_PAGE_FACTORY_H
