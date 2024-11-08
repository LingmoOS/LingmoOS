/*
* Copyright (C) 2021 Tianjin LINGMO Information Technology Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/

#ifndef UNOTEDBUSSERVER_H
#define UNOTEDBUSSERVER_H

#include <QObject>
#include <QMap>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QDir>

struct Note{
    int id;
    time_t editTime;
    QString noteText;
};

class UNoteDbusServer : public QObject
{
    Q_OBJECT
    //定义Interface名称
    Q_CLASSINFO("D-Bus Interface", "org.lingmo.note.interface")

public:
    explicit UNoteDbusServer();

public slots:
    QVariantMap keywordMatch(QStringList keyList);
public:
     bool exportAllNotes(QList<Note>&);
private:
    QMap<QString, QVariant> loadSqlDB();

private:
    QVariantMap keywordsMatched;
};

#endif // UNOTEDBUSSERVER_H
