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

class UsbConnLogSql : public QObject
{
    Q_OBJECT

public:
    explicit UsbConnLogSql(const QString &connName, QObject *parent = nullptr);
    ~UsbConnLogSql();

    void writeConnectionRecord(const QString &name, const QString &datetime, const QString &status,
                               const QString &serial, const QString &vendorID, const QString &productID,
                               const QString &parentSize, const QString &size);
    QList<UsbConnectionInfo> readConnectionRecord();
    // 清空过期日志
    void cleanUsbConnectionLog();
    // 清空全部日志
    void clearUsbConnectionLog();

public Q_SLOTS:
    void initDb();

private:
private:
    QString m_connName;
    QString m_localCacheDbPath;
    QSqlDatabase m_db;
    QSqlQuery m_sqlQuery;
};
