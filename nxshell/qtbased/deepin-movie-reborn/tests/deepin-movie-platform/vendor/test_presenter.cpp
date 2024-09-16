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
#include "src/vendor/presenter.h"
#include "movie_configuration.h"

using namespace dmr;

TEST(Presenter, slotplay)
{
//    MainWindow *w = dApp->getMainWindowWayland();
    MainWindow *w = new MainWindow;
    MovieConfiguration::get().init();
    Presenter *presenter = new Presenter(w);

    presenter->slotopenUrlRequested(QUrl::fromLocalFile("/data/source/deepin-movie-reborn/movie/bensound-sunny.mp3"));
    presenter->slotplay();
    presenter->slotpause();
    presenter->slotplaynext();
    presenter->slotplayprev();
    presenter->slotvolumeRequested(1.5);
    presenter->slotstateChanged();
    presenter->slotvolumeChanged();


    presenter->slotseek(qlonglong(200));

    presenter->slotloopStatusRequested(Mpris::LoopStatus::None);

    presenter->slotloopStatusRequested(Mpris::LoopStatus::Track);

    presenter->slotloopStatusRequested(Mpris::LoopStatus::Playlist);

    presenter->slotloopStatusRequested(Mpris::LoopStatus::InvalidLoopStatus);

    presenter->slotplayModeChanged(PlaylistModel::PlayMode::OrderPlay);

    presenter->slotplayModeChanged(PlaylistModel::PlayMode::SingleLoop);

    presenter->slotplayModeChanged(PlaylistModel::PlayMode::ListLoop);

    presenter->slotplayModeChanged(PlaylistModel::PlayMode::ShufflePlay);

    presenter->slotstop();

    presenter->deleteLater();
    presenter = nullptr;

//    w->close();
//    w->deleteLater();
//    w = nullptr;
}

//TEST(Presenter, slotloopStatusRequested)
//{
////    Presenter *presenter = dApp->getPresenter();
//    MainWindow w;
////    auto &mc = MovieConfiguration::get();
//    MovieConfiguration::get().init();
////    PlayerEngine *engine =  w->engine();
//    Presenter *presenter = new Presenter(&w);

////    QTest::qWait(500);
//    presenter->slotloopStatusRequested(Mpris::LoopStatus::None);
////    QTest::qWait(500);
//    presenter->slotloopStatusRequested(Mpris::LoopStatus::Track);
////    QTest::qWait(500);
//    presenter->slotloopStatusRequested(Mpris::LoopStatus::Playlist);
////    QTest::qWait(500);
//    presenter->slotloopStatusRequested(Mpris::LoopStatus::InvalidLoopStatus);
//}

//TEST(Presenter, slotplayModeChanged)
//{
////    Presenter *presenter = dApp->getPresenter();
//    MainWindow w;
////    auto &mc = MovieConfiguration::get();
//    MovieConfiguration::get().init();
////    PlayerEngine *engine =  w->engine();
//    Presenter *presenter = new Presenter(&w);

////    QTest::qWait(500);
//    presenter->slotplayModeChanged(PlaylistModel::PlayMode::OrderPlay);
////    QTest::qWait(500);
//    presenter->slotplayModeChanged(PlaylistModel::PlayMode::SingleLoop);
////    QTest::qWait(500);
//    presenter->slotplayModeChanged(PlaylistModel::PlayMode::ListLoop);
////    QTest::qWait(500);
//    presenter->slotplayModeChanged(PlaylistModel::PlayMode::ShufflePlay);
//}
