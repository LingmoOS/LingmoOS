// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_codeflodarea.h"
#include "QPaintEvent"
test_codeflodarea::test_codeflodarea()
{

}

TEST_F(test_codeflodarea, CodeFlodArea)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString("abc"));
    ASSERT_TRUE(pWindow->currentWrapper()->textEditor()->m_pLeftAreaWidget->m_pFlodArea->m_pLeftAreaWidget != nullptr);

    delete pWindow;
    pWindow = nullptr;
}

TEST_F(test_codeflodarea, paintEvent)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString("abc"));
    QPaintEvent *pEvent = new QPaintEvent(pWindow->currentWrapper()->textEditor()->m_pLeftAreaWidget->m_pFlodArea->rect());
    pWindow->currentWrapper()->textEditor()->m_pLeftAreaWidget->m_pFlodArea->paintEvent(pEvent);

    delete pEvent;
    pEvent = nullptr;
    delete pWindow;
    pWindow = nullptr;
}
