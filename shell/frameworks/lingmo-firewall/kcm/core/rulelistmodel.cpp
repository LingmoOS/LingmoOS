// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2011 Craig Drummond <craig.p.drummond@gmail.com>
// SPDX-FileCopyrightText: 2018 Alexis Lopes Zubeta <contact@azubieta.net>
// SPDX-FileCopyrightText: 2020 Tomaz Canabrava <tcanabrava@kde.org>
/*
 * UFW KControl Module
 */

#include "rulelistmodel.h"
#include <klocalizedstring.h>

RuleListModel::RuleListModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

void RuleListModel::move(int from, int to)
{
    if (to < 0 || to >= m_rules.count()) {
        return;
    }

    int newPos = to > from ? to + 1 : to;
    bool validMove = beginMoveRows(QModelIndex(), from, from, QModelIndex(), newPos);
    if (validMove) {
        m_rules.move(from, to);
        endMoveRows();
    }
}

int RuleListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_rules.count();
}

int RuleListModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 6;
}

QVariant RuleListModel::data(const QModelIndex &index, int role) const
{
    Q_UNUSED(role)
    const auto checkIndexFlags = QAbstractItemModel::CheckIndexOption::IndexIsValid | QAbstractItemModel::CheckIndexOption::ParentIsInvalid;

    if (!checkIndex(index, checkIndexFlags)) {
        return {};
    }

    const Rule *rule = m_rules.at(index.row());

    switch (index.column()) {
    case ActionColumn:
        return rule->actionStr();
    case FromColumn:
        return rule->fromStr();
    case ToColumn:
        return rule->toStr();
    case Ipv6Column:
        return rule->ipv6() ? QStringLiteral("IPv6") : QStringLiteral("IPv4");
    case LoggingColumn:
        return rule->loggingStr();
    }
    return QVariant();
}

void RuleListModel::setProfile(const Profile &profile)
{
    qDebug() << "Profile on the model received. enabled? " << profile.enabled();

    beginResetModel();
    m_profile = profile;
    m_rules = m_profile.rules();
    endResetModel();
}

QVariant RuleListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(orientation)
    Q_UNUSED(role)
    switch (section) {
    case ActionColumn:
        return i18nc("@title:column", "Action");
    case FromColumn:
        return i18nc("@title:column", "From");
    case ToColumn:
        return i18nc("@title:column", "To");
    case Ipv6Column:
        return i18nc("@title:column", "IP");
    case LoggingColumn:
        return i18nc("@title:column", "Logging");
    }
    return QVariant();

}
