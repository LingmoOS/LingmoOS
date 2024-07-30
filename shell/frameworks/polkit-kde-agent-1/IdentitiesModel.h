/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2023 Devin Lin <devin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef IDENTITIESMODEL_H
#define IDENTITIESMODEL_H

#include <PolkitQt1/Identity>
#include <QAbstractListModel>
#include <QIcon>

class IdentitiesModel : public QAbstractListModel
{
    Q_OBJECT
public:
    IdentitiesModel(QObject *parent = nullptr);

    void setIdentities(const PolkitQt1::Identity::List &identities, bool withVoidItem);
    PolkitQt1::Identity::List identities() const
    {
        return m_identities;
    }

    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex &parent) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    int indexForUser(const QString &loginName) const;
    Q_INVOKABLE QString iconForIndex(int index) const;

    QHash<int, QByteArray> roleNames() const override;

private:
    PolkitQt1::Identity::List m_identities;

    struct Id {
        QString decoration;
        QString display;
        QString userRole;
        QString loginName;
    };
    QList<Id> m_ids;
};

#endif
