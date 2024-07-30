// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>

#pragma once

#include <QAbstractListModel>

#include "bugzillaintegration/reportinterface.h"

class ReproducibilityModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum class Role {
        String = Qt::UserRole,
        Integer,
    };
    Q_ENUM(Role)

    using QAbstractListModel::QAbstractListModel;

    [[nodiscard]] int rowCount(const QModelIndex &parent) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int intRole) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

private:
    static QList<ReportInterface::Reproducible> reproducibilities();
    const QList<ReportInterface::Reproducible> m_list = reproducibilities();
};
