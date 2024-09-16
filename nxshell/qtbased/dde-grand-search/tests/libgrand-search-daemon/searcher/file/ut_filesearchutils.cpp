// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global/grandsearch_global.h"
#include "searcher/file/filesearchutils.h"
#include "utils/specialtools.h"
#include "global/builtinsearch.h"
#include "global/searchhelper.h"
#include "configuration/configer.h"
#include "configuration/configer_p.h"
#include "stubext.h"
#include "global/searchconfigdefine.h"

#include <gtest/gtest.h>

GRANDSEARCH_USE_NAMESPACE

TEST(FileSearchUtilsTest, ut_packItem)
{
    stub_ext::StubExt st;
    st.set_lamda(&QFileInfo::fileName, []() { return "test"; });
    st.set_lamda(GrandSearch::SpecialTools::getMimeType, [](const QFileInfo &) {
        QMimeType mimeType;
        return mimeType;
    });
    st.set_lamda(&QMimeType::name, []() { return "test"; });
    st.set_lamda(&QMimeType::iconName, []() { return "test.icon"; });
    st.set_lamda(&Configer::group, []() { return ConfigerPrivate::tailerData(); });

    EXPECT_NO_FATAL_FAILURE(FileSearchUtils::packItem("test", "searcher name"));
}

TEST(FileSearchUtilsTest, ut_groupKey)
{
    EXPECT_EQ(GRANDSEARCH_GROUP_FILE, FileSearchUtils::groupKey(FileSearchUtils::File));
    EXPECT_EQ(GRANDSEARCH_GROUP_FOLDER, FileSearchUtils::groupKey(FileSearchUtils::Folder));
    EXPECT_EQ(GRANDSEARCH_GROUP_FILE_PICTURE, FileSearchUtils::groupKey(FileSearchUtils::Picture));
    EXPECT_EQ(GRANDSEARCH_GROUP_FILE_AUDIO, FileSearchUtils::groupKey(FileSearchUtils::Audio));
    EXPECT_EQ(GRANDSEARCH_GROUP_FILE_VIDEO, FileSearchUtils::groupKey(FileSearchUtils::Video));
    EXPECT_EQ(GRANDSEARCH_GROUP_FILE_DOCUMNET, FileSearchUtils::groupKey(FileSearchUtils::Document));
}

TEST(FileSearchUtilsTest, ut_getGroupByName)
{
    {
        stub_ext::StubExt st;
        st.set_lamda(&QFileInfo::isDir, []() { return true; });
        EXPECT_EQ(FileSearchUtils::Folder, FileSearchUtils::getGroupByName("test"));
    }

    {
        stub_ext::StubExt st;
        st.set_lamda(&QFileInfo::isDir, []() { return false; });
        st.set_lamda(&QFileInfo::suffix, []() { return "png"; });
        EXPECT_EQ(FileSearchUtils::Picture, FileSearchUtils::getGroupByName("test.png"));
    }
}

TEST(FileSearchUtilsTest, ut_getGroupBySuffix)
{
    EXPECT_EQ(FileSearchUtils::Unknown, FileSearchUtils::getGroupBySuffix(""));
    EXPECT_EQ(FileSearchUtils::Document, FileSearchUtils::getGroupBySuffix("txt"));
    EXPECT_EQ(FileSearchUtils::Picture, FileSearchUtils::getGroupBySuffix("png"));
    EXPECT_EQ(FileSearchUtils::Video, FileSearchUtils::getGroupBySuffix("mp4"));
    EXPECT_EQ(FileSearchUtils::Audio, FileSearchUtils::getGroupBySuffix("mp3"));
    EXPECT_EQ(FileSearchUtils::File, FileSearchUtils::getGroupBySuffix("zip"));
}

TEST(FileSearchUtilsTest, ut_getGroupByGroupName)
{
    EXPECT_EQ(FileSearchUtils::Unknown, FileSearchUtils::getGroupByGroupName("test"));
    EXPECT_EQ(FileSearchUtils::Folder, FileSearchUtils::getGroupByGroupName(FOLDER_GROUP));
    EXPECT_EQ(FileSearchUtils::Document, FileSearchUtils::getGroupByGroupName(DOCUMENT_GROUP));
    EXPECT_EQ(FileSearchUtils::Picture, FileSearchUtils::getGroupByGroupName(PICTURE_GROUP));
    EXPECT_EQ(FileSearchUtils::Video, FileSearchUtils::getGroupByGroupName(VIDEO_GROUP));
    EXPECT_EQ(FileSearchUtils::Audio, FileSearchUtils::getGroupByGroupName(AUDIO_GROUP));
    EXPECT_EQ(FileSearchUtils::File, FileSearchUtils::getGroupByGroupName(FILE_GROUP));
}

