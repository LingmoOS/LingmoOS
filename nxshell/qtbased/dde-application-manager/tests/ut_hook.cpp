// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "applicationHooks.h"
#include <QDir>
#include <QStringList>
#include <gtest/gtest.h>

TEST(ApplicationHookTest, load)
{
    auto file =
        QDir::currentPath() + QDir::separator() + "data" + QDir::separator() + "hooks.d" + QDir::separator() + "1-test.json";
    auto hook = ApplicationHook::loadFromFile(file);
    EXPECT_TRUE(hook);
    EXPECT_EQ(hook->hookName(), QString{"1-test.json"});
    EXPECT_EQ(hook->execPath(), QString{"/usr/bin/stat"});

    QStringList tmp{"for test"};
    EXPECT_EQ(hook->args(), tmp);

    tmp.push_front("/usr/bin/stat");
    auto elem = generateHooks({std::move(hook).value()});
    EXPECT_EQ(elem, tmp);
}
