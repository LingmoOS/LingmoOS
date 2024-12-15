// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DATATYPEDEF_H
#define DATATYPEDEF_H

#include "common/opsstateinterface.h"

#include <QMap>
#include <QVector>
#include <QReadWriteLock>
#include <QDateTime>

struct VNoteFolder;
struct VNoteItem;
struct VNoteBlock;
struct VNOTE_ITEMS_MAP;

typedef QMap<qint64, VNoteFolder *> VNOTE_FOLDERS_DATA_MAP;
typedef QMap<qint64, VNoteItem *> VNOTE_ITEMS_DATA_MAP;
typedef QMap<qint64, VNOTE_ITEMS_MAP *> VNOTE_ALL_NOTES_DATA_MAP;
typedef QVector<VNoteBlock *> VNOTE_DATA_VECTOR;

//记事本数据
struct VNOTE_FOLDERS_MAP {
    ~VNOTE_FOLDERS_MAP();

    VNOTE_FOLDERS_DATA_MAP folders;
    QReadWriteLock lock;

    //TODO:
    //    Release data if true when destructor is called.
    //Only can be set TRUE in data manager
    bool autoRelease {false};
};
//单个记事本记事项数据
struct VNOTE_ITEMS_MAP {
    ~VNOTE_ITEMS_MAP();

    VNOTE_ITEMS_DATA_MAP folderNotes;
    QReadWriteLock lock;

    //TODO:
    //    Release data if true when destructor is called.
    //Only can be set TRUE in data manager
    bool autoRelease {false};
};

//所有记事本数据
struct VNOTE_ALL_NOTES_MAP {
    ~VNOTE_ALL_NOTES_MAP();

    VNOTE_ALL_NOTES_DATA_MAP notes;
    QReadWriteLock lock;

    //TODO:
    //    Release data if true when destructor is called.
    //Only can be set TRUE in data manager
    bool autoRelease {false};
};

struct VNOTE_DATAS {
    ~VNOTE_DATAS();

    const VNOTE_DATA_VECTOR &dataConstRef();

protected:
    //新建数据块
    VNoteBlock *newBlock(int type);
    //添加数据块
    void addBlock(VNoteBlock *block);
    //指定数据块后面插入数据块
    void addBlock(VNoteBlock *before, VNoteBlock *block);
    //删除数据块
    void delBlock(VNoteBlock *block);
    //向内存缓存中添加数据块
    void classifyAddBlk(VNoteBlock *block);
    //从内存缓存中删除数据块
    void classifyDelBlk(VNoteBlock *block);
    //Ordered data set
    VNOTE_DATA_VECTOR datas;

    //Classify data
    VNOTE_DATA_VECTOR textBlocks;
    VNOTE_DATA_VECTOR voiceBlocks;

    friend struct VNoteItem;
    friend class MetaDataParser;
    friend class ExportNoteWorker;
    friend class FileCleanupWorker;
};

//新建录音时的缓存语音记录
struct VDataSafer {
    enum {
        INVALID_ID = -1
    };

    enum SaferType {
        Safe,
        Unsafe,
        ExceptionSafer,
    };

    bool isValid() const;

    //设置数据类型
    void setSaferType(SaferType type);

    SaferType saferType {ExceptionSafer};

    qint32 id {INVALID_ID};
    qint64 folder_id {INVALID_ID};
    qint32 note_id {INVALID_ID};
    qint32 state {0};
    QString path;
    QString meta_data;
    QDateTime createTime;

    friend QDebug &operator<<(QDebug &out, const VDataSafer &safer);
};

typedef QVector<VDataSafer> SafetyDatas;

enum IconsType {
    DefaultIcon = 0x0,
    DefaultGrayIcon,
    MaxIconsType
};

#endif // DATATYPEDEF_H
