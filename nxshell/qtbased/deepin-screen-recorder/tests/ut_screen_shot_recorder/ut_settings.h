// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QDebug>
#include "../../src/settings.h"


using namespace testing;

class SettingsTest:public testing::Test{

public:
    Settings *settings;
    virtual void SetUp() override{
        settings = new Settings;
        std::cout << "start SettingsTest" << std::endl;
    }

    virtual void TearDown() override{
        std::cout << "end SettingsTest" << std::endl;
        delete settings;
    }
};

TEST_F(SettingsTest, configPath)
{
    EXPECT_FALSE(settings->configPath().isEmpty());
}

TEST_F(SettingsTest, set_getOption)
{
    QString key = "SettingsTest";
    QVariant value = 1;
    settings->setOption(key, value);
    EXPECT_EQ(value, settings->getOption(key));
}

