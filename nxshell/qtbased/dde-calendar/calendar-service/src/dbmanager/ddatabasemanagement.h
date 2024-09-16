// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DDATABASEMANAGEMENT_H
#define DDATABASEMANAGEMENT_H

#include "dschedule.h"
#include "dscheduletype.h"
#include "dreminddata.h"

#include <QString>
#include <QSqlDatabase>

/**
 * @brief The DDataBaseManagement class
 * 数据库初始化 ，管理数据的创建和对旧版本的数据迁移
 */

class DDataBaseManagement
{
public:
    DDataBaseManagement();

    bool hasTransfer() const;

private:
    QString newDatabasePath() const;
    void setNewDatabasePath(const QString &newDatabasePath);

    QString oldDatabasePath() const;
    void setOldDatabasePath(const QString &oldDatabasePath);

    //数据库所在文件夹是否存在，若不存在则创建该文件夹
    bool databaseExists(const QString &databasePath, bool create = true);

    //旧数据库jab表是否存在是否为农历日程字段
    bool hasLunnarField(QSqlDatabase &db);
    //存在日程类型相关数据
    bool hasTypeDB(QSqlDatabase &db);
    //存在提醒任务表
    bool hasRemindDB(QSqlDatabase &db);

    DScheduleType::List queryOldJobTypeData(QSqlDatabase &db);
    DSchedule::List queryOldJobData(QSqlDatabase &db, const bool haslunar);

    QVector<DTypeColor> queryOldTypeColorData(QSqlDatabase &db);
    DRemindData::List querOldRemindData(QSqlDatabase &db);

private:
    //新数据库路径地址
    QString m_newDatabasePath;
    //旧数据库路径地址
    QString m_oldDatabasePath;
    QString m_newDatabaseName;
    QString m_oldDatabaseName;

    bool  m_hasTransfer = false;

    QMap<int, QString> m_typeMap;
    QMap<int, QString> m_sysColorID;    //日程类型内置颜色新旧id对应map
    QMap<int, QString> m_schedule;
    QMap<int, QString> m_typeColorID; //日程类型颜色新旧id对应map
};

#endif // DDATABASEMANAGEMENT_H
