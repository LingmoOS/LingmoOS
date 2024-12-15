// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "database.h"

DataBase::DataBase()
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");

    // QString _dataBasePath = qApp->applicationDirPath();
    QString dbPath = QString("%1/%2/%3/downloader.db")
                         .arg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation))
                         .arg(qApp->organizationName())
                         .arg(qApp->applicationName());
    QFileInfo databaseFile(dbPath);

    //如果数据库文件不存在
    if (!databaseFile.isFile()) {
        //拷贝数据库文件
        qDebug() << "error: no db file  "; //<< QString(UOS_DONWLOAD_DATABASE_PATH) + UOS_DOWNLOAD_DATABASE_FILENAME << QString(_dataBasePath) + UOS_DOWNLOAD_DATABASE_FILENAME << endl;
        QFile::copy(QString(UOS_DONWLOAD_DATABASE_PATH) + UOS_DOWNLOAD_DATABASE_OLD_FILENAME,
                    dbPath);
        QString strold = UOS_DONWLOAD_DATABASE_PATH + "downloader.db";
        QByteArray ba = strold.toLatin1();
        char *oldDatabaseFile = ba.data();
        remove(oldDatabaseFile);
        QFile::copy(strold, dbPath);
    }

    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dbPath);
    m_db.open();
}

DataBase &DataBase::Instance()
{
    static DataBase *d = nullptr;
    if (d == nullptr) {
        d = new DataBase();
    }
    return *d;
}

QSqlDatabase &DataBase::getDB()
{
    return m_db;
}

void DataBase::destory()
{
    m_db.close();
}
