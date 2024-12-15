// SPDX-FileCopyrightText: 2021 - 2023 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QBuffer>
#include <QFile>
#include <QLocale>
#include <QSignalSpy>
#include <QDir>

#include <gtest/gtest.h>

#include "dconfigresource.h"
#include "dconfigconn.h"
#include "test_helper.hpp"

static constexpr char const *LocalPrefix = "/tmp/example/";
static constexpr char const *APP_ID = "org.foo.appid";
static constexpr char const *FILE_NAME = "example";

static QString configPath()
{
    const QString metaPath = QString("%1/usr/share/dsg/configs/%2").arg(LocalPrefix, APP_ID);
    return QString("%1/%2.json").arg(metaPath, FILE_NAME);
}
static QString noAppIdConfigPath()
{
    const QString metaPath = QString("%1/usr/share/dsg/configs/").arg(LocalPrefix);

    return QString("%1/%2.json").arg(metaPath, FILE_NAME);
}

static EnvGuard dsgDataDir;
class ut_DConfigResource : public testing::Test
{
protected:
    static void SetUpTestCase() {
        auto path = configPath();
        if (!QFile::exists(path)) {
            QDir("").mkpath(QFileInfo(path).path());
        }

        ASSERT_TRUE(QFile::copy(":/config/example.json", path));
        ASSERT_TRUE(QFile::copy(":/config/example.json", noAppIdConfigPath()));
        qputenv("DSG_CONFIG_CONNECTION_DISABLE_DBUS", "true");
        dsgDataDir.set("DSG_DATA_DIRS", "/usr/share/dsg");
    }
    static void TearDownTestCase() {
        QFile::remove(configPath());
        QFile::remove(noAppIdConfigPath());
        qunsetenv("DSG_CONFIG_CONNECTION_DISABLE_DBUS");
        dsgDataDir.restore();
    }
    virtual void SetUp() override {
        resource.reset(new DSGConfigResource(FILE_NAME, "", LocalPrefix));
    }
    virtual void TearDown() override {

    }

    QScopedPointer<DSGConfigResource> resource;
};

TEST_F(ut_DConfigResource, load) {

    ASSERT_TRUE(resource->load(APP_ID));
    ASSERT_TRUE(resource->load(VirtualInterAppId));
}
TEST_F(ut_DConfigResource, load_fail) {

    DSGConfigResource resource2("example_notexist", "");
    ASSERT_FALSE(resource2.load(APP_ID));
}
TEST_F(ut_DConfigResource, createConn) {

    resource->load(APP_ID);
    ASSERT_TRUE(resource->createConn(APP_ID, TestUid));

    resource->load(VirtualInterAppId);
    ASSERT_TRUE(resource->createConn(VirtualInterAppId, TestUid));
}
TEST_F(ut_DConfigResource, getConn) {

    resource->load(APP_ID);
    resource->createConn(APP_ID, TestUid);
    ASSERT_TRUE(resource->getConn(APP_ID, TestUid));

    resource->load(VirtualInterAppId);
    resource->createConn(VirtualInterAppId, TestUid);
    ASSERT_TRUE(resource->getConn(VirtualInterAppId, TestUid));

    ASSERT_EQ(resource->connSize(), 2);
}
TEST_F(ut_DConfigResource, fallbackToGenericConfig) {

    resource->load(APP_ID);
    ASSERT_TRUE(resource->fallbackToGenericConfig());
}

class ut_DConfigConn : public testing::Test
{
protected:
    static void SetUpTestCase() {
        auto path = configPath();
        if (!QFile::exists(path)) {
            QDir("").mkpath(QFileInfo(path).path());
        }

        ASSERT_TRUE(QFile::copy(":/config/example.json", path));
        ASSERT_TRUE(QFile::copy(":/config/example.json", noAppIdConfigPath()));
        qputenv("DSG_CONFIG_CONNECTION_DISABLE_DBUS", "true");
        dsgDataDir.set("DSG_DATA_DIRS", "/usr/share/dsg");
    }
    static void TearDownTestCase() {
        QFile::remove(configPath());
        QFile::remove(noAppIdConfigPath());
        QDir(LocalPrefix).removeRecursively();
        qunsetenv("DSG_CONFIG_CONNECTION_DISABLE_DBUS");
        dsgDataDir.restore();
    }
    virtual void SetUp() override {
        resource.reset(new DSGConfigResource("example", "", LocalPrefix));
        resource->load(APP_ID);
        conn = resource->createConn(APP_ID, TestUid);
        ASSERT_TRUE(conn);
    }
    virtual void TearDown() override {

    }
    DSGConfigConn* conn;
    QScopedPointer<DSGConfigResource> resource;
};

