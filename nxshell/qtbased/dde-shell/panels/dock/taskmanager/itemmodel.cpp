// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dsglobal.h"
#include "itemmodel.h"
#include "abstractitem.h"
#include "taskmanagersettings.h"

#include <algorithm>

#include <QVariant>
#include <QPointer>
#include <QAbstractListModel>
#include <QJsonArray>
#include <QJsonObject>

namespace dock {
ItemModel* ItemModel::instance()
{
    static ItemModel* appItemModel = nullptr;
    if (appItemModel == nullptr) {
        appItemModel = new ItemModel();
    }
    return appItemModel;
}

ItemModel::ItemModel(QObject* parent)
    : QAbstractListModel(parent)
    , m_recentSize(3)
{
}

QHash<int, QByteArray> ItemModel::roleNames() const
{
    return {{ItemModel::ItemIdRole, "itemId"},
        {ItemModel::NameRole, "name"},
        {ItemModel::IconNameRole, "iconName"},
        {ItemModel::ActiveRole, "active"},
        {ItemModel::AttentionRole, "attention"},
        {ItemModel::MenusRole, "menus"},
        {ItemModel::DockedRole, "docked"},
        {ItemModel::WindowsRole, "windows"},
        {ItemModel::DesktopFilesIconsRole, "desktopfileIcons"},
        {ItemModel::DockedDirRole, "dockedDir"}
    };
}

int ItemModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_items.size();
}

QVariant ItemModel::data(const QModelIndex &index, int role) const
{
    if (!hasIndex(index.row(), index.column(), index.parent())) {
        return QVariant();
    }

    auto item = m_items[index.row()];
    switch (role) {
        case ItemModel::ItemIdRole: return item->id();
        case ItemModel::NameRole: return item->name();
        case ItemModel::IconNameRole: return item->icon();
        case ItemModel::ActiveRole: return item->isActive();
        case ItemModel::AttentionRole: return item->isAttention();
        case ItemModel::MenusRole: return item->menus();
        case ItemModel::DockedRole: return item->isDocked();
        case ItemModel::WindowsRole: return item->data().toStringList();
        case ItemModel::DesktopFilesIconsRole: return item->data().toStringList();
        case ItemModel::DockedDirRole: return item->data().toString();
    }
    return QVariant();
}

void ItemModel::moveTo(const QString &id, int dIndex)
{
    auto sItem = getItemById(id);
    auto dItem = m_items.at(dIndex);

    int sIndex = m_items.indexOf(sItem);
    if (sIndex == dIndex) {
        return;
    }
    beginMoveRows(QModelIndex(), sIndex, sIndex, QModelIndex(), dIndex > sIndex ? (dIndex + 1) : dIndex);
    m_items.move(sIndex, dIndex);
    endMoveRows();

    if (sItem->isDocked() || dItem->isDocked()) {
        TaskManagerSettings::instance()->setDockedDesktopFiles(dumpDockedItems());
    }
}

QJsonArray ItemModel::dumpDockedItems() const
{
    QJsonArray result;

    foreach(auto item, m_items) {
        if (!item->isDocked())
            continue;

        QJsonObject tmp;
        tmp["id"] = item->id();
        tmp["type"] = item->type();
        result.append(tmp);
    }

    return result;
}

QPointer<AbstractItem> ItemModel::getItemById(const QString& id) const
{
    auto it = std::find_if(m_items.begin(), m_items.end(),[id](QPointer<AbstractItem> item){
        return item->id() == id;
    });

    return it == m_items.end() ? nullptr : *it;
}

void ItemModel::addItem(QPointer<AbstractItem> item)
{
    if (m_items.contains(item)) return;

    connect(item.get(), &AbstractItem::destroyed, this, &ItemModel::onItemDestroyed, Qt::UniqueConnection);
    connect(item.get(), &AbstractItem::nameChanged, this, &ItemModel::onItemChanged, Qt::UniqueConnection);
    connect(item.get(), &AbstractItem::iconChanged, this, &ItemModel::onItemChanged, Qt::UniqueConnection);
    connect(item.get(), &AbstractItem::activeChanged, this, &ItemModel::onItemChanged, Qt::UniqueConnection);
    connect(item.get(), &AbstractItem::attentionChanged, this, &ItemModel::onItemChanged, Qt::UniqueConnection);
    connect(item.get(), &AbstractItem::menusChanged, this, &ItemModel::onItemChanged, Qt::UniqueConnection);
    connect(item.get(), &AbstractItem::dockedChanged, this, &ItemModel::onItemChanged, Qt::UniqueConnection);
    connect(item.get(), &AbstractItem::dataChanged, this, &ItemModel::onItemChanged, Qt::UniqueConnection);

    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_items.append(item);
    endInsertRows();
}

void ItemModel::onItemDestroyed()
{
    auto item = qobject_cast<AbstractItem*>(sender());
    auto beginIndex = m_items.indexOf(item);
    auto lastIndex = m_items.lastIndexOf(item);

    if (beginIndex == -1 || lastIndex == -1) return;

    beginRemoveRows(QModelIndex(), beginIndex, lastIndex);
    m_items.removeAll(item);
    endRemoveRows();
}

void ItemModel::onItemChanged()
{
    auto item = qobject_cast<AbstractItem*>(sender());
    if (!item) return;
    QModelIndexList indexes = match(index(0, 0, QModelIndex()),
                                    ItemModel::ItemIdRole, item->id(), 1, Qt::MatchExactly);
    Q_EMIT dataChanged(indexes.first(), indexes.last());
}
}
