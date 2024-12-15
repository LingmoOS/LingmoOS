// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "../common/common.h"
#include "../common/gsettingkey.h"
#include "../common/comdata.h"

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

class VirusScanLogSql : public QObject
{
    Q_OBJECT

public:
    explicit VirusScanLogSql(const QString &connName, QObject *parent = nullptr);
    ~VirusScanLogSql();

public Q_SLOTS:
    void initDb();

private:
    QString m_connName;
    QString m_localCacheDbPath;
    QSqlDatabase m_db;
    QSqlQuery m_sqlQuery;
};
