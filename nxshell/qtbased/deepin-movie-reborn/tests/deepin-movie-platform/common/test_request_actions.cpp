// Copyright (C) 2020 ~ 2021, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "src/widgets/platform/platform_toolbox_proxy.h"
#include <gtest/gtest.h>
#include "src/common/mainwindow.h"
#include "src/common/platform/platform_mainwindow.h"
#include "src/libdmr/player_engine.h"
#include "application.h"
#include <QtTest>
#include <QTest>
#include <QTestEventList>
#include "src/common/actions.h"
#include "src/widgets/burst_screenshots_dialog.h"
#include <unistd.h>
#include <QDebug>
#include <QTimer>
#include <QAbstractButton>
#include "dmr_settings.h"
#include "movieinfo_dialog.h"
#include <DSettingsDialog>

TEST(requestAction, onlineSub)
{
    Platform_MainWindow* w = dApp->getMainWindow();
    PlayerEngine* engine =  w->engine();
    QUrl url(QUrl::fromLocalFile("/data/source/deepin-movie-reborn/movie/天空之眼 高清1080P.mp4"));

    w->show();

    QTest::qWait(500);
    if(engine->addPlayFile(url))
    {
        engine->playByName(url);
    }

    QTest::qWait(1000);
    w->requestAction(ActionFactory::ActionKind::MatchOnlineSubtitle);
    QTest::qWait(200);
    w->requestAction(ActionFactory::ActionKind::HideSubtitle);
    QTest::qWait(200);
    w->requestAction(ActionFactory::ActionKind::HideSubtitle);

    QTestEventList testEventList;
    testEventList.addKeyClick(Qt::Key_Left, Qt::ShiftModifier,100); //sub delay
//    QTest::qWait(200);
    w->requestAction(ActionFactory::ActionKind::SubDelay);
    testEventList.addKeyClick(Qt::Key_Left, Qt::ShiftModifier,100);
    testEventList.addKeyClick(Qt::Key_Right, Qt::ShiftModifier,100);    //sub advance
    testEventList.addKeyClick(Qt::Key_Right, Qt::ShiftModifier,100);
    testEventList.addKeyClick(Qt::Key_Right, Qt::ShiftModifier,100);
    testEventList.simulate(w);
}

//will cause the program abort of arm platform
/*TEST(requestAction,windowAbove)
{
    MainWindow* w = dApp->getMainWindow();

    QTest::qWait(500);
    w->requestAction(ActionFactory::ActionKind::WindowAbove);   //置顶
    QTest::qWait(500);
    w->requestAction(ActionFactory::ActionKind::WindowAbove);   //取消置顶
}*/

TEST(requestAction, sound)
{
    Platform_MainWindow* w = dApp->getMainWindow();

    QTest::qWait(200);
    w->requestAction(ActionFactory::ActionKind::Stereo);
    QTest::qWait(200);
    w->requestAction(ActionFactory::ActionKind::LeftChannel);
    QTest::qWait(200);
    w->requestAction(ActionFactory::ActionKind::RightChannel);
}

