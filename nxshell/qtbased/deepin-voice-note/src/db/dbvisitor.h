// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DBVISITOR_H
#define DBVISITOR_H

#include "common/datatypedef.h"

#include <QSqlQuery>
#include <QScopedPointer>

class DbVisitor
{
public:
    explicit DbVisitor(QSqlDatabase &db, const void *inParam, void *result = nullptr /*out*/);
    virtual ~DbVisitor();
    //sql执行结果数据处理
    virtual bool visitorData();
    //准备sql语句
    virtual bool prepareSqls() = 0;
    //获取对象
    QSqlQuery *sqlQuery();
    //获取所有执行的sql语句
    const QStringList &dbvSqls();

    struct ExtraData {
        union {
            bool flag {false};
            //TODO:
            //    Add expand data here
        } data;
    };
    //扩展，用于执行一些特殊功能
    ExtraData &extraData();

public:
    //记事本表字段
    struct DBFolder {
        enum {
            folder_id = 0,
            category_id,
            folder_name,
            default_icon,
            icon_path,
            folder_state,
            max_noteid,
            create_time,
            modify_time,
            delete_time,
            encrypt,
        };

        static const QStringList folderColumnsName;
    };
    //记事项表字段
    struct DBNote {
        enum {
            note_id = 0,
            folder_id,
            note_type,
            note_title,
            meta_data,
            note_state,
            create_time,
            modify_time,
            delete_time,
            is_top,
            encrypt,
        };

        static const QStringList noteColumnsName;
    };
    //语音记录缓存表字段
    struct DBSafer {
        enum {
            id = 0,
            folder_id,
            note_id,
            path,
            state,
            meta_data,
            create_time,
        };

        static const QStringList saferColumnsName;
    };

protected:
    //Check & replace the "'" in the string.
    void checkSqlStr(QString &sql);
    //sql处理的结果
    union {
        VNOTE_FOLDERS_MAP *folders;
        VNOTE_ALL_NOTES_MAP *notes;
        VNoteFolder *newFolder;
        VNoteItem *newNote;
        SafetyDatas *safetyDatas;
        qint32 *count;
        qint64 *id;
        void *ptr;
    } results;
    //参数，用于生成sql语句
    union {
        const VNoteFolder *newFolder;
        const VNoteItem *newNote;
        const VDataSafer *safer;
        const qint32 *count;
        const qint64 *id;
        const void *ptr;
    } param;

    QScopedPointer<QSqlQuery> m_sqlQuery {nullptr};

    QStringList m_dbvSqls;

    ExtraData m_extraData; //Use defined, default not used.
};

//记事本数据查询
class FolderQryDbVisitor : public DbVisitor
{
public:
    explicit FolderQryDbVisitor(QSqlDatabase &db, const void *inParam, void *result);

    virtual bool visitorData() override;
    virtual bool prepareSqls() override;
};

//记事本最大id查询
class MaxIdFolderDbVisitor : public DbVisitor
{
public:
    explicit MaxIdFolderDbVisitor(QSqlDatabase &db, const void *inParam, void *result);

    virtual bool visitorData() override;
    virtual bool prepareSqls() override;
};

//添加记事本
class AddFolderDbVisitor : public DbVisitor
{
public:
    explicit AddFolderDbVisitor(QSqlDatabase &db, const void *inParam, void *result);

    virtual bool visitorData() override;
    virtual bool prepareSqls() override;
};

//记事本重命名
class RenameFolderDbVisitor : public DbVisitor
{
public:
    explicit RenameFolderDbVisitor(QSqlDatabase &db, const void *inParam, void *result);

    virtual bool prepareSqls() override;
};

//删除记事本
class DelFolderDbVisitor : public DbVisitor
{
public:
    explicit DelFolderDbVisitor(QSqlDatabase &db, const void *inParam, void *result);

    virtual bool prepareSqls() override;
};

//记事项查询
class NoteQryDbVisitor : public DbVisitor
{
public:
    explicit NoteQryDbVisitor(QSqlDatabase &db, const void *inParam, void *result);

    virtual bool visitorData() override;
    virtual bool prepareSqls() override;
};

//添加记事项
class AddNoteDbVisitor : public DbVisitor
{
public:
    explicit AddNoteDbVisitor(QSqlDatabase &db, const void *inParam, void *result);

    virtual bool visitorData() override;
    virtual bool prepareSqls() override;
};

//记事项重命名
class RenameNoteDbVisitor : public DbVisitor
{
public:
    explicit RenameNoteDbVisitor(QSqlDatabase &db, const void *inParam, void *result);

    virtual bool prepareSqls() override;
};

//记事项更新
class UpdateNoteDbVisitor : public DbVisitor
{
public:
    explicit UpdateNoteDbVisitor(QSqlDatabase &db, const void *inParam, void *result);

    virtual bool prepareSqls() override;
};

//更新记事项置顶属性
class UpdateNoteTopDbVisitor : public DbVisitor
{
public:
    explicit UpdateNoteTopDbVisitor(QSqlDatabase &db, const void *inParam, void *result);

    virtual bool prepareSqls() override;
};

//更新笔记所属记事本id
class UpdateNoteFolderIdDbVisitor : public DbVisitor
{
public:
    explicit UpdateNoteFolderIdDbVisitor(QSqlDatabase &db, const void *inParam, void *result);

    virtual bool prepareSqls() override;
};

//记事项删除
class DelNoteDbVisitor : public DbVisitor
{
public:
    explicit DelNoteDbVisitor(QSqlDatabase &db, const void *inParam, void *result);

    virtual bool prepareSqls() override;
};
#endif
