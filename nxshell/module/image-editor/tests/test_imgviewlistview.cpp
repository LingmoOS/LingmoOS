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
#include "viewpanel/contents/imgviewlistview.h"


//view panel
TEST_F(gtestview, imgviewlistview)
{
    LibImgViewListView *widget = new LibImgViewListView();
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

TEST_F(gtestview, imgviewlistview_openNext_NULL)
{
    LibImgViewListView *widget = new LibImgViewListView();
    widget->resize(1090, 1080);
    widget->show();
    widget->openNext();

    widget->deleteLater();
    widget = nullptr;

}

TEST_F(gtestview, imgviewlistview_openpre_NULL)
{
    LibImgViewListView *widget = new LibImgViewListView();
    widget->resize(1090, 1080);
    widget->show();
    widget->openPre();

    widget->deleteLater();
    widget = nullptr;

}

TEST_F(gtestview, LibImgViewDelegate_setItemSize)
{
    LibImgViewDelegate *widget = new LibImgViewDelegate();
    widget->setItemSize(QSize(100,100));

    widget->deleteLater();
    widget = nullptr;

}