TEST(requestAction, playMode)
{
    Platform_MainWindow* w = dApp->getMainWindow();
    PlayerEngine *engine =  w->engine();
    Platform_ToolboxProxy *toolboxProxy = w->toolbox();
    DButtonBoxButton *playBtn = toolboxProxy->playBtn();
    DButtonBoxButton *nextBtn = toolboxProxy->nextBtn();
    DButtonBoxButton *prevBtn = toolboxProxy->prevBtn();

    QList<QUrl> listPlayFiles;

    listPlayFiles << QUrl::fromLocalFile("/data/source/deepin-movie-reborn/movie/demo.mp4")\
                  << QUrl::fromLocalFile("/data/source/deepin-movie-reborn/movie/bensound-sunny.mp3");

    const QList<QUrl> &valids = engine->addPlayFiles(listPlayFiles);
    qDebug() << __func__ << valids;
    w->show();

    QTest::qWait(100);
    w->requestAction(ActionFactory::ActionKind::OrderPlay);
    QTest::qWait(100);
//    w->requestAction(ActionFactory::ActionKind::GotoPlaylistNext);
    QTest::mouseMove(nextBtn, QPoint(), 200);
    QTest::mouseClick(nextBtn, Qt::LeftButton, Qt::NoModifier, QPoint(), 600);
    QTest::qWait(500);
//    w->requestAction(ActionFactory::ActionKind::GotoPlaylistPrev);
    QTest::mouseMove(prevBtn, QPoint(), 200);
    QTest::mouseClick(prevBtn, Qt::LeftButton, Qt::NoModifier, QPoint(), 600);

    QTest::qWait(300);
    w->requestAction(ActionFactory::ActionKind::ShufflePlay);
    QTest::qWait(100);
//    w->requestAction(ActionFactory::ActionKind::GotoPlaylistNext);
    QTest::mouseMove(nextBtn, QPoint(), 200);
    QTest::mouseClick(nextBtn, Qt::LeftButton, Qt::NoModifier, QPoint(), 600);
    QTest::qWait(500);
//    w->requestAction(ActionFactory::ActionKind::GotoPlaylistPrev);
    QTest::mouseMove(prevBtn, QPoint(), 200);
    QTest::mouseClick(prevBtn, Qt::LeftButton, Qt::NoModifier, QPoint(), 600);


    QTest::qWait(300);
    w->requestAction(ActionFactory::ActionKind::SinglePlay);
    QTest::qWait(100);
//    w->requestAction(ActionFactory::ActionKind::GotoPlaylistNext);
    QTest::mouseMove(nextBtn, QPoint(), 200);
    QTest::mouseClick(nextBtn, Qt::LeftButton, Qt::NoModifier, QPoint(), 600);
    QTest::qWait(500);
//    w->requestAction(ActionFactory::ActionKind::GotoPlaylistPrev);
    QTest::mouseMove(prevBtn, QPoint(), 200);
    QTest::mouseClick(prevBtn, Qt::LeftButton, Qt::NoModifier, QPoint(), 600);

    QTest::qWait(300);
    w->requestAction(ActionFactory::ActionKind::SingleLoop);
    QTest::qWait(100);
//    w->requestAction(ActionFactory::ActionKind::GotoPlaylistNext);
    QTest::mouseMove(nextBtn, QPoint(), 200);
    QTest::mouseClick(nextBtn, Qt::LeftButton, Qt::NoModifier, QPoint(), 600);
    QTest::qWait(500);
//    w->requestAction(ActionFactory::ActionKind::GotoPlaylistPrev);
    QTest::mouseMove(prevBtn, QPoint(), 200);
    QTest::mouseClick(prevBtn, Qt::LeftButton, Qt::NoModifier, QPoint(), 600);

    QTest::qWait(300);
    w->requestAction(ActionFactory::ActionKind::ListLoop);
    QTest::qWait(100);
//    w->requestAction(ActionFactory::ActionKind::GotoPlaylistNext);
    QTest::mouseMove(nextBtn, QPoint(), 200);
    QTest::mouseClick(nextBtn, Qt::LeftButton, Qt::NoModifier, QPoint(), 600);
    QTest::qWait(500);
//    w->requestAction(ActionFactory::ActionKind::GotoPlaylistPrev);
    QTest::mouseMove(prevBtn, QPoint(), 200);
    QTest::mouseClick(prevBtn, Qt::LeftButton, Qt::NoModifier, QPoint(), 600);
}

TEST(requestAction, playSpeed)
{
    Platform_MainWindow* w = dApp->getMainWindow();

    QTest::qWait(200);
    w->requestAction(ActionFactory::ActionKind::ZeroPointFiveTimes);
    QTest::qWait(200);
    w->requestAction(ActionFactory::ActionKind::OnePointTwoTimes);
    QTest::qWait(200);
    w->requestAction(ActionFactory::ActionKind::OnePointFiveTimes);
    QTest::qWait(200);
    w->requestAction(ActionFactory::ActionKind::Double);
    QTest::qWait(200);
    w->requestAction(ActionFactory::ActionKind::OneTimes);
//    QTest::qWait(500);
//    w->requestAction(ActionFactory::ActionKind::SeekAbsolute);
}

