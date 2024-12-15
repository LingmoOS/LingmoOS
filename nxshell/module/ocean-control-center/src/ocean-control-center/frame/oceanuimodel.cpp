// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "oceanuimodel.h"

#include "oceanuiobject.h"
#include "oceanuiobject_p.h"

#include <QIcon>
#include <QLoggingCategory>

namespace oceanuiV25 {
static Q_LOGGING_CATEGORY(oceanuiLog, "ocean.oceanui.model");

enum OceanUIModelRole {
    OceanUIItemRole = Qt::UserRole + 300,
    OceanUIPageTypeRole,
    OceanUIViewItemPositionRole,
};

enum OceanUIItemPostition {
    Invalid,
    Beginning,
    Middle,
    End,
    OnlyOne,
};

OceanUIModel::OceanUIModel(QObject *parent)
    : QAbstractItemModel(parent)
    , m_root(nullptr)
{
}

OceanUIModel::~OceanUIModel() { }

OceanUIObject *OceanUIModel::root() const
{
    return m_root;
}

void OceanUIModel::setRoot(OceanUIObject *root)
{
    beginResetModel();
    m_root = root;
    endResetModel();
    Q_EMIT rootChanged(m_root);
    if (!m_root)
        return;
    connect(m_root, &OceanUIObject::childAboutToBeAoceand, this, &OceanUIModel::AboutToAddObject);
    connect(m_root, &OceanUIObject::childAoceand, this, &OceanUIModel::addObject);
    connect(m_root, &OceanUIObject::childAboutToBeRemoved, this, &OceanUIModel::AboutToRemoveObject);
    connect(m_root, &OceanUIObject::childRemoved, this, &OceanUIModel::removeObject);
    connect(m_root, &OceanUIObject::childAboutToBeMoved, this, &OceanUIModel::AboutToMoveObject);
    connect(m_root, &OceanUIObject::childMoved, this, &OceanUIModel::moveObject);
    for (auto &&obj : OceanUIObject::Private::FromObject(m_root)->getChildren()) {
        connectObject(obj);
    }
}

OceanUIObject *OceanUIModel::getObject(int row)
{
    if (row < 0 || row >= m_root->getChildren().size()) {
        return nullptr;
    }
    return m_root->getChildren().at(row);
}

QHash<int, QByteArray> OceanUIModel::roleNames() const
{
    QHash<int, QByteArray> names;
    names[Qt::DisplayRole] = "display";
    names[Qt::StatusTipRole] = "description";
    names[Qt::DecorationRole] = "decoration";
    names[OceanUIItemRole] = "item";
    names[OceanUIPageTypeRole] = "pageType";
    names[OceanUIViewItemPositionRole] = "position";
    return names;
}

QModelIndex OceanUIModel::index(const OceanUIObject *object)
{
    if (object == m_root) {
        return QModelIndex();
    }

    OceanUIObject *parent = OceanUIObject::Private::FromObject(object)->getParent();
    if (!parent) {
        // DGM: actually, it can happen (for instance if the entity is displayed in the local DB of a 3D view)
        // ccLog::Error(QString("An error occurred while creating DB tree index: object '%1' has no parent").arg(object->getName()));
        return QModelIndex();
    }

    int pos = OceanUIObject::Private::FromObject(parent)->getChildIndex(object);
    assert(pos >= 0);

    return createIndex(pos, 0, (void *)object);
}

QModelIndex OceanUIModel::index(int row, int column, const QModelIndex &parentIndex) const
{
    // qWarning() << __FUNCTION__ << __LINE__ << row << column << parentIndex;
    if (row < 0 || row >= m_root->getChildren().size()) {
        return QModelIndex();
    }
    return createIndex(row, column, (void *)m_root->getChildren().at(row));

    if (!hasIndex(row, column, parentIndex)) {
        return QModelIndex();
    }
    OceanUIObject *parent = (parentIndex.isValid() ? static_cast<OceanUIObject *>(parentIndex.internalPointer()) : m_root);
    const OceanUIObject *child = OceanUIObject::Private::FromObject(parent)->getChild(row);
    return child ? createIndex(row, column, (void *)child) : QModelIndex();
}

QModelIndex OceanUIModel::parent(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }
    OceanUIObject *childItem = static_cast<OceanUIObject *>(index.internalPointer());
    if (!childItem) {
        assert(false);
        return QModelIndex();
    }
    OceanUIObject *parentItem = OceanUIObject::Private::FromObject(childItem)->getParent();

