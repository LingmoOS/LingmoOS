// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <DConfigFile>
#include <DStandardPaths>
#include <QBuffer>
#include <QDir>

#include <gtest/gtest.h>
#include "test_helper.hpp"

#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
#define type typeId  // In qt6 type is deprecated and typeId should be used, this macro is for more convenient compatibility with qt6
#endif

DCORE_USE_NAMESPACE

static constexpr char const *LocalPrefix = "/tmp/example";
static QString NoAppId;

class ut_DConfigFile : public testing::Test
{
protected:
    static void SetUpTestCase() {
        home.set("HOME", "/tmp/home");
        dsgDataDir.set("DSG_DATA_DIRS", PREFIX"/share/dsg");
    }
    static void TearDownTestCase() {
        dsgDataDir.restore();
        home.restore();
    }
    virtual void TearDown() override;

    const char *APP_ID = "org.foo.appid";
    const char *FILE_NAME = "org.foo.name";
    QString metaPath = QString("%1" PREFIX"/share/dsg/configs/%2").arg(LocalPrefix, APP_ID);
    QString metaGlobalPath = QString("%1" PREFIX"/share/dsg/configs").arg(LocalPrefix);
    QString overridePath = QString("%1" PREFIX"/share/dsg/configs/overrides/%2/%3").arg(LocalPrefix, APP_ID, FILE_NAME);
    const char *OTHER_APP_ID = "tests_other";
    QString noAppidMetaPath = QString("%1" PREFIX"/share/dsg/configs").arg(LocalPrefix);
    QString noAppidOverridePath = QString("%1" PREFIX"/share/dsg/configs/overrides/%2").arg(LocalPrefix, FILE_NAME);
    uint uid = getuid();
    static EnvGuard dsgDataDir;
    static EnvGuard home;
};
EnvGuard ut_DConfigFile::dsgDataDir;
EnvGuard ut_DConfigFile::home;


void ut_DConfigFile::TearDown() {
    QDir(LocalPrefix).removeRecursively();
}

TEST_F(ut_DConfigFile, testLoad) {
    QByteArray meta = R"delimiter(
{
    "magic": "dsg.config.meta",
    "version": "1.0",
    "contents": {
        "canExit": {
            "value": false,
            "serial": 0,
            "name": "I am name",
            "name[zh_CN]": "我是名字",
            "description": "我是描述",
            "description[en_US]": "I am description",
            "permissions": "readwrite",
            "visibility": "private"
        }
    }
}
        )delimiter";

    QBuffer buffer;
    buffer.setData(meta);

    DConfigFile config(APP_ID, FILE_NAME);
    ASSERT_TRUE(config.load(&buffer, {}));
    ASSERT_EQ(config.meta()->keyList(), QStringList{QLatin1String("canExit")});

    QScopedPointer<DConfigCache> userCache(config.createUserCache(uid));
    ASSERT_EQ(config.value("canExit", userCache.get()).toBool(), false);

    ASSERT_EQ(config.meta()->version().major, 1);
    ASSERT_EQ(config.meta()->version().minor, 0);

    ASSERT_EQ(config.meta()->visibility("canExit"), DConfigFile::Private);

    ASSERT_EQ(config.meta()->displayName("canExit", QLocale::AnyLanguage), "I am name");
    ASSERT_EQ(config.meta()->displayName("canExit", QLocale::Chinese), QString("我是名字"));

    ASSERT_EQ(config.meta()->description("canExit", QLocale::AnyLanguage), "我是描述");
    ASSERT_EQ(config.meta()->description("canExit", QLocale::English), "I am description");

    ASSERT_EQ(config.meta()->permissions("canExit"), DConfigFile::ReadWrite);
}

