// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "../common/common.h"

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

class SecurityLogSql : public QObject
{
    Q_OBJECT

public:
    explicit SecurityLogSql(const QString &connName, QObject *parent = nullptr);
    ~SecurityLogSql();

    // 添加安全日志
    void addSecurityLog(int nType, QString sInfo);
    // 删除过滤条件下的安全日志
    bool deleteSecurityLog(int nLastDate, int nType);

public Q_SLOTS:
    void initDb();

private:
    QString getSecurityLogCmd();
    // 得到日期过滤 的开始日期
    QString getBeginDate(QDate date);

private:
    QString m_connName;
    QString m_localCacheDbPath;
    QSqlDatabase m_db;

    int m_securityLogType; // 日志过滤类型
    int m_securityLogDate; // 日志过滤日期
};