TEST(FileSearchUtilsTest, ut_parseContent1)
{
    auto info = FileSearchUtils::parseContent("test");
    EXPECT_EQ("test", info.keyword);
}

TEST(FileSearchUtilsTest, ut_parseContent2)
{
    QString content = R"({
                      "Group":[
                          "text",
                          "app",
                          ""
                      ],
                      "Keyword":[
                          "abc",
                          "xyz",
                          ""
                      ],
                      "Suffix":[
                          "jpg",
                          ""
                      ]
                  })";
    auto info = FileSearchUtils::parseContent(content);
    EXPECT_TRUE(info.isCombinationSearch);
}

TEST(FileSearchUtilsTest, ut_fileShouldVisible)
{
    {
        FileSearchUtils::Group group = FileSearchUtils::File;
        FileSearchUtils::SearchInfo info;
        EXPECT_TRUE(FileSearchUtils::fileShouldVisible("test", group, info));
    }

    {
        stub_ext::StubExt st;
        st.set_lamda(&QFileInfo::isDir, []() { return true; });

        FileSearchUtils::Group group = FileSearchUtils::File;
        FileSearchUtils::SearchInfo info;
        info.isCombinationSearch = true;
        EXPECT_FALSE(FileSearchUtils::fileShouldVisible("test", group, info));
    }

    {
        stub_ext::StubExt st;
        st.set_lamda(&QFileInfo::isDir, []() { return false; });
        st.set_lamda(&QFileInfo::suffix, []() { return "png"; });

        FileSearchUtils::Group group = FileSearchUtils::Picture;
        FileSearchUtils::SearchInfo info;
        info.isCombinationSearch = true;
        EXPECT_FALSE(FileSearchUtils::fileShouldVisible("test.png", group, info));
    }

    {
        stub_ext::StubExt st;
        st.set_lamda(&QFileInfo::isDir, []() { return false; });
        st.set_lamda(&QFileInfo::suffix, []() { return "png"; });

        FileSearchUtils::Group group = FileSearchUtils::Picture;
        FileSearchUtils::SearchInfo info;
        info.isCombinationSearch = true;
        info.groupList << FileSearchUtils::File;
        EXPECT_TRUE(FileSearchUtils::fileShouldVisible("test.png", group, info));
        EXPECT_EQ(group, FileSearchUtils::File);
    }
}

TEST(FileSearchUtilsTest, ut_tailerData)
{
    stub_ext::StubExt st;

    QVariantHash tailConfig({ { GRANDSEARCH_TAILER_PARENTDIR, true },
                              { GRANDSEARCH_TAILER_TIMEMODEFIED, true } });
    UserPreferencePointer upp(new UserPreference(tailConfig));
    st.set_lamda(&Configer::group, [&upp](Configer *, const QString &name) {
        if (name == GRANDSEARCH_TAILER_GROUP)
            return upp;

        return UserPreferencePointer();
    });

    QFileInfo info(QDir::homePath());
    auto res = FileSearchUtils::tailerData(info);

    EXPECT_TRUE(res.contains(GRANDSEARCH_PROPERTY_ITEM_TAILER));
    auto datas = res[GRANDSEARCH_PROPERTY_ITEM_TAILER].toStringList();
    EXPECT_FALSE(datas.isEmpty());
}

TEST(FileSearchUtilsTest, ut_filterByBlacklist)
{
    stub_ext::StubExt st;

    QVariantHash tailConfig({ { GRANDSEARCH_BLACKLIST_PATH, { "/test" } } });
    UserPreferencePointer upp(new UserPreference(tailConfig));
    st.set_lamda(&Configer::group, [&upp](Configer *, const QString &name) {
        if (name == GRANDSEARCH_BLACKLIST_GROUP)
            return upp;

        return UserPreferencePointer();
    });

    EXPECT_TRUE(FileSearchUtils::filterByBlacklist("/test/xxx"));
}
