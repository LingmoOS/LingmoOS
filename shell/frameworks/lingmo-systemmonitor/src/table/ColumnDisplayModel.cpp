/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "ColumnDisplayModel.h"

#include <QDebug>

ColumnDisplayModel::ColumnDisplayModel(QObject *parent)
    : QIdentityProxyModel(parent)
{
}

void ColumnDisplayModel::setSourceModel(QAbstractItemModel *newSourceModel)
{
    m_idRoleNumber = -1;
    QIdentityProxyModel::setSourceModel(newSourceModel);
}

QHash<int, QByteArray> ColumnDisplayModel::roleNames() const
{
    auto roles = QIdentityProxyModel::roleNames();
    roles.insert(DisplayStyleRole, "displayStyle");
    return roles;
}

QVariant ColumnDisplayModel::data(const QModelIndex &index, int role) const
{
    if (role == DisplayStyleRole && sourceModel()) {
        auto id = sourceModel()->data(mapToSource(index), idRoleNumber()).toString();
        return m_columnDisplay.value(id, QStringLiteral("hidden"));
    }

    return QIdentityProxyModel::data(index, role);
}

void ColumnDisplayModel::setDisplay(int row, const QString &display)
{
    auto id = sourceModel()->data(sourceModel()->index(row, 0), idRoleNumber()).toString();
    if (id.isEmpty()) {
        return;
    }

    auto changed = false;
    if (!m_columnDisplay.contains(id)) {
        m_columnDisplay.insert(id, display);
        changed = true;
    } else if (m_columnDisplay.value(id) != display) {
        m_columnDisplay[id] = display;
        changed = true;
    }

    if (changed) {
        auto i = index(row, 0);
        Q_EMIT dataChanged(i, i, {DisplayStyleRole});
        Q_EMIT columnDisplayChanged();
    }
}

void ColumnDisplayModel::setDisplayById(const QString &id, const QString &display)
{
    auto changed = false;
    if (!m_columnDisplay.contains(id)) {
        m_columnDisplay.insert(id, display);
        changed = true;
    } else if (m_columnDisplay.value(id) != display) {
        m_columnDisplay[id] = display;
        changed = true;
    }

    if (changed) {
        auto i = indexForId(id);
        if (!i.isValid()) {
            return;
        }

        Q_EMIT dataChanged(i, i, {DisplayStyleRole});
        Q_EMIT columnDisplayChanged();
    }
}

QModelIndex ColumnDisplayModel::indexForId(const QString &id)
{
    for (int i = 0; i < sourceModel()->rowCount(); i++) {
        const auto index = sourceModel()->index(i, 0);
        if (index.data(idRoleNumber()).toString() == id) {
            return mapFromSource(index);
        }
    }

    return {};
}

QVariantMap ColumnDisplayModel::columnDisplay() const
{
    QVariantMap result;
    for (auto itr = m_columnDisplay.cbegin(); itr != m_columnDisplay.cend(); ++itr) {
        result.insert(itr.key(), itr.value());
    }
    return result;
}

void ColumnDisplayModel::setColumnDisplay(const QVariantMap &newColumnDisplay)
{
    QHash<QString, QString> display;
    for (auto itr = newColumnDisplay.begin(); itr != newColumnDisplay.end(); ++itr) {
        display.insert(itr.key(), itr.value().toString());
    }

    m_columnDisplay = display;
    Q_EMIT columnDisplayChanged();
    if (sourceModel()) {
        Q_EMIT dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
    }
}

QStringList ColumnDisplayModel::visibleColumnIds() const
{
    QHash<QString, int> sourceRowMapping;
    for (int i = 0; i < rowCount(); ++i) {
        auto id = sourceModel()->data(sourceModel()->index(i, 0), idRoleNumber()).toString();
        sourceRowMapping.insert(id, i);
    }

    QStringList result;
    std::copy_if(m_columnDisplay.keyBegin(), m_columnDisplay.keyEnd(), std::back_inserter(result), [this](const auto &key) {
        return m_columnDisplay.value(key) != QStringLiteral("hidden");
    });

    std::stable_sort(result.begin(), result.end(), [sourceRowMapping](const QString &first, const QString &second) {
        return sourceRowMapping.value(first) < sourceRowMapping.value(second);
    });

    return result;
}

QString ColumnDisplayModel::idRole() const
{
    return m_idRole;
}

void ColumnDisplayModel::setIdRole(const QString &newIdRole)
{
    if (newIdRole == m_idRole) {
        return;
    }

    beginResetModel();
    m_idRole = newIdRole;
    m_idRoleNumber = -1;
    endResetModel();
    Q_EMIT idRoleChanged();
}

int ColumnDisplayModel::idRoleNumber() const
{
    if (m_idRoleNumber == -1 && !m_idRole.isEmpty() && sourceModel()) {
        m_idRoleNumber = sourceModel()->roleNames().key(m_idRole.toUtf8());
    }
    return m_idRoleNumber;
}

#include "moc_ColumnDisplayModel.cpp"