TEST_F(ut_DConfigFile, setValueTypeCheck) {

    FileCopyGuard guard(":/data/dconf-example.meta.json", QString("%1/%2.json").arg(metaPath, FILE_NAME));
    DConfigFile config(APP_ID, FILE_NAME);
    config.load(LocalPrefix);
    QScopedPointer<DConfigCache> userCache(config.createUserCache(uid));
    userCache->load(LocalPrefix);
    {
        const auto type = config.value("canExit", userCache.get()).type();
        ASSERT_TRUE(config.setValue("canExit", false, "test", userCache.get()));
        ASSERT_TRUE(config.setValue("canExit", "true", "test", userCache.get()));
        ASSERT_FALSE(config.setValue("canExit", "true2", "test", userCache.get()));
        ASSERT_EQ(config.value("canExit", userCache.get()).type(), type);
    }
    {
        const auto type = config.value("key2", userCache.get()).type();
        ASSERT_TRUE(config.setValue("key2", "121", "test", userCache.get()));
        ASSERT_FALSE(config.setValue("key2", 121, "test", userCache.get()));
        ASSERT_EQ(config.value("key2", userCache.get()).type(), type);
    }
    {
        const auto type = config.value("number", userCache.get()).type();
        ASSERT_TRUE(config.setValue("number", 1, "test", userCache.get()));
        ASSERT_TRUE(config.setValue("number", 2.0, "test", userCache.get()));
        ASSERT_TRUE(config.setValue("number", "3", "test", userCache.get()));
        ASSERT_FALSE(config.setValue("number", "1ab", "test", userCache.get()));
        ASSERT_EQ(config.value("number", userCache.get()).type(), type);
    }
    {
        const auto type = config.value("numberDouble", userCache.get()).type();
        ASSERT_TRUE(config.setValue("numberDouble", 1.2, "test", userCache.get()));
        ASSERT_EQ(config.value("numberDouble", userCache.get()), 1.2);
    }
    {
        const auto type = config.value("array", userCache.get()).type();
        const QStringList array{"value1", "value2"};
        ASSERT_TRUE(config.setValue("array", QStringList(), "test", userCache.get()));
        ASSERT_TRUE(config.setValue("array", array, "test", userCache.get()));
        ASSERT_TRUE(config.setValue("array", QJsonDocument::fromJson("[]").toVariant(), "test", userCache.get()));
        ASSERT_FALSE(config.setValue("array", "", "test", userCache.get()));
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        ASSERT_FALSE(config.setValue("array", "value1", "test", userCache.get()));
#else
        ASSERT_TRUE(config.setValue("array", "value1", "test", userCache.get()));
#endif
        ASSERT_EQ(config.value("array", userCache.get()).type(), type);
    }
    {
        const auto type = config.value("array_map", userCache.get()).type();
        QVariantList array;
        QVariantMap map1;
        map1["key1"] = "value1";
        map1["key2"] = "value2";
        array.append(map1);
        ASSERT_EQ(config.value("array_map", userCache.get()).toList(), array);
        ASSERT_TRUE(config.setValue("array_map", QVariantList(), "test", userCache.get()));
        ASSERT_TRUE(config.setValue("array_map", array, "test", userCache.get()));
        ASSERT_TRUE(config.setValue("array_map", QJsonDocument::fromJson("[]").toVariant(), "test", userCache.get()));
        ASSERT_FALSE(config.setValue("array_map", "", "test", userCache.get()));
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        ASSERT_FALSE(config.setValue("array_map", "value1", "test", userCache.get()));
#else
        ASSERT_TRUE(config.setValue("array_map", "value1", "test", userCache.get()));
#endif
        ASSERT_EQ(config.value("array_map", userCache.get()).type(), type);
    }
    {
        const auto type = config.value("map", userCache.get()).type();
        QVariantMap map;
        map.insert("key1", "value1");
        map.insert("key2", "value2");
        ASSERT_TRUE(config.setValue("map", QVariantMap(), "test", userCache.get()));
        ASSERT_TRUE(config.setValue("map", map, "test", userCache.get()));
        ASSERT_TRUE(config.setValue("map", QJsonDocument::fromJson("{}").toVariant(), "test", userCache.get()));
        ASSERT_FALSE(config.setValue("map", QJsonDocument::fromJson("[]").toVariant(), "test", userCache.get()));
        ASSERT_FALSE(config.setValue("map", "key1", "test", userCache.get()));
        ASSERT_EQ(config.value("map", userCache.get()).type(), type);
    }
    {
        const auto type = config.value("map_array", userCache.get()).type();
        QVariantMap map;
        map.insert("key1", QStringList{"value1"});
        map.insert("key2", QStringList{"value2"});
        #if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        ASSERT_EQ(config.value("map_array", userCache.get()).toMap(), map);
        #else
        auto ret = config.value("map_array", userCache.get()).toMap();
        ASSERT_EQ(ret.keys(), map.keys());
        auto value1 = ret.values();
        auto value2 = map.values();
        ASSERT_EQ(value1.size(), value2.size());
        for(std::size_t i = 0; i < value1.size(); ++i){
            ASSERT_EQ(value1[i].toStringList(), value2[i].toStringList());
        }
        #endif
        ASSERT_TRUE(config.setValue("map_array", QVariantMap(), "test", userCache.get()));
        ASSERT_TRUE(config.setValue("map_array", map, "test", userCache.get()));
        ASSERT_TRUE(config.setValue("map_array", QJsonDocument::fromJson("{}").toVariant(), "test", userCache.get()));
        ASSERT_FALSE(config.setValue("map_array", "", "test", userCache.get()));
        ASSERT_FALSE(config.setValue("map_array", "value1", "test", userCache.get()));
        ASSERT_EQ(config.value("map_array", userCache.get()).type(), type);
    }
}

