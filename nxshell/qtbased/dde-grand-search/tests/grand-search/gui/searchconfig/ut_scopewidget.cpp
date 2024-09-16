// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gui/searchconfig/scopewidget.h"
#include "gui/searchconfig/switchwidget/switchwidget.h"
#include "business/config/searchconfig.h"

#include "stubext.h"

#include <gtest/gtest.h>

#include <QFileInfo>
#include <QDir>

using namespace testing;
using namespace GrandSearch;

TEST(ScopeWidgetTest, constructor)
{
    stub_ext::StubExt stu;

    bool ut_isValid = false;
    stu.set_lamda(&ScopeWidget::isValid, [&]() {
        return ut_isValid;
    });

    bool ut_call_updateIcons = false;
    stu.set_lamda(&ScopeWidget::updateIcons, [&]() {
        ut_call_updateIcons = true;
    });

    ScopeWidget *w = new ScopeWidget;

    EXPECT_TRUE(w);
    EXPECT_TRUE(ut_call_updateIcons);

    delete w;
}

TEST(ScopeWidgetTest, onSwitchStateChanged)
{
    ScopeWidget w;

    stub_ext::StubExt stu;

    SwitchWidget *switchWidget = new SwitchWidget;
    stu.set_lamda(&QObject::sender, [&]() {
        return switchWidget;
    });

    bool ut_call_setConfig = false;
    stu.set_lamda(&SearchConfig::setConfig, [&]() {
        ut_call_setConfig = true;
    });

    w.onSwitchStateChanged(true);

    EXPECT_TRUE(ut_call_setConfig);

    delete switchWidget;
}

TEST(ScopeWidgetTest, updateIcons)
{
    ScopeWidget w;

    stub_ext::StubExt stu;

    bool ut_call_setIcon = false;
    stu.set_lamda(&SwitchWidget::setIcon, [&]() {
        ut_call_setIcon = true;
    });

    w.updateIcons();
    EXPECT_TRUE(ut_call_setIcon);
}

TEST(ScopeWidgetTest, isValid)
{
    ScopeWidget w;

    QString item;

    // 测试名称为空
    bool result = w.isValid(item);
    EXPECT_FALSE(result);

    // 测试配置不存在
    item = "testName";
    result = w.isValid(item);
    EXPECT_FALSE(result);

    // 测试配置存在
    stub_ext::StubExt stu;

    QFileInfoList fis;
    fis.append(QFileInfo("/home/null"));

    auto ut_entryInfoList = (QFileInfoList (QDir::*)(const QStringList &, QDir::Filters, QDir::SortFlags) const)(&QDir::entryInfoList);
    stu.set_lamda(ut_entryInfoList, [&]() {
        return fis;
    });

    QVariantHash conf;
    conf.insert("Grand Search/Name", "testName");
    stu.set_lamda(&QSettings::value, [&](QSettings *set, const QString &key, const QVariant &defaultValue)->QVariant {
         return conf.value(key, defaultValue);
    });

    result = w.isValid(item);
    EXPECT_TRUE(result);
}

