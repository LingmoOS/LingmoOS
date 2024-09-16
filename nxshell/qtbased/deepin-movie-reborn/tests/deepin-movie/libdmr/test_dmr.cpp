// Copyright (C) 2020 ~ 2021, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QtTest>
#include <QTest>
#include <QTestEventList>
#include <QDebug>
#include <QTimer>
#include <QAbstractButton>
#include <DSettingsDialog>
#include <dwidgetstype.h>
#include <QWidget>

#include <unistd.h>
#include <gtest/gtest.h>

#include "application.h"
#include "player_widget.h"
#include "player_engine.h"
#include "compositing_manager.h"
#include "movie_configuration.h"

TEST(libdmr, libdmrTest)
{
    using namespace dmr;
    PlayerWidget *player = new PlayerWidget();
    player->engine().changeVolume(120);
#ifndef __aarch64__
    player->play(QUrl::fromLocalFile("/data/source/deepin-movie-reborn/movie/demo.mp4"));
#endif

    auto &mc = MovieConfiguration::get();
    mc.updateUrl(QUrl("movie1"), "sub-delay", -2.5);
    mc.updateUrl(QUrl("movie1"), "sub-delay", 1.5);
    mc.updateUrl(QUrl("movie2"), "sub-delay", 1.0);
    mc.updateUrl(QUrl("movie1"), "volume", 20);

    auto res = mc.queryByUrl(QUrl("movie1"));
    mc.removeUrl(QUrl("movie1"));
    mc.updateUrl(QUrl("movie1"), "volume", 30);
    mc.updateUrl(QUrl("movie2"), "volume", 40);
    res = mc.queryByUrl(QUrl("movie1"));
    mc.clear();

    QTest::qWait(100);
    delete player;
}

TEST(libdmr, utils)
{
    using namespace dmr;

    MainWindow* w = dApp->getMainWindow();

    bool ret = utils::CompareNames("/data/source/deepin-movie-reborn/movie/demo.mp4", "/data/source/deepin-movie-reborn/movie/demo.mp3");
    ret = utils::CompareNames("/data/source/deepin-movie-reborn/movie/demo.mp4", "/data/source/deepin-movie-reborn//movie/demo.mp4");
    //QCOMPARE(ret, true);
    utils::UnInhibitPower(20);
    utils::MoveToCenter(w);
    utils::Time2str(90000);
    utils::ValidateScreenshotPath(QString("/data/source/deepin-movie-reborn"));
    utils::ValidateScreenshotPath(QString("~/uos"));
    utils::MakeRoundedPixmap(QPixmap("/data/source/deepin-movie-reborn/test.jpg"), 10, 10);
    utils::MakeRoundedPixmap(QSize(20, 30), QPixmap("/data/source/deepin-movie-reborn/test.jpg"), 10, 10, 20);
    utils::ShowInFileManager("/data/source/deepin-movie-reborn/movie/demo.mp4");
    utils::ShowInFileManager("/data/source/deepin-movie-reborn/test");  //path is not exist
}

TEST(libdmr, playlistModel)
{
    MainWindow *w = dApp->getMainWindow();
    PlayerEngine *engine =  w->engine();
    engine->playlist().savePlaylist();
    engine->playlist().clearPlaylist();

    QUrl url = QUrl::fromLocalFile("/data/source/deepin-movie-reborn/movie/demo.mp4");
    bool is = false;
    engine->playlist().getMovieInfo(url, &is);
    engine->playlist().getMovieCover(url);
}

TEST(libdmr, compositingManager)
{
    if (CompositingManager::get().composited()) {
        CompositingManager::detectOpenGLEarly();
        CompositingManager::detectPciID();
    }
    bool run = CompositingManager::get().runningOnNvidia();
    qDebug() << __func__ << "isRunningOnNvidia:  " << run ;
    run = CompositingManager::get().runningOnVmwgfx();
    qDebug() << __func__ << "isRunningOnVmwgfx:  " << run ;
    CompositingManager::get().setTestFlag(CompositingManager::get().isTestFlag());
}

TEST(libdmr, movieConfiguration)
{
    MovieConfiguration::get().removeFromListUrl(
                QUrl("/data/source/deepin-movie-reborn/Hachiko.A.Dog's.Story.ass"),
                ConfigKnownKey::ExternalSubs, QString());
}

