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
//    m_mainwindow->close();
//    delete m_mainwindow;
//    m_movieapp->quit();
}

void Application::setMainWindow(Platform_MainWindow *window)
{
    if (nullptr != window) {
        m_mainwindow = window;
    }
}
void Application::setMainWindowWayland(Platform_MainWindow *window)
{
    if (nullptr != window) {
        m_mainwindow_wayland = window;
    }
}

Platform_MainWindow * Application::getMainWindow()
{
    if(nullptr == m_mainwindow)
        m_mainwindow = new Platform_MainWindow();
    return m_mainwindow;
}
Platform_MainWindow * Application::getMainWindowWayland()
{
    if(nullptr == m_mainwindow_wayland)
        qputenv("QT_WAYLAND_SHELL_INTEGRATION", "kwayland-shell");
        //qputenv("_d_disableDBusFileDialog", "true");
        setenv("PULSE_PROP_media.role", "video", 1);
        QSurfaceFormat format;
        format.setRenderableType(QSurfaceFormat::OpenGLES);
        format.setDefaultFormat(format);
        utils::set_wayland(true);
        bool iswayland = utils::first_check_wayland_env();
        m_mainwindow_wayland = new Platform_MainWindow();
    return m_mainwindow_wayland;
}

Presenter * Application::getPresenter()
{
    if(m_presenter == nullptr)
    {
        m_presenter = new Presenter(getMainWindowWayland());
    }
    return m_presenter;
}
MovieApp * Application::initMovieApp(Platform_MainWindow *mw)
{
    if(m_movieapp == nullptr)
    {
        m_movieapp = new MovieApp(mw,this);
    }
    return m_movieapp;
}
