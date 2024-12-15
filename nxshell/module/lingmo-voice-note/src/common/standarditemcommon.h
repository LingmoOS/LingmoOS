// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FOLDERTREECOMMON_H
#define FOLDERTREECOMMON_H

#include <QObject>
#include <QStandardItemModel>

//用于treeview,listview的数据项
class StandardItemCommon : public QObject
{
    Q_OBJECT
public:
    enum StandardItemType {
        Invalid = 0,
        NOTEPADROOT, //记事本一级目录
        NOTEPADITEM, //记事本项
        NOTEITEM //笔记项
    };
    Q_ENUM(StandardItemType)
    explicit StandardItemCommon();
    //生成数据项
    static QStandardItem *createStandardItem(void *data, StandardItemType type);
    //获取数据类型
    static StandardItemType getStandardItemType(const QModelIndex &index);
    //获取数据内容
    static void *getStandardItemData(const QModelIndex &index);
};

#endif // FOLDERTREECOMMON_H
