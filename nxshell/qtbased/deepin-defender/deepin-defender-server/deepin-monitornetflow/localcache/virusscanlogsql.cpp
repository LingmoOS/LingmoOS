// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "virusscanlogsql.h"

#include <QDir>
#include <QDebug>
#include <QDateTime>
#include <QFileInfo>

#define DEFENDER_DATA_DIR_PATH "/usr/share/deepin-defender/" // 数据库路径
#define LOCAL_CACHE_DB_NAME "localcache.db" // 数据库路径
#define TABLE_NAME_VIRUS_SCAN_LOG "VirusScanLog"

VirusScanLogSql::VirusScanLogSql(const QString &connName, QObject *parent)
    : QObject(parent)
    , m_connName(connName)
    , m_sqlQuery(nullptr)
{
    initDb();
}

VirusScanLogSql::~VirusScanLogSql()
{
}

void VirusScanLogSql::initDb()
{
    //初始化数据库
    QDir defenderDataDir;
    if (!defenderDataDir.exists(DEFENDER_DATA_DIR_PATH)) {
        defenderDataDir.mkdir(DEFENDER_DATA_DIR_PATH);
    }
    m_db = QSqlDatabase::addDatabase("QSQLITE", m_connName);
    m_localCacheDbPath = QString("%1%2").arg(DEFENDER_DATA_DIR_PATH).arg(LOCAL_CACHE_DB_NAME);
    m_db.setDatabaseName(m_localCacheDbPath);

    if (!m_db.open()) {
        qDebug() << "local db open error!";
        return;
    }

    // 如果 localcache.db数据库已经存在，则服务启动时不需要刷新数据
    QStringList tableStrList = m_db.tables();
    m_sqlQuery = QSqlQuery(m_db);
    if (!tableStrList.contains(TABLE_NAME_VIRUS_SCAN_LOG)) {
        // 服务第一次启动启动时，创建包名解析表 flowDetail
        QString cmd = QString("CREATE TABLE %1 (id Long primary key, key text, value text);")
                          .arg(TABLE_NAME_VIRUS_SCAN_LOG);
        bool bRet = m_sqlQuery.exec(cmd);
        if (!bRet) {
            qDebug() << Q_FUNC_INFO << m_sqlQuery.lastError();
            return;
        }
    }
}
