/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     ZhangYong <zhangyong@uniontech.com>
 *
 * Maintainer: ZhangYong <ZhangYong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
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


