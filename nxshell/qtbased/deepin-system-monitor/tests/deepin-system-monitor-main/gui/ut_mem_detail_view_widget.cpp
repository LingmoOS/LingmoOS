// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//Self
#include "mem_detail_view_widget.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>
#include "addr_pri.h"

//Qt
#include <QSignalSpy>

/***************************************STUB begin*********************************************/

/***************************************STUB end**********************************************/

class UT_MemDetailViewWidget : public ::testing::Test
{
public:
    UT_MemDetailViewWidget() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        static QWidget widget;
        m_tester = new MemDetailViewWidget(&widget);
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    MemDetailViewWidget *m_tester;
};

TEST_F(UT_MemDetailViewWidget, initTest)
{

}

TEST_F(UT_MemDetailViewWidget, test_detailFontChanged_01)
{
    QFont font;
    m_tester->detailFontChanged(font);
}


TEST_F(UT_MemDetailViewWidget, test_onModelUpdate_01)
{
    m_tester->onModelUpdate();
}
