// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MOCK_SECURITY_LOG_DATA_INTERFACE_H
#define MOCK_SECURITY_LOG_DATA_INTERFACE_H

#include "../../deepin-defender/src/window/modules/securitylog/securitylogadaptorinterface.h"

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDate>
#include <QDir>

class MockSecurityLogDataInterface : public QObject
    , public SecurityLogAdaptorInterface
{
    Q_OBJECT
public:
    explicit MockSecurityLogDataInterface(QObject *parent = nullptr)
        : QObject(parent)
    {
        if (QSqlDatabase::contains("SecurityLog")) {
            m_sqlDb = QSqlDatabase::database("SecurityLog");
        } else {
            m_sqlDb = QSqlDatabase::addDatabase("QSQLITE", "SecurityLog");
        }
        // 使用相对路径上固定的数据库文件
        QString path = QDir::currentPath();
        // 不能直接使用QRC路径访问数据库
        // 建立全局事件将数据库文件从QRC拷贝到构建目录
        m_sqlDb.setDatabaseName("localcache.db");
    }

    virtual ~MockSecurityLogDataInterface() {}

    virtual QSqlDatabase GetDatabase()
    {
        return m_sqlDb;
    }

    // lastdate 设定删除多少天以前
    // type 设定删除哪些种类的日志
    virtual bool DeleteSecurityLog(int lastdate, int type)
    {
        // 不要修改以下两行的内容
        QString deleteCmd = "DELETE FROM SecurityLog WHERE datetimes between '%1 23:59:59' and '%2 23:59:59'";
        QString deleteCmdWithType = "DELETE FROM SecurityLog WHERE datetimes between '%1 23:59:59' and '%2 23:59:59' and type=%3";

        if (!m_sqlDb.open()) {
            return false;
        }

        QString cmd;
        // 获取固定的删除日期
        QString deleteDate;
        int deleteDay = 0;
        switch (lastdate) {
        case 0:
            // 前一天
            deleteDay = -1;
            break;
        case 1:
            // 前三天
            deleteDay = -3;
            break;
        case 2:
            // 前七天
            deleteDay = -7;
            break;
        default:
            // 前三十天
            deleteDay = -30;
        }

        QString today = QDate::currentDate().toString("yyyy-MM-dd");
        deleteDate = QDate::currentDate().addDays(deleteDay).toString("yyyy-MM-dd");
        if (0 == type) {
            cmd = QString(deleteCmd).arg(deleteDate).arg(today);
        } else {
            cmd = QString(deleteCmdWithType).arg(deleteDate).arg(today).arg(type);
        }

        QSqlQuery query(m_sqlDb);
        if (!query.exec(cmd)) {
            m_sqlDb.close();
            return false;
        }

        m_sqlDb.close();
        return true;
    }

    virtual void AddSecurityLog(int nType, const QString &sInfo)
    {
        // 当前时间
        QString sDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

        // 打开数据
        if (!m_sqlDb.open()) {
            return;
        }

        // 添加数据
        QSqlQuery query(m_sqlDb);
        if (!query.exec(QString("INSERT INTO SecurityLog (datetimes, type, info) VALUES ('%1', '%2', '%3')").arg(sDateTime).arg(nType).arg(sInfo))) {
        }

        m_sqlDb.close();
    }

private:
    QSqlDatabase m_sqlDb;
};

#endif // MOCK_SECURITY_LOG_DATA_INTERFACE_H
