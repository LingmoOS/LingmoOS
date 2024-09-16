// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "text-preview/textpreview_global.h"
#include "text-preview/textpreviewplugin.h"
#include "text-preview/textview.h"

#include <stubext.h>

#include <gtest/gtest.h>

TEXT_PREVIEW_USE_NAMESPACE

TEST(TextPreviewPlugin, ut_init)
{
    TextPreviewPlugin tp;
    EXPECT_EQ(tp.m_view, nullptr);

    stub_ext::StubExt stub;
    bool ui = false;
    stub.set_lamda(&TextView::initUI,[&ui](){
        ui = true;
    });
    tp.init(nullptr);
    EXPECT_NE(tp.m_view, nullptr);
    EXPECT_TRUE(ui);
}

TEST(TextPreviewPlugin, ut_previewItem)
{
    stub_ext::StubExt stub;
    bool source = false;
    stub.set_lamda(&TextView::setSource,[&source](){
        source = true;
    });

    TextPreviewPlugin tp;
    tp.init(nullptr);

    GrandSearch::ItemInfo item;
    EXPECT_FALSE(tp.previewItem(item));
    EXPECT_FALSE(source);

    source = false;
    item.insert(PREVIEW_ITEMINFO_ITEM, "");
    EXPECT_FALSE(tp.previewItem(item));
    EXPECT_FALSE(source);

    source = false;
    item.insert(PREVIEW_ITEMINFO_ITEM, "/test");
    EXPECT_TRUE(tp.previewItem(item));
    EXPECT_TRUE(source);
}

TEST(TextPreviewPlugin, ut_item)
{
    TextPreviewPlugin tp;
    tp.m_item.insert("test", "1");

    EXPECT_EQ(tp.item(), tp.m_item);
}

TEST(TextPreviewPlugin, ut_contentWidget)
{
    TextPreviewPlugin tp;
    tp.init(nullptr);

    EXPECT_EQ(tp.contentWidget(), tp.m_view);
}

TEST(TextPreviewPlugin, ut_stopPreview)
{
    TextPreviewPlugin tp;
    EXPECT_TRUE(tp.stopPreview());
}

TEST(TextPreviewPlugin, ut_showToolBar)
{
    TextPreviewPlugin tp;
    EXPECT_TRUE(tp.showToolBar());
}

TEST(TextPreviewPlugin, ut_toolBarWidget)
{
    TextPreviewPlugin tp;
    EXPECT_EQ(tp.toolBarWidget(), nullptr);
}

TEST(TextPreviewPlugin, ut_getAttributeDetailInfo)
{
    TextPreviewPlugin tp;
    EXPECT_TRUE(tp.getAttributeDetailInfo().isEmpty());
}
