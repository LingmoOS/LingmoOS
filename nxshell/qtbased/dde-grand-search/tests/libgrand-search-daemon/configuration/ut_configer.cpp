// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global/grandsearch_global.h"
#include "global/commontools.h"
#include "configuration/configer.h"
#include "configuration/configer_p.h"

#include <stubext.h>

#include <gtest/gtest.h>

#include <QTest>
GRANDSEARCH_USE_NAMESPACE

TEST(UserPreference, ut_initDefault)
{
    Configer conf;
    ASSERT_EQ(conf.d->m_root.get(), nullptr);
    conf.initDefault();

    ASSERT_NE(conf.d->m_root.get(), nullptr);
    EXPECT_NE(conf.d->m_root->group(GRANDSEARCH_PREF_SEARCHERENABLED).get(), nullptr);
#ifdef ENABLE_DEEPINANYTHING
    EXPECT_NE(conf.d->m_root->group(GRANDSEARCH_CLASS_FILE_DEEPIN).get(), nullptr);
#else
    EXPECT_EQ(conf.d->m_root->group(GRANDSEARCH_CLASS_FILE_DEEPIN).get(), nullptr);
#endif
    EXPECT_NE(conf.d->m_root->group(GRANDSEARCH_CLASS_FILE_FSEARCH).get(), nullptr);
}

TEST(UserPreference, ut_init)
{
    Configer conf;
    stub_ext::StubExt st;
    bool init = false;
    st.set_lamda(&Configer::initDefault, [&init]() {
        init = true;
    });

    bool load = false;
    st.set_lamda(&Configer::onLoadConfig, [&load]() {
        load = true;
    });

    auto orgFunc = (bool (QFileInfo::*)() const) & QFileInfo::exists;
    st.set_lamda(orgFunc, []() {
        return true;
    });

    ASSERT_EQ(conf.d->m_watcher, nullptr);
    ASSERT_TRUE(conf.d->m_configPath.isEmpty());

    //测试更换&释放 watcher
    auto watcher = new QFileSystemWatcher;
    conf.d->m_watcher = watcher;

    EXPECT_TRUE(conf.init());
    EXPECT_NE(conf.d->m_watcher, nullptr);
    EXPECT_FALSE(conf.d->m_configPath.isEmpty());
    EXPECT_TRUE(init);
    EXPECT_TRUE(load);
}

TEST(UserPreference, ut_group)
{
    Configer conf;
    ASSERT_EQ(conf.d->m_root.get(), nullptr);
    EXPECT_EQ(conf.group("test"), nullptr);

    conf.d->m_root.reset(new UserPreference({}));
    UserPreferencePointer va(new UserPreference({}));
    conf.d->m_root->setValue("test", QVariant::fromValue(va));

    EXPECT_EQ(conf.group("test"), va.get());
}

TEST(ConfigerPrivate, ut_defaultSearcher)
{
    auto up = ConfigerPrivate::defaultSearcher();
#ifdef ENABLE_DEEPINANYTHING
    EXPECT_TRUE(up->value(GRANDSEARCH_CLASS_FILE_DEEPIN, false));
#else
    EXPECT_FALSE(up->value(GRANDSEARCH_CLASS_FILE_DEEPIN, false));
#endif
    EXPECT_TRUE(up->value(GRANDSEARCH_CLASS_FILE_FSEARCH, false));
    EXPECT_TRUE(up->value(GRANDSEARCH_CLASS_APP_DESKTOP, false));
    EXPECT_TRUE(up->value(GRANDSEARCH_CLASS_SETTING_CONTROLCENTER, false));
    EXPECT_TRUE(up->value(GRANDSEARCH_CLASS_WEB_STATICTEXT, false));
}

TEST(ConfigerPrivate, ut_fileSearcher)
{
    auto up = ConfigerPrivate::fileSearcher();
    EXPECT_TRUE(up->value(GRANDSEARCH_GROUP_FOLDER, false));
    EXPECT_TRUE(up->value(GRANDSEARCH_GROUP_FILE, false));
    EXPECT_TRUE(up->value(GRANDSEARCH_GROUP_FILE_VIDEO, false));
    EXPECT_TRUE(up->value(GRANDSEARCH_GROUP_FILE_AUDIO, false));
    EXPECT_TRUE(up->value(GRANDSEARCH_GROUP_FILE_PICTURE, false));
    EXPECT_TRUE(up->value(GRANDSEARCH_GROUP_FILE_DOCUMNET, false));
}

TEST(ConfigerPrivate, ut_updateConfig1)
{
    Configer conf;
    EXPECT_FALSE(conf.d->updateConfig1(nullptr));
    QSettings set;
    EXPECT_FALSE(conf.d->updateConfig1(&set));

    conf.initDefault();
    EXPECT_TRUE(conf.d->updateConfig1(&set));
}

TEST(ConfigerPrivate, resetPath)
{
    stub_ext::StubExt st;
    st.set_lamda(CommonTools::bindPathTransform, []() { return "/data/home"; });

    Configer conf;
    QString path("/");
    conf.d->resetPath(path);
    EXPECT_EQ(QString("/"), path);
    path = QString("/data/home");
    conf.d->resetPath(path);
    EXPECT_EQ(QString("/home/"), path);
    path = QString("/data/home/aaa");
    conf.d->resetPath(path);
    EXPECT_EQ(QString("/home/aaa/"), path);
}
