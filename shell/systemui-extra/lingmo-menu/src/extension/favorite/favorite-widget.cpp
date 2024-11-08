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

#include "favorite-widget.h"
#include "app-favorite-model.h"
#include "favorites-model.h"

namespace LingmoUIMenu {

FavoriteWidget::FavoriteWidget(QObject *parent) : WidgetExtension(parent)
{
    m_metadata.insert(WidgetMetadata::Id, "favorite");
    m_metadata.insert(WidgetMetadata::Icon, "non-starred-symbolic");
    m_metadata.insert(WidgetMetadata::Name, tr("Favorite"));
    m_metadata.insert(WidgetMetadata::Tooltip, tr("favorite"));
    m_metadata.insert(WidgetMetadata::Version, "1.0.0");
    m_metadata.insert(WidgetMetadata::Description, "favorite");
    m_metadata.insert(WidgetMetadata::Main, "qrc:///qml/extensions/FavoriteExtension.qml");
    m_metadata.insert(WidgetMetadata::Type, WidgetMetadata::Widget);
    m_metadata.insert(WidgetMetadata::Flag, WidgetMetadata::Normal);

    FavoritesModel::instance().setSourceModel(&AppFavoritesModel::instance());
    FavoritesModel::instance().sort(0);
    m_data.insert("favoriteAppsModel", QVariant::fromValue(&FavoritesModel::instance()));
    m_data.insert("folderModel", QVariant::fromValue(&FolderModel::instance()));
}

int FavoriteWidget::index() const
{
    return 0;
}

MetadataMap FavoriteWidget::metadata() const
{
    return m_metadata;
}

QVariantMap FavoriteWidget::data()
{
    return m_data;
}

void FavoriteWidget::receive(const QVariantMap &data)
{
    WidgetExtension::receive(data);
}

} // LingmoUIMenu
