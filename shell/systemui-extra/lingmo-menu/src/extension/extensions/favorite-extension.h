/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
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

#ifndef LINGMO_MENU_FAVORITE_EXTENSION_H
#define LINGMO_MENU_FAVORITE_EXTENSION_H

#include "../menu-extension-iface.h"

namespace LingmoUIMenu {

class FavoriteAppsModel;

class FavoriteExtension : public MenuExtensionIFace
{
    Q_OBJECT
public:
    explicit FavoriteExtension(QObject *parent = nullptr);
    ~FavoriteExtension();

    void receive(QVariantMap data) override;
    int index() override;
    QString name() override;
    QUrl url() override;
    QVariantMap data() override;

private:
    QVariantMap m_data;
    FavoriteAppsModel *m_favoriteAppsModel = nullptr;

private:
    void updateFavoriteData();
    void openFavoriteApp(const QString &path);
};

} // LingmoUIMenu

#endif //LINGMO_MENU_FAVORITE_EXTENSION_H
