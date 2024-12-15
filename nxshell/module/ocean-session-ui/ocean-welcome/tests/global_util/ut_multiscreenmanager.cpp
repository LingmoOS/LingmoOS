// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#define private public
#include "multiscreenmanager.h"
#include "mainwidget.h"
#undef private

#include <QApplication>
#include <QTest>

class Ut_MultiScreenManager : public testing::Test
{
public:
    void SetUp() override
    {
        manager = new MultiScreenManager();
    }

    void TearDown() override
    {
        delete manager;
        manager = nullptr;
    }

public:
    MultiScreenManager *manager = nullptr;
};

TEST_F(Ut_MultiScreenManager, coverageTest)
{
    auto createFrame = [&] (QScreen *screen) -> QWidget* {
        MainWidget *w = new MainWidget;
        w->setScreen(screen);
        return w;
    };
    manager->register_for_mutil_screen(createFrame);
    QTest::qWait(200);
    if (!qApp->screens().isEmpty()) {
        manager->onScreenRemoved(qApp->screens().first());
    }
}