TEST_F(ut_DConfigFile, fileIODevice) {

    FileCopyGuard guard(":/data/dconf-example.meta.json", QString("%1/%2.json").arg(metaPath, FILE_NAME));
    {
        DConfigFile config(APP_ID, FILE_NAME);
        ASSERT_TRUE(config.load(LocalPrefix));
        QScopedPointer<DConfigCache> userCache(config.createUserCache(uid));
        ASSERT_TRUE(userCache->load(LocalPrefix));
    }
    {
        DConfigFile config(APP_ID, FILE_NAME);
        config.load(LocalPrefix);
        QScopedPointer<DConfigCache> userCache(config.createUserCache(uid));
        userCache->load(LocalPrefix);

        config.setValue("canExit", false, "test", userCache.get());
        config.setValue("key2", QString("128"), "test", userCache.get());

        ASSERT_TRUE(config.save(LocalPrefix));
        ASSERT_TRUE(userCache->save(LocalPrefix));
    }
    {
        DConfigFile config(APP_ID, FILE_NAME);
        ASSERT_TRUE(config.load(LocalPrefix));
        QScopedPointer<DConfigCache> userCache(config.createUserCache(uid));
        ASSERT_TRUE(userCache->load(LocalPrefix));
        ASSERT_EQ(config.value("canExit", userCache.get()), false);
        ASSERT_EQ(config.value("key2", userCache.get()).toString(), QString("128"));
    }
}

TEST_F(ut_DConfigFile, appmeta) {

    FileCopyGuard guard(":/data/dconf-example.meta.json", QString("%1/%2.json").arg(metaPath, FILE_NAME));
    {
        DConfigFile config(APP_ID, FILE_NAME);

        config.load(LocalPrefix);
        ASSERT_TRUE(config.load(LocalPrefix));
        QScopedPointer<DConfigCache> userCache(config.createUserCache(uid));
        ASSERT_TRUE(userCache->load(LocalPrefix));
        ASSERT_EQ(config.value("key3", userCache.get()).toString(), QString("application"));
    }
}

TEST_F(ut_DConfigFile, globalmeta) {

    FileCopyGuard guard(":/data/dconf-global.meta.json", QString("%1/%2.json").arg(metaPath, FILE_NAME));
    DConfigFile config(APP_ID, FILE_NAME);
    ASSERT_TRUE(config.load(LocalPrefix));
    QScopedPointer<DConfigCache> userCache(config.createUserCache(uid));
    ASSERT_TRUE(userCache->load(LocalPrefix));
    ASSERT_EQ(config.value("key3", userCache.get()), QString("global"));
}

TEST_F(ut_DConfigFile, meta) {

    FileCopyGuard guard(":/data/dconf-example.meta.json", QString("%1/%2.json").arg(metaPath, FILE_NAME));
    FileCopyGuard guard2(":/data/dconf-global.meta.json", QString("%1/%2.json").arg(metaGlobalPath, FILE_NAME));
    DConfigFile config(APP_ID, FILE_NAME);
    ASSERT_TRUE(config.load(LocalPrefix));
    QScopedPointer<DConfigCache> userCache(config.createUserCache(uid));
    ASSERT_TRUE(userCache->load(LocalPrefix));
    ASSERT_EQ(config.value("key3", userCache.get()), QString("application"));
    const QStringList array{"value1", "value2"};
    ASSERT_EQ(config.value("array", userCache.get()).toStringList(), array);
    QVariantMap map;
    map.insert("key1", "value1");
    map.insert("key2", "value2");
    ASSERT_EQ(config.value("map", userCache.get()).toMap(), map);
}

