// Copyright (C) 2020 ~ 2021, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QtTest>
#include <QTest>
#include <QTestEventList>
#include <QTimer>
#include "application.h"
#include <unistd.h>
#include "src/vendor/movieapp.h"
#include "movie_configuration.h"

using namespace dmr;

#ifndef __mips__
TEST(MovieApp, testMprisapp)
{
    Platform_MainWindow* w = new Platform_MainWindow;
    auto &mc = MovieConfiguration::get();
    MovieConfiguration::get().init();
    PlayerEngine *engine =  w->engine();
    Platform_ToolboxProxy *toolboxProxy = w->toolbox();

    w->resize(850, 600);
    utils::MoveToCenter(w);
    w->show();

//    QTest::qWait(500);
    Settings::get().settings()->setOption("base.play.emptylist", true); //退出时清空播放列表

    MovieApp *movieapp = new MovieApp(w);
    movieapp->initMpris("movie");
    movieapp->show();

//    QTest::qWait(300);
    movieapp->quit();
    movieapp->deleteLater();
    movieapp = nullptr;

    w->deleteLater();
    w = nullptr;

//    QTimer::singleShot(1000,[=]{movieapp->show();});
//    QTest::qWait(300);
//    w->testMprisapp();
}
#endif
