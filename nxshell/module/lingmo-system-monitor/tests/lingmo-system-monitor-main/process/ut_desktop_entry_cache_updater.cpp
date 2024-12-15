// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "process/desktop_entry_cache.h"
#include "process/desktop_entry_cache_updater.h"
//gtest
#include "stub.h"
#include <gtest/gtest.h>
//Qt
#include <QFileInfo>
using namespace core::process;

class UT_DesktopEntryCacheUpdater : public ::testing::Test
{
public:
    UT_DesktopEntryCacheUpdater() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new DesktopEntryCacheUpdater();
    }

    virtual void TearDown()
    {
        if(m_tester){
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    DesktopEntryCacheUpdater *m_tester;
};

TEST_F(UT_DesktopEntryCacheUpdater, initTest)
{

}

TEST_F(UT_DesktopEntryCacheUpdater, test_createEntry_001)
{
    const QString filePath("test.desktop");
    const QString name = "test1";
    const QString displayName = "DisplayTest1";
    const QString exec = QString("/usr/bin/%1").arg(name);
    const QString icon = "/usr/share/test1/icon";
    const QString startup = name;

    QFile file(filePath);

    // 创建desktop文件
    if(file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QByteArray data;
        file.write("[Desktop Entry]\n");
        file.write(QString("Name=%1\n").arg(name).toUtf8());
        file.write(QString("GenericName=%1\n").arg(displayName).toUtf8());
        file.write("Type=Application\n");
        file.write(QString("Exec=%1\n").arg(exec).toUtf8());
        file.write(QString("Icon=%1\n").arg(icon).toUtf8());
        file.write("Categories=Utility\n");
        file.write(QString("StartupWMClass=%1\n").arg(startup).toUtf8());
        file.close();
    }

    QFileInfo info(filePath);
    DesktopEntry entry = m_tester->createEntry(info);

    EXPECT_EQ(entry->name, name);
    EXPECT_EQ(entry->displayName, name);
    EXPECT_EQ(entry->exec.first(), exec);
    EXPECT_EQ(entry->icon, icon);
    EXPECT_EQ(entry->startup_wm_class, startup);

    if(file.exists()) {
        file.remove();
    }

}
