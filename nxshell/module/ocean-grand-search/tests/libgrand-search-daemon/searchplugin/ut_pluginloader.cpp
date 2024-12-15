// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global/grandsearch_global.h"
#include "searchplugin/loader/pluginloader.h"
#include "searchplugin/convertors/dataconvertor.h"
#include "global/builtinsearch.h"
#include "utils/searchpluginconf.h"

#include <stubext.h>

#include <gtest/gtest.h>

#include <QTest>
#include <QDir>

GRANDSEARCH_USE_NAMESPACE

TEST(PluginLoader, ut_setPluginPath)
{
    stub_ext::StubExt st;
    st.set_lamda(&QDir::isReadable, [](QDir *dir) {
        return dir->absolutePath() == "/home";
    });

    PluginLoader load;
    QStringList paths = {"/home","/root"};
    load.setPluginPath(paths);
    EXPECT_EQ(load.m_paths, QStringList{"/home"});
}

TEST(PluginLoader, ut_load)
{
    stub_ext::StubExt st;
    st.set_lamda(&QDir::isReadable, [](QDir *dir) {
        return dir->absolutePath() == "/home";
    });

    st.set_lamda(&DataConvertor::isSupported, [](DataConvertor *, const QString &ver) {
        return ver == "test";
    });


    QFileInfoList fis;
    //构造内部搜索项
    DEF_BUILTISEARCH_NAMES;
    QStringList defPaths;
    for(const QString &path : predefBuiltinSearches) {
        defPaths.append("/home/" + path);
        fis.append(QFileInfo(defPaths.last()));
    }

    fis.append(QFileInfo("/home/null"));
    fis.append(QFileInfo("/home/inver"));
    fis.append(QFileInfo("/home/vaild"));
    fis.append(QFileInfo("/home/vaild"));
    fis.append(QFileInfo("/home/vaild"));

    auto entryInfoList = (QFileInfoList (QDir::*)(const QStringList &, QDir::Filters, QDir::SortFlags) const)(&QDir::entryInfoList);
    st.set_lamda(entryInfoList, [fis](QDir *dir, const QStringList &nameFilters, QDir::Filters filters, QDir::SortFlags) -> QFileInfoList{
        EXPECT_EQ(nameFilters, QStringList{"*.conf"});
        EXPECT_EQ(filters, QDir::Files);
        EXPECT_EQ(dir->absolutePath(), QString("/home"));
        if (dir->absolutePath() == "/home") {
            return fis;
        } else {
            return QFileInfoList{};
        }
    });

    int vailidCount = 0;
    st.set_lamda(&PluginLoader::readInfo, [defPaths, &vailidCount](PluginLoader *, const QString &path, GrandSearch::SearchPluginInfo &info) {
        info.ifsVersion = "test";
        info.name = path;
        if (path == "/home/null")
            return false;
        else if (defPaths.contains(path)) {
            info.name.remove("/home/");
            return true;
        }
        else if (path == "/home/inver") {
            info.ifsVersion = "inver";
            return true;
        } else if (path == "/home/vaild") {
            info.service = QString::number(vailidCount++);
            return true;
        }
        return false;
    });

    PluginLoader load;
    QStringList paths = {"/home","/root"};
    load.m_paths = paths;
    load.m_plugins.insert("org", {});

    EXPECT_TRUE(load.load());
    EXPECT_EQ(load.m_plugins.size(), 1);

    EXPECT_FALSE(load.m_plugins.contains("org"));
    for (const QString &path : defPaths)
        EXPECT_FALSE(load.m_plugins.contains(path));

    EXPECT_FALSE(load.m_plugins.contains("/home/null"));
    EXPECT_FALSE(load.m_plugins.contains("/home/inver"));
    EXPECT_TRUE(load.m_plugins.contains("/home/vaild"));
    EXPECT_EQ(vailidCount, 3);
}

TEST(PluginLoader, ut_plugins)
{
    PluginLoader load;
    load.m_plugins.insert("test",{});

    EXPECT_EQ(load.m_plugins.size(), load.plugins().size());
}

TEST(PluginLoader, ut_getPlugin)
{
    PluginLoader load;
    GrandSearch::SearchPluginInfo info;
    info.name = "test";
    load.m_plugins.insert("test", info);

    GrandSearch::SearchPluginInfo ret;
    EXPECT_FALSE(load.getPlugin("test1", ret));
    EXPECT_EQ(ret.name, GrandSearch::SearchPluginInfo().name);

    ret = GrandSearch::SearchPluginInfo();
    EXPECT_TRUE(load.getPlugin("test", ret));
    EXPECT_EQ(ret.name, info.name);
}

