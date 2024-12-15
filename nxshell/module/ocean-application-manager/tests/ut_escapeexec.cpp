// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dbus/applicationservice.h"
#include <gtest/gtest.h>
#include <QString>

TEST(UnescapeExec, blankSpace)
{
    QList<std::pair<QString, QList<QString>>> testCases{
        {
            R"(/usr/bin/hello\sworld --arg1=val1 -h --str="rrr ggg bbb")",
            {
                "/usr/bin/hello world",
                "--arg1=val1",
                "-h",
                "--str=rrr ggg bbb",
            },
        },
        {
            R"("/usr/bin/hello world" a b -- "c d")",
            {
                "/usr/bin/hello world",
                "a",
                "b",
                "--",
                "c d",
            },
        },
        {
            R"("/usr/bin/hello\t\nworld" a b -- c d)",
            {
                "/usr/bin/hello\t\nworld",
                "a",
                "b",
                "--",
                "c",
                "d",
            },
        },
    };

    for (auto &testCase : testCases) {
        EXPECT_EQ(ApplicationService::unescapeExecArgs(testCase.first), testCase.second);
    }
}
