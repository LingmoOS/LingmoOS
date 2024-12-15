// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include "dsecurestring.h"

DCORE_USE_NAMESPACE


class ut_DSecureString : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;
    DSecureString *secureString;
    QString string;
};

void ut_DSecureString::SetUp()
{
    secureString = new DSecureString(string);
}

void ut_DSecureString::TearDown()
{
    if (secureString) {
        delete secureString;
        secureString = nullptr;
    }
}

TEST_F(ut_DSecureString, testString)
{
    QString test = secureString->fromLatin1("test");
    ASSERT_TRUE(test == QString("test"));
}
