/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "ProcessSortFilterModel.h"

#include <QDebug>

#include <processcore/process_data_model.h>

using namespace KSysGuard;

ProcessSortFilterModel::ProcessSortFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setSortRole(ProcessDataModel::Value);
    setSortCaseSensitivity(Qt::CaseInsensitive);
    setSortLocaleAware(true);

    setFilterRole(ProcessDataModel::Value);
    setFilterCaseSensitivity(Qt::CaseInsensitive);
    setRecursiveFilteringEnabled(true);
    setAutoAcceptChildRows(true);
}

void ProcessSortFilterModel::setSourceModel(QAbstractItemModel *newSourceModel)
{
    auto oldSourceModel = sourceModel();

    if (newSourceModel == oldSourceModel) {
        return;
    }

    if (oldSourceModel) {
        oldSourceModel->disconnect(this);
    }

    QSortFilterProxyModel::setSourceModel(newSourceModel);
    if (newSourceModel) {
        connect(newSourceModel, &QAbstractItemModel::modelReset, this, &ProcessSortFilterModel::findColumns);
        connect(newSourceModel, &QAbstractItemModel::columnsInserted, this, &ProcessSortFilterModel::findColumns);
        connect(newSourceModel, &QAbstractItemModel::columnsRemoved, this, &ProcessSortFilterModel::findColumns);
        connect(newSourceModel, &QAbstractItemModel::columnsMoved, this, &ProcessSortFilterModel::findColumns);
        findColumns();
    }
}

bool ProcessSortFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    // not handled a reset yet, we'll invalidate at the end of modelReset anyway
    if (m_uidColumn == -1 && m_pidColumn == -1 && filterKeyColumn() == -1) {
        return false;
    }

    auto source = sourceModel();

    bool result = true;

    if (m_viewMode != ViewAll && m_uidColumn != -1) {
        auto uid = source->data(source->index(sourceRow, m_uidColumn, sourceParent), ProcessDataModel::Value).toUInt();

        switch (m_viewMode) {
        case ViewOwn:
            result = m_currentUser.userId().nativeId() == uid;
            break;
        case ViewUser:
            result = uid >= 1000 && uid < 65534;
            break;
        case ViewSystem:
            result = uid < 1000 || uid >= 65534;
            break;
        default:
            break;
        }
    }

    if (!m_filterPids.isEmpty()) {
        auto pid = source->data(source->index(sourceRow, m_pidColumn, sourceParent), ProcessDataModel::Value);
        result = m_filterPids.contains(pid);
    }

    if (!result) {
        return false;
    }

    if (filterString().isEmpty()) {
        return true;
    }

    const QString name = source->data(source->index(sourceRow, 0, sourceParent), filterRole()).toString();

    const QString filter = filterString();
    const QList<QStringView> splitFilterStrings = QStringView(filter).split(QLatin1Char(','), Qt::SkipEmptyParts);

    for (const QStringView &string : splitFilterStrings) {
        if (name.contains(string.trimmed(), Qt::CaseInsensitive)) {
            return true;
        }
    }

    return false;
}

bool ProcessSortFilterModel::filterAcceptsColumn(int sourceColumn, const QModelIndex &sourceParent) const
{
    Q_UNUSED(sourceParent)

    auto attribute = sourceModel()->headerData(sourceColumn, Qt::Horizontal, ProcessDataModel::Attribute).toString();
    if (m_hiddenAttributes.contains(attribute)) {
        return false;
    }

    return true;
}

QString ProcessSortFilterModel::filterString() const
{
    return m_filterString;
}

void ProcessSortFilterModel::setFilterString(const QString &newFilterString)
{
    if (newFilterString == m_filterString) {
        return;
    }

    m_filterString = newFilterString;
    setFilterWildcard(m_filterString);
    Q_EMIT filterStringChanged();
}

ProcessSortFilterModel::ViewMode ProcessSortFilterModel::viewMode() const
{
    return m_viewMode;
}

void ProcessSortFilterModel::setViewMode(ViewMode newViewMode)
{
    if (newViewMode == m_viewMode) {
        return;
    }

    m_viewMode = newViewMode;
    invalidateFilter();
    Q_EMIT viewModeChanged();
}

QStringList ProcessSortFilterModel::hiddenAttributes() const
{
    return m_hiddenAttributes;
}

void ProcessSortFilterModel::setHiddenAttributes(const QStringList &newHiddenAttributes)
{
    if (newHiddenAttributes == m_hiddenAttributes) {
        return;
    }

    m_hiddenAttributes = newHiddenAttributes;
    invalidateFilter();
    Q_EMIT hiddenAttributesChanged();
}

QVariantList ProcessSortFilterModel::filterPids() const
{
    return m_filterPids;
}

void ProcessSortFilterModel::setFilterPids(const QVariantList &newFilterPids)
{
    if (newFilterPids == m_filterPids) {
        return;
    }

    m_filterPids = newFilterPids;
    invalidateFilter();
    Q_EMIT filterPidsChanged();
}

void ProcessSortFilterModel::sort(int column, Qt::SortOrder order)
{
    QSortFilterProxyModel::sort(column, order);
    Q_EMIT sorted();
}

void ProcessSortFilterModel::findColumns()
{
    m_uidColumn = -1;
    m_pidColumn = -1;
    int nameColumn = -1;

    auto source = sourceModel();

    for (auto column = 0; column < source->columnCount(); ++column) {
        auto attribute = source->headerData(column, Qt::Horizontal, ProcessDataModel::Attribute).toString();
        if (attribute == QStringLiteral("uid")) {
            m_uidColumn = column;
        } else if (attribute == QStringLiteral("pid")) {
            m_pidColumn = column;
        } else if (attribute == QStringLiteral("name")) {
            nameColumn = column;
        }
    }
    setFilterKeyColumn(nameColumn);
}

#include "moc_ProcessSortFilterModel.cpp"
