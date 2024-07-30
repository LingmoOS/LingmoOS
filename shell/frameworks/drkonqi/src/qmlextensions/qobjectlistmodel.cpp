// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>

#include "qobjectlistmodel.h"

QVariant QObjectListModel::data(const QModelIndex &index, int intRole) const
{
    if (!index.isValid()) {
        return {};
    }

    switch (static_cast<Role>(intRole)) {
    case Role::Object:
        return QVariant::fromValue(object(index));
    case Role::UserRole:
        return {};
    }

    return {};
}

QHash<int, QByteArray> QObjectListModel::roleNames() const
{
    static QHash<int, QByteArray> roles;
    if (!roles.isEmpty()) {
        return roles;
    }

    roles = QAbstractListModel::roleNames();
    const QMetaEnum roleEnum = QMetaEnum::fromType<Role>();
    for (int i = 0; i < roleEnum.keyCount(); ++i) {
        const int value = roleEnum.value(i);
        Q_ASSERT(value != -1);
        roles[value] = QByteArray("ROLE_") + roleEnum.valueToKey(value);
    }
    return roles;
}

#include "moc_qobjectlistmodel.cpp"
