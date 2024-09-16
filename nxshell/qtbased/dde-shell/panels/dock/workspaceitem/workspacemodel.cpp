// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "workspacemodel.h"
#include "xworkspaceworker.h"

#include "dsglobal.h"
#include <QGuiApplication>

namespace dock {

WorkspaceModel *WorkspaceModel::instance()
{
    static WorkspaceModel* workspaceModel = nullptr;
    if (workspaceModel == nullptr) {
        workspaceModel = new WorkspaceModel();
    }
    return workspaceModel;
}

int WorkspaceModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_items.size();
}

QVariant WorkspaceModel::data(const QModelIndex &index, int role) const
{
    if (!hasIndex(index.row(), index.column(), index.parent())) {
        return QVariant();
    }

    auto item = m_items[index.row()];
    switch (role) {
    case WorkspaceModel::NameRole:
        return item->name();
    case WorkspaceModel::ImageRole:
        return item->image();
    }
    return QVariant();
}

QHash<int, QByteArray> WorkspaceModel::roleNames() const
{
    return {
            {WorkspaceModel::NameRole, "workspaceName"},
            {WorkspaceModel::ImageRole, "screenImage"},
        };
}

void WorkspaceModel::preview(bool show)
{
    // todo preview
}

WorkspaceModel::WorkspaceModel(QObject *parent)
    : QAbstractListModel{parent}
    , m_worker(nullptr)
    , m_currentIndex(-1)
{
    auto platformName = QGuiApplication::platformName();
    if (QStringLiteral("wayland") == platformName) {

    }
    else if (QStringLiteral("xcb") == platformName) {
        m_worker = new XWorkspaceWorker(this);
    }
}

QList<WorkSpaceData *> WorkspaceModel::items() const
{
    return m_items;
}

void WorkspaceModel::setItems(const QList<WorkSpaceData *> &newItems)
{
    if (m_items == newItems)
        return;
    beginResetModel();
    foreach (auto item, m_items) {
        delete item;
    }
    m_items = newItems;
    endResetModel();
    emit itemsChanged();
}

int WorkspaceModel::currentIndex() const
{
    return m_currentIndex;
}

void WorkspaceModel::setCurrentIndex(int newCurrentIndex)
{
    if (m_currentIndex == newCurrentIndex) {
        return;
    }
    m_currentIndex = newCurrentIndex;
    emit currentIndexChanged(m_currentIndex);
}

}
