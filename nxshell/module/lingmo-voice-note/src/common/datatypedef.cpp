// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "datatypedef.h"
#include "common/vnoteforlder.h"
#include "common/vnoteitem.h"
#include "globaldef.h"

#include <DLog>

/**
 * @brief VNOTE_FOLDERS_MAP::~VNOTE_FOLDERS_MAP
 */
VNOTE_FOLDERS_MAP::~VNOTE_FOLDERS_MAP()
{
    if (autoRelease) {
        qInfo() << __FUNCTION__ << "Auto release folders";

        for (auto it : folders) {
            delete reinterpret_cast<VNoteFolder *>(it);
        }

        folders.clear();
    }
}

/**
 * @brief VNOTE_ITEMS_MAP::~VNOTE_ITEMS_MAP
 */
VNOTE_ITEMS_MAP::~VNOTE_ITEMS_MAP()
{
    if (autoRelease) {
        qInfo() << __FUNCTION__ << "Auto release folder notes";

        for (auto it : folderNotes) {
            delete reinterpret_cast<VNoteItem *>(it);
        }

        folderNotes.clear();
    }
}

/**
 * @brief VNOTE_ALL_NOTES_MAP::~VNOTE_ALL_NOTES_MAP
 */
VNOTE_ALL_NOTES_MAP::~VNOTE_ALL_NOTES_MAP()
{
    if (autoRelease) {
        qInfo() << __FUNCTION__ << "Auto release all notes in folders ";

        for (auto it : notes) {
            delete reinterpret_cast<VNOTE_ITEMS_MAP *>(it);
        }

        notes.clear();
    }
}

/**
 * @brief VNOTE_DATAS::~VNOTE_DATAS
 */
VNOTE_DATAS::~VNOTE_DATAS()
{
    for (auto it : datas) {
        delete it;
    }
}

/**
 * @brief VNOTE_DATAS::dataConstRef
 * @return 记事项数据
 */
const VNOTE_DATA_VECTOR &VNOTE_DATAS::dataConstRef()
{
    return datas;
}

/**
 * @brief VNOTE_DATAS::newBlock
 * @param type
 * @return 新建数据块
 */
VNoteBlock *VNOTE_DATAS::newBlock(int type)
{
    VNoteBlock *ptrBlock = nullptr;

    if (type == VNoteBlock::Text) {
        ptrBlock = new VNTextBlock();
    } else if (type == VNoteBlock::Voice) {
        ptrBlock = new VNVoiceBlock();
    }

    return ptrBlock;
}

/**
 * @brief VNOTE_DATAS::addBlock
 * @param block
 */
void VNOTE_DATAS::addBlock(VNoteBlock *block)
{
    datas.push_back(block);

    //Add block to classification data set
    classifyAddBlk(block);
}

/**
 * @brief VNOTE_DATAS::addBlock
 * @param before
 * @param block
 */
void VNOTE_DATAS::addBlock(VNoteBlock *before, VNoteBlock *block)
{
    //If before is null, insert block at the head
    if (nullptr != before) {
        int index = datas.indexOf(before);

        //If the before block is invalid,maybe some errors
        //happened,just add at the end
        if (index != -1) {
            datas.insert(index + 1, block);
        } else {
            datas.append(block);
            qInfo() << "Block not in datas:" << before
                    << " append at end:" << block;
        }
    } else {
        datas.insert(0, block);
    }

    //Add block to classification data set
    classifyAddBlk(block);
}

/**
 * @brief VNOTE_DATAS::delBlock
 * @param block
 */
void VNOTE_DATAS::delBlock(VNoteBlock *block)
{
    int index = datas.indexOf(block);

    if (index != -1) {
        datas.remove(index);
    }

    //Remove the block from classification data set
    classifyDelBlk(block);

    delete block;
}

/**
 * @brief VNOTE_DATAS::classifyAddBlk
 * @param block
 */
void VNOTE_DATAS::classifyAddBlk(VNoteBlock *block)
{
    if (VNoteBlock::Text == block->getType()) {
        textBlocks.push_back(block);
    } else if (VNoteBlock::Voice == block->getType()) {
        voiceBlocks.push_back(block);
    } else {
        qCritical() << __FUNCTION__ << "Unknown VNoteBlock type " << block->getType();
    }
}

/**
 * @brief VNOTE_DATAS::classifyDelBlk
 * @param block
 */
void VNOTE_DATAS::classifyDelBlk(VNoteBlock *block)
{
    if (VNoteBlock::Text == block->getType()) {
        int index = textBlocks.indexOf(block);

        if (textBlocks.indexOf(block) != -1) {
            textBlocks.remove(index);
        }
    } else if (VNoteBlock::Voice == block->getType()) {
        int index = voiceBlocks.indexOf(block);

        if (voiceBlocks.indexOf(block) != -1) {
            voiceBlocks.remove(index);
        }
    } else {
        qCritical() << __FUNCTION__ << "Unknown VNoteBlock type " << block->getType();
    }

    //Don't need delete the block anymore,
    //becuase it's already released.
}

/**
 * @brief VDataSafer::isValid
 * @return true可用
 */
bool VDataSafer::isValid() const
{
    //TODO:
    //    Now path field is uique.Other may
    // be empty.So only check it for validation.
    return !(path.isEmpty() || folder_id == INVALID_ID || note_id == INVALID_ID);
}

/**
 * @brief VDataSafer::setSaferType
 * @param type
 */
void VDataSafer::setSaferType(VDataSafer::SaferType type)
{
    saferType = type;
}

QDebug &operator<<(QDebug &out, const VDataSafer &safer)
{
    const QStringList saferTypes = {
        "Safe",
        "UnSafe",
        "ExceptionSafer",
    };

    out << "VDataSafer { "
        << "saferType=" << saferTypes[safer.saferType] << ","
        << "Id=" << safer.id << ","
        << "folder_id=" << safer.folder_id << ","
        << "note_id=" << safer.note_id << ","
        << "path=" << safer.path << ","
        << "state=" << safer.state << ","
        << "meta_data=" << safer.meta_data << ","
        << "createTime=" << safer.createTime.toString(VNOTE_TIME_FMT)
        << " }\n";

    return out;
}
