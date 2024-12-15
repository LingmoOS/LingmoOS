// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test.h"
#include "ddciicon.h"

#include <QDebug>

DGUI_USE_NAMESPACE

class ut_DDciIcon : public DTest
{
public:
    ut_DDciIcon()
        : icon(QStringLiteral(":/images/dci_heart.dci")) {}
    ~ut_DDciIcon() override {}

    DDciIcon icon;
};

TEST_F(ut_DDciIcon, isNull)
{
    ASSERT_FALSE(icon.isNull());
}

TEST_F(ut_DDciIcon, actualSize)
{
    DDciIconMatchResult res = icon.matchIcon(64, DDciIcon::Light, DDciIcon::Normal);
    int size = icon.actualSize(res);

    ASSERT_EQ(size, 100);
}

TEST_F(ut_DDciIcon, pixmap)
{
    EXPECT_EQ(icon.pixmap(1, 0, DDciIcon::Light).size().height(), 100); // invalid size 0
    EXPECT_EQ(icon.pixmap(1, 64, DDciIcon::Light).size().height(), 64);
    EXPECT_EQ(icon.pixmap(1.25, 64, DDciIcon::Light).size().height(), 80); // 64 * 1.25
    EXPECT_EQ(icon.pixmap(1, 100, DDciIcon::Light).size().height(), 100);
    EXPECT_EQ(icon.pixmap(1, 256, DDciIcon::Light).size().height(), 256);
}
