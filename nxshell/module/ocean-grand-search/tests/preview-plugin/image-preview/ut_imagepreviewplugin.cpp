// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "image-preview/imagepreview_global.h"
#include "image-preview/imagepreviewinterface.h"
#include "image-preview/imagepreviewplugin.h"
#include "image-preview/imageview.h"

#include <stubext.h>

#include <gtest/gtest.h>

#include <QTest>

IMAGE_PREVIEW_USE_NAMESPACE

TEST(ImagePreviewPluginTest, constructor)
{
    ImagePreviewPlugin *plugin = new ImagePreviewPlugin;

    EXPECT_TRUE(plugin);
    EXPECT_FALSE(plugin->m_imageView);

    delete plugin;
}

TEST(ImagePreviewPluginTest, init)
{
    ImagePreviewPlugin plugin;
    EXPECT_NO_FATAL_FAILURE(plugin.init(nullptr));
}

TEST(ImagePreviewPluginTest, previewItem)
{
    ImagePreviewPlugin plugin;

    stub_ext::StubExt stu;

    bool ut_call_loadImage = false;
    stu.set_lamda(&ImageView::loadImage, [&](){
        ut_call_loadImage = true;
    });

    QSize ut_sourceSize(100, 100);
    stu.set_lamda(&ImageView::sourceSize, [&](){
        return ut_sourceSize;
    });

    ItemInfo item;
    item.insert(PREVIEW_ITEMINFO_ITEM, "/home/test/test.png");
    item.insert(PREVIEW_ITEMINFO_TYPE, "png");

    bool result = plugin.previewItem(item);

    EXPECT_TRUE(result);
    EXPECT_TRUE(ut_call_loadImage);
}

TEST(ImagePreviewPluginTest, item)
{
    ImagePreviewPlugin plugin;

    ItemInfo info;
    info.insert("name", "test");
    plugin.m_item = info;

    ItemInfo result = plugin.item();
    EXPECT_EQ(result, info);
}

TEST(ImagePreviewPluginTest, contentWidget)
{
    ImagePreviewPlugin plugin;

    ImageView *view = new ImageView;
    plugin.m_imageView = view;

    QWidget *result = plugin.contentWidget();
    EXPECT_EQ(result, view);

    plugin.m_imageView = nullptr;
    delete view;
}

TEST(ImagePreviewPluginTest, stopPreview)
{
    ImagePreviewPlugin plugin;

    ImageView *view = new ImageView;
    plugin.m_imageView = view;

    stub_ext::StubExt stu;

    bool ut_call_stopPreview = false;

    stu.set_lamda(&ImageView::stopPreview, [&](){
        ut_call_stopPreview = true;
        return ut_call_stopPreview;
    });

    bool result = plugin.stopPreview();
    EXPECT_TRUE(result);
    EXPECT_TRUE(ut_call_stopPreview);
}

TEST(ImagePreviewPluginTest, getAttributeDetailInfo)
{
    ImagePreviewPlugin plugin;

    DetailTagInfo tagInfos;
    tagInfos.insert(DetailInfoProperty::Text, QVariant("testKey"));
    tagInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideNone));

    DetailContentInfo contentInfos;
    contentInfos.insert(DetailInfoProperty::Text, QVariant("testValue"));
    contentInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideMiddle));

    DetailInfo detailInfo = qMakePair(tagInfos, contentInfos);

    DetailInfoList lst{detailInfo};

    plugin.m_detailInfos = lst;

    DetailInfoList result = plugin.getAttributeDetailInfo();
    EXPECT_EQ(result, plugin.m_detailInfos);
}

TEST(ImagePreviewPluginTest, toolBarWidget)
{
    ImagePreviewPlugin plugin;

    QWidget *result = plugin.toolBarWidget();

    EXPECT_EQ(result, nullptr);
}

TEST(ImagePreviewPluginTest, showToolBar)
{
    ImagePreviewPlugin plugin;

    bool result = plugin.showToolBar();

    EXPECT_TRUE(result);
}
