// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DeviceWidget.h"
#include "DeviceInfo.h"
#include "DeviceInput.h"
#include "PageInfoWidget.h"
#include "PageListView.h"
#include "DeviceListView.h"
#include "PageSingleInfo.h"
#include "ut_Head.h"
#include "stub.h"

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>
#include <QSignalSpy>

#include <gtest/gtest.h>

class DeviceWidget_UT : public UT_HEAD
{
public:
    void SetUp()
    {
        m_dWidget = new DeviceWidget;
    }
    void TearDown()
    {
        delete m_dWidget;
    }
    DeviceWidget *m_dWidget;
};

TEST_F(DeviceWidget_UT, ut_updateListView)
{
    QList<QPair<QString, QString>> list;
    list.append(QPair<QString, QString>("/", "/##/"));
    m_dWidget->updateListView(list);
    EXPECT_EQ(1,m_dWidget->mp_ListView->mp_ListView->mp_ItemModel->rowCount());
}

TEST_F(DeviceWidget_UT, ut_updateDevice)
{
    DeviceInput *device = new DeviceInput;
    QMap<QString, QString> mapinfo;
    mapinfo.insert("/", "/");
    device->setInfoFromHwinfo(mapinfo);
    QList<DeviceBaseInfo *> bInfo;
    m_dWidget->mp_PageInfo = new PageInfoWidget(m_dWidget);
    m_dWidget->updateDevice("", bInfo);
    EXPECT_FALSE(m_dWidget->mp_PageInfo->mp_PageSignalInfo->isVisible());
    delete device;
}

TEST_F(DeviceWidget_UT, ut_updateOverview)
{
    QMap<QString, QString> mapinfo;
    mapinfo.insert("/", "/");
    m_dWidget->updateOverview(mapinfo);
    EXPECT_FALSE(m_dWidget->mp_PageInfo->mp_PageSignalInfo->isVisible());
}

TEST_F(DeviceWidget_UT, ut_slotRefreshInfo)
{
    m_dWidget->initWidgets();
    QSignalSpy spy(m_dWidget,SIGNAL(itemClicked(QString)));
    m_dWidget->slotUpdateUI();
    EXPECT_EQ(1,spy.count());
}

TEST_F(DeviceWidget_UT, ut_resizeEvent)
{
    QResizeEvent resizeevent(QSize(10, 10), QSize(10, 10));
    m_dWidget->resizeEvent(&resizeevent);
    EXPECT_FALSE(m_dWidget->mp_ListView->isVisible());
}