TEST_F(ut_DConfigFile, fileOverride) {

    FileCopyGuard guard(":/data/dconf-example.meta.json", QString("%1/%2.json").arg(metaPath, FILE_NAME));
    {
        DConfigFile config(APP_ID, FILE_NAME);
        config.load(LocalPrefix);
        QScopedPointer<DConfigCache> userCache(config.createUserCache(uid));
        ASSERT_TRUE(userCache->load(LocalPrefix));
        ASSERT_EQ(config.value("key3", userCache.get()), QString("application"));
    }

    FileCopyGuard guard1(":/data/dconf-example.override.json", QString("%1/%2.json").arg(overridePath, FILE_NAME));
    {
        DConfigFile config(APP_ID, FILE_NAME);
        config.load(LocalPrefix);
        QScopedPointer<DConfigCache> userCache(config.createUserCache(uid));
        ASSERT_TRUE(userCache->load(LocalPrefix));
        ASSERT_EQ(config.value("key3", userCache.get()), QString("override"));
    }

    FileCopyGuard guard2(":/data/dconf-override/dconf-example.override.a.json", QString("%1/a/%2.json").arg(overridePath, FILE_NAME));
    {
        {
            DConfigFile config(APP_ID, FILE_NAME, "/a");
            config.load(LocalPrefix);
            QScopedPointer<DConfigCache> userCache(config.createUserCache(uid));
            ASSERT_TRUE(userCache->load(LocalPrefix));
            ASSERT_EQ(config.value("key3", userCache.get()).toString(), QString("override /a"));
        }
    }

    FileCopyGuard guard3(":/data/dconf-override/dconf-example.override.a.b.json", QString("%1/a/b/%2.json").arg(overridePath, FILE_NAME));
    {
        {
            DConfigFile config(APP_ID, FILE_NAME, "/a/b");
            config.load(LocalPrefix);
            QScopedPointer<DConfigCache> userCache(config.createUserCache(uid));
            ASSERT_TRUE(userCache->load(LocalPrefix));
            ASSERT_EQ(config.value("key3", userCache.get()).toString(), QString("override /a/b"));
        }
    }
}

TEST_F(ut_DConfigFile, fileOverrideNoExistItem) {

    FileCopyGuard guard(":/data/dconf-example.meta.json", QString("%1/%2.json").arg(metaPath, FILE_NAME));
    FileCopyGuard guard2(":/data/dconf-example.override.noexistitem.json", QString("%1/%2.json").arg(overridePath, FILE_NAME));
    {
        DConfigFile config(APP_ID, FILE_NAME);
        ASSERT_TRUE(config.load(LocalPrefix));
        ASSERT_FALSE(config.meta()->keyList().contains("noexistitem"));
    }
}

TEST_F(ut_DConfigFile, noAppIdWithGlobalConfiguration) {

    FileCopyGuard guard(":/data/dconf-example.meta.json", QString("%1/%2.json").arg(noAppidMetaPath, FILE_NAME));
    {
        DConfigFile config(NoAppId, FILE_NAME);
        ASSERT_TRUE(config.load(LocalPrefix));
        ASSERT_EQ(config.value("key3"), QString("application"));
        config.setValue("key3", "global-with-no-appid", "test");
        config.save(LocalPrefix);
    }
    {
        DConfigFile config(NoAppId, FILE_NAME);
        config.load(LocalPrefix);
        ASSERT_EQ(config.value("key3"), "global-with-no-appid");
    }
}

TEST_F(ut_DConfigFile, noAppIdWithUserConfiguration) {

    FileCopyGuard guard(":/data/dconf-example.meta.json", QString("%1/%2.json").arg(noAppidMetaPath, FILE_NAME));
    {
        DConfigFile config(NoAppId, FILE_NAME);
        ASSERT_TRUE(config.load(LocalPrefix));
        QScopedPointer<DConfigCache> userCache(config.createUserCache(uid));
        ASSERT_TRUE(userCache->load(LocalPrefix));
        ASSERT_EQ(config.value("key2", userCache.get()), QString("125"));
        config.setValue("key2", "user-with-no-appid", "test", userCache.get());
        ASSERT_TRUE(userCache->save(LocalPrefix));
    }
    {
        DConfigFile config(NoAppId, FILE_NAME);
        config.load(LocalPrefix);
        QScopedPointer<DConfigCache> userCache(config.createUserCache(uid));
        userCache->load(LocalPrefix);
        ASSERT_EQ(config.value("key2", userCache.get()), "user-with-no-appid");
    }
}

