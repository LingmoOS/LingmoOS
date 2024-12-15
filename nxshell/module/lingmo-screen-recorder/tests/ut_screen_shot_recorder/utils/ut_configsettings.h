// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QDebug>
#include "../../src/utils/configsettings.h"
#include "../../src/utils/saveutils.h"


using namespace testing;

class ConfigSettingsTest:public testing::Test{

public:
    ConfigSettings *configInstance = nullptr;
    virtual void SetUp() override{
        configInstance = ConfigSettings::instance();
        std::cout << "start ConfigSettingsTest" << std::endl;
    }

    virtual void TearDown() override{
        std::cout << "end ConfigSettingsTest" << std::endl;
    }
};

TEST_F(ConfigSettingsTest, configsettings)
{
    EXPECT_NE(nullptr, configInstance);
    // <
    EXPECT_LT(0, configInstance->keys("common").size());
    EXPECT_NE("", configInstance->value("common", "default_savepath", ""));
}
/*
TEST_F(ConfigSettingsTest, setTemporarySaveAction)
{
    configInstance->setTemporarySaveAction(std::pair<bool, SaveAction>(true, SaveToDesktop));
    std::pair<bool, SaveAction> value = configInstance->getTemporarySaveAction();
    EXPECT_EQ(true, value.first);
    EXPECT_EQ(SaveToDesktop, value.second);
}
*/
TEST_F(ConfigSettingsTest, keys)
{
    QStringList keys = configInstance->keys("common");
    qDebug() << keys;
    EXPECT_LT(0, keys.size());
}
TEST_F(ConfigSettingsTest, value)
{
    QVariant defaultVaue = 99;
    QVariant value = configInstance->value("common", "themeType", defaultVaue);
    qDebug() << value << defaultVaue;
    EXPECT_NE(defaultVaue, value);
}
TEST_F(ConfigSettingsTest, setValue)
{
    QString group = "common";
    QString key = "themeType";
    QVariant val = 99;
    QVariant defaultVaue = -19;
    QVariant tempVal =  configInstance->value(group, key, defaultVaue);
    EXPECT_NE(defaultVaue, tempVal);
    configInstance->setValue(group, key, defaultVaue);
    QVariant change =  configInstance->value(group, key, tempVal);
    EXPECT_NE(change, tempVal);
    EXPECT_EQ(change, defaultVaue);

    configInstance->setValue(group, key, tempVal);
}
