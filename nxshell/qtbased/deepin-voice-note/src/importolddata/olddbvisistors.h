// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OLDDBVISISTORS_H
#define OLDDBVISISTORS_H

#include "db/dbvisitor.h"

//加载老记事本数据
class OldFolderQryDbVisitor : public DbVisitor
{
public:
    explicit OldFolderQryDbVisitor(QSqlDatabase &db, const void *inParam, void *result);
    //处理结果
    virtual bool visitorData() override;
    //生成sql
    virtual bool prepareSqls() override;
};

//加载老数据库记事项数据
class OldNoteQryDbVisitor : public DbVisitor
{
public:
    explicit OldNoteQryDbVisitor(QSqlDatabase &db, const void *inParam, void *result);

    enum OldNoteType {
        Voice = 0,
        Text,
    };
    //处理结果
    virtual bool visitorData() override;
    //生成sql
    virtual bool prepareSqls() override;
};

#endif // OLDDBVISISTORS_H
