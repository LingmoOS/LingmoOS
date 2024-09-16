// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TRASHCLEANUNINSTALLAPPSQL_H
#define TRASHCLEANUNINSTALLAPPSQL_H

#include "defenderdatabaseoperationbase.h"

class TrashCleanUninstallAppSql : public DefenderDatabaseOperationBase
{
    Q_OBJECT
public:
    enum UninstalledAppSqlCmdType { GET_ALL_RECORDS, INSERT_A_RECORD, DELETE_A_RECORD };

public:
    typedef QList<QPair<QString, QString>> QueryRecordsList;
    explicit TrashCleanUninstallAppSql(QObject *parent = nullptr);
    virtual ~TrashCleanUninstallAppSql() override;

Q_SIGNALS:
    void sendQueryResultSignal(QVariantList);

private:
    QString tableName() override;
    void sendResult(int, QSqlQuery &) override;
    QString formatCmd(int, const QVariantList & = {}) override;
    virtual bool createTable() override;
};

#endif // TRASHCLEANUNINSTALLAPPSQL_H
