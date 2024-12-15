// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global/commontools.h"

#include <stubext.h>

#include <gtest/gtest.h>

#include <QTest>

using namespace GrandSearch::CommonTools;

TEST(CommonToolsTest, fstabBindInfo)
{
    stub_ext::StubExt st;
    st.set_lamda(stat, [](const char *, struct stat *info) {
        info->st_mtime = 1;
        return 0;
    });

    struct fstab fs;
    fs.fs_mntops = "defaults,bind";
    fs.fs_spec = "/data/home";
    fs.fs_file = "/home";
    bool first = true;
    st.set_lamda(getfsent, [&]() -> fstab * {
        if (first) {
            first = false;
            return &fs;
        }

        return nullptr;
    });

    const auto &table = fstabBindInfo();
    EXPECT_TRUE(!table.isEmpty());
    EXPECT_FALSE(first);
}

TEST(CommonToolsTest, bindPathTransform_1)
{
    QString bindPath = bindPathTransform("/", true);
    EXPECT_EQ("/", bindPath);

    stub_ext::StubExt st;
    st.set_lamda(fstabBindInfo, [] { return QMap<QString, QString>(); });
    bindPath = bindPathTransform("/home", true);
    EXPECT_EQ("/home", bindPath);
}

TEST(CommonToolsTest, bindPathTransform_2)
{
    stub_ext::StubExt st;
    st.set_lamda(fstabBindInfo, [] {
        QMap<QString, QString> table { { "/data/home", "/home" } };
        return table;
    });

    auto bindPath = bindPathTransform("/home", true);
    EXPECT_EQ("/data/home", bindPath);

    bindPath = bindPathTransform("/data/home", false);
    EXPECT_EQ("/home", bindPath);
}
