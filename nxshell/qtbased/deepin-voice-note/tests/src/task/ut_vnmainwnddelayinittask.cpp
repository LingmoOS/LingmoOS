// Copyright (C) 2019 ~ 2019 UnionTech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_vnmainwnddelayinittask.h"
#include "vnmainwnddelayinittask.h"
#include "views/vnotemainwindow.h"

UT_VNMainWndDelayInitTask::UT_VNMainWndDelayInitTask()
{
}

TEST_F(UT_VNMainWndDelayInitTask, UT_VNMainWndDelayInitTask_run_001)
{
    VNMainWndDelayInitTask *work = new VNMainWndDelayInitTask(nullptr);
    work->run();

    delete work;
}
