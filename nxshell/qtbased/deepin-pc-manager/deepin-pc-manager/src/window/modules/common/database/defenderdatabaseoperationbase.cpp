// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "defenderdatabaseoperationbase.h"

#include <QDebug>
#include <QDir>
#include <QSqlError>
#include <QUuid>

// 固定逻辑
// 特殊处理或错误判断请重写InitDataBase
DefenderDatabaseOperationBase::DefenderDatabaseOperationBase(QObject *parent)
    : QObject(parent)
    , m_isInited(false)
{
}

DefenderDatabaseOperationBase::~DefenderDatabaseOperationBase()
{
    if (m_db.isOpen()) {
        m_query.finish();
        m_db.close();
    }
}

bool DefenderDatabaseOperationBase::init()
{
    if (!initDataBase() || tableName().isEmpty()) {
        return false;
    }
    m_isInited = true;

    // 检查表是否存在
    QString checkTableExistCmd = "SELECT name FROM sqlite_master "
                                 "WHERE type='table' "
                                 "AND name = '%1'";
    checkTableExistCmd = checkTableExistCmd.arg(tableName());
    // m_query为私有而且不计划改为protect
    // 所以在此处单独处理
    if (!executeQuery(checkTableExistCmd)) {
        m_isInited = false;
        return m_isInited;
    }
    QString name;
    if (m_query.first()) {
        name = m_query.value(0).toString();
    }
    // 结束查询防止锁数据库
    m_query.finish();

    // 表不存在时创建
    if (name.isEmpty()) {
        if (!createTable()) {
            m_isInited = false;
            return m_isInited;
        }
    }
    return m_isInited;
}

bool DefenderDatabaseOperationBase::initDataBase()
{
    // 初始化数据库
    QDir defenderDataDir;
    if (!defenderDataDir.exists(DEFENDER_DATA_DIR_PATH)) {
        defenderDataDir.mkdir(DEFENDER_DATA_DIR_PATH);
    }

    if (QSqlDatabase::contains(connectName())) {
        m_db = QSqlDatabase::database(connectName());
    } else {
        m_db = QSqlDatabase::addDatabase("QSQLITE", connectName());
    }

    QString path = getDBPath();
    if (path.isEmpty()) {
        return false;
    }
    m_db.setDatabaseName(path);

    if (!m_db.isValid()) {
        qDebug() << path << "-"
                 << "db not valid error!";
        return false;
    }

    if (!m_db.open()) {
        qDebug() << path << "-"
                 << "db open error!";
        return false;
    }
    m_query = QSqlQuery(m_db);

    return true;
}

bool DefenderDatabaseOperationBase::executeQuery(const QString &cmd)
{
    if (!m_isInited) {
        return false;
    }
    return m_query.exec(cmd);
}

// 私有方法不是使用父类使有变量，所以在此方法外判断是否初始化成功
void DefenderDatabaseOperationBase::sendResult(int cmdType, QSqlQuery &query)
{
    Q_UNUSED(cmdType);
    Q_UNUSED(query);

    // 增加信号处理到下面
    // switch(cmdType)
    return;
}

// 注意 增 删 改，均无返回
// 仅查询或某些功能性语句会有返回结果
void DefenderDatabaseOperationBase::executeCmd(int commandID, const QVariantList &params)
{
    if (!m_isInited) {
        return;
    }
    // 参数校验
    QString cmd = formatCmd(commandID, params);

    if (cmd.isEmpty()) {
        return;
    }

    if (executeQuery(cmd)) {
        sendResultSignal(commandID);
    } else {
        QSqlError err = m_query.lastError();
        QString errInfo = err.text();
        qDebug() << "executeCmd query failed : " << errInfo;
    }
}

// 子类重写，自行组装sql命令
QString DefenderDatabaseOperationBase::formatCmd(int cmdType, const QVariantList &params)
{
    Q_UNUSED(cmdType);
    Q_UNUSED(params);
    return "";
}

// 子类重写
// 根据命令ID触发不同的自定义信号
// 从m_query中获取所需数据
void DefenderDatabaseOperationBase::sendResultSignal(int cmdType)
{
    if (!m_isInited) {
        return;
    }
    sendResult(cmdType, m_query);
    m_query.finish();
    return;
}

// 创建连接名
QString DefenderDatabaseOperationBase::connectName()
{
    if (m_connName.isEmpty()) {
        m_connName = QUuid::createUuid().toString();
    }
    return m_connName;
}

// 返回db文件的保存位置
QString DefenderDatabaseOperationBase::getDBPath()
{
    return QString("%1%2").arg(DEFENDER_DATA_DIR_PATH).arg(LOCAL_CACHE_DB_NAME);
}

// 子类重写方法
QString DefenderDatabaseOperationBase::tableName()
{
    return "";
}

// 检查表是否存在，否则创建表
bool DefenderDatabaseOperationBase::createTable()
{
    return false;
}
