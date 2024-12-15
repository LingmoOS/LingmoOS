// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gtestview.h"
#include "accessibility/ac-desktop-define.h"
#include <DGuiApplicationHelper>
#include "imageviewer.h"
#include "imageengine.h"
#include "service/imagedataservice.h"
#include "viewpanel/navigationwidget.h"
#include "service/commonservice.h"
#define  private public
#include "viewpanel/contents/imgviewwidget.h"


//view panel
TEST_F(gtestview, imgviewwidget)
{
    MyImageListWidget *widget = new MyImageListWidget();
    widget->resize(1090, 1080);
    widget->show();
    QTest::mouseMove(widget, QPoint(1000, 1075), 200);
    QTest::mouseMove(widget, QPoint(200, 500), 200);
    QTest::mousePress(widget,Qt::LeftButton, Qt::NoModifier, QPoint(200, 400), 200);
    QTest::mouseMove(widget, QPoint(20, 20), 200);
    QTest::mouseRelease(widget,Qt::LeftButton, Qt::NoModifier, QPoint(200, 400), 200);
    QTest::mouseDClick(widget, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20), 200);

    widget->deleteLater();
    widget = nullptr;

}

TEST_F(gtestview, imgviewwidget_sigRightMousePress)
{
    MyImageListWidget *widget = new MyImageListWidget();
    widget->resize(1090, 1080);
    widget->show();
    QTest::qWait(200);
    LibCommonService::instance()->sigRightMousePress();
    QTest::qWait(200);
    LibCommonService::instance()->sigRightMousePress();
    widget->deleteLater();
    widget = nullptr;

}

TEST_F(gtestview, imgviewwidget_getCurrentImgInfo)
{
    MyImageListWidget *widget = new MyImageListWidget();
    widget->resize(1090, 1080);
    widget->show();
    widget->getCurrentImgInfo();
    widget->deleteLater();
    widget = nullptr;

}

TEST_F(gtestview, imgviewwidget_animationValueChanged)
{
    MyImageListWidget *widget = new MyImageListWidget();
    widget->resize(1090, 1080);
    widget->show();
    widget->animationValueChanged(400);
    widget->deleteLater();
    widget = nullptr;

}

TEST_F(gtestview, imgviewwidget_animationStart)
{
    MyImageListWidget *widget = new MyImageListWidget();
    widget->resize(1090, 1080);
    widget->show();
    widget->animationStart(true,200,500);
    QTest::qWait(200);
    widget->deleteLater();
    widget = nullptr;

}
TEST_F(gtestview, imgviewwidget_animationStart_400)
{
    MyImageListWidget *widget = new MyImageListWidget();
    widget->resize(1090, 1080);
    widget->show();
    widget->animationStart(true,200,400);
    QTest::qWait(200);
    widget->deleteLater();
    widget = nullptr;

}

TEST_F(gtestview, imgviewwidget_thumbnailIsMoving)
{
    MyImageListWidget *widget = new MyImageListWidget();
    widget->resize(1090, 1080);
    widget->show();
    QTest::qWait(800);
    widget->thumbnailIsMoving();
    QTest::qWait(800);
    widget->deleteLater();
    widget = nullptr;

}

TEST_F(gtestview, imgviewwidget_moveCenterWidget)
{
    MyImageListWidget *widget = new MyImageListWidget();
    widget->resize(1090, 1080);
    widget->show();
    QTest::qWait(800);
    widget->moveCenterWidget();
    QTest::qWait(800);
    widget->deleteLater();
    widget = nullptr;

}
