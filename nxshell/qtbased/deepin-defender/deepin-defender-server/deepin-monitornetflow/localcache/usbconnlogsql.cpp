// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "usbconnlogsql.h"

#include <QDir>
#include <QDebug>
#include <QDateTime>
#include <QFileInfo>

#define DEFENDER_DATA_DIR_PATH "/usr/share/deepin-defender/" // 数据库路径
#define LOCAL_CACHE_DB_NAME "localcache.db" // 数据库路径
#define TABLE_NAME_USB_LOG "UsbConnLog"

UsbConnLogSql::UsbConnLogSql(const QString &connName, QObject *parent)
    : QObject(parent)
    , m_connName(connName)
    , m_sqlQuery(nullptr)
{
    initDb();
}

UsbConnLogSql::~UsbConnLogSql()
{
}

void UsbConnLogSql::writeConnectionRecord(const QString &name, const QString &datetime, const QString &status,
                                          const QString &serial, const QString &vendorID, const QString &productID,
                                          const QString &parentSize, const QString &size)
{
    QString cmd = QString("INSERT INTO %1 (name, datetime, status, serial, vendorID, productID, parentSize, size) "
                          "VALUES ('%2', '%3', '%4', '%5', '%6', '%7', '%8', '%9');")
                      .arg(TABLE_NAME_USB_LOG)
                      .arg(name)
                      .arg(datetime)
                      .arg(status)
                      .arg(serial)
                      .arg(vendorID)
                      .arg(productID)
                      .arg(parentSize)
                      .arg(size);
    if (!m_sqlQuery.exec(cmd)) {
        qDebug() << Q_FUNC_INFO << m_sqlQuery.lastError();
    }
}

QList<UsbConnectionInfo> UsbConnLogSql::readConnectionRecord()
{
    QList<UsbConnectionInfo> list;
    QString cmd = QString("SELECT * FROM %1;").arg(TABLE_NAME_USB_LOG);
    if (!m_sqlQuery.exec(cmd)) {
        qDebug() << Q_FUNC_INFO << m_sqlQuery.lastError();
    }

    while (m_sqlQuery.next()) {
        UsbConnectionInfo info;
        info.sName = m_sqlQuery.value(1).toString();
        info.sDatetime = m_sqlQuery.value(2).toString();
        info.sStatus = m_sqlQuery.value(3).toString();
        info.sSerial = m_sqlQuery.value(4).toString();
        info.sVendorID = m_sqlQuery.value(5).toString();
        info.sProductID = m_sqlQuery.value(6).toString();
        info.sParentSize = m_sqlQuery.value(7).toString();
        info.sSize = m_sqlQuery.value(8).toString();
        list.append(info);
    }

    return list;
}

// 清空过期日志
void UsbConnLogSql::cleanUsbConnectionLog()
{
    QDateTime limitTime = QDateTime::currentDateTime().addDays(-30);
    QString command = QString("DELETE FROM %1 WHERE datetime<'%2';")
                          .arg(TABLE_NAME_USB_LOG)
                          .arg(limitTime.toString("yyyy-MM-dd hh:mm:ss"));

    if (!m_sqlQuery.exec(command)) {
        qDebug() << m_sqlQuery.lastError();
    }
}

// 清空全部日志
void UsbConnLogSql::clearUsbConnectionLog()
{
    QString command = QString("DELETE FROM %1;")
                          .arg(TABLE_NAME_USB_LOG);

    if (!m_sqlQuery.exec(command)) {
        qDebug() << m_sqlQuery.lastError();
    }
}

void UsbConnLogSql::initDb()
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
    if (!tableStrList.contains(TABLE_NAME_USB_LOG)) {
        // 服务第一次启动启动时，创建包名解析表 flowDetail
        QString cmd = QString("CREATE TABLE %1 (id Long primary key, name text, datetime text,"
                              "status text, serial text, vendorID text, productID text,"
                              "parentSize text, size text);")
                          .arg(TABLE_NAME_USB_LOG);
        bool bRet = m_sqlQuery.exec(cmd);
        if (!bRet) {
            qDebug() << Q_FUNC_INFO << m_sqlQuery.lastError();
            return;
        }
    }
}
