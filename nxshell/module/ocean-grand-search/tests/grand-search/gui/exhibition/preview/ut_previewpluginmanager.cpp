// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gui/exhibition/preview/previewpluginmanager.h"
#include "utils/previewpluginconf.h"

#include "stubext.h"

#include <gtest/gtest.h>

using namespace testing;
using namespace GrandSearch;

TEST(PreviewPluginManagerTest, constructor)
{
    stub_ext::StubExt stu;

    bool ut_call_readConfig = false;
    stu.set_lamda(&PreviewPluginManager::readPluginConfig, [&](){
       ut_call_readConfig = true;
       return true;
    });

    PreviewPluginManager *m = new PreviewPluginManager;

    ASSERT_TRUE(m);

    EXPECT_TRUE(ut_call_readConfig);

    delete m;
}

TEST(PreviewPluginManagerTest, getPreviewPlugin_item)
{
    PreviewPluginManager m;

    stub_ext::StubExt stu;

    bool ut_load = false;
    stu.set_lamda(&QPluginLoader::load, [&](){
       return ut_load;
    });

    PreviewPluginInfo infoA;
    infoA.bValid = false;

    PreviewPluginInfo infoB;
    infoB.bValid = true;
    infoB.mimeTypes.append("inode/directory");
    infoB.path = "/test/";

    m.m_plugins.append(infoA);
    m.m_plugins.append(infoB);

    MatchedItem item;
    item.item = "/usr/bin/";

    PreviewPlugin *p = m.getPreviewPlugin(item);
    EXPECT_EQ(p, nullptr);

    m.m_plugins.removeLast();
    infoB.mimeTypes.append("inode/*");
    m.m_plugins.append(infoB);

    ut_load = true;
    m.getPreviewPlugin(item);
    EXPECT_EQ(p, nullptr);
}

TEST(PreviewPluginManagerTest, isMimeTypeMatch)
{
    PreviewPluginManager m;

    QString mimetype = "inode/directory";

    QStringList supportMimeType{"inode/directory"};

    bool result = m.isMimeTypeMatch(mimetype, supportMimeType);
    EXPECT_TRUE(result);

    supportMimeType.clear();
    supportMimeType.append("inode/*");

    result = m.isMimeTypeMatch(mimetype, supportMimeType);
    EXPECT_TRUE(result);

    supportMimeType.clear();
    supportMimeType.append("test");

    result = m.isMimeTypeMatch(mimetype, supportMimeType);
    EXPECT_FALSE(result);
}

TEST(PreviewPluginManagerTest, clearPluginInfo)
{
    PreviewPluginManager m;

    PreviewPluginInfo infoA;
    infoA.bValid = false;

    infoA.pPlugin = new QPluginLoader;

    m.m_plugins.append(infoA);
    m.clearPluginInfo();
    EXPECT_TRUE(m.m_plugins.isEmpty());
}

TEST(PreviewPluginManagerTest, readPluginConfig)
{
    PreviewPluginManager mw;

    stub_ext::StubExt stu;

    bool ut_call_loadConfig = false;
    stu.set_lamda(&PreviewPluginManager::loadConfig, [&](){
       ut_call_loadConfig = true;
       return ut_call_loadConfig;
    });

    mw.readPluginConfig();
    EXPECT_TRUE(ut_call_loadConfig);
}

TEST(PreviewPluginManagerTest, loadConfig)
{
    PreviewPluginManager m;

    stub_ext::StubExt stu;

    bool ut_isReadable = false;
    stu.set_lamda(&QDir::isReadable, [&](){
        return ut_isReadable;
    });

    m.m_paths.clear();
    m.m_paths.append("/usr/bin/");

    m.loadConfig();
    EXPECT_TRUE(m.m_plugins.isEmpty());

    ut_isReadable = true;

    QFileInfoList fis;
    fis.append(QFileInfo("/home/null"));

    auto ut_entryInfoList = (QFileInfoList (QDir::*)(const QStringList &, QDir::Filters, QDir::SortFlags) const)(&QDir::entryInfoList);
    stu.set_lamda(ut_entryInfoList, [&](){
        return fis;
    });

    bool ut_readInfo = false;
    stu.set_lamda(&PreviewPluginManager::readInfo, [&](){
        return ut_readInfo;
    });

    m.loadConfig();
    EXPECT_TRUE(m.m_plugins.isEmpty());

    ut_readInfo = true;

    bool ut_dowmwardCompatiobility = false;
    stu.set_lamda(&PreviewPluginManager::downwardCompatibility, [&](){
        return ut_dowmwardCompatiobility;
    });

    m.loadConfig();
    EXPECT_TRUE(m.m_plugins.isEmpty());

    ut_dowmwardCompatiobility = true;
    m.loadConfig();
    EXPECT_FALSE(m.m_plugins.isEmpty());
}

