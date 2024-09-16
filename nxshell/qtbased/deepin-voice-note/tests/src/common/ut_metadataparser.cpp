// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_metadataparser.h"
#include "metadataparser.h"
#include "vnoteitem.h"

UT_MetaDataParser::UT_MetaDataParser()
{
}

TEST_F(UT_MetaDataParser, UT_MetaDataParser_parse_001)
{
    MetaDataParser metadataparser;
    QVariant metadata = "{\"dataCount\":1,\"noteDatas\":[{\"text\":\"sdfgsgssrgstg\",\"type\":1}],\"voiceMaxId\":0}";
    VNoteItem *noteData = new VNoteItem();
    EXPECT_TRUE(noteData != nullptr);
    metadataparser.parse(metadata, noteData);
    metadata = "{\"dataCount\":5,\"noteDatas\":[{\"text\":\"\",\"type\":1},{\"createTime\":\"2020-08-18 19:21:15.710\",\"state\":false,\"text\":\"\",\"title\":\"语音1\",\"type\":2,\"voicePath\":\"/usr/share/music/bensound-sunny.mp3\",\"voiceSize\":2650},{\"text\":\"142424\",\"type\":1},{\"createTime\":\"2020-08-18 19:23:24.006\",\"state\":false,\"text\":\"\",\"title\":\"语音2\",\"type\":2,\"voicePath\":\"/usr/share/music/bensound-sunny.mp3\",\"voiceSize\":3860},{\"text\":\"\",\"type\":1}],\"voiceMaxId\":2}";
    metadataparser.parse(metadata, noteData);
    metadataparser.makeMetaData(noteData, metadata);
    delete noteData;
}

TEST_F(UT_MetaDataParser, UT_MetaDataParser_parse_002)
{
    MetaDataParser metadataparser;
    VNVoiceBlock *voiceData = new VNVoiceBlock();
    EXPECT_TRUE(voiceData != nullptr) << "bool";
    QVariant metadata = "{\"createTime\":\"2021-09-16 17:19:22.065\",\"state\":false,\"text\":\"\",\"title\":\"20210916 17.19.22\",\"transSize\":\"00:01\",\"type\":2,"
                        "\"voicePath\":\"/home/uos/.local/share/deepin/deepin-voice-note/voicenote/20210916171920.mp3\",\"voiceSize\":1420}";
    EXPECT_TRUE(metadataparser.parse(metadata, voiceData)) << "voice";
    EXPECT_TRUE(voiceData->blockText.isEmpty()) << "text";
    EXPECT_EQ("20210916 17.19.22", voiceData->voiceTitle) << "title";
    EXPECT_FALSE(voiceData->state) << "state";
    EXPECT_EQ("/home/uos/.local/share/deepin/deepin-voice-note/voicenote/20210916171920.mp3", voiceData->voicePath) << "path";
    EXPECT_EQ(1420, voiceData->voiceSize) << "size";
    EXPECT_EQ(QDateTime::fromString("2021-09-16 17:19:22.065", VNOTE_TIME_FMT), voiceData->createTime) << "title";
    delete voiceData;
}

TEST_F(UT_MetaDataParser, UT_MetaDataParser_parse_003)
{
    MetaDataParser metadataparser;
    VNVoiceBlock *voiceData = new VNVoiceBlock();
    EXPECT_TRUE(voiceData != nullptr) << "bool";

    QVariant metadata = "";
    EXPECT_FALSE(metadataparser.parse(metadata, voiceData)) << "empty";

    metadata = "{\"dataCount\":1,\"noteDatas\":[{\"text\":\"sdfgsgssrgstg\",\"type\":1}],\"voiceMaxId\":0}";
    EXPECT_FALSE(metadataparser.parse(metadata, voiceData));

    metadata = "{\"createTime\":\"2021-09-16 17:19:22.065\",\"state\":false,\"text\":\"\",\"title\":\"20210916 17.19.22\",\"transSize\":\"00:01\",\"type\":0,"
               "\"voicePath\":\"/home/uos/.local/share/deepin/deepin-voice-note/voicenote/20210916171920.mp3\",\"voiceSize\":1420}";
    EXPECT_FALSE(metadataparser.parse(metadata, voiceData)) << "type is 0";

    metadata = "{\"createTime\":\"2021-09-16 17:19:22.065\",\"state\":false,\"text\":\"\",\"title\":\"20210916 17.19.22\",\"transSize\":\"00:01\",\"type\":5,"
               "\"voicePath\":\"/home/uos/.local/share/deepin/deepin-voice-note/voicenote/20210916171920.mp3\",\"voiceSize\":1420}";
    EXPECT_FALSE(metadataparser.parse(metadata, voiceData)) << "type is 5";

    VNTextBlock *textData = new VNTextBlock();
    metadata = "{\"createTime\":\"2021-09-16 17:19:22.065\",\"state\":false,\"text\":\"aaa\",\"title\":\"20210916 17.19.22\",\"transSize\":\"00:01\",\"type\":1,"
               "\"voicePath\":\"/home/uos/.local/share/deepin/deepin-voice-note/voicenote/20210916171920.mp3\",\"voiceSize\":1420}";
    EXPECT_TRUE(metadataparser.parse(metadata, textData)) << "type is 5";
    EXPECT_EQ("aaa", textData->blockText);

    delete voiceData;
    delete textData;
}

TEST_F(UT_MetaDataParser, UT_MetaDataParser_makeMetaData_001)
{
    MetaDataParser metadataparser;
    VNoteItem *noteData = new VNoteItem();
    EXPECT_TRUE(noteData != nullptr);
    QVariant metadata = "{\"dataCount\":1,\"noteDatas\":[{\"text\":\"sdfgsgssrgstg\",\"type\":1}],\"voiceMaxId\":0}";
    metadataparser.parse(metadata, noteData);
    metadata = "";
    metadataparser.makeMetaData(noteData, metadata);
    EXPECT_EQ("{\"dataCount\":1,\"noteDatas\":[{\"text\":\"sdfgsgssrgstg\",\"type\":1}],\"voiceMaxId\":0}", metadata) << "htmlCode is empty";

    noteData->htmlCode = "abc";
    metadataparser.makeMetaData(noteData, metadata);
    delete noteData;
}
