// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#define protected public
#define private public
//#include "cgraphicsview.h"
#include "pageview.h"
#include "blurtool.h"
#include <qaction.h>
#undef protected
#undef private
#include "toptoolbar.h"
#include "application.h"

#define protected public
#define private public

#include <QDebug>
#include <DLineEdit>

#include "publicApi.h"

#if TEST_BLUR_ITEM

TEST(BlurItem, TestBlurItemCreateView)
{
    createNewViewByShortcutKey();
}

TEST(BlurItem, TestBlurItemOtherFunction)
{
    //模糊工具的标准使用步骤已经在pictureitem的测试中存在，这里仅测试未覆盖到的函数
    PageView *view = getCurView();
    drawApp->setCurrentTool(blur);
    QTest::qWait(200);

    auto tool = dynamic_cast<BlurTool *>(drawApp->drawBoard()->tool(blur));

    //event filter
    QObject *o = view;
    QKeyEvent e1(QEvent::Type::ShortcutOverride, Qt::Key::Key_0, Qt::KeyboardModifier::ControlModifier);
    tool->eventFilter(o, &e1);
    QTest::qWait(2000);

    QKeyEvent e2(QEvent::Type::ShortcutOverride, Qt::Key::Key_Z, Qt::KeyboardModifier::ControlModifier);
    tool->eventFilter(o, &e2);
    QTest::qWait(2000);
}

#endif
