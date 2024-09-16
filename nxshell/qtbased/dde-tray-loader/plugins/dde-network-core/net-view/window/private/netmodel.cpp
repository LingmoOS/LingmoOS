// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "netmodel.h"

#include "netitem.h"

#include <QDebug>
#include <QIcon>

namespace dde {
namespace network {

NetModel::NetModel(QObject *parent)
    : QAbstractItemModel(parent)
    , m_treeRoot(nullptr)
{
}

NetModel::~NetModel() { }

void NetModel::setRoot(NetItem *object)
{
    if (m_treeRoot) {
        disconnectObject(m_treeRoot);
    }
    m_treeRoot = object;
    connectObject(m_treeRoot);
}

NetItem *NetModel::toObject(const QModelIndex &index)
{
    NetItem *obj = static_cast<NetItem *>(index.internalPointer());
    if (!obj) {
        obj = m_treeRoot;
    }
    return obj;
}

QVariant NetModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }
    const NetItem *item = static_cast<const NetItem *>(index.internalPointer());
    switch (role) {
    case Qt::DisplayRole:
        return item->name();
    case SORTROLE:
        return item->sortValue();
    case IDROLE:
        return item->id();
    case TYPEROLE:
        return item->itemType();
    default:
        break;
    }

    return QVariant();
}

QModelIndex NetModel::index(int row, int column, const QModelIndex &parentIndex) const
{
    if (!hasIndex(row, column, parentIndex)) {
        return QModelIndex();
    }

    NetItem *parent = (parentIndex.isValid() ? static_cast<NetItem *>(parentIndex.internalPointer()) : m_treeRoot);
    assert(parent);
    if (!parent) {
        return QModelIndex();
    }

    const NetItem *child = parent->getChild(row);
    return child ? createIndex(row, column, (void *)child) : QModelIndex();
}

QModelIndex NetModel::index(const NetItem *object)
{
    assert(object);

    if (object == m_treeRoot) {
        return QModelIndex();
    }

    NetItem *parent = object->getParent();
    if (!parent) {
        return QModelIndex();
    }

    int pos = parent->getChildIndex(object);
    assert(pos >= 0);

    return createIndex(pos, 0, (void *)object);
}

QModelIndex NetModel::parent(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }

    NetItem *childItem = static_cast<NetItem *>(index.internalPointer());
    if (!childItem) {
        assert(false);
        return QModelIndex();
    }
    NetItem *parentItem = childItem->getParent();
    assert(parentItem);
    if (!parentItem || parentItem == m_treeRoot) {
        return QModelIndex();
    }

    return createIndex(parentItem->getIndex(), 0, parentItem);
}

int NetModel::rowCount(const QModelIndex &parent) const
{
    NetItem *parentItem = nullptr;
    if (!parent.isValid())
        parentItem = m_treeRoot;
    else
        parentItem = static_cast<NetItem *>(parent.internalPointer());

    assert(parentItem);
    return (parentItem ? parentItem->getChildrenNumber() : 0);
}

int NetModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 1;
}

void NetModel::connectObject(const NetItem *obj)
{
    QList<const NetItem *> objs;
    objs.append(obj);
    while (!objs.isEmpty()) {
        const NetItem *o = objs.takeFirst();

        connect(o, &NetItem::dataChanged, this, &NetModel::updateObject);
        connect(o, &NetItem::childAboutToBeAdded, this, &NetModel::AboutToAddObject);
        connect(o, &NetItem::childAdded, this, &NetModel::addObject);
        connect(o, &NetItem::childAboutToBeRemoved, this, &NetModel::AboutToRemoveObject);
        connect(o, &NetItem::childRemoved, this, &NetModel::removeObject);
        int i = o->getChildrenNumber();
        while (i--) {
            objs.append(o->getChild(i));
        }
    }
}

void NetModel::disconnectObject(const NetItem *obj)
{
    QList<const NetItem *> objs;
    objs.append(obj);
    while (!objs.isEmpty()) {
        const NetItem *o = objs.takeFirst();
        disconnect(o, nullptr, this, nullptr);
        int i = o->getChildrenNumber();
        while (i--) {
            objs.append(o->getChild(i));
        }
    }
}

void NetModel::updateObject()
{
    NetItem *obj = qobject_cast<NetItem *>(sender());
    if (obj) {
        QModelIndex i = index(obj);
        emit dataChanged(i, i, { SORTROLE });
    }
}

void NetModel::AboutToAddObject(const NetItem *parent, int pos)
{
    QModelIndex i = index(parent);
    beginInsertRows(i, pos, pos);
}

void NetModel::addObject(const NetItem *child)
{
    endInsertRows();
    connectObject(child);
}

void NetModel::AboutToRemoveObject(const NetItem *parent, int pos)
{
    QModelIndex i = index(parent);
    beginRemoveRows(i, pos, pos);
}

void NetModel::removeObject(const NetItem *child)
{
    endRemoveRows();
    disconnectObject(child);
}

} // namespace network
} // namespace dde
