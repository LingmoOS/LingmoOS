/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "uNoteDbusServer.h"

UNoteDbusServer::UNoteDbusServer()
{

}

QVariantMap UNoteDbusServer::keywordMatch(QStringList keyList)
{
    QVariantMap keyMap;

    qDebug() << keyList.count();
    QString sql;

    if(keyList.count() >= 1) {
        sql = QStringLiteral("SELECT id, md_content, full_title FROM active_notes WHERE md_content LIKE '%%1%'").arg(keyList.at(0));
        for(int i = 1; i < keyList.count(); i++) {
            QString tmpSql = QStringLiteral(" AND md_content LIKE '%%1%'").arg(keyList.at(i));
            sql.append(tmpSql);
        }
    }

    qDebug() << sql <<QSqlDatabase::drivers();

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(QDir::homePath() + "/.config/lingmo-note/kyNotes.db");
    if(!db.open()){
        //如果数据库打开失败
        qWarning() << "error!" << db.lastError().text();
    }

    QSqlQuery query;
    if(query.exec(sql)) {
        while(query.next()){
            qDebug() << "Dat:::" << query.value(0).toString() << query.value(1).toString() << query.value(2).toString();
            QVariantList content;
            content << query.value(1).toString().mid(query.value(1).toString().indexOf(keyList.at(0)), 20) << query.value(2).toString();
            qDebug() << "Dat###" << query.value(1).toString().mid(query.value(1).toString().indexOf(keyList.at(0)), 20);
            QVariant var = QVariant::fromValue<QVariantList>(content);
            keyMap.insert(query.value(0).toString(), var);
        }
    }
    qDebug() << keyMap;
    return keyMap;
}

QMap<QString, QVariant> UNoteDbusServer::loadSqlDB()
{
    qDebug() << QSqlDatabase::drivers();
    QVariantMap keyMap;
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");

    db.setDatabaseName(QDir::homePath() + "/.config/lingmo-note/kyNotes.db");
    if(!db.open()){
        //如果数据库打开失败
        qWarning() << "error!" << db.lastError();
    }

    QSqlQuery query;
    if(query.exec("SELECT id, md_content FROM active_notes")) {
        while(query.next()){
            keyMap.insert(query.value(0).toString(), query.value(1));
            qDebug() << query.value(0).toString() << query.value(1).toString();
        }
    }

    return keyMap;
}

//增加一个读取所有的接口
bool UNoteDbusServer::exportAllNotes(QList<Note> &data)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(QDir::homePath() + "/.config/lingmo-note/kyNotes.db");
    if(!db.open()){
        //如果数据库打开失败
        qWarning() << "error!" << db.lastError().text();
        return false;
    }
    QString sql = QString("SELECT id, modification_date, md_content FROM active_notes");
    QSqlQuery query;
    if(query.exec(sql)) {
        while(query.next()){
            int id = query.value(0).toInt();
            time_t modTime = query.value(1).toInt();
            QString content = query.value(2).toString();
            qDebug() << id << modTime << content;
            Note note;
            note.id = id;
            note.editTime = modTime;
            note.noteText = content;
            data.push_back(note);
        }
    }
    return true;
}