TEST(PluginLoader, ut_readInfo)
{
    stub_ext::StubExt st;
    QStringList groups = {PLUGININTERFACE_CONF_ROOT};
    st.set_lamda(&QSettings::childGroups, [&groups](QSettings *set) {
        if (set->fileName().startsWith("/home"))
            return groups;
        else
            return QStringList();
    });

    QStringList called;
    QVariantHash conf;
    st.set_lamda(&QSettings::value, [&groups, &called, &conf](QSettings *set, const QString &key, const QVariant &defaultValue)->QVariant {
         if (!set->fileName().startsWith("/home"))
            return "";
         called.append(key);
         return conf.value(key, defaultValue);
    });

    PluginLoader load;
    GrandSearch::SearchPluginInfo info;
    EXPECT_FALSE(load.readInfo("/var", info));
    EXPECT_TRUE(info.name.isEmpty());
    EXPECT_TRUE(called.isEmpty());

    called.clear();
    info = GrandSearch::SearchPluginInfo();
    EXPECT_FALSE(load.readInfo("/home", info));
    EXPECT_TRUE(info.name.isEmpty());
    EXPECT_TRUE(called.size() == 1);
    EXPECT_TRUE(called.contains(PLUGININTERFACE_CONF_NAME));

    //name
    called.clear();
    info = GrandSearch::SearchPluginInfo();
    conf.insert(PLUGININTERFACE_CONF_NAME, "test");

    EXPECT_FALSE(load.readInfo("/home", info));
    EXPECT_EQ(info.name, QString("test"));
    EXPECT_TRUE(called.size() == 2);
    EXPECT_TRUE(called.contains(PLUGININTERFACE_CONF_NAME));
    EXPECT_TRUE(called.contains(PLUGININTERFACE_CONF_INTERFACEVERSION));

    //dup name
    load.m_plugins.insert("test",{});
    called.clear();
    info = GrandSearch::SearchPluginInfo();
    EXPECT_FALSE(load.readInfo("/home", info));
    EXPECT_EQ(info.name, QString("test"));
    EXPECT_TRUE(called.size() == 1);
    EXPECT_TRUE(called.contains(PLUGININTERFACE_CONF_NAME));


    load.m_plugins.clear();
    called.clear();
    info = GrandSearch::SearchPluginInfo();
    conf.insert(PLUGININTERFACE_CONF_PRIORITY, GrandSearch::SearchPluginInfo::Priority::High);
    conf.insert(PLUGININTERFACE_CONF_INTERFACEVERSION, "1.0");
    conf.insert(PLUGININTERFACE_CONF_MODE, "auto");
    conf.insert(PLUGININTERFACE_CONF_EXEC, "/usr/bin/false");
    conf.insert(PLUGININTERFACE_CONF_DBUSSERVICE, "srv");
    conf.insert(PLUGININTERFACE_CONF_DBUSADDRESS, "addr");
    conf.insert(PLUGININTERFACE_CONF_DBUSINTERFACE, "ifs");


    EXPECT_TRUE(load.readInfo("/home", info));
    EXPECT_TRUE(called.size() == 8);
    EXPECT_TRUE(called.contains(PLUGININTERFACE_CONF_NAME));
    EXPECT_TRUE(called.contains(PLUGININTERFACE_CONF_INTERFACEVERSION));
    EXPECT_TRUE(called.contains(PLUGININTERFACE_CONF_MODE));
    EXPECT_TRUE(called.contains(PLUGININTERFACE_CONF_PRIORITY));
    EXPECT_TRUE(called.contains(PLUGININTERFACE_CONF_EXEC));
    EXPECT_TRUE(called.contains(PLUGININTERFACE_CONF_DBUSSERVICE));
    EXPECT_TRUE(called.contains(PLUGININTERFACE_CONF_DBUSADDRESS));
    EXPECT_TRUE(called.contains(PLUGININTERFACE_CONF_DBUSINTERFACE));
    EXPECT_EQ(info.name, QString("test"));
    EXPECT_EQ(info.ifsVersion, QString("1.0"));
    EXPECT_EQ(info.mode, GrandSearch::SearchPluginInfo::Mode::Auto);
    EXPECT_EQ(info.priority, GrandSearch::SearchPluginInfo::Priority::High);
    EXPECT_EQ(info.exec, QString("/usr/bin/false"));
    EXPECT_EQ(info.service, QString("srv"));
    EXPECT_EQ(info.address, QString("addr"));
    EXPECT_EQ(info.interface, QString("ifs"));
    EXPECT_EQ(info.from, QString("/home"));
}
