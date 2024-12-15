// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "util.h"

#include <QSize>
#include <QTest>

#include <gtest/gtest.h>

class Tst_Util : public testing::Test
{
};

TEST_F(Tst_Util, loadSvg_test)
{
    QSize size = QSize(50, 50);
    loadSvg("", 20);

    // 文件名为空时，
    loadSvg("", size);

    // 文件名不为空时
    loadSvg("/usr/share/backgrounds/default_background.jpg", size);

    renderSVG("", size);
}

TEST_F(Tst_Util, settingsPtr_test)
{
    QVERIFY(ModuleSettingsPtr("", "", nullptr) == nullptr);
}