TEST(PreviewPluginManagerTest, readInfo)
{
    PreviewPluginManager m;

    stub_ext::StubExt stu;

    QStringList groups;
    stu.set_lamda(&QSettings::childGroups, [&]() {
        return groups;
    });

    QString path("/usr/bin");
    PreviewPluginInfo info;

    // 测试无组信息
    bool result = m.readInfo(path, info);
    EXPECT_FALSE(result);

    // 添加组
    groups.append(PREVIEW_PLUGINIFACE_CONF_ROOT);

    QStringList called;
    QVariantHash conf;
    stu.set_lamda(&QSettings::value, [&](QSettings *set, const QString &key, const QVariant &defaultValue)->QVariant {
         return conf.value(key, defaultValue);
    });

    // 测试名称为空
    result = m.readInfo(path, info);
    EXPECT_FALSE(result);

    // 添加名称
    conf.insert(PREVIEW_PLUGINIFACE_CONF_NAME, "testName");

    PreviewPluginInfo *ut_info = new PreviewPluginInfo;
    auto ut_getPreviewPlugin = (PreviewPluginInfo*(PreviewPluginManager::*)(const QString &))(&PreviewPluginManager::getPreviewPlugin);
    stu.set_lamda(ut_getPreviewPlugin, [&](){
        return ut_info;
    });

    // 测试插件已经存在
    result = m.readInfo(path, info);
    EXPECT_FALSE(result);

    // 删除插件
    delete ut_info;
    ut_info = nullptr;

    // 测试版本号为空
    result = m.readInfo(path, info);
    EXPECT_FALSE(result);

    // 添加版本号
    conf.insert(PREVIEW_PLUGINIFACE_CONF_VERSION, "testVersion");

    // 测试mimetype为空
    result = m.readInfo(path, info);
    EXPECT_FALSE(result);

    // 添加mimetype列表
    conf.insert(PREVIEW_PLUGINIFACE_CONF_MIMETYPES, "testMimeType");
    result = m.readInfo(path, info);
    EXPECT_TRUE(result);
}

TEST(PreviewPluginManagerTest, getPreviewPlugin_name)
{
    PreviewPluginManager m;

    QString name;
    PreviewPluginInfo *previewPluginInfo = m.getPreviewPlugin(name);
    EXPECT_EQ(previewPluginInfo, nullptr);

    name = "testName";
    previewPluginInfo = m.getPreviewPlugin(name);
    EXPECT_EQ(previewPluginInfo, nullptr);

    PreviewPluginInfo infoA;
    infoA.name = name;

    m.m_plugins.append(infoA);

    previewPluginInfo = m.getPreviewPlugin(name);
    EXPECT_EQ(previewPluginInfo->name, infoA.name);
}

TEST(PreviewPluginManagerTest, setPluginPath)
{
    PreviewPluginManager m;

    stub_ext::StubExt stu;

    bool ut_isReadable = false;
    stu.set_lamda(&QDir::isReadable, [&](){
        return ut_isReadable;
    });

    QStringList lst{"/usr/bin/", "/home/"};

    m.setPluginPath(lst);
    EXPECT_TRUE(m.m_paths.isEmpty());
    EXPECT_NE(m.m_paths, lst);

    ut_isReadable = true;
    m.setPluginPath(lst);
    EXPECT_FALSE(m.m_paths.isEmpty());
    EXPECT_EQ(m.m_paths, lst);
}

TEST(PreviewPluginManagerTest, downwardCompatibility)
{
    PreviewPluginManager m;

    bool result =  m.downwardCompatibility(QString());
    EXPECT_TRUE(result);
}
