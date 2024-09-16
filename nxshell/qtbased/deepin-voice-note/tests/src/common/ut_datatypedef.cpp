// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_datatypedef.h"
#include "datatypedef.h"
#include "vnoteforlder.h"
#include "vnoteitem.h"
#include "globaldef.h"
#include <DLog>

UT_DataTypeDef::UT_DataTypeDef()
{
}

TEST_F(UT_DataTypeDef, UT_DataTypeDef_dataConstRef_001)
{
    VNOTE_DATAS vnote_datas;
    VNOTE_DATA_VECTOR vnote_data_vector;
    EXPECT_TRUE(vnote_data_vector.size() == 0);

    VNoteBlock *ptrBlock = nullptr;
    vnote_datas.datas.push_back(ptrBlock);
    vnote_data_vector = vnote_datas.dataConstRef();
    EXPECT_FALSE(vnote_data_vector.size() == 0);
}

TEST_F(UT_DataTypeDef, UT_DataTypeDef_newBlock_001)
{
    VNOTE_DATAS vnote_datas;
    VNoteBlock *ptrBlock = nullptr;
    ptrBlock = vnote_datas.newBlock(1);
    EXPECT_EQ(ptrBlock->blockType, VNoteBlock::Text);
    vnote_datas.delBlock(ptrBlock);
    ptrBlock = vnote_datas.newBlock(2);
    EXPECT_EQ(ptrBlock->blockType, VNoteBlock::Voice);
    vnote_datas.delBlock(ptrBlock);
}

TEST_F(UT_DataTypeDef, UT_DataTypeDef_addBlock_001)
{
    VNOTE_DATAS vnote_datas;
    VNoteBlock *ptrBlock = vnote_datas.newBlock(1);
    vnote_datas.addBlock(ptrBlock);
    EXPECT_NE(vnote_datas.datas.size(), 0);
}

TEST_F(UT_DataTypeDef, UT_DataTypeDef_addBlock_002)
{
    VNOTE_DATAS vnote_datas;
    VNoteBlock *ptrBlock = vnote_datas.newBlock(1);
    VNoteBlock *ptrBlock1 = vnote_datas.newBlock(2);
    vnote_datas.addBlock(ptrBlock);
    vnote_datas.addBlock(ptrBlock, ptrBlock1);
    EXPECT_NE(vnote_datas.datas.size(), 0);

    VNoteBlock *ptrBlock2 = vnote_datas.newBlock(1);
    vnote_datas.addBlock(ptrBlock2, ptrBlock2);
    EXPECT_NE(vnote_datas.datas.size(), 0);

    ptrBlock2 = nullptr;
    VNoteBlock *ptrBlock3 = vnote_datas.newBlock(2);
    vnote_datas.addBlock(ptrBlock2, ptrBlock3);
    EXPECT_NE(vnote_datas.datas.size(), 0);
}

TEST_F(UT_DataTypeDef, UT_DataTypeDef_delBlock_001)
{
    VNOTE_DATAS vnote_datas;
    VNoteBlock *ptrBlock = vnote_datas.newBlock(1);
    VNoteBlock *ptrBlock1 = vnote_datas.newBlock(2);
    vnote_datas.addBlock(ptrBlock);
    vnote_datas.addBlock(ptrBlock, ptrBlock1);
    EXPECT_NE(vnote_datas.datas.size(), 0);

    vnote_datas.delBlock(ptrBlock);
    EXPECT_NE(vnote_datas.datas.size(), 0);
}

TEST_F(UT_DataTypeDef, UT_DataTypeDef_classifyAddBlk_001)
{
    VNOTE_DATAS vnote_datas;
    VNoteBlock *ptrBlock = vnote_datas.newBlock(2);
    ptrBlock->blockType = VNoteBlock::InValid;
    vnote_datas.classifyAddBlk(ptrBlock);
    EXPECT_EQ(vnote_datas.datas.size(), 0);
    vnote_datas.classifyDelBlk(ptrBlock);
    vnote_datas.delBlock(ptrBlock);
}

TEST_F(UT_DataTypeDef, UT_DataTypeDef_classifyDelBlk_001)
{
    VNOTE_DATAS vnote_datas;
    VNoteBlock *ptrBlock = vnote_datas.newBlock(2);
    vnote_datas.classifyAddBlk(ptrBlock);
    ASSERT_NE(0, vnote_datas.voiceBlocks.size()) << "add voice, size is 0";
    EXPECT_TRUE(vnote_datas.voiceBlocks.contains(ptrBlock));
    vnote_datas.classifyDelBlk(ptrBlock);
    EXPECT_FALSE(vnote_datas.voiceBlocks.contains(ptrBlock)) << "remove voice";

    ptrBlock->blockType = VNoteBlock::InValid;
    vnote_datas.classifyAddBlk(ptrBlock);
    int voiceSize = vnote_datas.voiceBlocks.size();
    int textSize = vnote_datas.textBlocks.size();
    vnote_datas.classifyDelBlk(ptrBlock);
    EXPECT_EQ(voiceSize, vnote_datas.voiceBlocks.size());
    EXPECT_EQ(textSize, vnote_datas.textBlocks.size());
    vnote_datas.delBlock(ptrBlock);
}

TEST_F(UT_DataTypeDef, UT_DataTypeDef_isValid_001)
{
    VDataSafer vdatasafer;
    ASSERT_FALSE(vdatasafer.isValid()) << "init";
    vdatasafer.path = "/home";
    vdatasafer.folder_id = 2;
    vdatasafer.note_id = 3;
    EXPECT_TRUE(vdatasafer.isValid()) << "true";
}

TEST_F(UT_DataTypeDef, UT_DataTypeDef_setSaferType_001)
{
    VDataSafer vdatasafer;
    vdatasafer.setSaferType(vdatasafer.Unsafe);
    EXPECT_EQ(vdatasafer.saferType, vdatasafer.Unsafe);
}

TEST_F(UT_DataTypeDef, UT_DataTypeDef_qdebugout_001)
{
    VDataSafer vdatasafer;
    vdatasafer.path = "test";
    vdatasafer.meta_data = "test";
    vdatasafer.createTime = QDateTime::currentDateTime();
    qDebug() << "" << vdatasafer;
}

TEST_F(UT_DataTypeDef, UT_DataTypeDef_TEST_VNOTE_FOLDERS_MAP_001)
{
    VNOTE_FOLDERS_MAP vnote_floders_map;
    VNoteFolder *vnotefloder = nullptr;
    vnote_floders_map.folders.insert(1, vnotefloder);
    vnote_floders_map.autoRelease = true;
}

TEST_F(UT_DataTypeDef, TUT_DataTypeDef_EST_VNOTE_ITEMS_MAP_001)
{
    VNOTE_ITEMS_MAP vnote_items_map;
    VNoteItem *vnoteitem = nullptr;
    vnote_items_map.folderNotes.insert(1, vnoteitem);
    vnote_items_map.autoRelease = true;
}

TEST_F(UT_DataTypeDef, UT_DataTypeDef_TEST_VNOTE_ALL_NOTES_MAP_001)
{
    VNOTE_ALL_NOTES_MAP vnote_all_notes_map;
    VNOTE_ITEMS_MAP *vnote_items_map = nullptr;
    vnote_all_notes_map.notes.insert(1, vnote_items_map);
    vnote_all_notes_map.autoRelease = true;
}
