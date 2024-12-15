// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "capplication.h"

CApplication *CApplication::cameraCore = nullptr;

CApplication::CApplication(int &argc, char **argv)
    : QtSingleApplication(argc, argv)
{
    m_mainwindow = nullptr;
#ifndef TABLE_ENVIRONMENT
    m_bpanel = false;
#else
    m_bpanel = true;
#endif

    if (cameraCore == nullptr)
        cameraCore = this;

    dApplication()->installEventFilter(this);
    dApplication()->setQuitOnLastWindowClosed(true);

    Dtk::Core::DVtableHook::overrideVfptrFun(dApplication(), &DApplication::handleQuitAction, CamApp, &CApplication::QuitAction);

}

CApplication *CApplication::CamApplication()
{
    return CApplication::cameraCore;
}

void CApplication::setMainWindow(CMainWindow *window)
{
    if (!m_mainwindow)
        m_mainwindow = window;
}

bool CApplication::isPanelEnvironment()
{
    return m_bpanel;
}

CMainWindow *CApplication::getMainWindow()
{
    return m_mainwindow;
}

void CApplication::QuitAction()
{
    emit CamApp->popupConfirmDialog();
}

