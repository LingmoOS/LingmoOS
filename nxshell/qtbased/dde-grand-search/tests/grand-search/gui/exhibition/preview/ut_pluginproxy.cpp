// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gui/exhibition/preview/previewwidget.h"
#include "gui/exhibition/preview/pluginproxy.h"
#include "gui/exhibition/preview/generalwidget/detailwidget.h"

#include "stubext.h"

#include <gtest/gtest.h>

using namespace testing;
using namespace GrandSearch;

TEST(PluginProxyTest, constructor)
{
    PreviewWidget w;
    PluginProxy *p = new PluginProxy(&w);

    ASSERT_TRUE(p);
}

TEST(PluginProxyTest, updateDetailInfo)
{
    PreviewWidget w;
    PluginProxy *p = new PluginProxy(&w);

    stub_ext::StubExt stu;

    bool ut_call_set = false;
    stu.set_lamda(&DetailWidget::setDetailInfoList, [&](){
       ut_call_set = true;
    });

    p->updateDetailInfo(nullptr);
    EXPECT_FALSE(ut_call_set);

    ut_call_set = false;
    w.m_preview = w.m_generalPreview;
    p->updateDetailInfo(w.m_generalPreview.get());
    EXPECT_TRUE(ut_call_set);

    QThread threadA;
    QThread threadB;
    stu.set_lamda(ADDR(QObject, thread), [&](QObject *obj){
        if (obj == qApp)
            return &threadA;
        return &threadB;
    });

    ut_call_set = false;
    p->updateDetailInfo(w.m_generalPreview.get());
    EXPECT_FALSE(ut_call_set);
}
