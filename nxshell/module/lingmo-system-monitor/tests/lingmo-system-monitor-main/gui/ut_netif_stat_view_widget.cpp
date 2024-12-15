// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//Self
#include "netif_stat_view_widget.h"
#include "system/device_db.h"
#include "system/netif.h"
#include "system/netif_info_db.h"
#include "netif_item_view_widget.h"
#include "system/net_info.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>
#include "addr_pri.h"

//Qt
#include <QSignalSpy>
#include <QResizeEvent>

/***************************************STUB begin*********************************************/
void stub_onModelUpdate_updateData()
{
    return;
}
const QMap<QByteArray, NetifInfoPtr> stub_updateWidgetGeometry_infoDB_1()
{
    QMap<QByteArray, NetifInfoPtr> map;
    NetifInfoPtr ptr;
    map.insert("1", ptr);
    return map;
}


const QMap<QByteArray, NetifInfoPtr> stub_updateWidgetGeometry_infoDB_2()
{
    QMap<QByteArray, NetifInfoPtr> map;
    NetifInfoPtr ptr;
    map.insert("1", ptr);
    map.insert("2", ptr);
    return map;
}

const QMap<QByteArray, NetifInfoPtr> stub_updateWidgetGeometry_infoDB_3()
{
    QMap<QByteArray, NetifInfoPtr> map;
    NetifInfoPtr ptr;
    map.insert("1", ptr);
    map.insert("2", ptr);
    map.insert("3", ptr);
    return map;
}

const QMap<QByteArray, NetifInfoPtr> stub_updateWidgetGeometry_infoDB_4()
{
    QMap<QByteArray, NetifInfoPtr> map;
    NetifInfoPtr ptr;
    map.insert("1", ptr);
    map.insert("2", ptr);
    map.insert("3", ptr);
    map.insert("4", ptr);
    return map;
}

QMap<QByteArray, NetifInfoPtr> stub_onModelUpdate_infoDB()
{
    QMap<QByteArray, NetifInfoPtr> map;

    std::shared_ptr<core::system::NetifInfo> ptr(new core::system::NetifInfo, [](core::system::NetifInfo *p){delete p;});
    map.insert("1", ptr);
    return map;
}

QMap<QByteArray, NetifInfoPtr> stub_onModelUpdate_infoDB_plus()
{
    QMap<QByteArray, NetifInfoPtr> map;

    std::shared_ptr<core::system::NetifInfo> ptr(new core::system::NetifInfo, [](core::system::NetifInfo *p){delete p;});
    std::shared_ptr<core::system::NetifInfo> ptr2(new core::system::NetifInfo, [](core::system::NetifInfo *p){delete p;});
    map.insert("1", ptr);
    map.insert("2", ptr2);
    return map;
}

/***************************************STUB end**********************************************/

class UT_NetifStatViewWidget : public ::testing::Test
{
public:
    UT_NetifStatViewWidget() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        static QWidget parent;
        m_tester = new NetifStatViewWidget(&parent);
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    NetifStatViewWidget *m_tester;
};

TEST_F(UT_NetifStatViewWidget, initTest)
{

}

TEST_F(UT_NetifStatViewWidget, test_updateWidgetGeometry_01)
{
    Stub stub;
    stub.set(ADDR(core::system::NetifInfoDB, infoDB), stub_updateWidgetGeometry_infoDB_1);
    m_tester->updateWidgetGeometry();
}

TEST_F(UT_NetifStatViewWidget, test_updateWidgetGeometry_02)
{
    Stub stub;
    stub.set(ADDR(core::system::NetifInfoDB, infoDB), stub_updateWidgetGeometry_infoDB_2);
    m_tester->updateWidgetGeometry();
}

TEST_F(UT_NetifStatViewWidget, test_updateWidgetGeometry_03)
{
    Stub stub;
    stub.set(ADDR(core::system::NetifInfoDB, infoDB), stub_updateWidgetGeometry_infoDB_3);
    m_tester->updateWidgetGeometry();
}

TEST_F(UT_NetifStatViewWidget, test_resizeEvent_01)
{
    static QResizeEvent event(QSize(10, 10), QSize(20, 20));
    m_tester->resizeEvent(&event);
}

TEST_F(UT_NetifStatViewWidget, test_onSetItemActiveStatus_01)
{
    std::shared_ptr<NetifItemViewWidget> ptr(new NetifItemViewWidget, [](NetifItemViewWidget *p){delete p;});
    m_tester->m_mapItemView.insert("1", ptr.get());
    Stub stub;
    stub.set(ADDR(core::system::NetifInfoDB, infoDB), stub_onModelUpdate_infoDB);
    m_tester->onSetItemActiveStatus("1");
}

TEST_F(UT_NetifStatViewWidget, test_onSetItemActiveStatus_02)
{
    std::shared_ptr<NetifItemViewWidget> ptr(new NetifItemViewWidget, [](NetifItemViewWidget *p){delete p;});
    m_tester->m_mapItemView.insert("1", ptr.get());
    Stub stub;
    stub.set(ADDR(core::system::NetifInfoDB, infoDB), stub_onModelUpdate_infoDB_plus);
    m_tester->onSetItemActiveStatus("1");
}


