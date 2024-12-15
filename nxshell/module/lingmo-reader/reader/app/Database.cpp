// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Database.h"
#include "DocSheet.h"
#include "Global.h"
#include "DocSheet.h"

#include <QApplication>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <QSqlError>
#include <QSqlQuery>

Transaction::Transaction(QSqlDatabase &database)
    : m_committed(false), m_database(database)
{
    if (!m_database.transaction()) {
        qInfo() << m_database.lastError();
    }
}

Transaction::~Transaction()
{
    if (!m_committed) {
        if (!m_database.rollback()) {
            qInfo() << m_database.lastError();
        }
    }
}

void Transaction::commit()
{
    m_committed = m_database.commit();
}


Database *Database::s_instance = nullptr;

Database *Database::instance()
{
    if (s_instance == nullptr) {
        s_instance = new Database(qApp);
    }

    return s_instance;
}

Database::~Database()
{
    m_database.close();
    s_instance = nullptr;
}

bool Database::prepareOperation()
{
    Transaction transaction(m_database);

    QSqlQuery query(m_database);
    if (!query.exec("CREATE TABLE operation "
                    "(filePath TEXT primary key"
                    ",layoutMode INTEGER"
                    ",mouseShape INTEGER"
                    ",scaleMode INTEGER"
                    ",rotation INTEGER"
                    ",scaleFactor REAL"
                    ",sidebarVisible INTEGER"
                    ",sidebarIndex INTEGER"
                    ",currentPage INTEGER)")) {
        qInfo() << query.lastError();
        return false;
    }

    if (!query.isActive()) {
        qInfo() << query.lastError();
        return false;
    }

    transaction.commit();
    return true;
}

bool Database::readOperation(DocSheet *sheet)
{
    if(!sheet)
        return false;

    QSqlQuery query(m_database);
    query.prepare(" select * from operation where filePath = :filePath");
    query.bindValue(":filePath", sheet->filePath());
    query.exec();
    if (query.next()) {
        sheet->m_operation.layoutMode = static_cast<Dr::LayoutMode>(query.value("layoutMode").toInt());
        sheet->m_operation.mouseShape = static_cast<Dr::MouseShape>(query.value("mouseShape").toInt());
        sheet->m_operation.scaleMode = static_cast<Dr::ScaleMode>(query.value("scaleMode").toInt());
        sheet->m_operation.rotation = static_cast<Dr::Rotation>(query.value("rotation").toInt());
        sheet->m_operation.scaleFactor = qBound(0.1, query.value("scaleFactor").toDouble(), 5.0);
        sheet->m_operation.sidebarVisible = query.value("sidebarVisible").toInt();
        sheet->m_operation.sidebarIndex = query.value("sidebarIndex").toInt();
        sheet->m_operation.currentPage = query.value("currentPage").toInt();
        return true;
    }
    return false;
}

bool Database::saveOperation(DocSheet *sheet)
{
    if(!sheet)
        return false;

    QSqlQuery query(m_database);
    query.prepare(" replace into "
                  " operation(filePath,layoutMode,mouseShape,scaleMode,rotation,scaleFactor,sidebarVisible,sidebarIndex,currentPage)"
                  " VALUES(:filePath,:layoutMode,:mouseShape,:scaleMode,:rotation,:scaleFactor,:sidebarVisible,:sidebarIndex,:currentPage)");
    query.bindValue(":filePath", sheet->filePath());
    query.bindValue(":layoutMode", sheet->m_operation.layoutMode);
    query.bindValue(":mouseShape", sheet->m_operation.mouseShape);
    query.bindValue(":scaleMode", sheet->m_operation.scaleMode);
    query.bindValue(":rotation", sheet->m_operation.rotation);
    query.bindValue(":scaleFactor", sheet->m_operation.scaleFactor);
    query.bindValue(":sidebarVisible", sheet->m_operation.sidebarVisible);
    query.bindValue(":sidebarIndex", sheet->m_operation.sidebarIndex);
    query.bindValue(":currentPage", sheet->m_operation.currentPage);

    if(!query.exec()){
        qInfo() << query.lastError().text();
        return false;
    }
    return true;
}

bool Database::prepareBookmark()
{
    Transaction transaction(m_database);

    QSqlQuery query(m_database);
    if (!query.exec("CREATE TABLE bookmark(filePath TEXT,bookmarkIndex INTEGER)")) {
        qInfo() << query.lastError();
        return false;
    }

    if (!query.isActive()) {
        qInfo() << query.lastError();
        return false;
    }

    transaction.commit();
    return true;
}

bool Database::readBookmarks(const QString &filePath, QSet<int> &bookmarks)
{
    if (m_database.isOpen()) {
        QSqlQuery query(m_database);

        if (!query.prepare(" select * from bookmark where filePath = :filePath")) {
            qInfo() << query.lastError();
            return false;
        }
        query.bindValue(":filePath", filePath);

        if (!query.exec()) {
            qInfo() << query.lastError().text();
            return false;
        }

        while (query.next()) {
            bookmarks.insert(query.value("bookmarkIndex").toInt());
        }

        return true;
    }

    return false;
}

bool Database::saveBookmarks(const QString &filePath, const QSet<int> bookmarks)
{
    if (m_database.isOpen()) {
        QSqlQuery query(m_database);

        Transaction transaction(m_database);

        if (!query.prepare("delete from bookmark where filePath = :filePath")) {
            qInfo() << query.lastError();
            return false;
        }

        query.bindValue(":filePath", filePath);

        if (!query.exec()) {
            qInfo() << query.lastError().text();
            return false;
        }

        foreach (int index, bookmarks) {
            if (!query.prepare(" insert into "
                               " bookmark(filePath,bookmarkIndex)"
                               " VALUES(:filePath,:bookmarkIndex)")) {
                qInfo() << query.lastError();
                return false;
            }

            query.bindValue(":filePath", filePath);

            query.bindValue(":bookmarkIndex", index);

            if (!query.exec()) {
                qInfo() << query.lastError().text();
                return false;
            }
        }

        transaction.commit();

        return true;
    }

    return false;
}

Database::Database(QObject *parent) : QObject(parent)
{
    const QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    QDir().mkpath(path);

    m_database = QSqlDatabase::addDatabase("QSQLITE");
    m_database.setDatabaseName(QDir(path).filePath("user.db"));
    if (!m_database.open()) {
        qInfo() << m_database.lastError();
    }

    if (m_database.isOpen()) {
        {
            QSqlQuery query(m_database);
            if (!query.exec("PRAGMA synchronous = OFF")) {
                qInfo() << query.lastError();
            }
            if (!query.exec("PRAGMA journal_mode = MEMORY")) {
                qInfo() << query.lastError();
            }
        }

        const QStringList tables = m_database.tables();

        if (!tables.contains("operation")) {
            prepareOperation();
        }

        if (!tables.contains("bookmark")) {
            prepareBookmark();
        }
    } else {
        qInfo() << m_database.lastError();
    }
}
