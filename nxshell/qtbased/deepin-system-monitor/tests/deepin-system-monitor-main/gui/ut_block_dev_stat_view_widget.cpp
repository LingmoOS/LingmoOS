// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//Self
#include "block_dev_stat_view_widget.h"
#include "system/block_device_info_db.h"
#include "block_dev_item_widget.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>
#include "addr_pri.h"

//Qt
#include <QSignalSpy>
#include <QResizeEvent>


/***************************************STUB begin*********************************************/

void stub_updateWidgetGeometry_showItem1()
{
    return;
}

void stub_updateWidgetGeometry_showItem2()
{
    return;
}

void stub_updateWidgetGeometry_showItemLg2()
{
    return;
}

QList<BlockDevice> stub_onUpdateData_deviceList()
{
    QList<BlockDevice> listDB;
    BlockDevice db1;
    BlockDevice db2;
    listDB.append(db1);
    listDB.append(db2);
    return  listDB;
}

/***************************************STUB end**********************************************/

class UT_BlockStatViewWidget : public ::testing::Test
{
public:
    UT_BlockStatViewWidget() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new BlockStatViewWidget();
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    BlockStatViewWidget *m_tester;
};

TEST_F(UT_BlockStatViewWidget, initTest)
{

}

TEST_F(UT_BlockStatViewWidget, test_resizeEvent_01)
{
    static QResizeEvent re(QSize(10, 10), QSize(20, 20));
    m_tester->resizeEvent(&re);
}

TEST_F(UT_BlockStatViewWidget, test_updateWidgetGeometry_01)
{
    BlockDevice blockDev1;
    m_tester->m_listDevice.append(blockDev1);

    Stub stub;
    stub.set(ADDR(BlockStatViewWidget, showItem1), stub_updateWidgetGeometry_showItem1);
    m_tester->updateWidgetGeometry();
    EXPECT_EQ(m_tester->m_listDevice.size(), 1);
}


TEST_F(UT_BlockStatViewWidget, test_updateWidgetGeometry_02)
{
    BlockDevice blockDev1;
    BlockDevice blockDev2;
    m_tester->m_listDevice.append(blockDev1);
    m_tester->m_listDevice.append(blockDev2);

    Stub stub;
    stub.set(ADDR(BlockStatViewWidget, showItem2), stub_updateWidgetGeometry_showItem2);
    m_tester->updateWidgetGeometry();
    EXPECT_EQ(m_tester->m_listDevice.size(), 2);
}

TEST_F(UT_BlockStatViewWidget, test_updateWidgetGeometry_03)
{
    BlockDevice blockDev1;
    BlockDevice blockDev2;
    BlockDevice blockDev3;
    m_tester->m_listDevice.append(blockDev1);
    m_tester->m_listDevice.append(blockDev2);
    m_tester->m_listDevice.append(blockDev3);

    Stub stub;
    stub.set(ADDR(BlockStatViewWidget, showItemLg2), stub_updateWidgetGeometry_showItemLg2);
    m_tester->updateWidgetGeometry();
    EXPECT_EQ(m_tester->m_listDevice.size(), 3);
}

TEST_F(UT_BlockStatViewWidget, test_onUpdateData_01)
{

    Stub stub;
    stub.set(ADDR(BlockDeviceInfoDB, deviceList), stub_onUpdateData_deviceList);
    m_tester->onUpdateData();
}

TEST_F(UT_BlockStatViewWidget, test_fontChanged_01)
{
    QFont font;
    font.setBold(true);
    BlockDevItemWidget widget;
    m_tester->m_listBlockItemWidget.append(&widget);

    m_tester->fontChanged(font);

    if (m_tester->m_listBlockItemWidget.size() > 0)
        EXPECT_EQ(m_tester->m_listBlockItemWidget[0]->m_font.bold(), font.bold());
}

TEST_F(UT_BlockStatViewWidget, test_onSetItemStatus_01)
{
    BlockDevItemWidget widget1;
    BlockDevItemWidget widget2;
    m_tester->m_mapDeviceItemWidget.insert(QString("item1"), &widget1);
    m_tester->m_mapDeviceItemWidget.insert(QString("item2"), &widget2);
    m_tester->onSetItemStatus(QString("item1"));

    QSignalSpy signalSpy(m_tester, &BlockStatViewWidget::changeInfo);
    EXPECT_TRUE(signalSpy.count() == 0);

    emit m_tester->changeInfo("item1");
    signalSpy.wait(50);
    EXPECT_TRUE(signalSpy.count() == 1);
}

