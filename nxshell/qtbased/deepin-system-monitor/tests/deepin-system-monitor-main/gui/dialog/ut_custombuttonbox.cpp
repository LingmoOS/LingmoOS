// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//Self
#include "dialog/custombuttonbox.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>
#include "addr_pri.h"

//Qt
#include <QSignalSpy>
#include <QFocusEvent>
#include <QPushButton>

/***************************************STUB begin*********************************************/

QPushButton* stub_focusInEvent_button()
{
    static QPushButton btn;
    return &btn;
}

/***************************************STUB end**********************************************/


class UT_CustomButtonBox : public ::testing::Test
{
public:
    UT_CustomButtonBox() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        static QWidget wid;
        m_tester = new CustomButtonBox(&wid);
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    CustomButtonBox *m_tester;
};

TEST_F(UT_CustomButtonBox, initTest)
{

}


TEST_F(UT_CustomButtonBox, test_focusInEvent_01)
{
    Stub stub;
    stub.set(ADDR(DButtonBox, button), stub_focusInEvent_button);
    static QFocusEvent ev(QEvent::FocusIn);

    m_tester->focusInEvent(&ev);
}
