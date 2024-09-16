// Copyright (C) 2019 ~ 2019 UnionTech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_loadfolderworker.h"
#include "task/loadfolderworker.h"

UT_LoadFolderWorker::UT_LoadFolderWorker(QObject *parent)
    : QObject(parent)
{
}

void UT_LoadFolderWorker::onFolderLoad(VNOTE_FOLDERS_MAP *folders)
{
    EXPECT_TRUE(nullptr != folders);
    if (folders) {
        delete folders;
        folders = nullptr;
    }
}

TEST_F(UT_LoadFolderWorker, UT_LoadFolderWorker_run_001)
{
    LoadFolderWorker work;
    connect(&work, &LoadFolderWorker::onFoldersLoaded, this, &UT_LoadFolderWorker::onFolderLoad);
    work.run();
}
