// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global/grandsearch_global.h"
#include "searchplugin/pluginmanager_p.h"

#include <stubext.h>

#include <gtest/gtest.h>

#include <QTest>

GRANDSEARCH_USE_NAMESPACE

TEST(PluginManager, ut_loadPlugin)
{
    stub_ext::StubExt st;
    bool load = false;
    st.set_lamda(&PluginLoader::load, [&load]() {
        load = true;
        return true;
    });
    PluginManager pm;
    ASSERT_EQ(pm.d->m_loader, nullptr);
    ASSERT_EQ(pm.d->m_process, nullptr);

    EXPECT_TRUE(pm.loadPlugin());
    EXPECT_TRUE(load);
    EXPECT_NE(pm.d->m_loader, nullptr);
    EXPECT_NE(pm.d->m_process, nullptr);
}

TEST(PluginManager, ut_loadPlugin1)
{
    stub_ext::StubExt st;
    bool load = false;
    st.set_lamda(&PluginLoader::load, [&load]() {
        load = true;
        return false;
    });
    PluginManager pm;

    EXPECT_FALSE(pm.loadPlugin());
    EXPECT_TRUE(load);
    EXPECT_NE(pm.d->m_loader, nullptr);
    EXPECT_NE(pm.d->m_process, nullptr);
}

TEST(PluginManager, ut_plugins)
{
    PluginManager pm;

    ASSERT_EQ(pm.d->m_loader, nullptr);
    EXPECT_TRUE(pm.plugins().isEmpty());

    pm.d->m_loader = new PluginLoader(&pm);
    pm.d->m_loader->m_plugins.insert("test",{});

    EXPECT_FALSE(pm.plugins().isEmpty());
}

TEST(PluginManager, ut_autoActivate)
{
    PluginManager pm;
    pm.d->m_loader = new PluginLoader(&pm);
    pm.d->m_process = new PluginProcess(&pm);

    stub_ext::StubExt st;
    QStringList autoList;
    st.set_lamda(&PluginProcess::startProgram, [&autoList](PluginProcess *, const QString &name) {
        EXPECT_FALSE(autoList.contains(name));
        autoList.append(name);
        return true;
    });

    {
        GrandSearch::SearchPluginInfo info;
        info.mode = GrandSearch::SearchPluginInfo::Auto;

        info.priority = GrandSearch::SearchPluginInfo::High;
        info.name = "AH";
        pm.d->m_loader->m_plugins.insert("AH", info);

        info.priority = GrandSearch::SearchPluginInfo::Middle;
        info.name = "AM";
        pm.d->m_loader->m_plugins.insert("AM", info);

        info.priority = GrandSearch::SearchPluginInfo::Low;
        info.name = "AL";
        pm.d->m_loader->m_plugins.insert("AL", info);

        info.mode = GrandSearch::SearchPluginInfo::Manual;
        info.priority = GrandSearch::SearchPluginInfo::High;
        info.name = "MH";
        pm.d->m_loader->m_plugins.insert("MH", info);

        info.priority = GrandSearch::SearchPluginInfo::Middle;
        info.name = "MM";
        pm.d->m_loader->m_plugins.insert("MM", info);

        info.priority = GrandSearch::SearchPluginInfo::Low;
        info.name = "ML";
        pm.d->m_loader->m_plugins.insert("ML", info);

        info.mode = GrandSearch::SearchPluginInfo::Trigger;
        info.priority = GrandSearch::SearchPluginInfo::High;
        info.name = "TH";
        pm.d->m_loader->m_plugins.insert("TH", info);

        info.priority = GrandSearch::SearchPluginInfo::Middle;
        info.name = "TM";
        pm.d->m_loader->m_plugins.insert("TM", info);

        info.priority = GrandSearch::SearchPluginInfo::Low;
        info.name = "TL";
        pm.d->m_loader->m_plugins.insert("TL", info);

        info.mode = GrandSearch::SearchPluginInfo::Auto;
        info.priority = GrandSearch::SearchPluginInfo::High;
        info.name = "AH2";
        pm.d->m_loader->m_plugins.insert("AH2", info);
    }

    pm.autoActivate();

    EXPECT_EQ(autoList.size(), 2);
    EXPECT_TRUE(autoList.contains("AH"));
    EXPECT_TRUE(autoList.contains("AH2"));
}

