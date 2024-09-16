// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include <QFile>
#include <QTextStream>
#include <QJsonObject>
#include "settings/dsettings.h"
#include "settings/dsettingsoption.h"
#include "settings/dsettingsgroup.h"
#include "settings/backend/gsettingsbackend.h"
#include "settings/backend/qsettingbackend.h"

DCORE_USE_NAMESPACE


class ut_DSettings : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;
    DSettings *settings;
    QString jsonContent;
};

void ut_DSettings::SetUp()
{
    settings = new DSettings;
    QFile file("/tmp/test.json");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;
    QTextStream out(&file);
    jsonContent = " { \"groups\": [{ "
                  " \"key\": \"base\", "
                  " \"name\": \"Basic settings\", "
                  " \"groups\": [{ "
                  " \"key\": \"open_action\", "
                  " \"name\": \"Open Action\", "
                  " \"options\": [{ "
                  " \"key\": \"alway_open_on_new\", "
                  " \"type\": \"checkbox\", "
                  " \"text\": \"Always Open On New Windows\", "
                  " \"default\": true "
                  " }]  "
                  " }] }]}";
    out << jsonContent;
    file.close();
}

void ut_DSettings::TearDown()
{
    if (settings) {
        delete settings;
        settings = nullptr;
    }
    QFile file("/tmp/test.json");
    if (file.exists())
        file.remove();
}

TEST_F(ut_DSettings, testDSettingSetBackend)
{
    QPointer<DSettings> tmpSetting = DSettings::fromJson(jsonContent.toLatin1());
    QScopedPointer<DSettings> scopeSettings(tmpSetting.data());
    QSettingBackend qBackend("/tmp/test.json");
    scopeSettings->setBackend(&qBackend);
    QStringList qKeys = qBackend.keys();
    ASSERT_TRUE(qKeys.isEmpty());
}

TEST_F(ut_DSettings, testDSettingFromJson)
{
    QPointer<DSettings> tmpSetting = DSettings::fromJson(jsonContent.toLatin1());
    QScopedPointer<DSettings> scopeSettings(tmpSetting.data());
    auto keys = scopeSettings->keys();
    ASSERT_TRUE(!keys.isEmpty());
}

TEST_F(ut_DSettings, testDSettingFromJsonFile)
{
    QPointer<DSettings> tmpSetting = DSettings::fromJsonFile("/tmp/test.json");
    QScopedPointer<DSettings> scopeSettings(tmpSetting.data());
    QStringList keys = scopeSettings->keys();
    ASSERT_TRUE(!keys.isEmpty());
}

TEST_F(ut_DSettings, testDSettingMeta)
{
    QPointer<DSettings> tmpSetting = DSettings::fromJsonFile("/tmp/test.json");
    QScopedPointer<DSettings> scopeSettings(tmpSetting.data());
    QJsonObject jsonObject = scopeSettings->meta();
    ASSERT_TRUE(!jsonObject.isEmpty());
}

TEST_F(ut_DSettings, testDSettingKeys)
{
    QPointer<DSettings> tmpSetting = DSettings::fromJsonFile("/tmp/test.json");
    QScopedPointer<DSettings> scopeSettings(tmpSetting.data());
    QStringList keys = scopeSettings->keys();
    ASSERT_TRUE(!keys.isEmpty());
}

TEST_F(ut_DSettings, testDSettingOptions)
{
    QPointer<DSettings> tmpSetting = DSettings::fromJsonFile("/tmp/test.json");
    QScopedPointer<DSettings> scopeSettings(tmpSetting.data());
    QList<QPointer<DSettingsOption>> options = scopeSettings->options();
    ASSERT_TRUE(!options.isEmpty());
}

TEST_F(ut_DSettings, testDSettingOption)
{
    QPointer<DSettings> tmpSetting = DSettings::fromJsonFile("/tmp/test.json");
    QScopedPointer<DSettings> scopeSettings(tmpSetting.data());
    QStringList keys = scopeSettings->keys();
    QPointer<DSettingsOption> option = scopeSettings->option(keys[0]);
    QString optionKey = option->key();
    ASSERT_TRUE(!optionKey.isEmpty());
    QString optionName = option->name();
    ASSERT_TRUE(optionName.isEmpty());
    ASSERT_TRUE(option->canReset());
    ASSERT_TRUE(option->defaultValue().toBool());
    ASSERT_TRUE(option->viewType() == "checkbox");
}

TEST_F(ut_DSettings, testDSettingValue)
{
    QPointer<DSettings> tmpSetting = DSettings::fromJsonFile("/tmp/test.json");
    QScopedPointer<DSettings> scopeSettings(tmpSetting.data());
    QStringList keys = scopeSettings->keys();
    QVariant value = scopeSettings->value(keys[0]);
    ASSERT_TRUE(value.toBool());
}

TEST_F(ut_DSettings, testDSettingGroupKeys)
{
    QPointer<DSettings> tmpSetting = DSettings::fromJsonFile("/tmp/test.json");
    QScopedPointer<DSettings> scopeSettings(tmpSetting.data());
    QStringList groupKeys = scopeSettings->groupKeys();
    ASSERT_TRUE(!groupKeys.isEmpty());
}

TEST_F(ut_DSettings, testDSettingGroups)
{
    QPointer<DSettings> tmpSetting = DSettings::fromJsonFile("/tmp/test.json");
    QScopedPointer<DSettings> scopeSettings(tmpSetting.data());
    QList<QPointer<DSettingsGroup>> groups = scopeSettings->groups();
    ASSERT_TRUE(!groups.isEmpty());
}

TEST_F(ut_DSettings, testDSettingGroup)
{
    QPointer<DSettings> tmpSetting = DSettings::fromJsonFile("/tmp/test.json");
    QScopedPointer<DSettings> scopeSettings(tmpSetting.data());
    QStringList keys = scopeSettings->keys();
    QPointer<DSettingsGroup> group = scopeSettings->group("base.open_action");
    QString optionKey = group->key();
    ASSERT_TRUE(!optionKey.isEmpty());
}

TEST_F(ut_DSettings, testDSettingGetOption)
{
    QPointer<DSettings> tmpSetting = DSettings::fromJsonFile("/tmp/test.json");
    QScopedPointer<DSettings> scopeSettings(tmpSetting.data());
    QStringList keys = scopeSettings->keys();
    QVariant option = scopeSettings->getOption(keys[0]);
    ASSERT_TRUE(option.toBool());
}

TEST_F(ut_DSettings, testDSettingSync)
{
    QPointer<DSettings> tmpSetting = DSettings::fromJson(jsonContent.toLatin1());
    QScopedPointer<DSettings> scopeSettings(tmpSetting.data());
    scopeSettings->sync();
    QStringList keys = scopeSettings->keys();
    ASSERT_TRUE(!keys.isEmpty());
}

TEST_F(ut_DSettings, testDSettingReset)
{
    QPointer<DSettings> tmpSetting = DSettings::fromJson(jsonContent.toLatin1());
    QScopedPointer<DSettings> scopeSettings(tmpSetting.data());
    scopeSettings->reset();
    QStringList keys = scopeSettings->keys();
    QVariant option = scopeSettings->getOption(keys[0]);
    ASSERT_TRUE(option.toBool());
}
