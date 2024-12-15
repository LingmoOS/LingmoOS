// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>

#include <QSignalSpy>
#include <QTest>

#include "private/dconfigwrapper_p.h"
#include "test_helper.hpp"

static constexpr char const *LocalPrefix = "/tmp/example";
static constexpr char const *APP_ID = BIN_NAME;
static constexpr char const *FILE_NAME = "example";
class ut_DConfigWrapper : public ::testing::Test
{
public:
    virtual void SetUp()
    {
        const QString &target = QString("%1/usr/share/dsg/configs/%2/%3.json").arg(LocalPrefix, APP_ID, FILE_NAME);

        if (!QFile::exists(QFileInfo(target).path()))
            QDir().mkpath(QFileInfo(target).path());

        QFile::copy(":/data/dconf-example.meta.json", target);

        qputenv("DSG_DCONFIG_FILE_BACKEND_LOCAL_PREFIX", LocalPrefix);
        qputenv("DSG_DCONFIG_BACKEND_TYPE", "FileBackend");
        qputenv("DSG_DATA_DIRS", "/usr/share/dsg");
    }
    virtual void TearDown() {
        QDir(LocalPrefix).removeRecursively();
    }
};

TEST_F(ut_DConfigWrapper, componentComplete)
{
    QScopedPointer<DConfigWrapper> config(new DConfigWrapper());

    config->classBegin();
    config->setName("example");
    config->setSubpath("");
    config->componentComplete();

    ASSERT_TRUE(config->isValid());
}

TEST_F(ut_DConfigWrapper, setValue)
{
    QScopedPointer<DConfigWrapper> config(new DConfigWrapper());

    config->classBegin();
    config->setName("example");
    config->componentComplete();

    ASSERT_TRUE(config->isValid());

    EXPECT_EQ(config->subpath(), "");

    ASSERT_TRUE(config->keyList().contains("key3"));

    QSignalSpy spy(config.data(), &DConfigWrapper::valueChanged);

    QString newValue = config->value("key3", QString("default")).toString() + QString("abc");
    config->setValue("key3", newValue);

    ASSERT_EQ(config->value("key3").toString(), newValue);

    ASSERT_TRUE(QTest::qWaitFor([&spy](){
        return spy.count() == 1;
    }, 500));

    config->setProperty("key3", "23");
    ASSERT_EQ(config->value("key3").toString(), "23");

    config->resetValue("key3");
    ASSERT_EQ(config->value("key3").toString(), "application");
}

TEST_F(ut_DConfigWrapper, setValueByQml)
{
    ControlHelper<QQuickItem> helper("qrc:/qml/Config.qml");
    ASSERT_TRUE(helper.object);

    DConfigWrapper *config = helper.object->findChild<DConfigWrapper *>();
    ASSERT_TRUE(config);
    ASSERT_TRUE(config->isValid());

    EXPECT_EQ(config->property("key2"), "key2");
    EXPECT_EQ(config->value("key2"), "key2");

    // `key3`'s value(defined in qml) is only fallback value instead of current value.
    EXPECT_EQ(config->property("key3"), "application");
    EXPECT_EQ(config->value("key3"), "application");

    QSignalSpy key3Change(helper.object, SIGNAL(key3Changed()));
    config->metaObject()->invokeMethod(helper.object, "setKey3", Q_ARG(QVariant, QString("2")));
    EXPECT_EQ(key3Change.count(), 1);
    EXPECT_EQ(config->value("key3").toString(), "2");
}
