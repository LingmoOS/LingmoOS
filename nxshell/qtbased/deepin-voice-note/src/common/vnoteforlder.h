// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VNOTEFORLDER_H
#define VNOTEFORLDER_H

#include "datatypedef.h"

#include <QtGlobal>
#include <QDateTime>
#include <QPixmap>
struct VNoteFolder {
public:
    VNoteFolder();
    ~VNoteFolder();
    //是否可用
    bool isValid();

    enum {
        INVALID_ID = -1
    };

    enum State {
        Normal,
        Deleted,
    };
    //id
    qint64 id {INVALID_ID};
    //类别，保留字段，暂时未用
    qint32 category {0};
    //记事项个数
    qint64 notesCount {0};
    //图标索引
    qint32 defaultIcon {0};
    //状态
    qint32 folder_state {State::Normal};
    //是否加密
    qint32 encryption {0};
    //记事本名称
    QString name;
    //图标路径
    QString iconPath;
    //创建时间
    QDateTime createTime;
    //修改时间
    QDateTime modifyTime;
    //删除时间
    QDateTime deleteTime;
    //排序编号
    qint32 sortNumber {-1};

    struct {
        //正常图标
        QPixmap icon;
        //置灰图标
        QPixmap grayIcon;
    } UI;
    //获取记事项最大id
    qint32 &maxNoteIdRef();
    //获取记事项个数
    qint32 getNotesCount();
    //获取记事项数据
    VNOTE_ITEMS_MAP *getNotes();

protected:
    bool fIsDataLoaded {false};

    //Current max note id in the folder,auto
    //increament
    qint32 maxNoteId {0};

    VNOTE_ITEMS_MAP *notes {nullptr};

    friend class VNoteFolderOper;
};

#endif // VNOTEFORLDER_H
