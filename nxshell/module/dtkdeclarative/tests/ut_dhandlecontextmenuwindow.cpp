// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>

#include "test_helper.hpp"
#include "dhandlecontextmenuwindow_p.h"

#include <QQuickItem>

TEST(ut_DHandleContextMenuWindow, window)
{
    ControlHelper<DHandleContextMenuWindow> helper("qrc:/qml/ContextMenuWindow.qml");
    ASSERT_TRUE(helper.object);
    helper.requestExposed();

    auto w = helper.object;
    auto windowImange = w->grabWindow();
    EXPECT_EQ(windowImange.pixelColor(QPoint(50, 50)), Qt::red);

    QTest::mouseClick(w, Qt::RightButton, Qt::NoModifier, QPoint( 50, 50 ), 100);
}
