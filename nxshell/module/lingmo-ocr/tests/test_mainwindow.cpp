// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <QTestEventList>
#include <QObject>
#include <QStandardPaths>

#define private public
#define protected public

#include "ocrapplication.h"
#include "mainwidget.h"
#include "view/imageview.h"
//初始拉起主界面
TEST(MainWindow, mainwindow)
{
    OcrApplication instance;
    instance.openFile("");
    QTest::qWait(2000);
}

//其余case
//初始拉起主界面,带有参数的
TEST(MainWindow, mainwindow_openFile)
{
    QString picPath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation) + "/ocr_test/testResource/test.png";
    OcrApplication instance;
    instance.openFile(picPath);
    bool bRet = false;
    while (!bRet) {
        if (!PaddleOCRApp::instance()->isRunning()) {
            bRet = true;
            QTest::qWait(1000);
        }
    }

    instance.openImage(QImage(picPath).scaled(200, 200));

    bRet = false;
    while (!bRet) {
        if (!PaddleOCRApp::instance()->isRunning()) {
            bRet = true;
            QTest::qWait(1000);
        }
    }

    instance.openImageAndName(QImage(picPath).scaled(10, 10), picPath);
    QTest::qWait(2000);
}

//初始拉起主界面,带有参数的
TEST(MainWidget, MainWidget_show)
{
    MainWidget *a = new MainWidget();
    DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::LightType;
    a->setIcons(themeType);
    a->loadHtml("");
    a->loadHtml("test");
    a->loadString("");
    a->loadString("test");
    a->slotCopy();
    a->resultEmpty();
    a->deleteLater();
    a = nullptr;
    QTest::qWait(2000);
}

TEST(ImageView, ImageView_show)
{
    QString picPath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation) + "/Wallpapers/luca-micheli-ruWkmt3nU58-unsplash.jpg";
    ImageView *imageView = new ImageView();
    imageView->openImage(picPath);
    imageView->fitImage();
    imageView->fitWindow();
    imageView->autoFit();
    imageView->imageRelativeScale();
    imageView->setScaleValue(1.1);
    imageView->setScaleValue(0.9);
    imageView->RotateImage(90);

    imageView->scaleAtPoint(QPoint(10, 10), 1.2);
    QPinchGesture *gesture = new QPinchGesture;
    imageView->pinchTriggered(gesture);
    delete gesture;

    QTest::qWait(2000);
    delete imageView;
}
