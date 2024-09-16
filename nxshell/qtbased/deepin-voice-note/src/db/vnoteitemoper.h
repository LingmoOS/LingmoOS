// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VNOTEITEMOPER_H
#define VNOTEITEMOPER_H

#include "common/datatypedef.h"

//记事项表操作
class VNoteItemOper
{
public:
    explicit VNoteItemOper(VNoteItem *note = nullptr);
    //获取所有记事项数据
    VNOTE_ALL_NOTES_MAP *loadAllVNotes();
    //修改名称
    bool modifyNoteTitle(const QString &title);
    //更新数据
    bool updateNote();
    //添加记事项
    VNoteItem *addNote(VNoteItem &note);
    //获取记事项
    VNoteItem *getNote(qint64 folderId, qint32 noteId);
    //获取一个记事本所有记事项
    VNOTE_ITEMS_MAP *getFolderNotes(qint64 folderId);
    //生成默认名称
    QString getDefaultNoteName(qint64 folderId);
    //生成默认语音名称
    QString getDefaultVoiceName() const;
    //删除记事项
    bool deleteNote();
    //更新置顶属性
    bool updateTop(int value);
    //更新folderid
    bool updateFolderId(VNoteItem *data);

protected:
    VNoteItem *m_note {nullptr};
};

#endif // VNOTEITEMOPER_H
