// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_vnoteitem.h"
#include "vnoteitem.h"
#include "vnoteforlder.h"

UT_VnoteItem::UT_VnoteItem()
{
}

TEST_F(UT_VnoteItem, UT_VnoteItem_isValid_001)
{
    VNoteItem vnoteitem;
    vnoteitem.noteId = 2;
    vnoteitem.folderId = 2;
    EXPECT_EQ(true, vnoteitem.isValid());
}

TEST_F(UT_VnoteItem, UT_VnoteItem_delNoteData_001)
{
    VNoteItem vnoteitem;
    vnoteitem.delNoteData();
}

TEST_F(UT_VnoteItem, UT_VnoteItem_search_001)
{
    VNoteItem vnoteitem;
    vnoteitem.noteTitle = "test";
    QString tmpstr = "test";
    EXPECT_TRUE(vnoteitem.search(tmpstr)) << "search title";
    EXPECT_FALSE(vnoteitem.search("1234")) << "search data";
    vnoteitem.htmlCode = "1234561";
    EXPECT_TRUE(vnoteitem.search("1234")) << "search htmlcode";
}

TEST_F(UT_VnoteItem, UT_VnoteItem_folder_001)
{
    VNoteItem vnoteitem;
    VNoteFolder *folder = new VNoteFolder();
    vnoteitem.setFolder(folder);
    EXPECT_NE(nullptr, vnoteitem.ownFolder) << "setFolder";
    EXPECT_EQ(folder, vnoteitem.folder()) << "getFolder";
    delete folder;
}

TEST_F(UT_VnoteItem, UT_VnoteItem_qdebugtest_001)
{
    VNoteItem vnoteitem;
    vnoteitem.noteId = 0;
    vnoteitem.folderId = 1;
    vnoteitem.noteTitle = "test";
    vnoteitem.createTime = QDateTime::currentDateTime();
    vnoteitem.modifyTime = QDateTime::currentDateTime();
    vnoteitem.deleteTime = QDateTime::currentDateTime();
    qDebug() << "" << vnoteitem;
}

TEST_F(UT_VnoteItem, UT_VnoteItem_metaDataRef_001)
{
    VNoteItem vnoteitem;
    QVariant data;
    vnoteitem.setMetadata(data);
    EXPECT_EQ(data, vnoteitem.metaData);
    EXPECT_EQ(data, vnoteitem.metaDataRef());
}

TEST_F(UT_VnoteItem, UT_VnoteItem_metaDataConstRef_001)
{
    VNoteItem vnoteitem;
    QVariant data;
    vnoteitem.setMetadata(data);
    EXPECT_EQ(data, vnoteitem.metaData);
    EXPECT_EQ(data, vnoteitem.metaDataConstRef());
}

TEST_F(UT_VnoteItem, UT_VnoteItem_newBlock_001)
{
    VNoteItem vnoteitem;
    VNoteBlock *ptrBlock = nullptr;
    ptrBlock = vnoteitem.newBlock(1);
    EXPECT_EQ(ptrBlock->blockType, VNoteBlock::Text);
    vnoteitem.delBlock(ptrBlock);
    ptrBlock = vnoteitem.newBlock(2);
    EXPECT_EQ(ptrBlock->blockType, VNoteBlock::Voice);
    vnoteitem.delBlock(ptrBlock);
}

TEST_F(UT_VnoteItem, UT_VnoteItem_addBlock_001)
{
    VNoteItem vnoteitem;
    VNoteBlock *ptrBlock = vnoteitem.newBlock(2);
    VNoteBlock *ptrBlock1 = vnoteitem.newBlock(2);
    vnoteitem.addBlock(ptrBlock);
    vnoteitem.addBlock(ptrBlock, ptrBlock1);
    vnoteitem.delBlock(ptrBlock);
    EXPECT_TRUE(vnoteitem.haveVoice());
    EXPECT_FALSE(vnoteitem.haveText());
    EXPECT_GE(vnoteitem.voiceCount(), 0);
}

TEST_F(UT_VnoteItem, setMetadata)
{
    VNoteItem vnoteitem;
    QString tmpstr = "{\"dataCount\":3,\"noteDatas\":[{\"text\":\"wafwaefawffvbdafvadfasdf\",\"type\":1},{\"createTime\":\"2020-08-25 14:39:21.473\",\"state\":false,\"text\":\"\",\"title\":\"Voice2\",\"type\":2,\"voicePath\":\"/usr/share/music/bensound-sunny.mp3\",\"voiceSize\":3630},{\"text\":\"wafwaefawffvbdafvadfasdf\",\"type\":1}],\"voiceMaxId\":3}";
    vnoteitem.setMetadata(tmpstr);
    EXPECT_EQ(vnoteitem.metaData, tmpstr);
}

TEST_F(UT_VnoteItem, UT_VnoteItem_haveVoice_001)
{
    VNoteItem vnoteitem;
    EXPECT_FALSE(vnoteitem.haveVoice()) << "html is empty";

    vnoteitem.htmlCode = "<div> </div>";
    EXPECT_FALSE(vnoteitem.haveVoice()) << "jsonkey is empty";

    vnoteitem.htmlCode = "<div jsonkey=\"123\"> </div>";
    EXPECT_TRUE(vnoteitem.haveVoice()) << "has jsonkey";
}

TEST_F(UT_VnoteItem, UT_VnoteItem_haveText_001)
{
    VNoteItem vnoteitem;
    EXPECT_FALSE(vnoteitem.haveText()) << "html is empty";

    vnoteitem.htmlCode = "<div> </div>";
    EXPECT_TRUE(vnoteitem.haveText()) << "jsonkey is empty";

    vnoteitem.htmlCode = "<p><br></p>";
    EXPECT_FALSE(vnoteitem.haveText()) << "has jsonkey";
}

TEST_F(UT_VnoteItem, UT_VnoteItem_voiceCount_001)
{
    VNoteItem vnoteitem;
    EXPECT_FALSE(vnoteitem.voiceCount()) << "html is empty";

    vnoteitem.htmlCode = "<div> </div>";
    EXPECT_FALSE(vnoteitem.voiceCount()) << "jsonkey is empty";

    vnoteitem.htmlCode = "<div jsonkey=\"{123}\"> </div>";
    EXPECT_TRUE(vnoteitem.voiceCount()) << "has jsonkey";
}

TEST_F(UT_VnoteItem, UT_VnoteItem_getVoiceJsons_001)
{
    VNoteItem vnoteitem;
    vnoteitem.htmlCode = "<div =\"123\"> </div>";
    EXPECT_EQ(0, vnoteitem.getVoiceJsons().size()) << "jsonkey is empty";

    vnoteitem.htmlCode = "<div jsonkey={\"123\"}> </div>";
    EXPECT_EQ(1, vnoteitem.getVoiceJsons().size()) << "has jsonkey";
}

TEST_F(UT_VnoteItem, UT_VnoteItem_getFullHtml_001)
{
    VNoteItem vnoteitem;
    vnoteitem.htmlCode = "<div =\"123\"> <img src=\"\"> </div>";
    EXPECT_TRUE(vnoteitem.getFullHtml().size()) << "path is empty";

    vnoteitem.htmlCode = "<div =\"123\"> <img src=\"/tmp/1.jpg\"> </div>";
    EXPECT_TRUE(vnoteitem.getFullHtml().size()) << "has path";
}

TEST_F(UT_VnoteItem, UT_VnoteItem_operator_001)
{
    VNoteItem vnoteitem;
    qDebug() << "" << vnoteitem;
}
