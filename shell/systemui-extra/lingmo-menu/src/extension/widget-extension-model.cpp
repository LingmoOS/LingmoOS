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

#include "widget-extension-model.h"
#include "menu-extension-loader.h"

#include <QDebug>

namespace LingmoUIMenu {

WidgetExtensionModel::WidgetExtensionModel(QObject *parent) : QAbstractListModel(parent)
{
    m_widgets = MenuExtensionLoader::instance()->widgets();

    for (int i = 0; i < m_widgets.size(); ++i) {
        connect(m_widgets.at(i), &WidgetExtension::dataUpdated, this, [i, this] {
            Q_EMIT dataChanged(QAbstractListModel::index(i), QAbstractListModel::index(i), {WidgetMetadata::Data});
        });
    }
}

QModelIndex WidgetExtensionModel::parent(const QModelIndex &child) const
{
    return {};
}

int WidgetExtensionModel::rowCount(const QModelIndex &parent) const
{
    return m_widgets.count();
}

int WidgetExtensionModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

QVariant WidgetExtensionModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, QAbstractItemModel::CheckIndexOption::IndexIsValid)) {
        return {};
    }

    int row = index.row();
    WidgetExtension *widget = m_widgets.at(row);

    auto key = WidgetMetadata::Key(role);
    switch (key) {
        case WidgetMetadata::Id:
        case WidgetMetadata::Icon:
        case WidgetMetadata::Name:
        case WidgetMetadata::Tooltip:
        case WidgetMetadata::Version:
        case WidgetMetadata::Description:
        case WidgetMetadata::Main:
        case WidgetMetadata::Type:
        case WidgetMetadata::Flag:
            return widget->metadata().value(key, {});
        case WidgetMetadata::Data:
            return widget->data();
        default:
            break;
    }

    return {};
}

QHash<int, QByteArray> WidgetExtensionModel::roleNames() const
{
    QHash<int, QByteArray> hash;

    hash.insert(WidgetMetadata::Id, "id");
    hash.insert(WidgetMetadata::Icon, "icon");
    hash.insert(WidgetMetadata::Name, "name");
    hash.insert(WidgetMetadata::Tooltip, "tooltip");
    hash.insert(WidgetMetadata::Version, "version");
    hash.insert(WidgetMetadata::Description, "description");
    hash.insert(WidgetMetadata::Main, "main");
    hash.insert(WidgetMetadata::Type, "type");
    hash.insert(WidgetMetadata::Flag, "flag");
    hash.insert(WidgetMetadata::Data, "data");

    return hash;
}

WidgetExtension *WidgetExtensionModel::widgetAt(int index) const
{
    if (index < 0 || index >= m_widgets.count()) {
        return nullptr;
    }

    return m_widgets.at(index);
}

void WidgetExtensionModel::notify(int index, const QVariantMap &data) const
{
    WidgetExtension *widget = widgetAt(index);
    if (widget) {
        widget->receive(data);
    }
}

WidgetExtensionModel *WidgetExtensionModel::instance()
{
    static WidgetExtensionModel model;
    return &model;
}

} // LingmoUIMenu
