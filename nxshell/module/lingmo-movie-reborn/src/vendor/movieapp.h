// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MOVIEAPP_H
#define MOVIEAPP_H

#include <QObject>
#include <MprisPlayer>
#include "mainwindow.h"
#include "platform/platform_mainwindow.h"
#include "presenter.h"

using namespace dmr;

class MprisPlayer;
class MovieApp : public QObject
{
public:
    MovieApp(MainWindow* mw, QObject* parent = nullptr);
    MovieApp(Platform_MainWindow* mw, QObject* parent = nullptr);

    void initUI();
    void initConnection();
    void initMpris(const QString &serviceName);
    void show();
public slots:
    void quit();

private:
    MainWindow* _mw = nullptr;
    Platform_MainWindow* _mw_platform = nullptr;
    Presenter* _presenter = nullptr;

};

#endif // MOVIEAPP_H
