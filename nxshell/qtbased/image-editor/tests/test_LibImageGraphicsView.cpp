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
#define  private public
#include "viewpanel/scen/imagegraphicsview.h"
#include "viewpanel/scen/imagesvgitem.h"


//view panel
TEST_F(gtestview, imagegraphicsview)
{
    LibImageGraphicsView *widget = new LibImageGraphicsView(nullptr);

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

TEST_F(gtestview, imagegraphicsview_slotsUpNull)
{
    LibImageGraphicsView *widget = new LibImageGraphicsView(nullptr);

    widget->resize(1090, 1080);
    widget->show();
    widget->slotsUp();

    widget->deleteLater();
    widget = nullptr;

}

TEST_F(gtestview, imagegraphicsview_slotsDownNull)
{
    LibImageGraphicsView *widget = new LibImageGraphicsView(nullptr);

    widget->resize(1090, 1080);
    widget->show();
    widget->slotsDown();


    widget->deleteLater();
    widget = nullptr;

}

TEST_F(gtestview, imagegraphicsview_slotsUpNormal)
{
    LibImageGraphicsView *widget = new LibImageGraphicsView(nullptr);

    widget->resize(1090, 1080);
    widget->show();
    widget->slotsDown();
    QString path= QApplication::applicationDirPath() + "/tif.tif";
    widget->setImage(path);

    widget->slotsUp();
    widget->slotsUp();
    widget->slotsUp();
    widget->slotsUp();

    widget->deleteLater();
    widget = nullptr;

}

TEST_F(gtestview, imagegraphicsview_slotsDownNormal)
{
    LibImageGraphicsView *widget = new LibImageGraphicsView(nullptr);

    widget->resize(1090, 1080);
    widget->show();
    widget->slotsDown();
    QString path= QApplication::applicationDirPath() + "/tif.tif";
    widget->setImage(path);

    widget->slotsDown();
    widget->slotsDown();
    widget->slotsDown();
    widget->slotsDown();

    widget->deleteLater();
    widget = nullptr;

}

TEST_F(gtestview, imagegraphicsview_visibleImageRect)
{
    LibImageGraphicsView *widget = new LibImageGraphicsView(nullptr);

    widget->resize(1090, 1080);
    widget->show();
    QString path= QApplication::applicationDirPath() + "/tif.tif";
    widget->setImage(path);
    widget->visibleImageRect();
    widget->deleteLater();
    widget = nullptr;

}

TEST_F(gtestview, imagegraphicsview_isFitImage)
{
    LibImageGraphicsView *widget = new LibImageGraphicsView(nullptr);

    widget->resize(1090, 1080);
    widget->show();
    QString path= QApplication::applicationDirPath() + "/tif.tif";
    widget->setImage(path);
    widget->isFitImage();
    widget->deleteLater();
    widget = nullptr;

}

TEST_F(gtestview, imagegraphicsview_isWholeImageVisible)
{
    LibImageGraphicsView *widget = new LibImageGraphicsView(nullptr);

    widget->resize(1090, 1080);
    widget->show();
    QString path= QApplication::applicationDirPath() + "/tif.tif";
    widget->setImage(path);
    widget->isWholeImageVisible();
    widget->deleteLater();
    widget = nullptr;

}

TEST_F(gtestview, imagegraphicsview_isFitWindow)
{
    LibImageGraphicsView *widget = new LibImageGraphicsView(nullptr);

    widget->resize(1090, 1080);
    widget->show();
    QString path= QApplication::applicationDirPath() + "/tif.tif";
    widget->setImage(path);
    widget->isFitWindow();
    widget->deleteLater();
    widget = nullptr;

}

TEST_F(gtestview, imagegraphicsview_slotRotatePixmap)
{
    LibImageGraphicsView *widget = new LibImageGraphicsView(nullptr);

    widget->resize(1090, 1080);
    widget->show();
    QString path= QApplication::applicationDirPath() + "/jpg.jpg";
    widget->setImage(path);
    widget->slotRotatePixmap(90);
    QTest::qWait(1500);
    widget->deleteLater();
    widget = nullptr;

}

TEST_F(gtestview, imagegraphicsview_slotRotatePixCurrentNull)
{
    LibImageGraphicsView *widget = new LibImageGraphicsView(nullptr);

    widget->resize(1090, 1080);
    widget->show();
    QString path= QApplication::applicationDirPath() + "/jpg.jpg";
    widget->setImage(path);

    widget->slotRotatePixCurrent();
    QTest::qWait(1500);
    widget->deleteLater();
    widget = nullptr;

}


TEST_F(gtestview, imagegraphicsview_slotRotatePixCurrentNormal)
{
    LibImageGraphicsView *widget = new LibImageGraphicsView(nullptr);

    widget->resize(1090, 1080);
    widget->show();
   QString path= QApplication::applicationDirPath() + "/jpg.jpg";
    widget->setImage(path);
    widget->m_rotateAngel=90;
    widget->slotRotatePixCurrent();
    QTest::qWait(1500);
    widget->deleteLater();
    widget = nullptr;

}

TEST_F(gtestview, imagegraphicsview_mouseDoubleClickEvent)
{
    LibImageGraphicsView *widget = new LibImageGraphicsView(nullptr);

    widget->resize(1090, 1080);
    widget->show();
   QString path= QApplication::applicationDirPath() + "/jpg.jpg";
    widget->setImage(path);
    QTest::mouseDClick(widget, Qt::LeftButton, Qt::NoModifier, QPoint(400, 400), 200);
    QTest::qWait(1500);
    widget->deleteLater();
    widget = nullptr;

}


TEST_F(gtestview, imagegraphicsview_OnFinishPinchAnimal)
{
    LibImageGraphicsView *widget = new LibImageGraphicsView(nullptr);

    widget->resize(1090, 1080);
    widget->show();
   QString path= QApplication::applicationDirPath() + "/jpg.jpg";
    widget->setImage(path);
    widget->OnFinishPinchAnimal();

    widget->deleteLater();
    widget = nullptr;

}


TEST_F(gtestview, LibImageSvgItem_type)
{
    LibImageSvgItem *Item = new LibImageSvgItem(QApplication::applicationDirPath() + "/svg.svg");

    Item->show();
    Item->type();

    Item->deleteLater();
    Item = nullptr;

}

TEST_F(gtestview, LibImageSvgItem_updateDefaultSize)
{
    LibImageSvgItem *Item = new LibImageSvgItem(QApplication::applicationDirPath() + "/svg.svg");

    Item->show();
    Item->updateDefaultSize();

    Item->deleteLater();
    Item = nullptr;

}

TEST_F(gtestview, LibImageSvgItem_setElementId)
{
    LibImageSvgItem *Item = new LibImageSvgItem(QApplication::applicationDirPath() + "/svg.svg");

    Item->show();
    Item->setElementId("id");

    Item->deleteLater();
    Item = nullptr;

}

TEST_F(gtestview, LibImageSvgItem_elementId)
{
    LibImageSvgItem *Item = new LibImageSvgItem(QApplication::applicationDirPath() + "/svg.svg");

    Item->show();
    Item->elementId();

    Item->deleteLater();
    Item = nullptr;

}

TEST_F(gtestview, LibImageSvgItem_isCachingEnabled)
{
    LibImageSvgItem *Item = new LibImageSvgItem(QApplication::applicationDirPath() + "/svg.svg");

    Item->show();
    Item->isCachingEnabled();

    Item->deleteLater();
    Item = nullptr;

}

