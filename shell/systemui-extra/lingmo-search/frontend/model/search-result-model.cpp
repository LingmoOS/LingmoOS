 /*
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 * Authors: zhangjiaping <zhangjiaping@kylinos.cn>
 *
 */
#include <QMetaEnum>
#include "search-result-model.h"
using namespace LingmoUISearch;

SearchResultModel::SearchResultModel(const QString &plugin_id)
{
    m_plugin_id = plugin_id;
    m_search_manager = new SearchResultManager(plugin_id);
    m_timer = new QTimer(this);
    m_timer->setInterval(500);
    initConnections();
}

QModelIndex SearchResultModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    if(row < 0 || row > m_items.length() - 1)
        return QModelIndex();
//    QVector<SearchPluginIface::ResultInfo> * m_info = &m_result_info_list;
    return createIndex(row, column, nullptr);

}

QModelIndex SearchResultModel::parent(const QModelIndex &child) const
{
    Q_UNUSED(child)
    return QModelIndex();
}

int SearchResultModel::rowCount(const QModelIndex &index) const
{
    return index.isValid() ? 0 : (m_isExpanded ? m_items.length() : NUM_LIMIT_SHOWN_DEFAULT);
}

int SearchResultModel::columnCount(const QModelIndex &index) const
{
    return index.isValid() ? 0 : 1;
}

QVariant SearchResultModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_items.count()) {
        return {};
    }
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

void SearchResultModel::appendInfo(const SearchPluginIface::ResultInfo &info)//TODO 代码逻辑可尝试梳理优化
{
    if (m_items.length() > 5 //搜索结果大于5个并且搜索结果处于收起状态时只存储数据无需刷新UI
            and !m_isExpanded) {
        m_items.append(new OneResult(info));
        return;
    }
    if (m_items.length() > 50
            and m_isExpanded) {//搜索结果大于50个并且搜索结果处于展开状态时只存储数据并启动定时，500ms刷新一次UI
        m_items.append(new OneResult(info));
        if (!m_timer->isActive()) {
            m_timer->start();
        }
        return;
    }
//    this->beginResetModel();
    beginInsertRows(QModelIndex(), m_items.length(), m_items.length());
    m_items.append(new OneResult(info));
//    this->endResetModel();
    endInsertRows();
    Q_EMIT this->itemListChanged(m_items.length());
    if (m_plugin_id != "Web Page") {
        Q_EMIT this->sendBestListData(m_plugin_id, m_items.at(0)->info());
    }
}

void SearchResultModel::startSearch(const QString &keyword)
{
    if (!m_items.isEmpty()) {
        this->beginResetModel();
        m_items.clear();
        this->endResetModel();
        Q_EMIT this->itemListChanged(m_items.length());
    }
    m_search_manager->startSearch(keyword);
}

void SearchResultModel::initConnections()
{
    connect(this, &SearchResultModel::stopSearch, m_search_manager, &SearchResultManager::stopSearch);
    connect(m_search_manager, &SearchResultManager::gotResultInfo, this, &SearchResultModel::appendInfo);
    connect(m_timer, &QTimer::timeout, [ = ] () {//500ms刷新一次UI,防止搜索结果数据量过大导致的UI卡顿
        Q_EMIT this->itemListChanged(m_items.length());
        m_timer->stop();
    });
}

const SearchPluginIface::ResultInfo &SearchResultModel::getInfo(const QModelIndex &index)
{
    return m_items.at(index.row())->info();
}

void SearchResultModel::setExpanded(const bool &is_expanded)
{
    this->beginResetModel();
    m_isExpanded = is_expanded;
    this->endResetModel();
    Q_EMIT this->itemListChanged(m_items.length());
}

const bool &SearchResultModel::isExpanded()
{
    return m_isExpanded;
}

void SearchResultModel::refresh()
{
    this->beginResetModel();
    this->endResetModel();
}

void SearchResultModel::stopSearchSlot()
{
    beginResetModel();
    qDeleteAll(m_items);
    m_items.clear();
    m_items.squeeze();
    endResetModel();
    Q_EMIT stopSearch();
}

SearchResultModel::~SearchResultModel()
{
    qDeleteAll(m_items);
    m_items.clear();
    m_items.squeeze();
}

 QHash<int, QByteArray> SearchResultModel::roleNames() const
 {
     QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
     QMetaEnum e = metaObject()->enumerator(metaObject()->indexOfEnumerator("AdditionalRoles"));

     for (int i = 0; i < e.keyCount(); ++i) {
         roles.insert(e.value(i), e.key(i));
     }
     return roles;
 }

 bool SearchResultModel::setData(const QModelIndex &index, const QVariant &value, int role)
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