TEST_F(ut_DConfigFile, noAppIdUserConfiguration) {

    // meta安装在公共目录，使用空的appid设置配置项， 则无论是否传入appid，获取的是均为空appid的cache值
    FileCopyGuard guard(":/data/dconf-example.meta.json", QString("%1/%2.json").arg(noAppidMetaPath, FILE_NAME));
    {
        // 不传入appid设置配置项
        DConfigFile config(NoAppId, FILE_NAME);
        config.load(LocalPrefix);
        QScopedPointer<DConfigCache> userCache(config.createUserCache(uid));
        userCache->load(LocalPrefix);
        config.setValue("key2", "user-with-no-appid", "test", userCache.get());
        userCache->save(LocalPrefix);
    }
    {
        // 不传入appid，获取的为空appid的值
        DConfigFile config(NoAppId, FILE_NAME);
        config.load(LocalPrefix);
        QScopedPointer<DConfigCache> userCache(config.createUserCache(uid));
        userCache->load(LocalPrefix);
        ASSERT_EQ(config.value("key2", userCache.get()), "user-with-no-appid");
    }
}

TEST_F(ut_DConfigFile, appIdOverrideNoAppIdUserConfiguration) {

    // meta安装在公共目录，appid覆盖了meta文件，使用空的appid设置配置项， 则无论是否传入appid，获取的是均为空appid的cache值
    FileCopyGuard guard(":/data/dconf-example.meta.json", QString("%1/%2.json").arg(noAppidMetaPath, FILE_NAME));
    FileCopyGuard guard2(":/data/dconf-example.meta.json", QString("%1/%2.json").arg(metaPath, FILE_NAME));
    {
        DConfigFile config(NoAppId, FILE_NAME);
        config.load(LocalPrefix);
        QScopedPointer<DConfigCache> userCache(config.createUserCache(uid));
        userCache->load(LocalPrefix);
        config.setValue("key2", "user-with-no-appid", "test", userCache.get());
        userCache->save(LocalPrefix);
    }
    {
        // 不传入appid，获取的为空appid的值
        DConfigFile config(NoAppId, FILE_NAME);
        config.load(LocalPrefix);
        QScopedPointer<DConfigCache> userCache(config.createUserCache(uid));
        userCache->load(LocalPrefix);
        ASSERT_EQ(config.value("key2", userCache.get()), "user-with-no-appid");
    }
    {
        // 传入appid设置配置项
        DConfigFile config(APP_ID, FILE_NAME);
        config.load(LocalPrefix);
        QScopedPointer<DConfigCache> userCache(config.createUserCache(uid));
        userCache->load(LocalPrefix);
        config.setValue("key2", "user-with-appid", "test", userCache.get());
        userCache->save(LocalPrefix);
    }
    {
        // 不传入appid，获取的是空appid的值
        DConfigFile config(NoAppId, FILE_NAME);
        config.load(LocalPrefix);
        QScopedPointer<DConfigCache> userCache(config.createUserCache(uid));
        userCache->load(LocalPrefix);
        ASSERT_EQ(config.value("key2", userCache.get()), "user-with-no-appid");
    }
    {
        // 传入appid，获取的是含appid的值
        DConfigFile config(APP_ID, FILE_NAME);
        config.load(LocalPrefix);
        QScopedPointer<DConfigCache> userCache(config.createUserCache(uid));
        userCache->load(LocalPrefix);
        ASSERT_EQ(config.value("key2", userCache.get()).toString().toStdString(), "user-with-appid");
    }
}

