// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global/grandsearch_global.h"
#include "configuration/userpreference.h"

#include <stubext.h>

#include <gtest/gtest.h>

#include <QTest>

GRANDSEARCH_USE_NAMESPACE

TEST(UserPreference, ut_test)
{
    QVariantHash datas;
    datas.insert("0", true);
    datas.insert("1", 1);
    UserPreference up(datas);

    EXPECT_EQ(up.group("2"), nullptr);

    {
        QVariant var;
        EXPECT_FALSE(up.innerValue("3", var));
        EXPECT_TRUE(up.innerValue("0", var));
        EXPECT_TRUE(var.toBool());

        EXPECT_TRUE(up.innerValue("1", var));
        EXPECT_EQ(var.toInt(), 1);
    }

    {
        EXPECT_FALSE(up.value("4", false));
        EXPECT_EQ(up.value("4", 999), 999);

        EXPECT_TRUE(up.value("0", false));
        EXPECT_EQ(up.value("1", 999), 1);
    }

    {
        up.setValue("0", false);
        up.setValue("3", QString("test"));

        QVariant var;
        UserPreferencePointer upp(new UserPreference({}));
        var.setValue(upp);
        up.setValue("4", var);

        EXPECT_FALSE(up.value("0", true));
        EXPECT_EQ(up.value("3", QString()), QString("test"));

        EXPECT_EQ(up.group("4").get(), upp.get());
    }
}
