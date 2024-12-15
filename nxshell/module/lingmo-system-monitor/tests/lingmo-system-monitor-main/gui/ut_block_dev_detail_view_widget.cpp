// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//Self
#include "block_dev_detail_view_widget.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>
#include "addr_pri.h"

//Qt
#include <QSignalSpy>
#include <QPainter>

/***************************************STUB begin*********************************************/

/***************************************STUB end**********************************************/

class UT_BlockDevDetailViewWidget : public ::testing::Test
{
public:
    UT_BlockDevDetailViewWidget() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        static QWidget widget;
        m_tester = new BlockDevDetailViewWidget(&widget);
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    BlockDevDetailViewWidget *m_tester;
};

TEST_F(UT_BlockDevDetailViewWidget, initTest)
{

}

TEST_F(UT_BlockDevDetailViewWidget, test_detailFontChanged_01)
{
    QFont font;
    font.setWeight(18);
    m_tester->detailFontChanged(font);
}

