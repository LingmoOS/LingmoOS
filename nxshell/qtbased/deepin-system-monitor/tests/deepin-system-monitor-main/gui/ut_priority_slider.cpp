// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//Self
#include "priority_slider.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>
#include "addr_pri.h"

//Qt
#include <QSignalSpy>
#include <QResizeEvent>

/***************************************STUB begin*********************************************/


/***************************************STUB end**********************************************/

class UT_PrioritySlider : public ::testing::Test
{
public:
    UT_PrioritySlider() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        static QWidget wid;
        m_tester = new PrioritySlider(Qt::Horizontal, &wid);
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    PrioritySlider *m_tester;
};

TEST_F(UT_PrioritySlider, initTest)
{

}

TEST_F(UT_PrioritySlider, test_resizeEvent_01)
{
    QResizeEvent ev(QSize(10, 10), QSize(20, 20));
    m_tester->resizeEvent(&ev);
}