TEST(PluginManager, ut_inactivate)
{
    PluginManager pm;
    pm.d->m_process = new PluginProcess(&pm);

    stub_ext::StubExt st;
    QString tname;
    st.set_lamda(&PluginProcess::terminate, [&tname](PluginProcess *, const QString &name) {
        tname = name;
    });

    pm.inactivate("test");
    EXPECT_EQ(tname, QString("test"));
}

TEST(PluginManagerPrivate, ut_readConf)
{
    stub_ext::StubExt st;
    bool load = false;
    st.set_lamda(&PluginLoader::load, [&load]() {
        load = !load;
        return load;
    });

    bool setPath = false;
    st.set_lamda(&PluginLoader::setPluginPath, [&setPath]() {
        setPath = !setPath;
    });

    PluginManager pm;
    ASSERT_EQ(pm.d->m_loader, nullptr);

    EXPECT_TRUE(pm.d->readConf());
    EXPECT_TRUE(load);
    EXPECT_TRUE(setPath);

    load = true;
    setPath = false;

    EXPECT_FALSE(pm.d->readConf());
    EXPECT_FALSE(load);
    EXPECT_FALSE(setPath);
}

TEST(PluginManager, ut_prepareProcess)
{
    PluginManager pm;
    pm.d->m_loader = new PluginLoader(&pm);
    pm.d->m_process = new PluginProcess(&pm);

    stub_ext::StubExt st;
    QStringList autoList;
    st.set_lamda(&PluginProcess::addProgram, [&autoList](PluginProcess *, const QString &name, const QString &path) {
        if (name == "AH2")
            return false;

        EXPECT_FALSE(autoList.contains(name));
        autoList.append(name);
        return true;
    });

    QStringList watchedList;
    st.set_lamda(&PluginProcess::setWatched, [&watchedList](PluginProcess *, const QString &name, bool) {
        EXPECT_FALSE(watchedList.contains(name));
        watchedList.append(name);
    });

    {
        GrandSearch::SearchPluginInfo info;
        info.mode = GrandSearch::SearchPluginInfo::Auto;

        info.priority = GrandSearch::SearchPluginInfo::High;
        info.name = "AH";
        pm.d->m_loader->m_plugins.insert("AH", info);

        info.priority = GrandSearch::SearchPluginInfo::High;
        info.name = "AH2";
        pm.d->m_loader->m_plugins.insert("AH2", info);

        info.priority = GrandSearch::SearchPluginInfo::Middle;
        info.name = "AM";
        pm.d->m_loader->m_plugins.insert("AM", info);

        info.priority = GrandSearch::SearchPluginInfo::Low;
        info.name = "AL";
        pm.d->m_loader->m_plugins.insert("AL", info);

        info.mode = GrandSearch::SearchPluginInfo::Trigger;
        info.priority = GrandSearch::SearchPluginInfo::High;
        info.name = "TH";
        pm.d->m_loader->m_plugins.insert("TH", info);

        info.priority = GrandSearch::SearchPluginInfo::Middle;
        info.name = "TM";
        pm.d->m_loader->m_plugins.insert("TM", info);

        info.priority = GrandSearch::SearchPluginInfo::Low;
        info.name = "TL";
        pm.d->m_loader->m_plugins.insert("TL", info);

        info.mode = GrandSearch::SearchPluginInfo::Manual;
        info.priority = GrandSearch::SearchPluginInfo::High;
        info.name = "MH";
        pm.d->m_loader->m_plugins.insert("MH", info);

        info.priority = GrandSearch::SearchPluginInfo::Middle;
        info.name = "MM";
        pm.d->m_loader->m_plugins.insert("MM", info);

        info.priority = GrandSearch::SearchPluginInfo::Low;
        info.name = "ML";
        pm.d->m_loader->m_plugins.insert("ML", info);
    }

    pm.d->prepareProcess();

    EXPECT_EQ(autoList.size(), 3);
    EXPECT_TRUE(autoList.contains("AH"));
    EXPECT_TRUE(autoList.contains("AM"));
    EXPECT_TRUE(autoList.contains("AL"));

    EXPECT_EQ(watchedList.size(), 2);
    EXPECT_TRUE(watchedList.contains("AH"));
    EXPECT_TRUE(watchedList.contains("AM"));
}
