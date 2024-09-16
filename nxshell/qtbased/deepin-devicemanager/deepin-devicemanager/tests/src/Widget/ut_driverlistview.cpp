// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DriverListView.h"
#include "ut_Head.h"
#include "stub.h"

#include <DStyle>
#include <DApplication>

#include <QPaintEvent>
#include <QRect>
#include <QPainter>
#include <QStandardItem>
#include <QFileInfo>
#include <QFileIconProvider>
#include <QSizePolicy>

#include <gtest/gtest.h>

DStyle *dl_style;

class UT_DriverListView : public UT_HEAD
{
public:
    void SetUp()
    {
        m_DriverListView = new DriverListView;
        dl_style = new DStyle;
    }
    void TearDown()
    {
        delete m_DriverListView;
        delete dl_style;
    }

    DriverListView *m_DriverListView;
};

TEST_F(UT_DriverListView, UT_DriverListView_initUI)
{
    m_DriverListView->initUI();

    EXPECT_EQ(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding), m_DriverListView->sizePolicy());
}

TEST_F(UT_DriverListView, UT_DriverListView_paintEvent)
{
    QPaintEvent event(QRect(m_DriverListView->rect()));
    m_DriverListView->paintEvent(&event);
}

TEST_F(UT_DriverListView, UT_DriverListView_keyPressEvent_001)
{
    QKeyEvent keyPressEvent(QEvent::KeyPress, Qt::Key_Space, Qt::ControlModifier);
    EXPECT_TRUE(m_DriverListView->event(&keyPressEvent));
}

TEST_F(UT_DriverListView, UT_DriverListView_keyPressEvent_002)
{
    QStandardItem *icomItem = new QStandardItem;
    QString path = "/data/home/jixiaomei/.local/share/Trash/files/deepin-devicemanager_5.6.12.13-1_arm64.deb";
    QFileInfo info(path);
    QFileIconProvider icon_provider;
    QIcon icon = icon_provider.icon(info);
    icomItem->setData(icon, Qt::DecorationRole);
    icomItem->setData(QVariant::fromValue(path), Qt::UserRole);
    QStandardItem *textItem = new QStandardItem("deepin-devicemanager_5.6.12.13-1_arm64.deb");
    QStandardItemModel *model = new QStandardItemModel();
    m_DriverListView->setModel(model);
    model->setItem(0, 0, icomItem);
    model->setItem(0, 1, textItem);

    QKeyEvent keyPressEvent(QEvent::KeyPress, Qt::Key_End, Qt::NoModifier);
    m_DriverListView->keyPressEvent(&keyPressEvent);
    EXPECT_EQ(0, m_DriverListView->currentIndex().row());

    delete icomItem;
    delete textItem;
    delete model;
}

TEST_F(UT_DriverListView, UT_DriverListView_keyPressEvent_003)
{
    QStandardItem *icomItem = new QStandardItem;
    QString path = "/data/home/jixiaomei/.local/share/Trash/files/deepin-devicemanager_5.6.12.13-1_arm64.deb";
    QFileInfo info(path);
    QFileIconProvider icon_provider;
    QIcon icon = icon_provider.icon(info);
    icomItem->setData(icon, Qt::DecorationRole);
    icomItem->setData(QVariant::fromValue(path), Qt::UserRole);
    QStandardItem *textItem = new QStandardItem("deepin-devicemanager_5.6.12.13-1_arm64.deb");
    QStandardItemModel *model = new QStandardItemModel();
    m_DriverListView->setModel(model);
    model->setItem(0, 0, icomItem);
    model->setItem(0, 1, textItem);

    QKeyEvent keyPressEvent(QEvent::KeyPress, Qt::Key_Home, Qt::NoModifier);
    m_DriverListView->keyPressEvent(&keyPressEvent);
    EXPECT_EQ(0, m_DriverListView->currentIndex().row());

    delete icomItem;
    delete textItem;
    delete model;
}

int ut_driverlistview_pixelMetric()
{
    return 20;
}

DStyle *ut_driverlistview_style()
{
    return dl_style;
}

TEST_F(UT_DriverListView, UT_DriverListView_drawRow)
{
    QStandardItem *icomItem = new QStandardItem;
    QString path = "/data/home/jixiaomei/.local/share/Trash/files/deepin-devicemanager_5.6.12.13-1_arm64.deb";
    QFileInfo info(path);
    QFileIconProvider icon_provider;
    QIcon icon = icon_provider.icon(info);
    icomItem->setData(icon, Qt::DecorationRole);
    icomItem->setData(QVariant::fromValue(path), Qt::UserRole);
    QStandardItem *textItem = new QStandardItem("deepin-devicemanager_5.6.12.13-1_arm64.deb");
    QStandardItemModel *model = new QStandardItemModel();
    m_DriverListView->setModel(model);
    model->setItem(0, 0, icomItem);
    model->setItem(0, 1, textItem);

    Stub stub;
    stub.set((int (DStyle::*)(DStyle::PixelMetric, const QStyleOption *, const QWidget * widget) const)ADDR(DStyle, pixelMetric), ut_driverlistview_pixelMetric);
    stub.set(ADDR(DApplication, style), ut_driverlistview_style);

    QStyleOptionViewItem option;
    QPainter painter(m_DriverListView);
    QModelIndex index = m_DriverListView->model()->index(0, 0);
    m_DriverListView->drawRow(&painter, option, index);
    EXPECT_FALSE(m_DriverListView->grab().isNull());

    delete icomItem;
    delete textItem;
    delete model;
}
