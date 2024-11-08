/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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

#include "app-model.h"
#include "app-manager.h"
#include "context-menu-manager.h"
#include "app-folder-helper.h"
#include "settings.h"

#include <QObject>
#include <QString>
#include <QDebug>
using namespace LingmoUIMenu;

AppModel::AppModel(QObject *parent) : QAbstractListModel(parent)
{
    reloadPluginData();
    connect(DataProviderManager::instance(), &DataProviderManager::dataChanged, this, &AppModel::onPluginDataChanged);
    connect(DataProviderManager::instance(), &DataProviderManager::pluginChanged, this, &AppModel::reloadPluginData);
    connect(GlobalSetting::instance(), &GlobalSetting::styleChanged, this, &AppModel::onStyleChanged);
}

int AppModel::rowCount(const QModelIndex &parent) const
{
    return m_apps.size();
}

QVariant AppModel::data(const QModelIndex &index, int role) const
{
    int i = index.row();
    if (i < 0 || i >= m_apps.size()) {
        return {};
    }

    switch (role) {
        case DataEntity::Id:
            return m_apps.at(i).id();
        case DataEntity::Type:
            return m_apps.at(i).type();
        case DataEntity::Icon:
            return m_apps.at(i).icon();
        case DataEntity::Name:
            return m_apps.at(i).name();
        case DataEntity::Comment:
            return m_apps.at(i).comment();
        case DataEntity::ExtraData:
            return m_apps.at(i).extraData();
        case DataEntity::Favorite:
            return m_apps.at(i).favorite() == 0 ? false : true;
        default:
            break;
    }

    return {};
}

QHash<int, QByteArray> AppModel::roleNames() const
{
    return DataEntity::AppRoleNames();
}

int AppModel::getLabelIndex(const QString &id)
{
    for (int index = 0; index < m_apps.count(); ++index) {
        if (m_apps.at(index).type() == DataType::Label && m_apps.at(index).id() == id) {
            return index;
        }
    }
    return -1;
}

QVariantList AppModel::folderApps(const QString &folderName)
{
    DataEntity item1{DataType::Normal, "name 1", "icon", "comment", "extra"};
    DataEntity item2{DataType::Normal, "name 2", "icon", "comment", "extra"};
    DataEntity item3{DataType::Label, "name 3", "icon", "comment", "extra"};
    DataEntity item4{DataType::Normal, "name 4", "icon", "comment", "extra"};

    QVariantList list;
    list.append(QVariant::fromValue(item1));
    list.append(QVariant::fromValue(item2));
    list.append(QVariant::fromValue(item3));
    list.append(QVariant::fromValue(item4));

    return list;
}

void AppModel::appClicked(const int &index)
{
    if (index < 0 || index >= m_apps.size()) {
        return;
    }

    AppManager::instance()->launchApp(m_apps.at(index).id());
}

void AppModel::reloadPluginData()
{
    QVector<DataEntity> data = DataProviderManager::instance()->data();
    resetModel(data);
}

void AppModel::onStyleChanged(const GlobalSetting::Key &key)
{
    if (key == GlobalSetting::IconThemeName) {
        Q_EMIT beginResetModel();
        Q_EMIT endResetModel();
    }
}

void AppModel::toRenameFolder(QString id)
{
    Q_EMIT renameText(id);
}

void AppModel::onPluginDataChanged(QVector<DataEntity> data, DataUpdateMode::Mode mode, quint32 index)
{
    switch (mode) {
        default:
        case DataUpdateMode::Reset: {
            resetModel(data);
            break;
        }
        case DataUpdateMode::Append: {
            appendData(data);
            break;
        }
        case DataUpdateMode::Prepend: {
            prependData(data);
            break;
        }
        case DataUpdateMode::Insert: {
            insertData(data, static_cast<int>(index));
            break;
        }
        case DataUpdateMode::Update: {
            updateData(data);
            break;
        }
    }
}

void AppModel::resetModel(QVector<DataEntity> &data)
{
    Q_EMIT beginResetModel();
    m_apps.swap(data);
    Q_EMIT endResetModel();
}

void AppModel::appendData(QVector<DataEntity> &data)
{
    Q_EMIT beginInsertRows(QModelIndex(), m_apps.size(), (m_apps.size() + data.size() - 1));
    m_apps.append(data);
    Q_EMIT endInsertRows();
}

void AppModel::prependData(QVector<DataEntity> &data)
{
    Q_EMIT beginInsertRows(QModelIndex(), 0, data.size() - 1);
    data.append(m_apps);
    m_apps.swap(data);
    Q_EMIT endInsertRows();
}

void AppModel::insertData(QVector<DataEntity> &data, int index)
{
    if (index < 0 || (!m_apps.isEmpty() && index >= m_apps.size())) {
        return;
    }

    Q_EMIT beginInsertRows(QModelIndex(), index, index + data.size() - 1);

    for (const auto &item : data) {
        m_apps.insert(index, item);
        ++index;
    }

    Q_EMIT endInsertRows();
}

void AppModel::updateData(QVector<DataEntity> &data)
{
    for (const DataEntity &item : data) {
        for (int i = 0; i < m_apps.length(); i++) {
            if (item.id() == m_apps.at(i).id()) {
                m_apps.replace(i, item);
                Q_EMIT dataChanged(index(i, 0), index(i, 0));
                break;
            }
        }
    }
}

void AppModel::openMenu(const int &index, MenuInfo::Location location)
{
    if (index < 0 || index >= m_apps.size()) {
        return;
    }

    ContextMenuManager::instance()->showMenu(m_apps.at(index), location, DataProviderManager::instance()->activatedProvider());
}

void AppModel::renameFolder(const QString &index, const QString &folderName)
{
    AppFolderHelper::instance()->renameFolder(index.toInt(), folderName);
}

void AppModel::addAppsToFolder(const QString &appIdA, const QString &appIdB, const QString &folderName)
{
    AppFolderHelper::instance()->addAppsToNewFolder(appIdA, appIdB, folderName);
}

void AppModel::addAppToFolder(const QString &appId, const QString &folderId)
{
    AppFolderHelper::instance()->addAppToFolder(appId, folderId.toInt());
}

QVariantList AppModel::getApps(int start, int end)
{
    if (start < 0 || start >= m_apps.size() || end < 0 || end > m_apps.size()) {
        return {};
    }

    QVariantList list;
    for (int i = start; i < end; ++i) {
        list.append(QVariant::fromValue(m_apps.at(i)));
    }

    return list;
}
