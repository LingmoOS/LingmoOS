// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utils.h"
#include <singlefontapplication.h>

#include <QSize>
#include <QPixmap>
#include <QRawFont>

#include <gtest/gtest.h>
#include "../third-party/stub/stub.h"
//#include "commonheaderfile.h"

namespace {
class TestUtils : public testing::Test
{

protected:
    void SetUp()
    {
        fm = new Utils();
    }
    void TearDown()
    {
        delete fm;
    }

    Utils *fm;

};
}

TEST_F(TestUtils, initTest)
{

}

TEST_F(TestUtils, checksuffixList)
{
    EXPECT_EQ(fm->suffixList().toStdString(), "Font Files (*.ttf *.ttc *.otf)");
}

TEST_F(TestUtils, checkisFontMimeType)
{
    EXPECT_EQ(fm->isFontMimeType("方正兰亭特黑_SC.otf"), false);
}

TEST_F(TestUtils, checkgetConfigPath)
{
    EXPECT_EQ(fm->getConfigPath().isEmpty(), false);
}


TEST_F(TestUtils, checkrenderSVG_001)
{
    EXPECT_EQ(fm->renderSVG("://ok.svg", QSize(32, 32)).isNull(), false);
}

TEST_F(TestUtils, checkconvertToPreviewString)
{
    EXPECT_EQ(fm->convertToPreviewString("", "hello").toStdString(), "hello");
}



