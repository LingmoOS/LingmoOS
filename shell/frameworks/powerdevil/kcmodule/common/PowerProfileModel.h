/*
 *  SPDX-FileCopyrightText: 2021 Tomaz Canabrava <tcanabrava@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#pragma once

#include <QAbstractListModel>

#include <QList>
#include <QString>

#include "powerdevilconfigcommonprivate_export.h"

class POWERDEVILCONFIGCOMMONPRIVATE_EXPORT PowerProfileModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum { Name = Qt::DisplayRole, Value = Qt::UserRole };

    PowerProfileModel(QObject *parent = nullptr);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    struct Data {
        QString name;
        QString value;
    };

    QList<Data> m_data;
};
