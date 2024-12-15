// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vnoteforlder.h"
#include "common/vnotedatamanager.h"

#include <DLog>

/**
 * @brief VNoteFolder::VNoteFolder
 */
VNoteFolder::VNoteFolder()
{
}

/**
 * @brief VNoteFolder::~VNoteFolder
 */
VNoteFolder::~VNoteFolder()
{
}

/**
 * @brief VNoteFolder::isValid
 * @return true 可用
 */
bool VNoteFolder::isValid()
{
    return (id > INVALID_ID) ? true : false;
}

/**
 * @brief VNoteFolder::maxNoteIdRef
 * @return 最大记事项id
 */
qint32 &VNoteFolder::maxNoteIdRef()
{
    return maxNoteId;
}

/**
 * @brief VNoteFolder::getNotesCount
 * @return 记事项数目
 */
qint32 VNoteFolder::getNotesCount()
{
    int nCount = 0;

    VNOTE_ITEMS_MAP *folderNotes = getNotes();

    if (Q_LIKELY(nullptr != folderNotes)) {
        folderNotes->lock.lockForRead();
        nCount = folderNotes->folderNotes.count();
        folderNotes->lock.unlock();
    }

    return nCount;
}

/**
 * @brief VNoteFolder::getNotes
 * @return 记事项数据
 */
VNOTE_ITEMS_MAP *VNoteFolder::getNotes()
{
    if (Q_UNLIKELY(nullptr == notes)) {
        VNOTE_ITEMS_MAP *folderNotes = VNoteDataManager::instance()->getFolderNotes(id);
        notes = folderNotes;
    }

    return notes;
}
