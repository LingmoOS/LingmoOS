/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2023 Devin Lin <devin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "IdentitiesModel.h"
#include <KLocalizedString>
#include <KUser>
#include <QDebug>

IdentitiesModel::IdentitiesModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

void IdentitiesModel::setIdentities(const PolkitQt1::Identity::List &identities, bool withVoidItem)
{
    beginResetModel();
    m_identities = identities;
    m_ids.clear();
    m_ids.reserve(m_identities.size() + 1);

    if (withVoidItem) {
        // Adds a Dummy user
        m_ids.append(Id{{}, i18n("Select User"), {}, {}});
    }

    // For each user
    for (const PolkitQt1::Identity &identity : identities) {
        // First check to see if the user is valid

        const QString identityString = identity.toString();
        qDebug() << "User: " << identityString;
        const KUser user(QString(identityString).remove("unix-user:"));
        if (!user.isValid()) {
            qWarning() << "User invalid: " << user.loginName();
            continue;
        }

        // Display user Full Name IF available
        QString display;
        if (!user.property(KUser::FullName).toString().isEmpty()) {
            display = i18nc("%1 is the full user name, %2 is the user login name", "%1 (%2)", user.property(KUser::FullName).toString(), user.loginName());
        } else {
            display = user.loginName();
        }

        QString icon;
        // load user icon face
        if (!user.faceIconPath().isEmpty()) {
            icon = user.faceIconPath();
        } else {
            icon = "user-identity";
        }
        m_ids.append(Id{icon, display, identityString, user.loginName()});
    }
    endResetModel();
}

QVariant IdentitiesModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid)) {
        return {};
    }

    const Id &id = m_ids[index.row()];
    switch (role) {
    case Qt::DecorationRole:
        return id.decoration;
    case Qt::DisplayRole:
        return id.display;
    case Qt::UserRole:
        return id.userRole;
    }
    return {};
}

int IdentitiesModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_ids.count();
}

Qt::ItemFlags IdentitiesModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return {};
    auto ret = Qt::ItemNeverHasChildren | Qt::ItemNeverHasChildren;
    if (!m_ids[index.row()].userRole.isEmpty())
        ret |= Qt::ItemIsEnabled;

    return ret;
}

int IdentitiesModel::indexForUser(const QString &loginName) const
{
    for (int i = 0, c = m_ids.count(); i < c; ++i) {
        if (m_ids[i].loginName == loginName) {
            return i;
        }
    }
    return -1;
}

QString IdentitiesModel::iconForIndex(int index) const
{
    if (index < 0 || index >= m_ids.size()) {
        return {};
    }
    return m_ids[index].decoration;
}

QHash<int, QByteArray> IdentitiesModel::roleNames() const
{
    return {{Qt::DecorationRole, "decoration"}, {Qt::DisplayRole, "display"}, {Qt::UserRole, "userRole"}};
}

#include "moc_IdentitiesModel.cpp"
