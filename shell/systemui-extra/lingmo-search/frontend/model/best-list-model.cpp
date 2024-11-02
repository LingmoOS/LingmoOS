/*
 *
 * Copyright (C) 2021, KylinSoft Co., Ltd.
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
 * Authors: jixiaoxu <jixiaoxu@kylinos.cn>
 *
 */
#include "best-list-model.h"
#include "search-plugin-manager.h"
using namespace LingmoUISearch;

BestListModel::BestListModel(QObject *parent)
    : QAbstractItemModel(parent)
{
}

QModelIndex BestListModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    if(row < 0 || row > m_items.length() - 1)
        return QModelIndex();
    return createIndex(row, column, nullptr);
}

QModelIndex BestListModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return QModelIndex();
}

int BestListModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : (m_isExpanded ? m_items.length() : NUM_LIMIT_SHOWN_DEFAULT);
}

int BestListModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : 1;
}

QVariant BestListModel::data(const QModelIndex &index, int role) const
{
    switch(role) {
        case Qt::DecorationRole: {
            return m_items.at(index.row())->info().icon;
        }
        case Qt::DisplayRole: {
            return m_items.at(index.row())->info().name;
        }
        case Qt::ToolTipRole: {
            return m_items.at(index.row())->info().toolTip;
        }
        case AdditionalRoles::ShowToolTip: {
            return m_items.at(index.row())->showToolTip();
        }

        default:
        return {};
    }
}

const SearchPluginIface::ResultInfo &BestListModel::getInfo(const QModelIndex &index)
{
    return m_items.at(index.row())->info();
}

const QString &BestListModel::getPluginInfo(const QModelIndex &index)
{
    return m_pluginIdList.at(index.row());
}

void BestListModel::setExpanded(const bool &is_expanded)
{
    this->beginResetModel();
    m_isExpanded = is_expanded;
    this->endResetModel();
    Q_EMIT this->itemListChanged(m_items.length());
}

const bool &BestListModel::isExpanded()
{
    return m_isExpanded;
}

void BestListModel::appendInfo(const QString &pluginId, const SearchPluginIface::ResultInfo &info)
{
    if (m_resourceTypes.contains(info.resourceType)) {
        for (const auto& item : m_items) {
            if (info.actionKey == item->info().actionKey && info.resourceType == item->info().resourceType) {
                return;
            }
        }
    } else {
        m_resourceTypes.append(info.resourceType);
    }
    if (m_pluginIdList.contains(pluginId)) {
        if (info.actionKey == m_items.at(m_pluginIdList.lastIndexOf(pluginId))->info().actionKey) {
            return;
        }
        this->beginResetModel();
        delete m_items.takeAt(m_pluginIdList.lastIndexOf(pluginId));
        m_items.insert(m_pluginIdList.lastIndexOf(pluginId), new OneResult(info));
        this->endResetModel();
        return;
    }
    this->beginResetModel();
//    qDebug()<<"plugin ID:"<<pluginId<<"Got a result. name ="<<info.name;
    m_pluginIdList.append(pluginId);
    m_items.append(new OneResult(info));
    QVector<OneResult *> result_info_list_tmp;
    QVector<QString> plugin_id_list_tmp;

    QList<PluginInfo> infoList = SearchPluginManager::getInstance()->getPluginIds();
    QVector<QString> orders(infoList.length());
    for (const PluginInfo& info : infoList) {
        if (info.isEnable() and info.order() > 0) {
            if (info.order() > orders.size()) {
                QVector<QString> tmpVct(info.order() - orders.size());
                orders.append(tmpVct);
            }
            orders[info.order() - 1] = info.name();
        }
    }
    orders.removeAll("");

    Q_FOREACH (const QString& plugin, orders) {
        if (m_pluginIdList.contains(plugin)) {
            result_info_list_tmp.append(m_items.at(m_pluginIdList.lastIndexOf(plugin)));
            plugin_id_list_tmp.append(plugin);
        }
    }
    m_items.clear();
    m_items.swap(result_info_list_tmp);
    m_pluginIdList.clear();
    m_pluginIdList.swap(plugin_id_list_tmp);
    this->endResetModel();
    Q_EMIT this->itemListChanged(m_items.length());

}

void BestListModel::removeInfo(const QString &pluginId)
{
    this->beginResetModel();
    int index = m_pluginIdList.lastIndexOf(pluginId);
    if (index == -1) {
        return;
    }
    m_items.removeAt(index);
    m_pluginIdList.removeAll(pluginId);
    this->endResetModel();
    Q_EMIT this->itemListChanged(m_items.length());
}

void BestListModel::moveInfo(const QString &pluginName, const int pos)
{
    this->beginResetModel();
    int index = m_pluginIdList.lastIndexOf(pluginName);
    if (index == -1) {
        return;
    }

    m_pluginIdList.removeAll(pluginName);
    if (pos > m_pluginIdList.size()) {
        m_pluginIdList.append(pluginName);
    } else {
        m_pluginIdList.insert(pos - 1, pluginName);
    }

    OneResult *info = m_items.at(index);
    m_items.removeAt(index);
    if (pos > m_items.size()) {
        m_items.append(info);
    } else {
        m_items.insert(pos - 1, info);
    }
    this->endResetModel();
}

void BestListModel::startSearch(const QString &keyword)
{
    Q_UNUSED(keyword)
    if (!m_items.isEmpty()) {
        this->beginResetModel();
        m_pluginIdList.clear();
        m_resourceTypes.clear();
        qDeleteAll(m_items);
        m_items.clear();
        this->endResetModel();
        Q_EMIT this->itemListChanged(m_items.length());
    }
}

BestListModel::~BestListModel()
{
    qDeleteAll(m_items);
    m_items.clear();
    m_items.squeeze();
}

bool BestListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(!index.isValid() || index.row() >= m_items.length()) {
        return false;
    }
    if(role == ShowToolTip) {
        m_items.at(index.row())->setShowToolTip(value.toBool());
        return true;
    }
    return false;
}

void BestListModel::stopSearchSlot() {
    beginResetModel();
    qDeleteAll(m_items);
    m_pluginIdList.clear();
    m_resourceTypes.clear();
    m_items.clear();
    m_items.squeeze();
    endResetModel();
    Q_EMIT this->itemListChanged(m_items.length());
//    Q_EMIT stopSearch();
}
