// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEST_COLORSELECTWIDGET_H
#define TEST_COLORSELECTWIDGET_H

#include "../../src/widgets/ColorSelectWdg.h"

#include <gtest/gtest.h>

class test_colorlabel : public testing::Test
{
protected:
    void SetUp()
    {
        colorLabel = new ColorLabel(QColor("#FF0000"));
    }
    void TearDown()
    {
        delete colorLabel;
    }

    ColorLabel *colorLabel = nullptr;
};

class test_colorselectwidget : public testing::Test
{
protected:
    void SetUp()
    {
        colorSelctWidget = new ColorSelectWdg("this is a test");
    }
    void TearDown()
    {
        delete colorSelctWidget;
    }

    ColorSelectWdg *colorSelctWidget = nullptr;
};

#endif // TEST_COLORSELECTWIDGET_H