    assert(parentItem);
    if (!parentItem || parentItem == m_root) {
        return QModelIndex();
    }

    return createIndex(OceanUIObject::Private::FromObject(parentItem)->getIndex(), 0, parentItem);
}

int OceanUIModel::rowCount(const QModelIndex &) const
{
    if (!m_root)
        return 0;

    return OceanUIObject::Private::FromObject(m_root)->getChildren().size();
}

int OceanUIModel::columnCount(const QModelIndex &parent) const
{
    // qCWarning(oceanuiLog) << __FUNCTION__ << __LINE__;
    if (!parent.isValid())
        return 0;

    return 1;
}

QVariant OceanUIModel::data(const QModelIndex &index, int role) const
{
    // qCWarning(oceanuiLog) << __FUNCTION__ << index << role;
    if (!index.isValid())
        return QVariant();
    OceanUIObject *item = static_cast<OceanUIObject *>(index.internalPointer());
    switch (role) {
    case OceanUIItemRole:
        return QVariant::fromValue(item);
    case OceanUIPageTypeRole:
        return QVariant::fromValue(item->pageType());
    case OceanUIViewItemPositionRole: {
        int count = rowCount();
        if (index.row() == 0) {
            return count == 1 ? OnlyOne : Beginning;
        } else if (index.row() == count - 1) {
            return End;
        } else {
            return Middle;
        }
    }
        return Invalid;
    case Qt::DisplayRole:
        return item->displayName();
    case Qt::StatusTipRole:
        return item->description();
    case Qt::DecorationRole:
        return QIcon::fromTheme(item->icon());
    }
    return QVariant();
}

void OceanUIModel::updateObject()
{
    OceanUIObject *obj = qobject_cast<OceanUIObject *>(sender());
    if (obj) {
        QModelIndex i = index(obj);
        emit dataChanged(i, i);
    }
}

void OceanUIModel::AboutToAddObject(const OceanUIObject *, int pos)
{
    beginInsertRows(QModelIndex(), pos, pos);
}

void OceanUIModel::addObject(const OceanUIObject *child)
{
    endInsertRows();
    connectObject(child);
}

void OceanUIModel::AboutToRemoveObject(const OceanUIObject *, int pos)
{
    beginRemoveRows(QModelIndex(), pos, pos);
}

void OceanUIModel::removeObject(const OceanUIObject *child)
{
    endRemoveRows();
    disconnectObject(child);
}

void OceanUIModel::AboutToMoveObject(const OceanUIObject *, int pos, int oldPos)
{
    beginMoveRows(QModelIndex(), oldPos, oldPos, QModelIndex(), pos);
}

void OceanUIModel::moveObject(const OceanUIObject *)
{
    endMoveRows();
}

void OceanUIModel::connectObject(const OceanUIObject *obj)
{
    connect(obj, &OceanUIObject::objectNameChanged, this, &OceanUIModel::updateObject);
    connect(obj, &OceanUIObject::displayNameChanged, this, &OceanUIModel::updateObject);
    connect(obj, &OceanUIObject::descriptionChanged, this, &OceanUIModel::updateObject);
    connect(obj, &OceanUIObject::iconChanged, this, &OceanUIModel::updateObject);
    connect(obj, &OceanUIObject::badgeChanged, this, &OceanUIModel::updateObject);
}

void OceanUIModel::disconnectObject(const OceanUIObject *obj)
{
    disconnect(obj, nullptr, this, nullptr);
}
} // namespace oceanuiV25
