/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "PageDataModel.h"

#include <QDebug>

#include <KConfig>
#include <KConfigGroup>

#include "PageDataObject.h"

PageDataModel::PageDataModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

QHash<int, QByteArray> PageDataModel::roleNames() const
{
    static QHash<int, QByteArray> roles{
        {DataRole, "data"},
    };
    return roles;
}

PageDataObject *PageDataModel::dataObject() const
{
    return m_data;
}

void PageDataModel::setDataObject(PageDataObject *newDataObject)
{
    if (newDataObject == m_data) {
        return;
    }

    if (m_data) {
        m_data->disconnect(this);
    }

    beginResetModel();
    m_data = newDataObject;
    endResetModel();

    if (m_data) {
        connect(m_data, &PageDataObject::childInserted, this, [this](int index) {
            beginInsertRows(QModelIndex{}, index, index);
            endInsertRows();
        });
        connect(m_data, &PageDataObject::childRemoved, this, [this](int index) {
            beginRemoveRows(QModelIndex{}, index, index);
            endRemoveRows();
        });
        connect(m_data, &PageDataObject::childMoved, this, [this](int from, int to) {
            if (from < to) {
                beginMoveRows(QModelIndex{}, from, from, QModelIndex{}, to + 1);
            } else {
                beginMoveRows(QModelIndex{}, from, from, QModelIndex{}, to);
            }
            endMoveRows();
        });
        connect(m_data, &PageDataObject::loaded, this, [this] {
            beginResetModel();
            endResetModel();
        });
    }

    Q_EMIT dataObjectChanged();
}

int PageDataModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid() || !m_data) {
        return 0;
    }

    return m_data->childCount();
}

QVariant PageDataModel::data(const QModelIndex &index, int role) const
{
    if (!m_data || !checkIndex(index, CheckIndexOption::IndexIsValid | CheckIndexOption::DoNotUseParent)) {
        return QVariant{};
    }

    auto data = m_data->childAt(index.row());
    if (!data) {
        return QVariant{};
    }

    switch (role) {
    case DataRole:
        return QVariant::fromValue(data);
    default:
        return QVariant{};
    }
}

int PageDataModel::countObjects(const QVariantMap &properties)
{
    if (!m_data) {
        return 0;
    }

    if (properties.isEmpty()) {
        return m_data->childCount();
    }

    int result = 0;
    const auto children = m_data->children();
    for (auto child : children) {
        auto match = std::all_of(properties.keyValueBegin(), properties.keyValueEnd(), [child](const std::pair<QString, QVariant> &entry) {
            return child->value(entry.first) == entry.second;
        });
        if (match) {
            result += 1;
        }
    }

    return result;
}

#include "moc_PageDataModel.cpp"
