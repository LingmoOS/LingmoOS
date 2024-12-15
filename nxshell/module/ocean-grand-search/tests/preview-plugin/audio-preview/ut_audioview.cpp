// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "audio-preview/audiopreview_global.h"
#include "audio-preview/audioview.h"
#include "global/commontools.h"

#include <stubext.h>

#include <gtest/gtest.h>

#include <QTest>

AUDIO_PREVIEW_USE_NAMESPACE

TEST(AudioViewTest, ut_setItemInfo_0)
{
    stub_ext::StubExt st;
    st.set_lamda(GrandSearch::CommonTools::lineFeed, [](){ return "aaaa.mp3"; });
    st.set_lamda(GrandSearch::CommonTools::formatFileSize, [](){ return "1 MB"; });

    GrandSearch::ItemInfo info;
    info[PREVIEW_ITEMINFO_ICON] = "audio-mpeg";
    info[PREVIEW_ITEMINFO_NAME] = "AAAA.mp3";
    info[PREVIEW_ITEMINFO_ITEM] = "/test.mp3";

    AudioView view;
    EXPECT_NO_FATAL_FAILURE(view.setItemInfo(info));
}

TEST(AudioViewTest, ut_setItemInfo_1)
{
    stub_ext::StubExt st;
    st.set_lamda(GrandSearch::CommonTools::lineFeed, [](){ return "aaaa.mp3"; });
    st.set_lamda(GrandSearch::CommonTools::formatFileSize, [](){ return "1 MB"; });

    GrandSearch::ItemInfo info;
    info[PREVIEW_ITEMINFO_ICON] = "";
    info[PREVIEW_ITEMINFO_NAME] = "AAAA.mp3";
    info[PREVIEW_ITEMINFO_ITEM] = "/test.mp3";

    AudioView view;
    EXPECT_NO_FATAL_FAILURE(view.setItemInfo(info));
}
