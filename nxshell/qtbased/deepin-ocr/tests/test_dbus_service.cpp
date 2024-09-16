// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <QTestEventList>
#include <QObject>
#include <QStandardPaths>
#include <QDBusConnection>

#define private public
#define protected public

#include "ocrapplication.h"
#include "service/dbusocr_adaptor.h"
#include "service/ocrinterface.h"
//初始拉起主界面
TEST(OcrInterface, mainwindow)
{
    OcrInterface *pOcr = new OcrInterface("com.deepin.Ocr", "/com/deepin/Ocr", QDBusConnection::sessionBus());
    qDebug() << __FUNCTION__ << __LINE__;
    QString imagePath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation) + "/Wallpapers/marian-kroell-qElMHWePpok-unsplash.jpg";;
    pOcr->openFile(imagePath);
    QTest::qWait(2000);
    QImage img = QImage(imagePath);
    pOcr->openImage(img);
    QTest::qWait(2000);
    pOcr->openImageAndName(img, "TestName");
    QTest::qWait(2000);
    delete pOcr;

}


TEST(DbusOcrAdaptor, MainWidget_show)
{
    //DbusOcrAdaptor *adaptor = new DbusOcrAdaptor(nullptr);
    //delete adaptor;
}


