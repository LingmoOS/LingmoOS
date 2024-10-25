/*
 *  SPDX-FileCopyrightText: 2022 Eugene Popov <popov895@ukr.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include "katesessionmanager.h"

#include <QAbstractListModel>
#include <QDateTime>

class SavedSessionsModel : public QAbstractListModel
{
public:
    explicit SavedSessionsModel(QObject *parent = nullptr);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    void refresh(const KateSessionList &sessionList);

private:
    struct SessionInfo {
        QDateTime lastOpened;
        QString name;

        bool operator<(const SessionInfo &other) const
        {
            return lastOpened > other.lastOpened;
        }
    };

    std::vector<SessionInfo> m_sessions;
};
