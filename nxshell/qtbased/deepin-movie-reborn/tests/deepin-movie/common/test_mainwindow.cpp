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
#include <QtCore/QMetaObject>
#include <QGuiApplication>
#include <QWidget>
#include <QFileInfo>

#include <unistd.h>
#include <gtest/gtest.h>

#define protected public
#define private public
#include "src/common/mainwindow.h"
#undef protected
#undef private
#include "application.h"
#include "src/libdmr/filefilter.h"
#include "src/libdmr/player_engine.h"
#include "src/widgets/toolbox_proxy.h"
#include "src/widgets/toolbutton.h"
#include "src/widgets/playlist_widget.h"
#include "src/widgets/slider.h"
#include "src/widgets/movieinfo_dialog.h"
#include "src/widgets/url_dialog.h"
#include "src/widgets/dmr_lineedit.h"
#include "src/common/actions.h"
#include "src/backends/mpv/mpv_glwidget.h"
#include "utils.h"
#include "actions.h"
#include "dbus_adpator.h"
#include "dbusutils.h"
#include "burst_screenshots_dialog.h"
#include "mpv_proxy.h"
#include "stub/stub.h"
#include "stub/addr_any.h"
#include "stub/stub_function.h"

using namespace dmr;
#ifdef false
TEST(PadMode, mainWindow)
{
    Stub stub;
    //Stub stub1;
    stub.set(ADDR(CompositingManager, isPadSystem), StubFunc::isPadSystemTrue_stub);
    stub.set(ADDR(CompositingManager, composited), StubFunc::isCompositedFalse_stub);
    QTest::qWait(1000);
    MainWindow mw;
    QTest::qWait(100);
    mw.show();

    QTest::qWait(800);
    mw.requestAction(ActionFactory::ActionKind::StartPlay);

    QTest::mouseClick(&mw, Qt::LeftButton, Qt::NoModifier, QPoint(), 500);

    QTest::qWait(500);
    stub.reset(ADDR(CompositingManager, isPadSystem));
    stub.reset(ADDR(CompositingManager, composited));
    mw.close();
    QTest::qWait(2000);
}
#endif

