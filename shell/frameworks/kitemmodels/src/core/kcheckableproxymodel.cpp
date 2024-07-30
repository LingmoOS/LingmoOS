/*
    SPDX-FileCopyrightText: 2010 Stephen Kelly <steveire@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kcheckableproxymodel.h"

#include <QItemSelectionModel>

class KCheckableProxyModelPrivate
{
    Q_DECLARE_PUBLIC(KCheckableProxyModel)
    KCheckableProxyModel *q_ptr;

    KCheckableProxyModelPrivate(KCheckableProxyModel *checkableModel)
        : q_ptr(checkableModel)
    {
    }

    QItemSelectionModel *m_itemSelectionModel = nullptr;

    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
};

KCheckableProxyModel::KCheckableProxyModel(QObject *parent)
    : QIdentityProxyModel(parent)
    , d_ptr(new KCheckableProxyModelPrivate(this))
{
}

KCheckableProxyModel::~KCheckableProxyModel() = default;

void KCheckableProxyModel::setSelectionModel(QItemSelectionModel *itemSelectionModel)
{
    Q_D(KCheckableProxyModel);
    d->m_itemSelectionModel = itemSelectionModel;
    Q_ASSERT(sourceModel() ? d->m_itemSelectionModel->model() == sourceModel() : true);
    connect(itemSelectionModel, &QItemSelectionModel::selectionChanged, this, [d](const QItemSelection &selected, const QItemSelection &deselected) {
        d->selectionChanged(selected, deselected);
    });
}

QItemSelectionModel *KCheckableProxyModel::selectionModel() const
{
    Q_D(const KCheckableProxyModel);
    return d->m_itemSelectionModel;
}

Qt::ItemFlags KCheckableProxyModel::flags(const QModelIndex &index) const
{
    if (!index.isValid() || index.column() != 0) {
        return QIdentityProxyModel::flags(index);
    }
    return QIdentityProxyModel::flags(index) | Qt::ItemIsUserCheckable;
}

QVariant KCheckableProxyModel::data(const QModelIndex &index, int role) const
{
    Q_D(const KCheckableProxyModel);

    if (role == Qt::CheckStateRole) {
        if (index.column() != 0) {
            return QVariant();
        }
        if (!d->m_itemSelectionModel) {
            return Qt::Unchecked;
        }

        return d->m_itemSelectionModel->selection().contains(mapToSource(index)) ? Qt::Checked : Qt::Unchecked;
    }
    return QIdentityProxyModel::data(index, role);
}

bool KCheckableProxyModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_D(KCheckableProxyModel);
    if (role == Qt::CheckStateRole) {
        if (index.column() != 0) {
            return false;
        }
        if (!d->m_itemSelectionModel) {
            return false;
        }

        Qt::CheckState state = static_cast<Qt::CheckState>(value.toInt());
        const QModelIndex srcIndex = mapToSource(index);
        bool result = select(QItemSelection(srcIndex, srcIndex), state == Qt::Checked ? QItemSelectionModel::Select : QItemSelectionModel::Deselect);
        Q_EMIT dataChanged(index, index);
        return result;
    }
    return QIdentityProxyModel::setData(index, value, role);
}

void KCheckableProxyModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    QIdentityProxyModel::setSourceModel(sourceModel);
    Q_ASSERT(d_ptr->m_itemSelectionModel ? d_ptr->m_itemSelectionModel->model() == sourceModel : true);
}

void KCheckableProxyModelPrivate::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_Q(KCheckableProxyModel);
    const auto lstSelected = q->mapSelectionFromSource(selected);
    for (const QItemSelectionRange &range : lstSelected) {
        Q_EMIT q->dataChanged(range.topLeft(), range.bottomRight());
    }
    const auto lstDeselected = q->mapSelectionFromSource(deselected);
    for (const QItemSelectionRange &range : lstDeselected) {
        Q_EMIT q->dataChanged(range.topLeft(), range.bottomRight());
    }
}

bool KCheckableProxyModel::select(const QItemSelection &selection, QItemSelectionModel::SelectionFlags command)
{
    Q_D(KCheckableProxyModel);
    d->m_itemSelectionModel->select(selection, command);
    return true;
}

QHash<int, QByteArray> KCheckableProxyModel::roleNames() const
{
    auto roles = QIdentityProxyModel::roleNames();
    roles[Qt::CheckStateRole] = QByteArrayLiteral("checkState");
    return roles;
}

#include "moc_kcheckableproxymodel.cpp"
