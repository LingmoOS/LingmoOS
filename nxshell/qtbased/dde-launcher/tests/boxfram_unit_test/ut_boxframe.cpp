// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 欺骗编译器的操作, 旨在单元测试中方便使用该类中的私有成员函数或者对象
#define private public
#include "boxframe.h"
#include "backgroundmanager.h"
#undef private

#include <QPixmap>
#include <QApplication>
#include <QMoveEvent>
#include <QPoint>
#include <QTest>
#include <QSignalSpy>

#include <gtest/gtest.h>

class Tst_Boxframe : public testing::Test
{};

TEST_F(Tst_Boxframe, checkBackground_test)
{
    BoxFrame frame;
    /* 模拟一下四种模式下修改屏幕的背景和模糊处理过的背景
    #define CUSTOM_MODE     0
    #define MERGE_MODE      1
    #define EXTEND_MODE     2
    #define SINGLE_MODE     3
    */
    for (int i = 0; i < 4; i++) {
        frame.m_bgManager->m_displayMode = i;
        frame.setBackground(frame.m_defaultBg);
        frame.setBlurBackground(frame.m_defaultBg);

        // 清空上一次的背景
        frame.m_lastUrl.clear();
        frame.m_lastBlurUrl.clear();
    }
}
