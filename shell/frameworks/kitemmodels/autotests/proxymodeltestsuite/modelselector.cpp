/*
    SPDX-FileCopyrightText: 2009 Stephen Kelly <steveire@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "modelselector.h"

ModelSelector::ModelSelector(ProxyModelTest *parent)
    : ProxyModelTestData(parent)
    , m_model(nullptr)
    , m_selectionModel(nullptr)
    , m_rootModel(nullptr)
{
    Q_ASSERT(parent);
}

void ModelSelector::setWatchedModel(QAbstractItemModel *model)
{
    m_model = model;
    connect(m_model, &QObject::destroyed, this, &ModelSelector::modelDestroyed);
}

void ModelSelector::setSelectionModel(QItemSelectionModel *selectionModel)
{
    if (selectionModel) {
        Q_ASSERT(!selectionModel->hasSelection());
    }
    m_selectionModel = selectionModel;
    connect(m_selectionModel, &QObject::destroyed, this, &ModelSelector::modelDestroyed);
}

void ModelSelector::setRootModel(DynamicTreeModel *rootModel)
{
    m_rootModel = rootModel;
}

void ModelSelector::setWatch(bool watch)
{
    if (!m_model) {
        return;
    }

    disconnect(m_model, &QAbstractItemModel::rowsInserted, this, &ModelSelector::rowsInserted);
    if (watch) {
        Q_ASSERT(m_model);
        connect(m_model, &QAbstractItemModel::rowsInserted, this, &ModelSelector::rowsInserted);
        if (m_model->hasChildren()) {
            rowsInserted(QModelIndex(), 0, m_model->rowCount() - 1);
        }
    }
}

void ModelSelector::rowsInserted(const QModelIndex &parent, int start, int end)
{
    Q_ASSERT(end >= start);
    Q_ASSERT(m_selectionModel);

    int row = start;
    static const int column = 0;
    QModelIndex idx = m_model->index(row, column, parent);

    while (idx.isValid() && row <= end) {
        int item = idx.data().toInt();
        if (m_selectedRows.contains(item)) {
            m_selectionModel->select(idx, QItemSelectionModel::SelectCurrent);
        }
        if (m_model->hasChildren(idx)) {
            rowsInserted(idx, 0, m_model->rowCount(idx) - 1);
        }
        idx = idx.sibling(++row, column);
    }
}

#include "moc_modelselector.cpp"
