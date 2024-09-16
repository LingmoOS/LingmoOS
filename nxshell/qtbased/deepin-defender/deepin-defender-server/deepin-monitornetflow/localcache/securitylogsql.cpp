// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "securitylogsql.h"

#include <QDir>
#include <QDebug>
#include <QDateTime>
#include <QFileInfo>

#define DEFENDER_DATA_DIR_PATH "/usr/share/deepin-defender/" // 数据库路径
#define LOCAL_CACHE_DB_NAME "localcache.db" // 数据库路径
#define TABLE_NAME_SECURITY_LOG "SecurityLog"

SecurityLogSql::SecurityLogSql(const QString &connName, QObject *parent)
    : QObject(parent)
    , m_connName(connName)
{
    initDb();
}

SecurityLogSql::~SecurityLogSql()
{
}

// 添加安全日志
void SecurityLogSql::addSecurityLog(int nType, QString sInfo)
{
    // 当前时间
    QString sDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

    // 打开数据
    if (!m_db.open()) {
        qDebug() << "security.db open failed! ";
    }

    // 添加数据
    QSqlQuery query(m_db);
    if (!query.exec(QString("INSERT INTO SecurityLog (datetimes, type, info) VALUES ('%1', '%2', '%3')").arg(sDateTime).arg(nType).arg(sInfo))) {
        qDebug() << "security log add error : " << query.lastError();
    }

    m_db.close();
}

// 删除过滤条件下的安全日志
bool SecurityLogSql::deleteSecurityLog(int nLastDate, int nType)
{
    m_securityLogDate = nLastDate;
    m_securityLogType = nType;

    // 打开数据
    if (!m_db.open()) {
        qDebug() << "security.db open failed! ";
    }

    // 添加数据
    QSqlQuery query(m_db);
    if (!query.exec(getSecurityLogCmd())) {
        m_db.close();
        return false;
    }

    m_db.close();
    return true;
}

// 获取日志刷新需要用到的sql语句
QString SecurityLogSql::getSecurityLogCmd()
{
    // 当前的日期
    QDate date = QDate::currentDate();
    QString sCurrentDate = date.toString("yyyy-MM-dd");

    QString sCmd;
    // 过滤条件为 日志类型为全部/日期为当天
    if (SECURITY_LOG_TYPE_ALL == m_securityLogType) {
        sCmd = QString("DELETE FROM SecurityLog WHERE datetimes between '%1 23:59:59' and '%2 23:59:59'").arg(getBeginDate(date)).arg(sCurrentDate);
    } else {
        sCmd = QString("DELETE FROM SecurityLog WHERE datetimes between '%1 23:59:59' and '%2 23:59:59' and type=%3").arg(getBeginDate(date)).arg(sCurrentDate).arg(m_securityLogType);
    }

    return sCmd;
}

// 得到日期过滤 的开始日期
QString SecurityLogSql::getBeginDate(QDate date)
{
    // 根据日期过滤条件得到 过滤开始时间
    QString sLastDate;
    if (LAST_DATE_NOW == m_securityLogDate) {
        sLastDate = date.addDays(-1).toString("yyyy-MM-dd");
    } else if (LAST_DATE_THREE == m_securityLogDate) {
        sLastDate = date.addDays(-3).toString("yyyy-MM-dd");
    } else if (LAST_DATE_SEVEN == m_securityLogDate) {
        sLastDate = date.addDays(-7).toString("yyyy-MM-dd");
    } else {
        sLastDate = date.addDays(-30).toString("yyyy-MM-dd");
    }
    return sLastDate;
}

void SecurityLogSql::initDb()
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
    QSqlQuery query(m_db);
    query = QSqlQuery(m_db);
    if (!tableStrList.contains(TABLE_NAME_SECURITY_LOG)) {
        // 服务第一次启动启动时，创建数据表
        QString cmd = QString("CREATE TABLE %1 (datetimes datetime NOT NULL, type INTEGER NOT NULL, "
                              "info VARCHAR NOT NULL, id INTEGER PRIMARY KEY AUTOINCREMENT);")
                          .arg(TABLE_NAME_SECURITY_LOG);
        bool bRet = query.exec(cmd);
        if (!bRet) {
            qDebug() << Q_FUNC_INFO << query.lastError();
            return;
        }
    }

    // 当前的日期
    QDate date = QDate::currentDate();
    QString sLastDate = date.addDays(-30).toString("yyyy-MM-dd");

    // 删除30天以前的数据
    if (!query.exec(QString("DELETE FROM SecurityLog WHERE datetimes < '%1 23:59:59'").arg(sLastDate))) {
        qDebug() << "security log add error : " << query.lastError();
    }

    m_db.close();
}
