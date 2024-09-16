// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include <QDir>
#include <QList>
#include <QProcessEnvironment>
#include <QDebug>

#include "test_helper.hpp"
#include "filesystem/dstandardpaths.h"

DCORE_USE_NAMESPACE

class ut_DStandardPaths : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;
};

void ut_DStandardPaths::SetUp()
{
    QDir dir("/tmp/etc/");
    if (!dir.exists())
        dir.mkdir("/tmp/etc/");
    DStandardPaths::setMode(DStandardPaths::Snap);
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    if (env.value("SNAP_USER_COMMON").isEmpty())
        qputenv("SNAP_USER_COMMON", "/tmp/etc");
}

void ut_DStandardPaths::TearDown()
{
    QDir dir("/tmp/etc/");
    if (dir.exists())
        dir.remove("/tmp/etc/");
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    if (env.value("SNAP_USER_COMMON") == "/tmp/etc")
        qputenv("SNAP_USER_COMMON", "");
    DStandardPaths::setMode(DStandardPaths::Auto);
}

TEST_F(ut_DStandardPaths, testDStandardPathsWritableLocation)
{
    QString dir = DStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    ASSERT_TRUE(dir == "/tmp/etc");

    DStandardPaths::setMode(DStandardPaths::Test);
    dir = DStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    ASSERT_TRUE(dir == QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));
}

TEST_F(ut_DStandardPaths, testDStandardPathsStandardLocations)
{
    QStringList dirs = DStandardPaths::standardLocations(QStandardPaths::DesktopLocation);
    ASSERT_TRUE(dirs.contains("/tmp/etc"));

    DStandardPaths::setMode(DStandardPaths::Test);
    dirs = DStandardPaths::standardLocations(QStandardPaths::DesktopLocation);
    ASSERT_TRUE(dirs == QStandardPaths::standardLocations(QStandardPaths::DesktopLocation));
}

TEST_F(ut_DStandardPaths, testD_Q)
{
    ASSERT_EQ(DStandardPaths::locate(QStandardPaths::ApplicationsLocation, "sh"),
                QStandardPaths::locate(QStandardPaths::ApplicationsLocation, "sh"));

    ASSERT_EQ(DStandardPaths::locateAll(QStandardPaths::ApplicationsLocation, "sh"),
                QStandardPaths::locateAll(QStandardPaths::ApplicationsLocation, "sh"));

    ASSERT_EQ(DStandardPaths::findExecutable("sh"), QStandardPaths::findExecutable("sh"));
}

TEST_F(ut_DStandardPaths, homepath)
{
    EnvGuard homeGuard;
    homeGuard.unset("HOME");
    auto homePath = DStandardPaths::homePath();
    ASSERT_EQ(DStandardPaths::homePath(getuid()), homePath);

    homeGuard.set("HOME", "/tmp/home", false);
    homePath = DStandardPaths::homePath();
    ASSERT_EQ("/tmp/home", homePath);
    homeGuard.restore();
}

TEST_F(ut_DStandardPaths, xdgpath)
{
    auto testFunc = [](DStandardPaths::XDG type, const QLatin1String &env, const QByteArray &sufix,
            const QString &prefix = DStandardPaths::homePath()) {
        EnvGuard guard;
        guard.unset(env.data());
        QString path = DStandardPaths::path(type);
        const QString &localpath = prefix + sufix;
        ASSERT_EQ(path, localpath);

        QByteArray custom = QByteArray("/tmp/home") + sufix;
        guard.set(env.data(), custom, false);
        qputenv(env.data(), custom.data());
        path = DStandardPaths::path(type);
        ASSERT_EQ(path, custom);
        guard.restore();
    };

    testFunc(DStandardPaths::XDG::DataHome, QLatin1String("XDG_DATA_HOME"), "/.local/share");
    testFunc(DStandardPaths::XDG::CacheHome, QLatin1String("XDG_CACHE_HOME"), "/.cache");
    testFunc(DStandardPaths::XDG::ConfigHome, QLatin1String("XDG_CONFIG_HOME"), "/.config");
    testFunc(DStandardPaths::XDG::RuntimeDir, QLatin1String("XDG_RUNTIME_DIR"), QByteArray::number(getuid()), "/run/user/");
}

TEST_F(ut_DStandardPaths, dsgpath)
{
    {
        EnvGuard guard;
        guard.unset("DSG_APP_DATA");
        QString path = DStandardPaths::path(DStandardPaths::DSG::AppData);
        ASSERT_EQ(path, QString());

        guard.set("DSG_APP_DATA", "/tmp/dsg");
        path = DStandardPaths::path(DStandardPaths::DSG::AppData);
        ASSERT_EQ(path, "/tmp/dsg");
        guard.restore();
    }
    {
        EnvGuard guard;
        guard.unset("DSG_DATA_DIRS");
        QString path = DStandardPaths::path(DStandardPaths::DSG::DataDir);
        ASSERT_EQ(path, QString(PREFIX"/share/dsg"));

        guard.set("DSG_DATA_DIRS", "/tmp/dsg");
        path = DStandardPaths::path(DStandardPaths::DSG::DataDir);
        ASSERT_EQ(path, "/tmp/dsg");
        guard.restore();
    }
}

TEST_F(ut_DStandardPaths, filepath)
{
    {
        EnvGuard guard;
        guard.unset("DSG_APP_DATA");
        QString path = DStandardPaths::filePath(DStandardPaths::DSG::AppData, "filename");
        ASSERT_EQ(path, QString());

        guard.set("DSG_APP_DATA", "/tmp/dsg");
        path = DStandardPaths::filePath(DStandardPaths::DSG::AppData, "filename");
        ASSERT_EQ(path, "/tmp/dsg/filename");
        guard.restore();
    }

    QString path = DStandardPaths::filePath(DStandardPaths::XDG::CacheHome, "filename");
    ASSERT_EQ(path, DStandardPaths::path(DStandardPaths::XDG::CacheHome).append("/filename"));
}
