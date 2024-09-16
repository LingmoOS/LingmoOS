// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//Self
#include "netif_summary_view_widget.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>
#include "addr_pri.h"

//Qt
#include <QSignalSpy>
#include <QMap>

using namespace common::format;
/***************************************STUB begin*********************************************/

bool stub_NetInfoDetailItemDelegate_isValid()
{
    return true;
}

bool stub_isWireless_true()
{
    return true;
}
/***************************************STUB end**********************************************/

class UT_NetifSummaryViewWidget : public ::testing::Test
{
public:
    UT_NetifSummaryViewWidget() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        static QWidget parent;
        m_tester = new NetifSummaryViewWidget(&parent);
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    NetifSummaryViewWidget *m_tester;
};

TEST_F(UT_NetifSummaryViewWidget, initTest)
{

}

TEST_F(UT_NetifSummaryViewWidget, test_paintEvent_01)
{
    EXPECT_TRUE(!m_tester->grab().isNull());
}

TEST_F(UT_NetifSummaryViewWidget, test_fontChanged_01)
{
    QFont font;
    font.setBold(true);
    m_tester->fontChanged(font);

    EXPECT_TRUE(m_tester->m_font.bold());
}

TEST_F(UT_NetifSummaryViewWidget, test_onModelUpdate_01)
{
    m_tester->onModelUpdate();
}

TEST_F(UT_NetifSummaryViewWidget, test_onNetifItemClicked_01)
{
    QString mac = "1";
    m_tester->onNetifItemClicked(mac);

    EXPECT_EQ(m_tester->m_strCurrentKey, mac);
}

class UT_NetInfoDetailItemDelegate : public ::testing::Test
{
public:
    UT_NetInfoDetailItemDelegate() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new NetInfoDetailItemDelegate;
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    NetInfoDetailItemDelegate *m_tester;
};

TEST_F(UT_NetInfoDetailItemDelegate, initTest)
{

}

TEST_F(UT_NetInfoDetailItemDelegate, test_setFont_01)
{
    QFont font;
    m_tester->setFont(font);
}


TEST_F(UT_NetInfoDetailItemDelegate, test_paint_01)
{
    QPixmap pixmap;
    QPainter painter(&pixmap);
    QStyleOptionViewItem item;
    static QWidget widget;
    item.widget = &widget;
    QModelIndex index;
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_NetInfoDetailItemDelegate_isValid);
    m_tester->paint(&painter, item, index);
}


TEST_F(UT_NetInfoDetailItemDelegate, test_sizeHint_01)
{
    QStyleOptionViewItem item;
    QModelIndex index;
    m_tester->sizeHint(item, index);
}

class UT_NetInfoModel : public ::testing::Test
{
public:
    UT_NetInfoModel() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new NetInfoModel;
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    NetInfoModel *m_tester;
};

TEST_F(UT_NetInfoModel, initTest)
{

}

TEST_F(UT_NetInfoModel, test_refreshNetifInfo_01)
{
    DeviceDB::instance()->netifInfoDB()->update();
    QByteArrayList keys = DeviceDB::instance()->netifInfoDB()->infoDB().keys();
    Stub stub;
    if (keys.size() > 0)
    m_tester->refreshNetifInfo(keys.takeFirst());
}


TEST_F(UT_NetInfoModel, test_refreshNetifInfo_02)
{
    DeviceDB::instance()->netifInfoDB()->update();
    QByteArrayList keys = DeviceDB::instance()->netifInfoDB()->infoDB().keys();
    Stub stub;
    stub.set(ADDR(NetifInfo, isWireless), stub_isWireless_true);
    if (keys.size() > 0)
    m_tester->refreshNetifInfo(keys.takeFirst());
}
