// Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef APPLICATION_H_
#define APPLICATION_H_

#include <DApplication>
#include "mainwindow.h"
#include "url_dialog.h"
#include "presenter.h"
#include "movieapp.h"

using namespace dmr;

class Application;

#if defined(dApp)
#undef dApp
#endif
#define dApp (static_cast<Application *>(QCoreApplication::instance()))

DWIDGET_USE_NAMESPACE


class Application : public DApplication
{
    Q_OBJECT

public:
    Application(int &argc, char **argv);
    ~Application();

    void setMainWindow(Platform_MainWindow *window);
    void setMainWindowWayland(Platform_MainWindow *window);
    Platform_MainWindow *getMainWindow();
    Platform_MainWindow *getMainWindowWayland();
    Presenter * getPresenter();
    MovieApp * initMovieApp(Platform_MainWindow *mw);

private:
    Platform_MainWindow *m_mainwindow {nullptr};
    Platform_MainWindow *m_mainwindow_wayland {nullptr};
    Presenter *m_presenter {nullptr};
    MovieApp *m_movieapp {nullptr};
};

#endif  // APPLICATION_H_
