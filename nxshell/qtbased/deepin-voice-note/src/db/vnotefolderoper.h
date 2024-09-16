// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VNOTEFOLDEROPER_H
#define VNOTEFOLDEROPER_H

#include "common/datatypedef.h"

#include <QPixmap>

//记事本表操作
class VNoteFolderOper
{
public:
    explicit VNoteFolderOper(VNoteFolder *folder = nullptr);
    //所有记事项是否加载
    inline bool isNoteItemLoaded();
    //获取数据
    VNOTE_FOLDERS_MAP *loadVNoteFolders();
    //添加记事本
    VNoteFolder *addFolder(VNoteFolder &folder);
    //获取记事本数据
    VNoteFolder *getFolder(qint64 folderId);
    //获取记事本个数
    qint32 getFoldersCount();
    //获取一个记事本中记事项个数
    qint32 getNotesCount(qint64 folderId);
    //获取所有记事本中记事项个数之和
    qint32 getNotesCount();
    //生成默认记事本名称
    QString getDefaultFolderName();
    //生成默认图标索引
    qint32 getDefaultIcon();
    //根据图标索引获取图标
    QPixmap getDefaultIcon(qint32 index, IconsType type);
    //删除一个记事本
    bool deleteVNoteFolder(qint64 folderId);
    bool deleteVNoteFolder(VNoteFolder *folder);
    //重命名记事本
    bool renameVNoteFolder(const QString &folderName);

protected:
    VNoteFolder *m_folder {nullptr};
};

#endif // VNOTEFOLDEROPER_H
