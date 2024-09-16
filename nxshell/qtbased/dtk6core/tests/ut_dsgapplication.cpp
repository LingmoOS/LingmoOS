// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include <QCoreApplication>
#include "dsgapplication.h"

DCORE_USE_NAMESPACE

TEST(ut_DSGApplication, id)
{
    if (!qgetenv("DSG_APP_ID").isEmpty()) {
        EXPECT_EQ(DSGApplication::id(), qgetenv("DSG_APP_ID"));
    }

    EXPECT_EQ(DSGApplication::id(), QCoreApplication::applicationName());

//    qputenv("DTK_DISABLED_FALLBACK_APPID", "1");
//    // Q_ASSERT id not empty...
//    EXPECT_EQ(DSGApplication::id(), "");
}

TEST(ut_DSGApplication, getId)
{
    ASSERT_EQ(DSGApplication::getId(QCoreApplication::applicationPid()), QByteArray());
}