TEST(requestAction, frame)
{
    Platform_MainWindow* w = dApp->getMainWindow();

    QTest::qWait(200);
    w->requestAction(ActionFactory::ActionKind::Ratio4x3Frame);
    QTest::qWait(200);
    w->requestAction(ActionFactory::ActionKind::Ratio16x9Frame);
    QTest::qWait(200);
    w->requestAction(ActionFactory::ActionKind::Ratio16x10Frame);
    QTest::qWait(200);
    w->requestAction(ActionFactory::ActionKind::Ratio185x1Frame);
    QTest::qWait(200);
    w->requestAction(ActionFactory::ActionKind::Ratio235x1Frame);
    QTest::qWait(200);
    w->requestAction(ActionFactory::ActionKind::DefaultFrame);

    QTest::qWait(200);
    w->requestAction(ActionFactory::ActionKind::ClockwiseFrame);
    QTest::qWait(200);
    w->requestAction(ActionFactory::ActionKind::CounterclockwiseFrame);

    QTest::qWait(200);
    w->requestAction(ActionFactory::ActionKind::NextFrame);
    QTest::qWait(200);
    w->requestAction(ActionFactory::ActionKind::PreviousFrame);
}

TEST(requestAction,mute)
{
    Platform_MainWindow* w = dApp->getMainWindow();

    QTest::qWait(200);
    w->requestAction(ActionFactory::ActionKind::ToggleMute);
    QTest::qWait(200);
    w->requestAction(ActionFactory::ActionKind::ToggleMute);
}

TEST(requestAction, subtitle)
{
    Platform_MainWindow* w = dApp->getMainWindow();
    PlayerEngine *engine =  w->engine();
    w->show();

    QTest::qWait(300);
    w->requestAction(ActionFactory::ActionKind::LoadSubtitle);
    QTest::qWait(100);
    engine->loadSubtitle(QFileInfo(QString("/data/source/deepin-movie-reborn/movie/Hachiko.A.Dog's.Story.ass")));
    QTest::qWait(100);
    w->requestAction(ActionFactory::ActionKind::HideSubtitle);
    QTest::qWait(100);
}

TEST(requestAction,goToScreenshotSolder)
{
    Platform_MainWindow* w = dApp->getMainWindow();

    QTest::qWait(200);
    w->requestAction(ActionFactory::ActionKind::GoToScreenshotSolder);
    EXPECT_TRUE(true);
}

TEST(requestAction,openFiles)
{
    Platform_MainWindow *w = dApp->getMainWindow();

    QTest::qWait(300);
    w->requestAction(ActionFactory::ActionKind::OpenFileList);
    QTest::qWait(100);

//    w->requestAction(ActionFactory::ActionKind::OpenDirectory);
//    QTest::qWait(100);

    w->requestAction(ActionFactory::ActionKind::PlaylistOpenItemInFM);
    utils::ShowInFileManager(QString("/usr/share/music/bensound-sunny.mp3"));
    QTest::qWait(100);
}

TEST(requestAction, settings)
{
    Platform_MainWindow* w = dApp->getMainWindow();

    Settings::get().settings()->setOption("base.screenshot.location",
                                          "/data/source/deepin-movie-reborn/movie/DMovie");
    QTest::qWait(200);
    w->requestAction(ActionFactory::ActionKind::Settings);
    QTest::qWait(200);
    EXPECT_TRUE(true);
}

TEST(requestAction,openCdrom)
{
    Platform_MainWindow* w = dApp->getMainWindow();

    QTest::qWait(200);
    w->requestAction(ActionFactory::ActionKind::OpenCdrom);
    EXPECT_TRUE(true);
}

TEST(requestAction,changeSubCodepage)
{
    Platform_MainWindow* w = dApp->getMainWindow();

    QTest::qWait(200);
    QList<QVariant> list = {"test"};
    w->requestAction(ActionFactory::ActionKind::ChangeSubCodepage, false, list);
}

