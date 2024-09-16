// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include "../testItems/publicApi.h"

#include "application.h"
#include "mainwindow.h"
#include "shortcut.h"

#include "QTestEventList"

#if TEST_DRAWHELP_ITEM

TEST(DrawHelpPannel, TestDrawHelpPannelCreateView)
{
    int i = 0;
    while (i++ < 50) {
        QTest::qWait(200);
        if (getCurView() != nullptr) {
            break;
        }
    }
    if (getCurView() == nullptr) {
        qDebug() << __FILE__ << __LINE__ << "get CGraphicsView is nullptr.";
    }
    ASSERT_NE(getCurView(), nullptr);

    DTestEventList e;
    e.addKeyClick(Qt::Key_N, Qt::ControlModifier);
    e.simulate(getCurView());
    QTest::qWait(200);

    i = 0;
    while (i++ < 50) {
        QTest::qWait(200);
        if (getCurView() != nullptr) {
            break;
        }
    }
    if (getCurView() == nullptr) {
        qDebug() << __FILE__ << __LINE__ << "get CGraphicsView is nullptr.";
    }
    ASSERT_NE(getCurView(), nullptr);
}

TEST(DrawHelpPannel, ShowShortCutKey)
{
    QAction *ac = getMainWindow()->findChild<QAction *>("shortCutManPannel");
    ASSERT_NE(ac, nullptr);
    ac->trigger();
    QTest::qWait(300);
}

#endif
