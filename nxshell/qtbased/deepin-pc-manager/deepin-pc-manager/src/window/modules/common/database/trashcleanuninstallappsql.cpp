// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "trashcleanuninstallappsql.h"

#include <QDebug>

#define QUERY_CMD "select * from uninstallAppinfo order by id desc;"
#define INSERT_CMD "insert into uninstallAppinfo (APPID,APPNAME) values ('%1','%2');"
#define DELETE_CMD "delete from uninstallAppinfo where appid = '%1'";
#define UNINSTALL_TABLE_NAME "uninstallAppinfo"

TrashCleanUninstallAppSql::TrashCleanUninstallAppSql(QObject *parent)
    : DefenderDatabaseOperationBase(parent)
{
}

TrashCleanUninstallAppSql::~TrashCleanUninstallAppSql() { }

// 提供给基类以检查表是否存在
QString TrashCleanUninstallAppSql::tableName()
{
    return UNINSTALL_TABLE_NAME;
}

// 重写以完成父类中的建表
bool TrashCleanUninstallAppSql::createTable()
{
    QString createCmd = "CREATE TABLE %1 "
                        "(id INTEGER PRIMARY KEY AUTOINCREMENT, "
                        "appid TEXT NOT NULL,"
                        "appname TEXT NOT NULL);";
    createCmd = createCmd.arg(UNINSTALL_TABLE_NAME);
    return executeQuery(createCmd);
}

// 已完成命令的输出格式化
// 通过特有的信号发送结果
void TrashCleanUninstallAppSql::sendResult(int cmdID, QSqlQuery &query)
{
    QVariantList queryResult;
    switch (cmdID) {
    case UninstalledAppSqlCmdType::INSERT_A_RECORD:
    case UninstalledAppSqlCmdType::DELETE_A_RECORD:
        // DO NOTHING
        Q_EMIT sendQueryResultSignal({});
        break;
    case UninstalledAppSqlCmdType::GET_ALL_RECORDS:
        while (query.next()) {
            QStringList record = { query.value(1).toString(), query.value(2).toString() };
            queryResult.append(QVariant::fromValue(record));
        }
        Q_EMIT sendQueryResultSignal(queryResult);
        break;
    }
}

// 待执行命令的格式化，不同ID对应不同形为
// 均为自定义内容
QString TrashCleanUninstallAppSql::formatCmd(int cmdID, const QVariantList &params)
{
    QString queryCmd;
    int paramSize = params.size();
    switch (cmdID) {
    case UninstalledAppSqlCmdType::INSERT_A_RECORD:
        queryCmd = INSERT_CMD;
        if (2 != paramSize) {
            qDebug() << "TrashCleanUninstallAppSql insert incorrect param size";
            return QString();
        }
        {
            // 一般来讲不会出现包ID空的情况
            QString appID = params.at(0).value<QString>();
            if (appID.isEmpty()) {
                return QString();
            }
            // 注意主键插入NULL以完成自增
            queryCmd = queryCmd.arg(appID).arg(params.at(1).value<QString>());
        }
        break;
    case UninstalledAppSqlCmdType::GET_ALL_RECORDS:
        queryCmd = QUERY_CMD;
        break;
    case UninstalledAppSqlCmdType::DELETE_A_RECORD:
        queryCmd = DELETE_CMD;
        if (1 != paramSize) {
            qDebug() << "TrashCleanUninstallAppSql delete incorrect param size";
            return QString();
        }
        {
            QString appID = params.first().value<QString>();
            queryCmd = queryCmd.arg(appID);
        }
        break;
    }
    return queryCmd;
}
