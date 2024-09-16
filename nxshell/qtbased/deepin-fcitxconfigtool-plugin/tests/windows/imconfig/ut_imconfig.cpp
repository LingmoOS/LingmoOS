// Copyright (C) 2021 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iostream>
#include "gtest/gtest.h"
#include "window/immodel/imconfig.h"
#include <QStringList>
class ut_imconfig : public ::testing::Test
{
protected:
    ut_imconfig()
    {
    }

    virtual ~ut_imconfig()
    {
    }

    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }
};

TEST_F(ut_imconfig, IMSwitch)
{
    IMConfig imconfig;
    QString imSwitch = imconfig.IMSwitchKey();
    imconfig.setIMSwitchKey(imSwitch);
}

TEST_F(ut_imconfig, defualtIMKey)
{
    IMConfig imconfig;
    QString defualtIMKey = imconfig.defaultIMKey();
    imconfig.setDefaultIMKey(defualtIMKey);
}

TEST_F(ut_imconfig, checkShortKey)
{
    IMConfig imconfig;
    QStringList tempSList;
    tempSList << "A"
              << "B"
              << "C";
    QString temp = "C";
    imconfig.checkShortKey(tempSList, temp);
}
