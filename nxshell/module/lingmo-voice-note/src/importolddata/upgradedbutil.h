// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UPGRADEDBUTIL_H
#define UPGRADEDBUTIL_H
#include "common/datatypedef.h"

#include <QtCore>

class UpgradeDbUtil
{
public:
    UpgradeDbUtil();

    static const QString UPGRADE_STATE;

    enum State {
        Invalid = 0,
        Loading,
        Processing,
        UpdateDone,
    };
    //设置升级标志
    static void saveUpgradeState(int state);
    //读取升级标志
    static int readUpgradeState();
    //是否需要升级
    static bool needUpdateOldDb(int state);
    //检查升级状态
    static void checkUpdateState(int state);
    //老数据库备份
    static void backUpOldDb();
    //删除语音文件
    static void clearVoices();
    //记事本升级
    static void doFolderUpgrade(VNoteFolder *folder);
    //记事项升级
    static void doFolderNoteUpgrade(qint64 newFolderId, qint64 oldFolderId);
};

#endif // UPGRADEDBUTIL_H