TEST_F(UT_BlockStatViewWidget, test_showItem1_01)
{
    BlockDevItemWidget widget;
    m_tester->m_listBlockItemWidget.append(&widget);

    BlockDevice blockDev1;
    BlockDevice blockDev2;
    m_tester->m_listDevice.append(blockDev1);
    m_tester->m_listDevice.append(blockDev2);
    m_tester->showItem1();

    QSignalSpy signalSpy(m_tester, &BlockStatViewWidget::changeInfo);
    EXPECT_TRUE(signalSpy.count() == 0);

    emit m_tester->changeInfo("item1");
    signalSpy.wait(50);
    EXPECT_TRUE(signalSpy.count() == 1);
}

TEST_F(UT_BlockStatViewWidget, test_showItem2_01)
{
    BlockDevItemWidget widget1;
    BlockDevItemWidget widget2;
    m_tester->m_listBlockItemWidget.append(&widget1);
    m_tester->m_listBlockItemWidget.append(&widget2);

    BlockDevice blockDev1;
    BlockDevice blockDev2;
    m_tester->m_listDevice.append(blockDev1);
    m_tester->m_listDevice.append(blockDev2);

    m_tester->showItem2();

    QSignalSpy signalSpy(m_tester, &BlockStatViewWidget::changeInfo);
    EXPECT_TRUE(signalSpy.count() == 0);

    emit m_tester->changeInfo("item1");
    signalSpy.wait(50);
    EXPECT_TRUE(signalSpy.count() == 1);
}

TEST_F(UT_BlockStatViewWidget, test_showItemLg2_01)
{
    BlockDevItemWidget widget1;
    BlockDevItemWidget widget2;
    BlockDevItemWidget widget3;
    BlockDevItemWidget widget4;
    m_tester->m_listBlockItemWidget.append(&widget1);
    m_tester->m_listBlockItemWidget.append(&widget2);
    m_tester->m_listBlockItemWidget.append(&widget3);
    m_tester->m_listBlockItemWidget.append(&widget4);

    BlockDevice blockDev1;
    BlockDevice blockDev2;
    BlockDevice blockDev3;
    BlockDevice blockDev4;
    m_tester->m_listDevice.append(blockDev1);
    m_tester->m_listDevice.append(blockDev2);
    m_tester->m_listDevice.append(blockDev3);
    m_tester->m_listDevice.append(blockDev4);

    m_tester->showItemLg2(4);

    QSignalSpy signalSpy(m_tester, &BlockStatViewWidget::changeInfo);
    EXPECT_TRUE(signalSpy.count() == 0);

    emit m_tester->changeInfo("item1");
    signalSpy.wait(50);
    EXPECT_TRUE(signalSpy.count() == 1);
}

TEST_F(UT_BlockStatViewWidget, test_showItemLg2_02)
{
    BlockDevItemWidget widget1;
    BlockDevItemWidget widget2;
    BlockDevItemWidget widget3;
    BlockDevItemWidget widget4;
    widget4.activeItemWidget(true);
    m_tester->m_listBlockItemWidget.append(&widget1);
    m_tester->m_listBlockItemWidget.append(&widget2);
    m_tester->m_listBlockItemWidget.append(&widget3);
    m_tester->m_listBlockItemWidget.append(&widget4);

    BlockDevice blockDev1;
    BlockDevice blockDev2;
    BlockDevice blockDev3;
    BlockDevice blockDev4;
    m_tester->m_listDevice.append(blockDev1);
    m_tester->m_listDevice.append(blockDev2);
    m_tester->m_listDevice.append(blockDev3);
    m_tester->m_listDevice.append(blockDev4);

    m_tester->showItemLg2(4);

    QSignalSpy signalSpy(m_tester, &BlockStatViewWidget::changeInfo);
    EXPECT_TRUE(signalSpy.count() == 0);

    emit m_tester->changeInfo("item1");
    signalSpy.wait(50);
    EXPECT_TRUE(signalSpy.count() == 1);
}

TEST_F(UT_BlockStatViewWidget, test_showItemLg2_03)
{
    BlockDevItemWidget widget1;
    BlockDevItemWidget widget2;
    BlockDevItemWidget widget3;
    BlockDevItemWidget widget4;
    m_tester->m_listBlockItemWidget.append(&widget1);
    m_tester->m_listBlockItemWidget.append(&widget2);
    m_tester->m_listBlockItemWidget.append(&widget3);
    m_tester->m_listBlockItemWidget.append(&widget4);

    BlockDevice blockDev1;
    BlockDevice blockDev2;
    BlockDevice blockDev3;
    BlockDevice blockDev4;
    m_tester->m_listDevice.append(blockDev1);
    m_tester->m_listDevice.append(blockDev2);
    m_tester->m_listDevice.append(blockDev3);
    m_tester->m_listDevice.append(blockDev4);

    m_tester->showItemLg2(3);

    QSignalSpy signalSpy(m_tester, &BlockStatViewWidget::changeInfo);
    EXPECT_TRUE(signalSpy.count() == 0);

    emit m_tester->changeInfo("item1");
    signalSpy.wait(50);
    EXPECT_TRUE(signalSpy.count() == 1);
}