TEST_F(UT_NetifStatViewWidget, test_fontChanged_01)
{
    static NetifItemViewWidget view1;
    static NetifItemViewWidget view2;
    static NetifItemViewWidget view3;
    m_tester->m_mapItemView.insert("11", &view1);
    m_tester->m_mapItemView.insert("22", &view2);
    m_tester->m_mapItemView.insert("33", &view3);

    QFont font;
    m_tester->fontChanged(font);
}

TEST_F(UT_NetifStatViewWidget, test_onModelUpdate_01)
{
    std::shared_ptr<NetifItemViewWidget> ptr(new NetifItemViewWidget, [](NetifItemViewWidget *p){delete p;});
    m_tester->m_mapItemView.insert("1", ptr.get());
    Stub stub;
    stub.set(ADDR(core::system::NetifInfoDB, infoDB), stub_onModelUpdate_infoDB);
//    stub.set(ADDR(NetifItemViewWidget, updateData), stub_onModelUpdate_updateData);
    m_tester->onModelUpdate();
}

TEST_F(UT_NetifStatViewWidget, test_onModelUpdate_02)
{
    std::shared_ptr<NetifItemViewWidget> ptr(new NetifItemViewWidget, [](NetifItemViewWidget *p){delete p;});
    m_tester->m_mapItemView.insert("2", ptr.get());
    Stub stub;
    stub.set(ADDR(core::system::NetifInfoDB, infoDB), stub_onModelUpdate_infoDB);
//    stub.set(ADDR(NetifItemViewWidget, updateData), stub_onModelUpdate_updateData);
    m_tester->onModelUpdate();
}

TEST_F(UT_NetifStatViewWidget, test_showItemOnlyeOne_01)
{
    Stub stub;
    stub.set(ADDR(core::system::NetifInfoDB, infoDB), stub_updateWidgetGeometry_infoDB_3);

    NetifItemViewWidget wid;
    m_tester->m_mapItemView.insert("1", &wid);
    m_tester->showItemOnlyeOne();
}


TEST_F(UT_NetifStatViewWidget, test_showItemOnlyeOne_02)
{
    Stub stub;
    stub.set(ADDR(core::system::NetifInfoDB, infoDB), stub_updateWidgetGeometry_infoDB_3);

    NetifItemViewWidget wid;
    m_tester->m_mapItemView.insert("4", &wid);
    m_tester->showItemOnlyeOne();
}


TEST_F(UT_NetifStatViewWidget, test_showItemDouble_01)
{
    Stub stub;
    stub.set(ADDR(core::system::NetifInfoDB, infoDB), stub_updateWidgetGeometry_infoDB_3);

    static QWidget tmpWidget;
    NetifItemViewWidget wid(&tmpWidget);
    m_tester->m_mapItemView.insert("1", &wid);
    m_tester->showItemDouble();
}

TEST_F(UT_NetifStatViewWidget, test_showItemDouble_02)
{
    m_tester->m_currentMac = "1";
    Stub stub;
    stub.set(ADDR(core::system::NetifInfoDB, infoDB), stub_updateWidgetGeometry_infoDB_3);

    static QWidget tmpWidget;
    NetifItemViewWidget wid(&tmpWidget);
    m_tester->m_mapItemView.insert("1", &wid);
    m_tester->showItemDouble();
}


TEST_F(UT_NetifStatViewWidget, test_showItemDouble_03)
{
    Stub stub;
    stub.set(ADDR(core::system::NetifInfoDB, infoDB), stub_updateWidgetGeometry_infoDB_3);

    static QWidget tmpWidget;
    NetifItemViewWidget wid(&tmpWidget);
    m_tester->m_mapItemView.insert("4", &wid);
    m_tester->showItemDouble();
}

TEST_F(UT_NetifStatViewWidget, test_showItemLgDouble_01)
{
    Stub stub;
    stub.set(ADDR(core::system::NetifInfoDB, infoDB), stub_updateWidgetGeometry_infoDB_4);

    static QWidget tmpWidget;
    NetifItemViewWidget wid(&tmpWidget);
    m_tester->m_mapItemView.insert("1", &wid);
    m_tester->m_mapItemView.insert("2", &wid);
    m_tester->m_mapItemView.insert("3", &wid);
    m_tester->m_mapItemView.insert("4", &wid);
    m_tester->showItemLgDouble();
}

TEST_F(UT_NetifStatViewWidget, test_showItemLgDouble_02)
{
    Stub stub;
    stub.set(ADDR(core::system::NetifInfoDB, infoDB), stub_updateWidgetGeometry_infoDB_4);

    static QWidget tmpWidget;
    NetifItemViewWidget wid(&tmpWidget);
    m_tester->m_mapItemView.insert("5", &wid);
    m_tester->showItemLgDouble();
}

TEST_F(UT_NetifStatViewWidget, test_showItemLgDouble_03)
{
    m_tester->m_currentMac = "1";
    Stub stub;
    stub.set(ADDR(core::system::NetifInfoDB, infoDB), stub_updateWidgetGeometry_infoDB_4);

    static QWidget tmpWidget;
    NetifItemViewWidget wid(&tmpWidget);
    m_tester->m_mapItemView.insert("1", &wid);
    m_tester->m_mapItemView.insert("2", &wid);
    m_tester->m_mapItemView.insert("3", &wid);
    m_tester->m_mapItemView.insert("4", &wid);
    m_tester->showItemLgDouble();
}


















