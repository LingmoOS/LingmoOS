// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <DConfig>
#include <QBuffer>
#include <QDir>
#include <QDebug>

#include <gtest/gtest.h>
#include "test_helper.hpp"
#include "backend/dsettingsdconfigbackend.h"

DCORE_USE_NAMESPACE

static EnvGuard dsgDataDir;
static constexpr char const *APP_ID = "tests";
static constexpr char const *FILE_NAME = "example";
class ut_DConfig : public testing::Test
{
protected:
    static void SetUpTestCase() {
        fileBackendLocalPerfix.set("DSG_DCONFIG_FILE_BACKEND_LOCAL_PREFIX", "/tmp/example");

        backendType.set("DSG_DCONFIG_BACKEND_TYPE", "FileBackend");
        dsgDataDir.set("DSG_DATA_DIRS", PREFIX"/share/dsg");
    }
    static void TearDownTestCase() {
        QDir(fileBackendLocalPerfix.value()).removeRecursively();
        fileBackendLocalPerfix.restore();

        backendType.restore();
        dsgDataDir.restore();
    }
    virtual void SetUp() override {}
    virtual void TearDown() override;

    static EnvGuard backendType;
    static EnvGuard fileBackendLocalPerfix;
    QString metaFilePath = QString("%1" PREFIX"/share/dsg/configs/%2/%3.json").arg(fileBackendLocalPerfix.value(), APP_ID, FILE_NAME);
    QString noAppIdMetaFilePath = QString("%1" PREFIX"/share/dsg/configs/%2.json").arg(fileBackendLocalPerfix.value(), FILE_NAME);
};
EnvGuard ut_DConfig::fileBackendLocalPerfix;
EnvGuard ut_DConfig::backendType;

void ut_DConfig::TearDown()
{
    QDir(fileBackendLocalPerfix.value()).removeRecursively();
}

TEST_F(ut_DConfig, backend) {

    FileCopyGuard guard(":/data/dconf-example.meta.json", metaFilePath);
    DConfig config(FILE_NAME);
    ASSERT_EQ(config.backendName(), QString("FileBackend"));
}

TEST_F(ut_DConfig, isValid) {

    FileCopyGuard guard(":/data/dconf-example.meta.json", metaFilePath);
    DConfig config(FILE_NAME);
    ASSERT_TRUE(config.isValid());
}

TEST_F(ut_DConfig, value) {

    FileCopyGuard guard(":/data/dconf-example.meta.json", metaFilePath);
    const QStringList array{"1", "2"};
    QVariantMap map;
    map.insert("key1", "value1");
    map.insert("key2", "value2");
    {
        DConfig config(FILE_NAME);
        config.setValue("key2", "126");
        ASSERT_EQ(config.value("key2").toString(), QString("126"));

        config.setValue("array", array);
        ASSERT_EQ(config.value("array").toStringList(), array);

        config.setValue("map", map);
        ASSERT_EQ(config.value("map").toMap(), map);
    }
    {
        DConfig config(FILE_NAME);
        ASSERT_EQ(config.value("key2").toString(), QString("126"));
        ASSERT_EQ(config.value("array").toStringList(), array);
        ASSERT_EQ(config.value("map").toMap(), map);
    }
    {
        DConfig config(FILE_NAME);
        config.reset("canExit");
        ASSERT_EQ(config.value("canExit").toBool(), true);

        config.reset("key2");
        ASSERT_EQ(config.value("key2").toString(), QString("125"));

        config.reset("number");
        ASSERT_EQ(config.value("number").toInt(), 1);

        config.reset("array");
        const QStringList &originArray {"value1", "value2"};
        ASSERT_EQ(config.value("array").toStringList(), originArray);

        config.reset("map");
        QVariantMap originMap;
        originMap.insert("key1", "value1");
        originMap.insert("key2", "value2");
        ASSERT_EQ(config.value("map").toMap(), originMap);
    }
}

TEST_F(ut_DConfig, keyList) {

    FileCopyGuard guard(":/data/dconf-example.meta.json", metaFilePath);
    DConfig config(FILE_NAME);
    QStringList keyList{QString("key2"), QString("canExit")};
    for (auto item : keyList) {
        ASSERT_TRUE(config.keyList().contains(item));
    }
}

