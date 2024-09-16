// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//Self
#include "netif_detail_view_widget.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>
#include "addr_pri.h"

//Qt
#include <QSignalSpy>

/***************************************STUB begin*********************************************/

/***************************************STUB end**********************************************/

class UT_NetifDetailViewWidget : public ::testing::Test
{
public:
    UT_NetifDetailViewWidget() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        static QWidget parent;
        m_tester = new NetifDetailViewWidget(&parent);
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    NetifDetailViewWidget *m_tester;
};

TEST_F(UT_NetifDetailViewWidget, initTest)
{

}

TEST_F(UT_NetifDetailViewWidget, test_detailFontChanged_01)
{
    QFont font;
    font.setBold(true);
    m_tester->detailFontChanged(font);
}

TEST_F(UT_NetifDetailViewWidget, test_updateData_01)
{
    m_tester->updateData();
}