TEST(MainWindow, init)
{
    MainWindow *w = dApp->getMainWindow();
    PlayerEngine *engine =  w->engine();
    int sid;

    w->checkWarningMpvLogsChanged("test", "Hardware does not support image size 3840x2160");

    sid = engine->sid();
    engine->isSubVisible();
    engine->selectSubtitle(0);
    engine->videoAspect();
    engine->volumeUp();
    engine->setDVDDevice("/data/home/");

    QMimeData mimeData;
    QList<QUrl> urls;
    urls << QUrl::fromLocalFile("/data/source/deepin-movie-reborn/movie/demo.mp4");
    mimeData.setUrls(urls);
    // Drop inside the mainwindow
    QDropEvent drop(w->pos(), Qt::CopyAction, &mimeData, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(w, &drop);
    QVERIFY(drop.isAccepted());
    QCOMPARE(drop.dropAction(), Qt::CopyAction);
    QTest::qWait(100);
}

TEST(MainWindow, nakedstream)
{
    MainWindow *w = dApp->getMainWindow();
    ToolboxProxy *toolboxProxy = w->toolbox();
    DMRSlider* dmrSlider = toolboxProxy->getSlider();
    VolumeSlider* volumeSlider = toolboxProxy->volumeSlider();
    VolumeButton *volBtn = toolboxProxy->volBtn();
    PlayerEngine *engine =  w->engine();
    QList<QUrl> listPlayFiles;
    listPlayFiles << QUrl::fromLocalFile("/data/source/deepin-movie-reborn/movie/test.264");

    w->show();
    engine->playlist().loadPlaylist();
    engine->playlist().clear();
    engine->addPlayFiles(listPlayFiles);
    engine->playByName(QUrl::fromLocalFile("/data/source/deepin-movie-reborn/movie/test.264"));

    // drop is playing video
    QMimeData mimeData;
    QList<QUrl> urls;
    urls << QUrl::fromLocalFile("/data/source/deepin-movie-reborn/movie/test.264");
    mimeData.setUrls(urls);

    QDragEnterEvent dragEnter(QPoint(0, 0), Qt::CopyAction, &mimeData, Qt::LeftButton, {});
    QApplication::sendEvent(w, &dragEnter);
    QVERIFY(dragEnter.isAccepted());
    QCOMPARE(dragEnter.dropAction(), Qt::CopyAction);

    QDragMoveEvent dragMove(QPoint(0, 0), Qt::CopyAction, &mimeData, Qt::LeftButton, Qt::NoModifier);
    qApp->sendEvent(w, &dragMove);

    QDropEvent drop(QPoint(0, 0), Qt::CopyAction, &mimeData, Qt::NoButton, {});
    QApplication::sendEvent(w, &drop);
    QVERIFY(drop.isAccepted());
    QCOMPARE(drop.dropAction(), Qt::CopyAction);

    QTest::mousePress(dmrSlider, Qt::LeftButton, Qt::NoModifier, QPoint(), 100);
    QTest::mouseMove(dmrSlider, QPoint(), 100);
    QTest::mouseRelease(dmrSlider, Qt::LeftButton, Qt::NoModifier, QPoint(), 100);
    QTest::keyClick(dmrSlider, Qt::Key_Left, Qt::NoModifier, 200);
    QTest::keyClick(dmrSlider, Qt::Key_Right, Qt::NoModifier, 200);

    QTest::mouseClick(volBtn, Qt::LeftButton, Qt::NoModifier, QPoint(), 200);
    QTest::qWait(500);
    QTest::mousePress(volumeSlider, Qt::LeftButton, Qt::NoModifier, QPoint(), 100);
    QTest::mouseMove(volumeSlider, QPoint(), 100);
    QTest::mouseRelease(volumeSlider, Qt::LeftButton, Qt::NoModifier, QPoint(), 100);
    QTest::keyClick(dmrSlider, Qt::Key_M, Qt::NoModifier, 200);
    QTest::keyClick(w, Qt::Key_Up, Qt::ControlModifier | Qt::AltModifier, 200);
    QTest::keyClick(w, Qt::Key_Down, Qt::ControlModifier | Qt::AltModifier, 200);

    // reset case
    QTest::mouseMove(volBtn, QPoint(), 200);
    QTest::keyClick(volBtn, Qt::Key_Enter, Qt::NoModifier, 200);
    QTest::qWait(1000);
    engine->playByName(QUrl::fromLocalFile("/data/source/deepin-movie-reborn/movie/demo.mp4"));
}

#ifndef __mips__
TEST(MainWindow, tabInteraction)
{
    MainWindow *w = dApp->getMainWindow();
    PlayerEngine *engine =  w->engine();
    ToolboxProxy *toolboxProxy = w->toolbox();
    ToolButton *fsBtn = toolboxProxy->fsBtn();
    VolumeButton *volBtn = toolboxProxy->volBtn();
    ToolButton *listBtn = toolboxProxy->listBtn();
    PlaylistWidget *playlistWidget;
    DListWidget *playlist;
    QList<QUrl> listPlayFiles;
    QTestEventList testEventList;

    w->show();

    listPlayFiles << QUrl::fromLocalFile("/data/source/deepin-movie-reborn/movie/demo.mp4")\
                  << QUrl::fromLocalFile("/data/source/deepin-movie-reborn/movie/天空之眼 高清1080P.mp4")\
                  << QUrl::fromLocalFile("/data/source/deepin-movie-reborn/movie/bensound-sunny.mp3");

    engine->playlist().loadPlaylist();
    const QList<QUrl> &valids = engine->addPlayFiles(listPlayFiles);

    QTest::qWait(500);

    QKeyEvent *tabEvent = new QKeyEvent(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
    QApplication::sendEvent(w, tabEvent);
    QApplication::sendEvent(w->windowHandle(), tabEvent);

    //volume control
    volBtn->setFocus();
    QTest::keyClick(volBtn, Qt::Key_Enter, Qt::NoModifier, 200);
    QTest::qWait(500);

    toolboxProxy->changeMuteState();
    for (int i = 0; i < 5; i++) {
        QTest::keyClick(volBtn, Qt::Key_Down, Qt::NoModifier, 100); //volume up 5
    }
    for (int i = 0; i < 10; i++) {
        QTest::keyClick(volBtn, Qt::Key_Up, Qt::NoModifier, 100); //volume up 5
    }

    //play list
    playlistWidget = w->playlist();
    playlist = playlistWidget->get_playlist();
    listBtn->setFocus();
    QTest::qWait(500);
    QTest::keyClick(listBtn, Qt::Key_Enter, Qt::NoModifier, 200);
    QTest::keyClick(playlistWidget, Qt::Key_Enter, Qt::NoModifier, 500);    //clear playlist
    for (int i = 0; i < 3; i++) {
        QTest::keyClick(playlist, Qt::Key_Tab, Qt::NoModifier, 100);
    }
    QTest::keyClick(w, Qt::Key_Escape, Qt::NoModifier, 500);    //close playlist by Esc

    QTest::qWait(500);
    engine->addPlayFiles(listPlayFiles);
    listBtn->setFocus();
    QTest::keyClick(listBtn, Qt::Key_Enter, Qt::NoModifier, 1000);
    QTest::keyClick(playlistWidget, Qt::Key_Tab, Qt::NoModifier, 500);
    QTest::keyClick(playlistWidget, Qt::Key_Down, Qt::NoModifier, 500);
    QTest::keyClick(playlistWidget, Qt::Key_Up, Qt::NoModifier, 500);
    QTest::keyClick(playlist, Qt::Key_Enter, Qt::NoModifier, 500);

    delete tabEvent;
    tabEvent = nullptr;
}
#endif

TEST(MainWindow, loadFile)
{
    MainWindow *w = dApp->getMainWindow();
    w->show();
    QList<QString> wrongPath;
    wrongPath << "/data/source/deepin-movie-reborn/movie/wrong.mp4";
    w->play(wrongPath);
    PlayerEngine *engine =  w->engine();
    QList<QUrl> listPlayFiles;
    listPlayFiles << QUrl::fromLocalFile("/data/source/deepin-movie-reborn/movie/demo.mp4")
                  << QUrl::fromLocalFile("/data/source/deepin-movie-reborn/movie/bensound-sunny.mp3");

    const QList<QUrl> &valids = engine->addPlayFiles(listPlayFiles);
    QCOMPARE(engine->isPlayableFile(valids[0]), true);
    if (!valids.empty()) {
        engine->playByName(valids[0]);
    }

    qDebug() << __func__ << "MainWindow.loadFile:" << engine->state();
    w->checkOnlineState(false);
    QTest::qWait(200);
    w->resize(900, 700);
    QTest::qWait(200);
    w->resize(300, 300);
    QTest::qWait(200);
    w->requestAction(ActionFactory::ActionKind::MatchOnlineSubtitle);
    QTest::qWait(200);
//    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::DarkType);
//    emit DGuiApplicationHelper::instance()->paletteTypeChanged(DGuiApplicationHelper::DarkType);
//    dApp->setProperty("themeType", DGuiApplicationHelper::DarkType);

    // video judge
    EXPECT_TRUE(FileFilter::instance()->isVideo(listPlayFiles[0]));
}

TEST(MainWindow, DBus)
{
    MainWindow *w = dApp->getMainWindow();
    ApplicationAdaptor *appAdaptor = new ApplicationAdaptor(w);
    DBusUtils utils;
    QVariant v = ApplicationAdaptor::redDBusProperty("com.deepin.daemon.Audio", "/com/deepin/daemon/Audio",
                                                     "com.deepin.daemon.Audio", "SinkInputs");
    QVariant v_invalid = ApplicationAdaptor::redDBusProperty("com.test", "/test", "com.test", "SinkInputs");
    v_invalid = DBusUtils::redDBusProperty("com.test", "/test", "com.test", "SinkInputs");

    if (v.isValid()) {
        QList<QDBusObjectPath> allSinkInputsList = v.value<QList<QDBusObjectPath> >();
        for (auto curPath : allSinkInputsList) {
            QVariant name = ApplicationAdaptor::redDBusProperty("com.deepin.daemon.Audio", curPath.path(),
                                                                "com.deepin.daemon.Audio.SinkInput", "Name");

            QString strMovie = QObject::tr("Movie");
            if (!name.isValid() || (!name.toString().contains(strMovie, Qt::CaseInsensitive) && !name.toString().contains("deepin-movie", Qt::CaseInsensitive)))
                continue;

            QString sinkInputPath = curPath.path();
            break;
        }
    }

    QVariant method = ApplicationAdaptor::redDBusMethod("com.deepin.daemon.Audio", "/com/deepin/daemon/Audio",
                                                        "com.deepin.daemon.Audio", "SinkInputs");
    QVariant method_invalid = ApplicationAdaptor::redDBusMethod("com.test", "/com/test", "com.test", "SinkInputs");
    method = DBusUtils::redDBusMethod("com.deepin.daemon.Audio", "/com/deepin/daemon/Audio",
                                      "com.deepin.daemon.Audio", "SinkInputs");
    method_invalid = DBusUtils::redDBusMethod("com.test", "/com/test", "com.test", "SinkInputs");
    appAdaptor->Raise();
    appAdaptor->openFile("/data/source/deepin-movie-reborn/movie/demo.mp4");

    //QDBusInterface *m_pDBus = new QDBusInterface("org.freedesktop.login1", "/org/freedesktop/login1", "org.freedesktop.login1.Manager", QDBusConnection::systemBus());
    w->sleepStateChanged(true);
    Stub stub;
    stub.set(ADDR(PlayerEngine, state), StubFunc::playerEngineState_Paused_stub);
    w->sleepStateChanged(false);
    stub.reset(ADDR(PlayerEngine, state));
}

TEST(MainWindow, hwdecChange)
{
    MainWindow *w = dApp->getMainWindow();
    PlayerEngine *engine =  w->engine();

    engine->changehwaccelMode(Backend::hwaccelClose);
    QTest::keyClick(w, Qt::Key_H, Qt::ControlModifier, 500);

    engine->changehwaccelMode(Backend::hwaccelAuto);
    engine->setBackendProperty("hwdec", "auto");
    w->setCurrentHwdec("");
    QTest::keyClick(w, Qt::Key_H, Qt::ControlModifier, 1000);
    QTest::keyClick(w, Qt::Key_H, Qt::ControlModifier, 500);
}

TEST(MainWindow, resizeWindow)
{
    MainWindow *w = dApp->getMainWindow();
    //缩放窗口
    w->move(200, 200);
    w->updateGeometry(CornerEdge::LeftEdge, QPoint(100, 100));
    w->updateGeometry(CornerEdge::TopEdge, QPoint(100, 100));
    w->updateGeometry(CornerEdge::RightEdge, QPoint(100, 100));
    w->updateGeometry(CornerEdge::BottomEdge, QPoint(100, 100));
    w->updateGeometry(CornerEdge::NoneEdge, QPoint(100, 100));
    w->updateGeometry(CornerEdge::TopLeftCorner, QPoint(100, 100));
    w->updateGeometry(CornerEdge::TopRightCorner, QPoint(1300, 100));
    w->updateGeometry(CornerEdge::BottomLeftCorner, QPoint(100, 100));
    w->updateGeometry(CornerEdge::BottomRightCorner, QPoint(100, 100));
}

TEST(MainWindow, touch)
{
    MainWindow *w = dApp->getMainWindow();
    ToolboxProxy *toolboxProxy = w->toolbox();
    QStackedWidget *progbarWidget = toolboxProxy->findChild<QStackedWidget *>(PROGBAR_WIDGET);

    PlayerEngine *engine =  w->engine();
    engine->stop();
    engine->playByName(QUrl::fromLocalFile("/data/source/deepin-movie-reborn/movie/demo.mp4"));

    QTest::qWait(500);
    if (!w->isFullScreen()) {
        qDebug() << __func__ << "进入全屏";
        QTest::mouseDClick(w, Qt::LeftButton, Qt::NoModifier, QPoint(), 500); //fullscreen
    }

#if !defined (__mips__ ) && !defined(__aarch64__)
    w->setTouched(true);
    Settings::get().settings()->setOption("base.play.showInthumbnailmode", true);
#endif
    QTest::qWait(500);
    QTest::mousePress(w->windowHandle(), Qt::LeftButton, Qt::MetaModifier, QPoint(100, 200), 200);
    QTest::mouseRelease(w->windowHandle(), Qt::LeftButton, Qt::MetaModifier, QPoint(200, 200), 200);

    QTest::mousePress(w->windowHandle(), Qt::LeftButton, Qt::MetaModifier, QPoint(400, 100), 200);
    QTest::mouseRelease(w->windowHandle(), Qt::LeftButton, Qt::MetaModifier, QPoint(400, 300), 200);

#if !defined (__mips__ ) && !defined(__aarch64__)
    while (progbarWidget->currentIndex() == 1) {
        QTest::qWait(200);
    }
#endif

    w->setTouched(true);
    QTest::mousePress(w->windowHandle(), Qt::LeftButton, Qt::MetaModifier, QPoint(300, 200), 200);
    QTest::mouseRelease(w->windowHandle(), Qt::LeftButton, Qt::MetaModifier, QPoint(100, 200), 200);

    QTest::mousePress(w->windowHandle(), Qt::LeftButton, Qt::MetaModifier, QPoint(400, 300), 200);
    QTest::mouseRelease(w->windowHandle(), Qt::LeftButton, Qt::MetaModifier, QPoint(400, 100), 200);

#if !defined (__mips__ ) && !defined(__aarch64__)
    Settings::get().settings()->setOption("base.play.showInthumbnailmode", false);
#endif
    if (w->isFullScreen()) {
        //quit fullscreen
        QTest::mouseDClick(w, Qt::LeftButton, Qt::NoModifier, QPoint(), 200);
    }
    w->setTouched(false);
}

TEST(MainWindow, shortCutPlay)
{
    MainWindow *w = dApp->getMainWindow();
    QTestEventList testEventList;
    PlayerEngine *engine =  w->engine();

    engine->playByName(QUrl::fromLocalFile("/data/source/deepin-movie-reborn/movie/demo.mp4"));
    qDebug() << __func__ << "playerEngineState:" << engine->state()
             << "playlistCount:" << engine->playlist().count();

    while (engine->state() == PlayerEngine::CoreState::Idle) {
        QTest::qWait(100);
    }

    //shortcut view
//    testEventList.addKeyClick(Qt::Key_Slash, Qt::ControlModifier | Qt::ShiftModifier, 500);

    BurstScreenshotsDialog bsd(engine->playlist().currentInfo());
    bsd.show();
    QTest::qWait(500);
    bsd.close();

    //screenshot
    qDebug() << __func__ << "shortCutPlay: start screenshot " << engine->state();
    testEventList.addKeyClick(Qt::Key_A, Qt::AltModifier, 500);
    //burst screenshot
    testEventList.addKeyClick(Qt::Key_S, Qt::AltModifier, 1000);

    testEventList.addKeyClick(Qt::Key_Right, Qt::NoModifier, 300); //fast forward
    for (int i = 0; i < 4; i++) {
        testEventList.addKeyClick(Qt::Key_Left, Qt::NoModifier, 300);  //fast backward
    }

    //playlist
    testEventList.addKeyClick(Qt::Key_F3, Qt::NoModifier, 800);    //playlist popup
    testEventList.addKeyClick(Qt::Key_Down, Qt::NoModifier, 400);
    testEventList.addKeyClick(Qt::Key_Enter, Qt::NoModifier, 500);      //play selected item
    testEventList.addKeyClick(Qt::Key_F3, Qt::NoModifier, 1800);    //playlist popup
    testEventList.addKeyClick(Qt::Key_Up, Qt::NoModifier, 400);
    testEventList.addKeyClick(Qt::Key_Enter, Qt::NoModifier, 500);
    qDebug() << __func__ << "playlist_count:" << engine->playlist().count();
    if (engine->playlist().count() >= 2) {
        testEventList.addKeyClick(Qt::Key_F3, Qt::NoModifier, 1900);    //playlist popup
        testEventList.addKeyClick(Qt::Key_Down, Qt::NoModifier, 500);
        testEventList.addKeyClick(Qt::Key_Delete, Qt::NoModifier, 500);    //delete from playlist
        testEventList.addKeyClick(Qt::Key_F3, Qt::NoModifier, 500);
    }

    if (w->engine()->state() == PlayerEngine::Idle) {
        w->engine()->playByName(QUrl::fromLocalFile("/data/source/deepin-movie-reborn/movie/demo.mp4"));
    }
    while (w->engine()->state() == PlayerEngine::Idle) {
        QTest::qWait(200);
    }

    //加速播放
    for (int i = 0; i < 10 ; i++) {
        testEventList.addKeyClick(Qt::Key_Right, Qt::ControlModifier, 50);
    }
    //减速播放
    for (int i = 0; i < 16 ; i++) {
        testEventList.addKeyClick(Qt::Key_Left, Qt::ControlModifier, 50);
    }

    //还原播放速度
    testEventList.addKeyClick(Qt::Key_R, Qt::ControlModifier, 200);

//    testEventList.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 300);
//    testEventList.addKeyClick(Qt::Key_Tab, Qt::NoModifier, 300);

    qDebug() << __func__ << "playerEngineState:" << engine->state();
    testEventList.simulate(w);
}

TEST(MainWindow, shortCutVolumeAndFrame)
{
    MainWindow *w = dApp->getMainWindow();
    QTestEventList testEventList;

    QVERIFY(w->toolbox()->getSlider()->isEnabled());

    //volume
    testEventList.addKeyClick(Qt::Key_Down, Qt::ControlModifier | Qt::AltModifier, 20);    //volume down
    for (int i = 0; i < 11; i++) {
        testEventList.addKeyClick(Qt::Key_Up, Qt::ControlModifier | Qt::AltModifier, 10);//volume up
    }
    for (int i = 0; i < 20; i++) {
        testEventList.addKeyClick(Qt::Key_Down, Qt::ControlModifier | Qt::AltModifier, 10);    //volume down
    }

    testEventList.addKeyClick(Qt::Key_M, Qt::NoModifier, 300); //mute

    testEventList.addKeyClick(Qt::Key_Left, Qt::ControlModifier | Qt::ShiftModifier, 100); //last frame
    testEventList.addKeyClick(Qt::Key_Left, Qt::ControlModifier | Qt::ShiftModifier, 100);
    testEventList.addKeyClick(Qt::Key_Right, Qt::ControlModifier | Qt::ShiftModifier, 100); //next frame
    testEventList.addKeyClick(Qt::Key_Right, Qt::ControlModifier | Qt::ShiftModifier, 100);
    testEventList.addKeyClick(Qt::Key_Right, Qt::ControlModifier | Qt::ShiftModifier, 100);
    testEventList.addKeyClick(Qt::Key_Space, Qt::NoModifier, 300); //play

    testEventList.simulate(w);
}

TEST(MainWindow, miniMode)
{
    MainWindow *w = dApp->getMainWindow();
    PlayerEngine *engine =  w->engine();

    engine->playByName(QUrl::fromLocalFile("/data/source/deepin-movie-reborn/movie/demo.mp4"));
    qDebug() << __func__ << engine->state() << "playlist count:" << engine->playlist().count();

    while (engine->state() == PlayerEngine::CoreState::Idle) {
        QTest::qWait(100);
    }
    qDebug() << __func__ << engine->state() << "playlist count:" << engine->playlist().count();

    QTest::keyClick(w, Qt::Key_F2, Qt::NoModifier, 500);
    if (!w->getMiniMode()) {
        w->requestAction(ActionFactory::ActionKind::ToggleMiniMode);
    }
#if defined(__aarch64__)
    DIconButton *miniPauseBtn = w->findChild<DIconButton *>("MiniPlayBtn");
#else
    DIconButton *miniPauseBtn = w->findChild<DIconButton *>("MiniPauseBtn");
#endif
    DIconButton *miniQuiteMiniBtn = w->findChild<DIconButton *>("MiniQuitMiniBtn");

    if (miniPauseBtn && miniQuiteMiniBtn) {
        QTest::mouseMove(miniPauseBtn, QPoint(), 1000);
        QTest::mouseClick(miniPauseBtn, Qt::LeftButton, Qt::NoModifier, QPoint(), 300);
        //w->customContextMenuRequested(w->pos());
        QTest::mouseMove(miniQuiteMiniBtn, QPoint(), 300);
        QTest::mouseClick(miniQuiteMiniBtn, Qt::LeftButton, Qt::NoModifier, QPoint(), 300);
    }

    QTest::qWait(500);
    w->requestAction(ActionFactory::ActionKind::ToggleMiniMode);

    if (miniPauseBtn) {
        DIconButton *miniCloseBtn = w->findChild<DIconButton *>("MiniCloseBtn");
        QTest::mouseMove(miniPauseBtn, QPoint(), 300);
        QTest::mouseClick(miniPauseBtn, Qt::LeftButton, Qt::NoModifier, QPoint(), 300);
        QTest::mouseMove(miniCloseBtn, QPoint(), 300);
        QTest::mouseClick(miniCloseBtn, Qt::LeftButton, Qt::NoModifier, QPoint(), 300);
        w->show();
    }
    QTest::keyClick(w, Qt::Key_Escape, Qt::NoModifier, 1000);
}

TEST(MainWindow, progBar)
{
    MainWindow *w = dApp->getMainWindow();
    PlayerEngine *engine =  w->engine();
    ToolboxProxy *toolboxProxy = w->toolbox();
    DMRSlider *progBarSlider = toolboxProxy->getSlider();
    QStackedWidget *progbarWidget = toolboxProxy->findChild<QStackedWidget *>(PROGBAR_WIDGET);

    engine->playByName(QUrl::fromLocalFile("/data/source/deepin-movie-reborn/movie/demo.mp4"));

    ////进度条模式
    QMouseEvent hover(QEvent::HoverEnter, QPoint(progBarSlider->pos().x(), progBarSlider->pos().y()),
                      Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(progBarSlider, &hover);

    QPoint point(progBarSlider->slider()->x() + 30, progBarSlider->slider()->y());
    QTest::mouseMove(progBarSlider->slider(), point, 200);
    QTest::mouseMove(progBarSlider->slider(), QPoint(point.x(), point.y() - 40), 200);
    QTest::mouseMove(progBarSlider->slider(), point, 200);
    QTest::mouseClick(progBarSlider->slider(), Qt::LeftButton, Qt::NoModifier, point, 200);
    //拖动
    QPoint startPoint(progBarSlider->slider()->x() + 100, progBarSlider->slider()->y());
    QPoint endPoint(progBarSlider->slider()->x() + 10, progBarSlider->slider()->y());
    QTest::mouseMove(progBarSlider->slider(), startPoint, 200);
    QTest::mousePress(progBarSlider->slider(), Qt::LeftButton, Qt::NoModifier, startPoint, 100);
    QTest::mouseMove(progBarSlider->slider(), endPoint, 200);
    QTest::mouseRelease(progBarSlider->slider(), Qt::LeftButton, Qt::NoModifier, endPoint, 200);

    startPoint = QPoint(progBarSlider->pos().x() + 60, progBarSlider->pos().y() + 50);
    endPoint = QPoint(progBarSlider->pos().x() + 90, progBarSlider->pos().y() + 50);
//    QTest::mouseMove(progBarSlider, startPoint, 200);
//    QTest::mousePress(progBarSlider, Qt::LeftButton, Qt::NoModifier, startPoint, 100);
//    QTest::mouseMove(progBarSlider, endPoint, 200);
//    QTest::mouseRelease(progBarSlider, Qt::LeftButton, Qt::NoModifier, endPoint, 100);

    //press
    QMouseEvent mousePress(QEvent::MouseButtonPress, startPoint, Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(progBarSlider, &mousePress);
    //move
    QMouseEvent mouseMove(QEvent::MouseMove, endPoint, Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(progBarSlider, &mouseMove);
    //release
    QMouseEvent mouseRelease(QEvent::MouseButtonRelease, endPoint, Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(progBarSlider, &mouseRelease);

    QEvent leaveEvent(QEvent::Leave);
    QEvent enterEvent(QEvent::Enter);
    QApplication::sendEvent(progBarSlider, &leaveEvent);
    QApplication::sendEvent(progBarSlider, &enterEvent);

    QWheelEvent wheelEvent = QWheelEvent(endPoint, 20, Qt::MidButton, Qt::NoModifier);
    QApplication::sendEvent(progBarSlider, &wheelEvent);

    ////胶片模式
#if !defined (__mips__ ) && !defined(__aarch64__)
    Settings::get().settings()->setOption("base.play.showInthumbnailmode", true);
    while (progbarWidget->currentIndex() == 1) { //等待胶片加载
        QTest::qWait(200);
    }

    QWidget *viewProgBar = (QWidget *)toolboxProxy->getViewProBar();
    startPoint = QPoint(viewProgBar->x() + 100, viewProgBar->y() + 20);
    endPoint = QPoint(viewProgBar->x() + 20, viewProgBar->y() + 20);
    QTest::mouseMove(viewProgBar, QPoint(viewProgBar->x() + 50, viewProgBar->y() + 20), 500);
    QTest::mouseClick(viewProgBar, Qt::LeftButton, Qt::NoModifier, QPoint(viewProgBar->x() + 50, viewProgBar->y() + 20), 500);
    QTest::mouseMove(viewProgBar, startPoint, 300);
    QTest::mousePress(viewProgBar, Qt::LeftButton, Qt::NoModifier, startPoint, 100);
    QTest::mouseMove(viewProgBar, endPoint, 500);
    QTest::mouseRelease(viewProgBar, Qt::LeftButton, Qt::NoModifier, endPoint, 500);
    QTest::qWait(500);

    Settings::get().settings()->setOption("base.play.showInthumbnailmode", false);
#endif
}

TEST(MainWindow, ViewProgBar)
{
    MainWindow *w = dApp->getMainWindow();
    PlayerEngine *engine =  w->engine();
    ToolboxProxy *toolboxProxy = w->toolbox();
    DMRSlider *progBarSlider = toolboxProxy->getSlider();
    QStackedWidget *progbarWidget = toolboxProxy->findChild<QStackedWidget *>(PROGBAR_WIDGET);
    QList<QPixmap> pmList;
    pmList.append(QPixmap(QString("/data/source/deepin-movie-reborn/test.jpg")));

    engine->playByName(QUrl::fromLocalFile("/data/source/deepin-movie-reborn/movie/demo.mp4"));

//    viewProgBarLoad *loadWorker = new viewProgBarLoad(engine, progBarSlider, toolboxProxy);
//    QTest::qWait(200);
//    loadWorker->start();
//    QObject::connect(loadWorker, SIGNAL(sigFinishiLoad(QSize)), toolboxProxy, SLOT(finishLoadSlot(QSize)));

    //loadWorker->loadViewProgBar(QSize(500, 50));
    //progbarWidget->setCurrentIndex(2);

    toolboxProxy->setThumbnailmode(true);
    toolboxProxy->resize(400,60);

//    QTest::qWait(1000);

    QWidget *viewProgBar = (QWidget *)toolboxProxy->getViewProBar();
    viewProgBar->show();
    QTest::qWait(200);
    QMouseEvent mouseMove(QEvent::MouseMove, QPoint(200, 20), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(viewProgBar, &mouseMove);
    QMouseEvent mousePress(QEvent::MouseButtonPress, QPoint(200, 20), Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(viewProgBar, &mousePress);
    mouseMove = QMouseEvent(QEvent::MouseMove, QPoint(250, 20), Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(viewProgBar, &mouseMove);
    QMouseEvent mousRelease(QEvent::MouseButtonRelease, QPoint(250, 20), Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(viewProgBar, &mousRelease);

    QEvent leave(QEvent::Leave);
    QApplication::sendEvent(viewProgBar, &leave);

//    QObject::disconnect(loadWorker, SIGNAL(sigFinishiLoad(QSize)), toolboxProxy, SLOT(finishLoadSlot(QSize)));
    QTest::qWait(600);
}

TEST(MainWindow, ThumbnailPreview)
{
    MainWindow *w = dApp->getMainWindow();
    PlayerEngine *engine =  w->engine();
    ToolboxProxy *toolboxProxy = w->toolbox();
    DMRSlider *progBarSlider = toolboxProxy->getSlider();
    QStackedWidget *progbarWidget = toolboxProxy->findChild<QStackedWidget *>(PROGBAR_WIDGET);

    ThumbnailWorker::get().setPlayerEngine(engine);
    ThumbnailWorker::get().requestThumb(QUrl("/data/source/deepin-movie-reborn/movie/demo.mp4"),
                                        200);

}

TEST(MainWindow, movieInfoDialog)
{
    MainWindow *w = dApp->getMainWindow();
    PlayerEngine *engine  =  w->engine();
    engine->playByName(QUrl::fromLocalFile("/data/source/deepin-movie-reborn/movie/demo.mp4"));
    qDebug() << __func__ << engine->state() << engine->playlist().count();

    while (engine->state() == PlayerEngine::CoreState::Idle) {
        QTest::qWait(100);
    }

    MovieInfoDialog mid(engine->playlist().currentInfo(), w);
    DLabel *filePathLbl = mid.findChild<DLabel *>("filePathLabel");
    QEvent leaveEvent(QEvent::Leave);
//    QEvent toolTipEvent(QEvent::ToolTip);
    QHelpEvent toolTipEvent(QEvent::ToolTip, filePathLbl->pos(), QPoint());

//    mid.setFont(QFont("Times"));
    QTest::qWait(200);
    mid.show();
    QTest::qWait(200);
    if (filePathLbl) {
        QTest::mouseMove(filePathLbl, QPoint(), 200);
        QTest::qWait(700);  //wait 700ms for tooltip event
        QApplication::sendEvent(filePathLbl, &toolTipEvent);
        QTest::mouseMove(w, QPoint(200, 300), 200);
        QApplication::sendEvent(filePathLbl, &leaveEvent);
        QTest::qWait(50);
    }
    mid.close();

//    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::LightType);
    emit DGuiApplicationHelper::instance()->paletteTypeChanged(DGuiApplicationHelper::LightType);

    QTest::qWait(100);
    mid.show();
    if (filePathLbl) {
        QTest::mouseMove(filePathLbl, QPoint(), 100);
        QApplication::sendEvent(filePathLbl, &toolTipEvent);
        QTest::mouseMove(w, QPoint(200, 300), 800);
    }
    QTest::qWait(100);
    mid.close();
}

TEST(MainWindow, VolumeMonitoring)
{
    MainWindow *w = dApp->getMainWindow();
    VolumeMonitoring volMonitor(w);
    volMonitor.start();
    QTest::qWait(100);
    volMonitor.timeoutSlot();
    QTest::qWait(100);
    volMonitor.stop();
    QTest::qWait(100);
}

TEST(MainWindow, SettingsDialog)
{
    MainWindow *w = dApp->getMainWindow();

    emit dApp->fontChanged(QFont("Helvetica"));
    DSettingsDialog *settingsDialog = w->initSettings();
    DLineEdit *savePathEdit = settingsDialog->findChild<DLineEdit *>("OptionSelectableLineEdit");
    QList<DPushButton *> Btns = settingsDialog->findChildren<DPushButton *>();

    if(savePathEdit) {
        emit savePathEdit->focusChanged(true);
        emit savePathEdit->textEdited("/data/source/deepin-movie-reborn/movie/DMovie");
        emit savePathEdit->editingFinished();
    }

//    AddrAny any;
//    std::map<std::string, void*> result;
//    any.get_local_func_addr_symtab("^createSelectableLineEditOptionHandle()$", result);
//    Stub stub;
//    std::map<std::string, void *>::iterator it;
//    for(it = result.begin(); it != result.end(); ++it) {
//        stub.set(it->second, StubFunc::createSelectableLineEditOptionHandle_lambda_stub);
//    }
    if(Btns[0]) {
        emit Btns[0]->clicked(false);
    }
}

TEST(MainWindow, reloadFile)
{
    MainWindow *w = dApp->getMainWindow();
    PlayerEngine *engine =  w->engine();
    QList<QUrl> listPlayFiles;
    listPlayFiles << QUrl::fromLocalFile("/data/source/deepin-movie-reborn/movie/demo.mp4")\
                  << QUrl::fromLocalFile("/data/source/deepin-movie-reborn/movie/bensound-sunny.mp3");
    QTest::qWait(100);
    engine->addPlayFiles(listPlayFiles);
}

TEST(ToolBox, playListWidget)
{
    MainWindow *w = dApp->getMainWindow();
    ToolboxProxy *toolboxProxy = w->toolbox();
    ToolButton *listBtn = toolboxProxy->listBtn();
    PlaylistWidget *playlistWidget = w->playlist();
    DListWidget *playlist = playlistWidget->get_playlist();
    DFloatingButton *playItemCloseBtn;
    //playlist item event
    QEvent tooltipEvent(QEvent::ToolTip);
    QEvent leaveEvent(QEvent::Leave);

    QEnterEvent enterEvent(QPoint(0, 0), listBtn->pos(), QPoint(0, 0));
    QApplication::sendEvent(listBtn, &enterEvent);
    QApplication::sendEvent(listBtn, &leaveEvent);
    QTest::qWait(100);

    QTest::mouseMove(listBtn, QPoint(), 200);
    QTest::mouseClick(listBtn, Qt::LeftButton, Qt::NoModifier, QPoint(), 300);  //playlist popup

    QTest::mouseMove(playlist->itemWidget(playlist->item(0)), QPoint(), 700);
    QTest::qWait(1000);
//    QApplication::sendEvent(playlist->itemWidget(playlist->item(0)), &tooltipEvent);
    QApplication::sendEvent(playlist->itemWidget(playlist->item(0)), &leaveEvent);
    QTest::mouseMove(playlist->itemWidget(playlist->item(1)), QPoint(), 200);
    QTest::mouseClick(playlist->itemWidget(playlist->item(1)), Qt::LeftButton, Qt::NoModifier, QPoint(), 200);
    QTest::mouseMove(playlist->itemWidget(playlist->item(0)), QPoint(), 200);
    QTest::mouseClick(playlist->itemWidget(playlist->item(0)), Qt::LeftButton, Qt::NoModifier, QPoint(), 200);
    QTest::mouseMove(playlist->itemWidget(playlist->item(1)), QPoint(), 200);
    QTest::mouseDClick(playlist->itemWidget(playlist->item(1)), Qt::LeftButton, Qt::NoModifier, QPoint(), 200);

    QTest::mouseMove(listBtn, QPoint(), 1000);
    QTest::mouseClick(listBtn, Qt::LeftButton, Qt::NoModifier, QPoint(), 200);  //playlist popup

    //event
//    emit playlist->model()->rowsMoved(playlistWidget, 0, 1, QModelIndex(), 1);
    QTest::qWait(100);
//    QContextMenuEvent *cme = new QContextMenuEvent(QContextMenuEvent::Mouse, playlist->itemWidget(playlist->item(0))->rect().center());
//    QTimer::singleShot(100,[=](){
//        emit ActionFactory::get().playlistContextMenu()->aboutToHide();
//        ActionFactory::get().playlistContextMenu()->clear();
//    });
//    QApplication::sendEvent(playlist->itemWidget(playlist->item(0)), cme);

    QPoint point(playlist->pos().x() + 300, playlist->pos().y() + 60);
    QTest::mouseMove(w, point, 200);
    QWheelEvent wheelEvent = QWheelEvent(point, 20, Qt::MidButton, Qt::NoModifier);
    QApplication::sendEvent(w, &wheelEvent);

    QTest::mouseMove(playlist->itemWidget(playlist->item(0)), QPoint(), 200);
    QTest::mouseClick(playlist->itemWidget(playlist->item(0)), Qt::LeftButton, Qt::NoModifier, QPoint(), 500);
    playItemCloseBtn = playlist->findChild<DFloatingButton *>(PLAYITEN_CLOSE_BUTTON);
    QTest::mouseMove(playItemCloseBtn, QPoint(), 100);
    QTest::mouseClick(playItemCloseBtn, Qt::LeftButton, Qt::NoModifier, QPoint(), 200);

    QTest::mouseMove(w, point, 200);
    QTest::mouseClick(w, Qt::LeftButton, Qt::NoModifier, point, 200);
}

TEST(ToolBox, playBtnBox)
{
    MainWindow *w = dApp->getMainWindow();
    PlayerEngine *engine =  w->engine();
    QList<QUrl> listPlayFiles;
    ToolboxProxy *toolboxProxy = w->toolbox();
    DButtonBoxButton *playBtn = toolboxProxy->playBtn();
    DButtonBoxButton *nextBtn = toolboxProxy->nextBtn();
    DButtonBoxButton *prevBtn = toolboxProxy->prevBtn();

    listPlayFiles << QUrl::fromLocalFile("/data/source/deepin-movie-reborn/movie/demo.mp4")\
                  << QUrl::fromLocalFile("/data/source/deepin-movie-reborn/movie/bensound-sunny.mp3");
    engine->addPlayFiles(listPlayFiles);

    QEvent enterEvent(QEvent::Enter);
    QEvent leaveEvent(QEvent::Leave);
    QMouseEvent mouseMove(QEvent::MouseMove, QPoint(0, 0), Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(playBtn, &enterEvent);
    QApplication::sendEvent(playBtn, &mouseMove);
    QApplication::sendEvent(playBtn, &leaveEvent);

    QApplication::sendEvent(nextBtn, &enterEvent);
    QApplication::sendEvent(nextBtn, &mouseMove);
    QApplication::sendEvent(nextBtn, &leaveEvent);

    QApplication::sendEvent(prevBtn, &enterEvent);
    QApplication::sendEvent(prevBtn, &mouseMove);
    QApplication::sendEvent(prevBtn, &leaveEvent);

    QTest::mouseMove(playBtn, QPoint(), 200);
    QTest::mouseClick(playBtn, Qt::LeftButton, Qt::NoModifier, QPoint(), 500); //pause
    QTest::mouseMove(w, QPoint(200, 300), 200);
    QTest::mouseMove(playBtn, QPoint(), 200);
    QTest::mouseClick(playBtn, Qt::LeftButton, Qt::NoModifier, QPoint(), 500); //play
    QTest::mouseMove(nextBtn, QPoint(), 200);
    QTest::mouseClick(nextBtn, Qt::LeftButton, Qt::NoModifier, QPoint(), 600);
    QTest::mouseMove(prevBtn, QPoint(), 200);
    QTest::mouseClick(prevBtn, Qt::LeftButton, Qt::NoModifier, QPoint(), 600);
}

TEST(ToolBox, UrlDialog)
{
    MainWindow *w = dApp->getMainWindow();
    UrlDialog *uDlg = new UrlDialog(w);
    LineEdit *lineEdit = uDlg->findChild<LineEdit *>();

    uDlg->show();
    QTest::mouseMove(uDlg->getButton(0), QPoint(), 200);
    QTest::mouseClick(uDlg->getButton(0), Qt::LeftButton, Qt::NoModifier, QPoint(), 200);

    uDlg->show();
    QTest::mouseMove(lineEdit, QPoint(), 200);
    QTest::keyClicks(lineEdit, QString("mail.263.net/"), Qt::NoModifier, 1);
    QTest::mouseMove(uDlg->getButton(1), QPoint(), 200);
    QTest::mouseClick(uDlg->getButton(1), Qt::LeftButton, Qt::NoModifier, QPoint(), 200);

    uDlg->show();
    QTest::mouseMove(lineEdit, QPoint(), 200);
    QTest::keyClicks(lineEdit, QString("https://www.baidu.com"), Qt::NoModifier, 1);
    QTest::mouseMove(uDlg->getButton(1), QPoint(), 200);
    QTest::mouseClick(uDlg->getButton(1), Qt::LeftButton, Qt::NoModifier, QPoint(), 200);

    uDlg->show();
    QTest::mouseMove(lineEdit, QPoint(), 200);
    QTest::keyClicks(lineEdit,
                     QString("https://stream7.iqilu.com/10339/upload_transcode/202002/18/20200218093206z8V1JuPlpe.mp4"),
                     Qt::NoModifier, 1);
    QTest::mouseMove(uDlg->getButton(1), QPoint(), 200);
    QTest::mouseClick(uDlg->getButton(1), Qt::LeftButton, Qt::NoModifier, QPoint(), 200);
    auto url = uDlg->url().toString();
    w->play({url});
    QTest::qWait(300);
}

TEST(ToolBox, fullScreenBtn)
{
    MainWindow *w = dApp->getMainWindow();
    ToolboxProxy *toolboxProxy = w->toolbox();
    ToolButton *fsBtn = toolboxProxy->fsBtn();
    PlayerEngine *engine =  w->engine();

    QList<QUrl> listPlayFiles;
    listPlayFiles << QUrl::fromLocalFile("/data/source/deepin-movie-reborn/movie/demo.mp4")
                  << QUrl::fromLocalFile("/data/source/deepin-movie-reborn/movie/bensound-sunny.mp3");

    const QList<QUrl> &valids = engine->addPlayFiles(listPlayFiles);
    QCOMPARE(engine->isPlayableFile(valids[0]), true);
    if (!valids.empty()) {
        engine->playByName(valids[0]);
    }

    QTest::mouseMove(fsBtn, QPoint(), 200);
    QTest::mouseClick(fsBtn, Qt::LeftButton, Qt::NoModifier, QPoint(), 500);

    QContextMenuEvent context(QContextMenuEvent::Mouse, QPoint(200, 200));
    QApplication::sendEvent(w, &context);

//    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::DarkType);
    emit DGuiApplicationHelper::instance()->paletteTypeChanged(DGuiApplicationHelper::DarkType);

    QEvent enter(QEvent::Enter);
    QApplication::sendEvent(fsBtn, &enter);

    QEvent leave(QEvent::Leave);
    QApplication::sendEvent(fsBtn, &leave);

    QTest::mouseMove(fsBtn, QPoint(), 200);
    QTest::mouseClick(fsBtn, Qt::LeftButton, Qt::NoModifier, QPoint(), 500);
}

TEST(ToolBox, volBtn)
{
    MainWindow *w = dApp->getMainWindow();
    ToolboxProxy *toolboxProxy = w->toolbox();
    VolumeButton *volBtn = toolboxProxy->volBtn();

    QTest::mouseMove(volBtn, QPoint(), 200);
    QTest::mouseClick(volBtn, Qt::LeftButton, Qt::NoModifier, QPoint(), 500);
    QTest::qWait(300);
    QVERIFY(toolboxProxy->volumeSlider()->isVisible());

    QWheelEvent wheelUpEvent(volBtn->rect().center(), 20, Qt::NoButton, Qt::NoModifier);
    QWheelEvent wheelDownEvent(volBtn->rect().center(), -20, Qt::NoButton, Qt::NoModifier);
    QEnterEvent enterEvent(QPoint(0, 0), volBtn->pos(), QPoint(0, 0));
    QEvent leaveEvent(QEvent::Leave);

    QTest::qWait(100);
    QApplication::sendEvent(volBtn, &wheelUpEvent);
    QTest::qWait(100);
    QApplication::sendEvent(volBtn, &wheelDownEvent);
    QApplication::sendEvent(volBtn, &enterEvent);
    QApplication::sendEvent(volBtn, &leaveEvent);
}

TEST(ToolBox, mainWindowEvent)
{
    MainWindow *w = dApp->getMainWindow();
    QMimeData mimeData;
    QList<QUrl> urls;
    QPoint point(w->pos().x() + 20, w->pos().y() + 20);
    urls << QUrl::fromLocalFile("/data/source/deepin-movie-reborn/movie/demo.mp4")\
         << QUrl::fromLocalFile("/data/source/deepin-movie-reborn/movie/bensound-sunny.mp3") \
         << QUrl::fromLocalFile("/data/source/deepin-movie-reborn/movie/Hachiko.A.Dog's.Story.ass");
    mimeData.setUrls(urls);

    QDragEnterEvent dragEnter(QPoint(0, 0), Qt::CopyAction, &mimeData, Qt::LeftButton, {});
    QApplication::sendEvent(w, &dragEnter);
    QVERIFY(dragEnter.isAccepted());
    QCOMPARE(dragEnter.dropAction(), Qt::CopyAction);

    QDragMoveEvent dragMove(QPoint(0, 0), Qt::CopyAction, &mimeData, Qt::LeftButton, Qt::NoModifier);
    qApp->sendEvent(w, &dragMove);

    // Drop inside the mainwindow
    QDropEvent drop(QPoint(0, 0), Qt::CopyAction, &mimeData, Qt::NoButton, {});
    QApplication::sendEvent(w, &drop);
    QVERIFY(drop.isAccepted());
    QCOMPARE(drop.dropAction(), Qt::CopyAction);

    QWheelEvent wheelEvent = QWheelEvent(QPoint(0, 0), 20, Qt::MidButton, Qt::NoModifier);
    QApplication::sendEvent(w, &wheelEvent);

    //右键菜单这里有内存泄露，暂时注释掉
    //QContextMenuEvent *cme = new QContextMenuEvent(QContextMenuEvent::Mouse, w->rect().center());
    //QTimer::singleShot(100, [ = ]() {
    //    ActionFactory::get().mainContextMenu()->clear();
    //});
    //QApplication::sendEvent(w, cme);

    QMouseEvent mouseMove = QMouseEvent(QEvent::MouseMove, QPointF(100.0, 100.0),Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(w, &mouseMove);

    QTest::mouseClick(w, Qt::LeftButton, Qt::NoModifier, QPoint(100, 100), 200);
    QTest::qWait(100);
    //shortcut view
    QTest::keyPress(w, Qt::Key_Slash, Qt::ControlModifier | Qt::ShiftModifier, 100);

    w->testCdrom();
    QTest::qWait(500);

    //delete cme;
    //cme = nullptr;
}

TEST(ToolBox, clearPlayList)
{
    MainWindow *w = dApp->getMainWindow();
    ToolboxProxy *toolboxProxy = w->toolbox();
    ToolButton *listBtn = toolboxProxy->listBtn();
    DPushButton *playlistClearBtn = w->findChild<DPushButton *>(CLEAR_PLAYLIST_BUTTON);

    QTest::mouseMove(listBtn, QPoint(), 200);
    QTest::mouseClick(listBtn, Qt::LeftButton, Qt::NoModifier, QPoint(), 200);

    QTest::mouseMove(playlistClearBtn, QPoint(), 700);
    QTest::mouseClick(playlistClearBtn, Qt::LeftButton, Qt::NoModifier, QPoint(), 200);

    QTest::mouseMove(listBtn, QPoint(), 200);
    QTest::mouseClick(listBtn, Qt::LeftButton, Qt::NoModifier, QPoint(), 200);

//    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::UnknownType);
    emit DGuiApplicationHelper::instance()->paletteTypeChanged(DGuiApplicationHelper::UnknownType);

    QTest::qWait(500);
    w->close();
}

