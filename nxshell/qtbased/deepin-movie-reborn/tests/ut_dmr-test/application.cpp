// Copyright (C) 2020 ~ 2021, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "application.h"

Application::Application(int &argc, char **argv)
    : DApplication(argc, argv)
{
}

Application::~Application()
{
}

void Application::setMainWindow(Window *window)
{
    if (nullptr != window) {
        m_mainwindow = window;
    }
}

Window * Application::getMainWindow()
{
    if(nullptr == m_mainwindow)
        m_mainwindow = new Window();
    return m_mainwindow;
}
