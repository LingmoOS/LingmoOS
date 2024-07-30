// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2011 Craig Drummond <craig.p.drummond@gmail.com>
// SPDX-FileCopyrightText: 2018 Alexis Lopes Zubeta <contact@azubieta.net>
// SPDX-FileCopyrightText: 2020 Tomaz Canabrava <tcanabrava@kde.org>
/*
 * UFW KControl Module
 */

#pragma once

#include <kcm_firewall_core_export.h>

#include <QAbstractTableModel>

#include "profile.h"

class KCM_FIREWALL_CORE_EXPORT RuleListModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum ProfileItemModelColumns { ActionColumn = 0, FromColumn, ToColumn, Ipv6Column, LoggingColumn, EditColumn};
    Q_ENUM(ProfileItemModelColumns)

    explicit RuleListModel(QObject *parent = nullptr);

    Q_INVOKABLE void move(int from, int to);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void setProfile(const Profile &profile);

private:
    Profile m_profile;
    QList<Rule *> m_rules;
};