TEST_F(ut_DConfigFile, setCachePathPrefix) {

    FileCopyGuard guard(":/data/dconf-example.meta.json", QString("%1/%2.json").arg(metaPath, FILE_NAME));
    {
        DConfigFile config(APP_ID, FILE_NAME);
        config.globalCache()->setCachePathPrefix("/configs-global");
        ASSERT_TRUE(config.load(LocalPrefix));
        QScopedPointer<DConfigCache> userCache(config.createUserCache(uid));
        userCache->setCachePathPrefix("/configs-user");
        ASSERT_TRUE(userCache->load(LocalPrefix));
        ASSERT_EQ(config.value("key2", userCache.get()), QString("125"));
        ASSERT_EQ(config.value("key3", userCache.get()), QString("application"));

        config.setValue("key2", QString("user-config"), "test", userCache.get());
        config.setValue("key3", "global-config", "test", userCache.get());
        config.save(LocalPrefix);
        userCache->save(LocalPrefix);
    }
    {
        DConfigFile config(APP_ID, FILE_NAME);
        config.globalCache()->setCachePathPrefix("/configs-global");
        ASSERT_TRUE(config.load(LocalPrefix));
        QScopedPointer<DConfigCache> userCache(config.createUserCache(uid));
        userCache->setCachePathPrefix("/configs-user");
        ASSERT_TRUE(userCache->load(LocalPrefix));

        ASSERT_EQ(config.value("key2", userCache.get()), QString("user-config"));
        ASSERT_EQ(config.value("key3", userCache.get()), QString("global-config"));
    }
}

TEST_F(ut_DConfigFile, setSubpath) {

    FileCopyGuard guard(":/data/dconf-example.meta.json", QString("%1/%2.json").arg(metaPath, FILE_NAME));
    {
        DConfigFile config(APP_ID, FILE_NAME);
        ASSERT_TRUE(config.load(LocalPrefix));
    }
    {
        DConfigFile config(APP_ID, FILE_NAME, "/a/b");
        ASSERT_TRUE(config.load(LocalPrefix));
    }
    {
        DConfigFile config(APP_ID, FILE_NAME, "/a/b/..");
        ASSERT_TRUE(config.load(LocalPrefix));
    }
    {
        DConfigFile config(APP_ID, FILE_NAME, "/../a/b");
        ASSERT_FALSE(config.load(LocalPrefix));
    }
    {
        DConfigFile config(APP_ID, FILE_NAME, "/a/b/../../..");
        ASSERT_FALSE(config.load(LocalPrefix));
    }
}

TEST_F(ut_DConfigFile, userPublic) {

    FileCopyGuard guard(":/data/dconf-example.meta.json", QString("%1/%2.json").arg(metaPath, FILE_NAME));
    {
        DConfigFile config(APP_ID, FILE_NAME);
        ASSERT_TRUE(config.load(LocalPrefix));
        ASSERT_TRUE(config.meta()->flags("publicConfig").testFlag(DConfigFile::UserPublic));
        ASSERT_FALSE(config.meta()->flags("canExit").testFlag(DConfigFile::UserPublic));
    }
}

class ut_DConfigFileCheckName : public ut_DConfigFile, public ::testing::WithParamInterface<std::tuple<QString, bool>>
{

};

TEST_P(ut_DConfigFileCheckName, checkName)
{
    const auto [fileName, isValid] = GetParam();
    FileCopyGuard guard(":/data/dconf-example.meta.json", QString("%1/%2.json").arg(metaPath, fileName));
    DConfigFile config(APP_ID, fileName);
    ASSERT_EQ(config.load(LocalPrefix), isValid);
}
INSTANTIATE_TEST_SUITE_P(checkName, ut_DConfigFileCheckName,
                         ::testing::Values(
                             std::tuple{QString("org-foo"), true},
                             std::tuple{QString("org foo"), true},
                             std::tuple{QString("org.foo2"), true},
                             std::tuple{QString("org/foo"), false},
                             std::tuple{QString("./org-foo"), false},
                             std::tuple{QString("../configs/org-foo"), false}));

class ut_DConfigFileCheckAppId : public ut_DConfigFile, public ::testing::WithParamInterface<std::tuple<QString, bool>>
{

};

TEST_P(ut_DConfigFileCheckAppId, checkAppId)
{
    const auto [appId, isValid] = GetParam();
    FileCopyGuard guard(":/data/dconf-example.meta.json", QString("%1/%2/%3.json").arg(noAppidMetaPath, appId, FILE_NAME));
    DConfigFile config(appId, FILE_NAME);
    ASSERT_EQ(config.load(LocalPrefix), isValid);
}
INSTANTIATE_TEST_SUITE_P(checkAppId, ut_DConfigFileCheckAppId,
                         ::testing::Values(
                             std::tuple{NoAppId, true},
                             std::tuple{QString("org-foo"), true},
                             std::tuple{QString("org foo"), false},
                             std::tuple{QString("org.foo2"), true},
                             std::tuple{QString("org/foo"), false},
                             std::tuple{QString("./org-foo"), false},
                             std::tuple{QString("../configs/org-foo"), false}));
