// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_probitwidget.h"
#include "../../src/widgets/probitwidget.h"


Ut_ProBitWidget::Ut_ProBitWidget()
{

}

TEST_F(Ut_ProBitWidget, setButtonEnabled)
{
    ProBitWidget *m_probitwidget = new ProBitWidget;
    m_probitwidget->setButtonEnabled(true);
    EXPECT_TRUE(m_probitwidget->m_firstbtn->isEnabled());
    EXPECT_TRUE(m_probitwidget->m_secondbtn->isEnabled());
    EXPECT_TRUE(m_probitwidget->m_thirdbtn->isEnabled());
    EXPECT_TRUE(m_probitwidget->m_fourthbtn->isEnabled());
    delete m_probitwidget;
}

TEST_F(Ut_ProBitWidget, getbutton)
{
    ProBitWidget *m_probitwidget = new ProBitWidget;
    EXPECT_EQ(m_probitwidget->getbutton(3), m_probitwidget->m_fourthbtn);
    delete m_probitwidget;
}
