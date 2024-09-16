// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_bookmarkwidget.h"

test_bookmarkwidget::test_bookmarkwidget()
{

}

TEST_F(test_bookmarkwidget, BookMarkWidget)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString("aabb"));

    ASSERT_TRUE(pWindow->currentWrapper()->textEditor()->m_pLeftAreaWidget->m_pBookMarkArea->m_leftAreaWidget != nullptr);
    delete pWindow;
    pWindow = nullptr;
}

TEST_F(test_bookmarkwidget, paintEvent)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString("aabb"));
    QPaintEvent *pEvent;
    pWindow->currentWrapper()->textEditor()->m_pLeftAreaWidget->m_pBookMarkArea->paintEvent(pEvent);

    delete pWindow;
    pWindow = nullptr;
}
