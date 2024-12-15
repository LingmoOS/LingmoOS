// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gtestview.h"
#include "slideshow/slideshowpanel.h"

TEST_F(gtestview, slider_test)
{
    QStringList list;
    list << QApplication::applicationDirPath() + "/gif.gif";
    list << QApplication::applicationDirPath() + "/gif2.gif";
    list << QApplication::applicationDirPath() + "/ico.ico";
    list << QApplication::applicationDirPath() + "/jpg.jpg";
    list << QApplication::applicationDirPath() + "/png.png";
    list << QApplication::applicationDirPath() + "/mng.mng";
    list << QApplication::applicationDirPath() + "/svg1.svg";
    list << QApplication::applicationDirPath() + "/svg2.svg";
    list << QApplication::applicationDirPath() + "/svg3.svg";
    QString path = QApplication::applicationDirPath() + "/gif.gif";

    QWidget *widgetParent = new QWidget();
    ViewInfo info;
    info.fullScreen = false;
    info.lastPanel = widgetParent;
    info.path = path;
    info.paths = list;
    info.viewMainWindowID = 0;

    LibSlideShowPanel *panel = new LibSlideShowPanel();
    panel->resize(1280, 1080);
    panel->showNormal();
    panel->showFullScreen();

    panel->startSlideShow(info);
    QTest::qWait(40000);
    info.paths = QStringList(path);

    panel->startSlideShow(info);

    QTest::mouseMove(panel, QPoint(1000, 1075), 1000);
    QTest::mouseMove(panel, QPoint(200, 500), 1000);
    QTest::mouseMove(panel, QPoint(20, 20), 1000);
    QTest::mouseDClick(panel, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20), 1000);

    QAction menuAction;

    menuAction.setProperty("MenuID", LibSlideShowPanel::IdPlay);
    panel->onMenuItemClicked(&menuAction);
    QTest::qWait(400);

    menuAction.setProperty("MenuID", LibSlideShowPanel:: IdPause);
    panel->onMenuItemClicked(&menuAction);
    QTest::qWait(400);

    menuAction.setProperty("MenuID", LibSlideShowPanel::IdPlayOrPause);
    panel->onMenuItemClicked(&menuAction);
    QTest::qWait(400);


    menuAction.setProperty("MenuID", LibSlideShowPanel::IdStopslideshow);
    panel->onMenuItemClicked(&menuAction);
    QTest::qWait(400);

    panel->onSingleAnimationEnd();
    panel->onESCKeyStopSlide();
    panel->onShowContinue();
    panel->onShowPrevious();
    panel->onShowNext();
    panel->onCustomContextMenuRequested();

    panel->slideshowbottombar->showContinue();
    panel->slideshowbottombar->showNext();
    panel->slideshowbottombar->showPrevious();
    panel->slideshowbottombar->showCancel();
    panel->slideshowbottombar->showPause();

    panel->deleteLater();
    panel = nullptr;

    widgetParent->deleteLater();
    widgetParent = nullptr;
}
TEST_F(gtestview, SlideShowBottomBar_test)
{
    SlideShowBottomBar *bar = new SlideShowBottomBar();

    bar->onPreButtonClicked();
    bar->isStop = true;
    bar->onPlaypauseButtonClicked();
    bar->isStop = false;
    bar->onPlaypauseButtonClicked();
    bar->onUpdatePauseButton();
    bar->onInitSlideShowButton();
    bar->onNextButtonClicked();
    bar->onCancelButtonClicked();

    bar->deleteLater();
    bar = nullptr;
}