TEST_F(ut_DConfig, OtherAppConfigfile) {

    FileCopyGuard guard(":/data/dconf-example.meta.json", metaFilePath);

    constexpr char const *APP_OTHER = "tests_other";
    FileCopyGuard gurand(":/data/dconf-example_other_app_configure.meta.json", QString("%1" PREFIX"/share/dsg/configs/%2/%3.json").arg(fileBackendLocalPerfix.value(), APP_OTHER, FILE_NAME));

    QScopedPointer<DConfig> config(DConfig::create(APP_OTHER, FILE_NAME));
    ASSERT_TRUE(config->isValid());
    ASSERT_EQ(config->value("appPublic").toString(), QString("publicValue"));
}

TEST_F(ut_DConfig, appIdWriteConfig) {

    FileCopyGuard guard(":/data/dconf-example.meta.json", noAppIdMetaFilePath);
    {
        // 内部会传递appid，强制appid不为空
        DConfig config(FILE_NAME);
        config.setValue("key2", "user-with-appid");
        ASSERT_EQ(config.value("key2"), "user-with-appid");
    }
    {
        // appid为公共id
        QScopedPointer<DConfig> config(DConfig::createGeneric(FILE_NAME));
        ASSERT_TRUE(config->isValid());
        ASSERT_EQ(config->value("key2").toString(), QString("125"));
    }
    {
        // 传递appid
        QScopedPointer<DConfig> config(DConfig::create(APP_ID, FILE_NAME));
        ASSERT_TRUE(config->isValid());
        ASSERT_EQ(config->value("key2").toString(), "user-with-appid");
    }
}

TEST_F(ut_DConfig, noAppidWirteConfig) {

    FileCopyGuard guard(":/data/dconf-example.meta.json", noAppIdMetaFilePath);
    FileCopyGuard guard2(":/data/dconf-example.meta.json", metaFilePath);
    {
        // 内部会传递appid，强制appid不为公共id
        QScopedPointer<DConfig> config(DConfig::createGeneric(FILE_NAME));
        ASSERT_TRUE(config->isValid());
        config->setValue("key2", "user-with-no-appid");
        ASSERT_EQ(config->value("key2"), "user-with-no-appid");
    }
    {
        // appid为公共id，获取公共id的值
        QScopedPointer<DConfig> config(DConfig::createGeneric(FILE_NAME));
        ASSERT_EQ(config->value("key2"), "user-with-no-appid");
    }
    {
        // 传递appid，fallback到公共id的值
        QScopedPointer<DConfig> config(DConfig::create(APP_ID, FILE_NAME));
        ASSERT_EQ(config->value("key2"), "user-with-no-appid");
    }
}

TEST_F(ut_DConfig, DSettingsDConfigBackend)
{
    FileCopyGuard guard(":/data/dconf-example.meta.json", metaFilePath);
    {
        DSettingsDConfigBackend backend(FILE_NAME);
        QStringList keyList{ QString("key2"), QString("canExit") };
        for (auto item : keyList) {
            ASSERT_TRUE(backend.keys().contains(item));
        }
    }

    {
        DSettingsDConfigBackend backend(FILE_NAME);
        ASSERT_EQ(backend.getOption("key2").toString(), QString("125"));
    }

    {

        DConfig config(FILE_NAME);
        config.setValue("key2", "126");
        // save cache file
    }

    {
        // reload cache file
        DSettingsDConfigBackend backend(FILE_NAME);
        ASSERT_EQ(backend.getOption("key2").toString(), QString("126"));
    }
}

TEST_F(ut_DConfig, isDefaultValue) {

    FileCopyGuard guard(":/data/dconf-example.meta.json", metaFilePath);
    {
        DConfig config(FILE_NAME);
        ASSERT_EQ(config.isDefaultValue("key2"), true);
    }
    {
        DConfig config(FILE_NAME);
        config.setValue("key2", "126");
        EXPECT_EQ(config.isDefaultValue("key2"), false);
    }
    {
        DConfig config(FILE_NAME);
        config.setValue("key2", "126");
        config.reset("key2");
        EXPECT_EQ(config.isDefaultValue("key2"), true);
    }
}
