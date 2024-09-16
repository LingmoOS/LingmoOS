// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vnmainwnddelayinittask.h"
#include "views/vnotemainwindow.h"

/**
 * @brief VNMainWndDelayInitTask::VNMainWndDelayInitTask
 * @param pMainWnd 主窗口
 * @param parent
 */
VNMainWndDelayInitTask::VNMainWndDelayInitTask(VNoteMainWindow *pMainWnd, QObject *parent)
    : VNTask(parent)
    , m_pMainWnd(pMainWnd)
{
}

/**
 * @brief VNMainWndDelayInitTask::run
 */
void VNMainWndDelayInitTask::run()
{
    if (nullptr != m_pMainWnd) {
        //Delay initialize work
        m_pMainWnd->initDelayWork();
    }
}
