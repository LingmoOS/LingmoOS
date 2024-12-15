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

#if TEST_HELPPANNEL_ITEM

TEST(DrawZHelpPannel, TestZDrawHelpPannelCreateView)
{
    createNewViewByShortcutKey();
}

TEST(DrawZHelpPannel, ZShowShortCutKey)
{
    QAction *ac = getMainWindow()->findChild<QAction *>("shortCutManPannel");
    ASSERT_NE(ac, nullptr);
    ac->trigger();
    QTest::qWait(300);
}
#endif