TEST_F(ut_DConfigConn, description_name) {
    ASSERT_EQ(conn->description("canExit", ""), "我是描述");
    ASSERT_EQ(conn->description("canExit", "en_US"), "I am description");
    ASSERT_EQ(conn->name("canExit", ""), "I am name");
    ASSERT_EQ(conn->name("canExit", "zh_CN"), QString("我是名字"));
    ASSERT_EQ(conn->name("canExit", QLocale(QLocale::Japanese).name()), "");
    ASSERT_EQ(conn->flags("canExit"), 0);
}

TEST_F(ut_DConfigConn, value) {
    conn->setValue("canExit", QDBusVariant{true});
    ASSERT_EQ(conn->value("canExit").variant(), true);
    const QStringList array{"value1", "value2"};
    QVariantMap map;
    map.insert("key1", "value1");
    map.insert("key2", "value2");
    ASSERT_EQ(conn->value("array").variant().toStringList(), array);
    ASSERT_EQ(conn->value("map").variant().toMap(), map);

    QVariantMap mapArray;
    mapArray.insert("key1", QStringList{"value1"});
    mapArray.insert("key2", QStringList{"value2"});
    QVariantList arrayMap;
    arrayMap.append(map);
    ASSERT_EQ(conn->value("map_array").variant().toMap(), mapArray);
    ASSERT_EQ(conn->value("array_map").variant().toList(), arrayMap);
}

TEST_F(ut_DConfigConn, setValue) {
    conn->setValue("canExit", QDBusVariant{false});

    QSignalSpy spy(conn, &DSGConfigConn::valueChanged);
    conn->setValue("canExit", QDBusVariant{true});
    ASSERT_EQ(conn->value("canExit").variant(), true);
    ASSERT_EQ(spy.count(), 1);
}

TEST_F(ut_DConfigConn, reset) {
    conn->setValue("canExit", QDBusVariant{false});

    QSignalSpy spy(conn, &DSGConfigConn::valueChanged);
    conn->reset("canExit");
    ASSERT_EQ(conn->value("canExit").variant(), true);
    ASSERT_EQ(spy.count(), 1);
}

TEST_F(ut_DConfigConn, visibility) {

    ASSERT_EQ(conn->visibility("canExit"), "private");
}

TEST_F(ut_DConfigConn, release) {
    QSignalSpy spy(conn, &DSGConfigConn::releaseChanged);
    ASSERT_EQ(spy.count(), 0);

    conn->release();

    ASSERT_EQ(spy.count(), 1);
    conn->release();
}

TEST_F(ut_DConfigConn, fallbackToGenericValue) {
    conn->reset("canExit");
    ASSERT_EQ(conn->value("canExit").variant(), true);

    // change generic value.
    ASSERT_TRUE(resource->load(VirtualInterAppId));
    auto genericConn = resource->createConn(VirtualInterAppId, TestUid);
    ASSERT_TRUE(genericConn);
    genericConn->setValue("canExit", QDBusVariant{false});
    resource->removeConn(genericConn->path());
    ASSERT_EQ(resource->connSize(), 1);

    // fallback to generic value
    ASSERT_EQ(conn->value("canExit").variant(), false);
}

TEST_F(ut_DConfigConn, appValue) {
    // change app value
    conn->setValue("canExit", QDBusVariant{false});
    ASSERT_EQ(conn->value("canExit").variant(), false);

    // change generic value.
    ASSERT_TRUE(resource->load(VirtualInterAppId));
    auto genericConn = resource->createConn(VirtualInterAppId, TestUid);
    ASSERT_TRUE(genericConn);
    genericConn->setValue("canExit", QDBusVariant{true});
    resource->removeConn(genericConn->path());
    ASSERT_EQ(resource->connSize(), 1);

    // can't fallback to generic value
    ASSERT_EQ(conn->value("canExit").variant(), false);
}

TEST_F(ut_DConfigConn, isDefaultValue) {
    conn->reset("canExit");
    ASSERT_TRUE(conn->isDefaultValue("canExit"));

    conn->setValue("canExit", QDBusVariant{false});
    ASSERT_FALSE(conn->isDefaultValue("canExit"));

    conn->reset("canExit");
    ASSERT_TRUE(conn->isDefaultValue("canExit"));
}
