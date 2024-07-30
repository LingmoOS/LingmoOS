/*
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 * SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "PageSortModel.h"

PageSortModel::PageSortModel(QObject *parent)
    : QAbstractProxyModel(parent)
{
}

QHash<int, QByteArray> PageSortModel::roleNames() const
{
    if (!sourceModel()) {
        return {};
    }
    auto sourceNames = sourceModel()->roleNames();
    sourceNames.insert(ShouldRemoveFilesRole, "shouldRemoveFiles");
    return sourceNames;
}
QVariant PageSortModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid | CheckIndexOption::ParentIsInvalid)) {
        return QVariant();
    }

    switch (role) {
    case PagesModel::HiddenRole:
        return m_hiddenProxy[mapToSource(index).row()];
    case ShouldRemoveFilesRole:
        return m_removeFiles[mapToSource(index).row()];
    }

    return QAbstractProxyModel::data(index, role);
}

bool PageSortModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid | CheckIndexOption::ParentIsInvalid)) {
        return false;
    }

    switch (role) {
    case PagesModel::HiddenRole:
        m_hiddenProxy[mapToSource(index).row()] = value.toBool();
        break;
    case ShouldRemoveFilesRole:
        m_removeFiles[mapToSource(index).row()] = value.toBool();
        break;
    default:
        return false;
    }

    Q_EMIT dataChanged(index, index, {role});
    return true;
}

void PageSortModel::setSourceModel(QAbstractItemModel *newSourceModel)
{
    beginResetModel();
    m_rowMapping.clear();
    m_hiddenProxy.clear();
    m_removeFiles.clear();
    if (newSourceModel) {
        for (int i = 0; i < newSourceModel->rowCount(); ++i) {
            m_rowMapping.append(i);
            m_hiddenProxy.append(newSourceModel->index(i, 0).data(PagesModel::HiddenRole).toBool());
            m_removeFiles.append(false);
        }
    }
    QAbstractProxyModel::setSourceModel(newSourceModel);
    endResetModel();
}

int PageSortModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_rowMapping.size();
}

int PageSortModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return 1;
}

QModelIndex PageSortModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return QModelIndex();
    } else if (m_rowMapping.size() <= row || column != 0) {
        return QModelIndex();
    }
    return createIndex(row, column);
}

QModelIndex PageSortModel::parent(const QModelIndex &child) const
{
    Q_UNUSED(child)
    return QModelIndex();
}

QModelIndex PageSortModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    if (!checkIndex(sourceIndex, CheckIndexOption::IndexIsValid | CheckIndexOption::ParentIsInvalid)) {
        return QModelIndex();
    }
    const int row = m_rowMapping.indexOf(sourceIndex.row());
    return row != -1 ? createIndex(row, sourceIndex.column()) : QModelIndex();
}

QModelIndex PageSortModel::mapToSource(const QModelIndex &proxyIndex) const
{
    if (!checkIndex(proxyIndex, CheckIndexOption::IndexIsValid | CheckIndexOption::ParentIsInvalid)) {
        return QModelIndex();
    }
    return sourceModel()->index(m_rowMapping.at(proxyIndex.row()), proxyIndex.column());
}

void PageSortModel::move(int fromRow, int toRow)
{
    beginMoveRows(QModelIndex(), fromRow, fromRow, QModelIndex(), fromRow < toRow ? toRow + 1 : toRow);
    m_rowMapping.move(fromRow, toRow);
    endMoveRows();
}

void PageSortModel::applyChangesToSourceModel() const
{
    auto *pagesModel = static_cast<PagesModel *>(sourceModel());
    QStringList newOrder, hiddenPages, toRemove;
    for (int i = 0; i < m_rowMapping.size(); ++i) {
        const QModelIndex sourceIndex = pagesModel->index(m_rowMapping[i]);
        const QString name = sourceIndex.data(PagesModel::FileNameRole).toString();
        newOrder.append(name);
        if (m_hiddenProxy[m_rowMapping[i]]) {
            hiddenPages.append(name);
        }
        if (m_removeFiles[m_rowMapping[i]]) {
            toRemove.append(name);
        }
    }
    pagesModel->setPageOrder(newOrder);
    pagesModel->setHiddenPages(hiddenPages);
    for (const auto &name : toRemove) {
        pagesModel->removeLocalPageFiles(name);
    }
}

#include "moc_PageSortModel.cpp"
