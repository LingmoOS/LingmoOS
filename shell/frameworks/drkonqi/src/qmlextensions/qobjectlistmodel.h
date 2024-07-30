// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>

#pragma once

#include <QAbstractListModel>
#include <QMetaEnum>

class QObjectListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum class Role {
        Object = Qt::UserRole,
        UserRole,
    };
    Q_ENUM(Role)

    using QAbstractListModel::QAbstractListModel;

    virtual QObject *object(const QModelIndex &index) const = 0;

    [[nodiscard]] QVariant data(const QModelIndex &index, int intRole) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;
};
