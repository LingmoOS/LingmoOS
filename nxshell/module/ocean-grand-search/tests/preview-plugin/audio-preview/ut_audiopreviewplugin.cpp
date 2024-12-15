// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "audio-preview/audiopreview_global.h"
#include "audio-preview/audiopreviewplugin.h"
#include "audio-preview/audiofileinfo.h"
#include "audio-preview/audioview.h"

#include <stubext.h>

#include <gtest/gtest.h>

#include <QTest>

AUDIO_PREVIEW_USE_NAMESPACE

TEST(AudioPreviewPluginTest, ut_init)
{
    AudioPreviewPlugin plugin;
    plugin.init(nullptr);
    EXPECT_TRUE(plugin.m_audioView);
}

TEST(AudioPreviewPluginTest, ut_previewItem)
{
    stub_ext::StubExt st;
    st.set_lamda(&AudioFileInfo::openAudioFile, [](){
        AudioFileInfo::AudioMetaData data;
        data.artist = "Artist";
        data.album = "Album";
        data.duration = "00:00:10";
        return data;
    });
    st.set_lamda(&AudioView::setItemInfo, [](){ return; });
    st.set_lamda(&QFileInfo::lastModified, [](){
        return QDateTime::currentDateTime();
    });
    st.set_lamda(&QFileInfo::absoluteFilePath, [](){ return "/home"; });

    GrandSearch::ItemInfo info;
    info[PREVIEW_ITEMINFO_ITEM] = "";

    AudioPreviewPlugin plugin;
    plugin.init(nullptr);
    EXPECT_FALSE(plugin.previewItem(info));

    info[PREVIEW_ITEMINFO_ITEM] = "/test.mp3";
    EXPECT_TRUE(plugin.previewItem(info));
}

TEST(AudioPreviewPluginTest, ut_item)
{
    AudioPreviewPlugin plugin;
    EXPECT_TRUE(plugin.item().isEmpty());
}

TEST(AudioPreviewPluginTest, ut_stopPreview)
{
    AudioPreviewPlugin plugin;
    EXPECT_TRUE(plugin.stopPreview());
}

TEST(AudioPreviewPluginTest, ut_contentWidget)
{
    AudioPreviewPlugin plugin;
    EXPECT_FALSE(plugin.contentWidget());
}

TEST(AudioPreviewPluginTest, ut_getAttributeDetailInfo)
{
    AudioPreviewPlugin plugin;
    EXPECT_TRUE(plugin.getAttributeDetailInfo().isEmpty());
}

TEST(AudioPreviewPluginTest, ut_toolBarWidget)
{
    AudioPreviewPlugin plugin;
    EXPECT_FALSE(plugin.toolBarWidget());
}

TEST(AudioPreviewPluginTest, ut_showToolBar)
{
    AudioPreviewPlugin plugin;
    EXPECT_TRUE(plugin.showToolBar());
}
