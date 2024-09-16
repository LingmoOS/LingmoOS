// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VNOTEDBMANAGER_H
#define VNOTEDBMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QMutex>

class DbVisitor;

class VNoteDbManager : public QObject
{
    Q_OBJECT
public:
    explicit VNoteDbManager(bool fOldDb = false, QObject *parent = nullptr);
    virtual ~VNoteDbManager();

    static VNoteDbManager *instance();

    static constexpr char const *DBVERSION = "1.0";

    static constexpr char const *FOLDER_TABLE_NAME = "vnote_folder_tbl";
    static constexpr char const *FOLDER_KEY = "folder_id";
    static constexpr char const *NOTES_TABLE_NAME = "vnote_items_tbl";
    static constexpr char const *NOTES_KEY = "note_id";
    static constexpr char const *CATEGORY_TABLE_NAME = "vnote_category_tbl";

    //icon_path: Not used, maybe used in future
    //expand_fields are place holder, will be used in future
    static constexpr char const *CREATETABLE_FMT = "\
         CREATE TABLE IF NOT EXISTS vnote_folder_tbl(\
            folder_id INTEGER PRIMARY KEY AUTOINCREMENT , \
            category_id INT DEFAULT 0, \
            folder_name TEXT NOT NULL, \
            default_icon INT DEFAULT 0, \
            icon_path TEXT,\
            folder_state INT DEFAULT 0, \
            max_noteid INT DEFAULT 0, \
            create_time DATETIME NOT NULL DEFAULT (STRFTIME ('%Y-%m-%d %H:%M:%f','now','localtime')), \
            modify_time DATETEXT NOT NULL DEFAULT (STRFTIME ('%Y-%m-%d %H:%M:%f','now','localtime')), \
            delete_time DATETEXT DEFAULT (STRFTIME ('%Y-%m-%d %H:%M:%f','now','localtime')), \
            expand_filed1 INT, \
            expand_filed2 INT, \
            expand_filed3 INT, \
            expand_filed4 TEXT, \
            expand_filed5 TEXT, \
            expand_filed6 TEXT \
         ); \
         CREATE TABLE IF NOT EXISTS vnote_items_tbl(\
            note_id INTEGER PRIMARY KEY AUTOINCREMENT, \
            folder_id INTEGER, \
            note_type INT NOT NULL DEFAULT 0, \
            note_title TEXT NOT NULL, \
            meta_data TEXT, \
            note_state INT DEFAULT 0, \
            create_time DATETIME NOT NULL DEFAULT (STRFTIME ('%Y-%m-%d %H:%M:%f', 'now', 'localtime')), \
            modify_time DATETEXT NOT NULL DEFAULT (STRFTIME ('%Y-%m-%d %H:%M:%f', 'now', 'localtime')), \
            delete_time DATETEXT DEFAULT (STRFTIME ('%Y-%m-%d %H:%M:%f','now','localtime')), \
            expand_filed1 INT, \
            expand_filed2 INT, \
            expand_filed3 INT, \
            expand_filed4 TEXT, \
            expand_filed5 TEXT, \
            expand_filed6 TEXT \
         ); \
         CREATE TABLE IF NOT EXISTS vnote_category_tbl(\
            id        INT DEFAULT 0, \
            name      TEXT NOT NULL, \
            icon      INT DEFAULT 0, \
            state     INT DEFAULT 0, \
            max_id    INT DEFAULT 0, \
            meta_data TEXT, \
            create_time DATETIME NOT NULL DEFAULT (STRFTIME ('%Y-%m-%d %H:%M:%f','now','localtime')), \
            modify_time DATETEXT NOT NULL DEFAULT (STRFTIME ('%Y-%m-%d %H:%M:%f','now','localtime')), \
            delete_time DATETEXT DEFAULT (STRFTIME ('%Y-%m-%d %H:%M:%f','now','localtime')), \
            expand_filed1 INT, \
            expand_filed2 INT, \
            expand_filed3 INT, \
            expand_filed4 TEXT, \
            expand_filed5 TEXT, \
            expand_filed6 TEXT \
         );";

    enum DB_TABLE {
        VNOTE_FOLDER_TBL,
        VNOTE_ITEM_TBL,
        VNOTE_MAX_TBL
    };

    QSqlDatabase &getVNoteDb();
    //执行插入操作
    bool insertData(DbVisitor *visitor /*in/out*/);
    //执行更新操作
    bool updateData(DbVisitor *visitor /*in/out*/);
    //执行查询操作
    bool queryData(DbVisitor *visitor /*in/out*/);
    //执行删除操作
    bool deleteData(DbVisitor *visitor /*in/out*/);
    //是否存在老记事本数据库
    static bool hasOldDataBase();
signals:

public slots:

protected:
    //初始化数据库
    int initVNoteDb(bool fOldDB = false);
    //创建数据表
    void createTablesIfNeed();

protected:
    QSqlDatabase m_vnoteDB;
    //    QScopedPointer<QSqlQuery> m_sqlQuery;

    QMutex m_dbLock;
    bool m_isDbInitOK {false};

    static VNoteDbManager *_instance;
};

#endif // VNOTEDBMANAGER_H
