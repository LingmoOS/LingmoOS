// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gui/exhibition/preview/generalwidget/detailwidget.h"
#include "gui/exhibition/preview/previewplugin.h"

#include "stubext.h"

#include <gtest/gtest.h>

using namespace testing;
using namespace GrandSearch;

TEST(DetailWidgetTest, constructor)
{
    DetailWidget *w = new DetailWidget;

    ASSERT_TRUE(w);

    delete w;
}

TEST(DetailWidgetTest, setDetailInfoList)
{
    DetailWidget w;

    QString testKey("testKey");
    QString testVal("testVal");
    DetailTagInfo tagInfos;
    tagInfos.insert(DetailInfoProperty::Text, QVariant(testKey));
    tagInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideNone));

    DetailContentInfo contentInfos;
    contentInfos.insert(DetailInfoProperty::Text, QVariant(testVal));
    contentInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideMiddle));

    DetailInfo detailInfo = qMakePair(tagInfos, contentInfos);

    DetailInfoList list{detailInfo, detailInfo};

    w.m_alignment = true;

    w.setDetailInfoList(list);

    EXPECT_FALSE(w.m_detailItems.isEmpty());
}
