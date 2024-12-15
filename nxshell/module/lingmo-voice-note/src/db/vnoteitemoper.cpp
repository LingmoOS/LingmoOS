// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vnoteitemoper.h"
#include "vnotefolderoper.h"
#include "vnotedbmanager.h"
#include "globaldef.h"
#include "common/metadataparser.h"
#include "common/vnoteitem.h"
#include "common/vnoteforlder.h"
#include "common/vnotedatamanager.h"
#include "db/dbvisitor.h"

#include <DLog>
#include <DApplication>

/**
 * @brief VNoteItemOper::VNoteItemOper
 * @param note 操作对象
 */
VNoteItemOper::VNoteItemOper(VNoteItem *note)
    : m_note(note)
{
}

/**
 * @brief VNoteItemOper::loadAllVNotes
 * @return 加载的所有数据
 */
VNOTE_ALL_NOTES_MAP *VNoteItemOper::loadAllVNotes()
{
    VNOTE_ALL_NOTES_MAP *notesMap = new VNOTE_ALL_NOTES_MAP();

    //DataManager data should set autoRelease flag
    notesMap->autoRelease = true;

    static struct timeval start, backups, end;

    gettimeofday(&start, nullptr);
    backups = start;

    NoteQryDbVisitor noteVisitor(VNoteDbManager::instance()->getVNoteDb(), nullptr, notesMap);

    if (VNoteDbManager::instance()->queryData(&noteVisitor)) {
        gettimeofday(&end, nullptr);

        qDebug() << "queryData(ms):" << TM(start, end);
    }

    return notesMap;
}

/**
 * @brief VNoteItemOper::modifyNoteTitle
 * @param title
 * @return true 成功
 */
bool VNoteItemOper::modifyNoteTitle(const QString &title)
{
    bool isUpdateOK = true;

    if (nullptr != m_note) {
        //back update old data
        QString oldTitle = m_note->noteTitle;
        QDateTime oldModifyTime = m_note->modifyTime;

        m_note->noteTitle = title;
        m_note->modifyTime = QDateTime::currentDateTime();

        RenameNoteDbVisitor renameNoteVisitor(
            VNoteDbManager::instance()->getVNoteDb(), m_note, nullptr);

        if (Q_UNLIKELY(!VNoteDbManager::instance()->updateData(&renameNoteVisitor))) {
            m_note->noteTitle = oldTitle;
            m_note->modifyTime = oldModifyTime;

            isUpdateOK = false;
        }
    }

    return isUpdateOK;
}

/**
 * @brief VNoteItemOper::updateNote
 * @return true 成功
 */
bool VNoteItemOper::updateNote()
{
    bool isUpdateOK = true;

    if (nullptr != m_note) {
        //backup
        QVariant oldMetaData = m_note->metaDataConstRef();
        QDateTime oldModifyTime = m_note->modifyTime;

        //Prepare meta data
        MetaDataParser metaParser;

        metaParser.makeMetaData(m_note, m_note->metaDataRef());

        m_note->modifyTime = QDateTime::currentDateTime();

        //Reset the max voice id when no voice file.
        if (!m_note->haveVoice()) {
            m_note->maxVoiceIdRef() = 0;
        }

        UpdateNoteDbVisitor updateNoteVisitor(
            VNoteDbManager::instance()->getVNoteDb(), m_note, nullptr);

        if (Q_UNLIKELY(!VNoteDbManager::instance()->updateData(&updateNoteVisitor))) {
            m_note->setMetadata(oldMetaData);
            m_note->modifyTime = oldModifyTime;

            isUpdateOK = false;
        }
    }

    return isUpdateOK;
}

/**
 * @brief VNoteItemOper::addNote
 * @param note
 * @return 记事项数据
 */
VNoteItem *VNoteItemOper::addNote(VNoteItem &note)
{
    VNoteFolderOper folderOps;
    VNoteFolder *folder = folderOps.getFolder(note.folderId);

    Q_ASSERT(nullptr != folder);

    folder->maxNoteIdRef()++;

    note.setFolder(folder);

    //Prepare meta data
    MetaDataParser metaParser;
    QVariant metaData;
    metaParser.makeMetaData(&note, note.metaDataRef());

    VNoteItem *newNote = new VNoteItem();
    AddNoteDbVisitor addNoteVisitor(VNoteDbManager::instance()->getVNoteDb(), &note, newNote);

    if (VNoteDbManager::instance()->insertData(&addNoteVisitor)) {
        if (Q_UNLIKELY(nullptr == VNoteDataManager::instance()->addNote(newNote))) {
            qInfo() << "Add to datamanager failed:"
                    << "New Note:" << newNote->noteId
                    << "Folder ID:" << newNote->folderId
                    << "Note type:" << newNote->noteType
                    << "Create time:" << newNote->createTime
                    << "Modify time:" << newNote->modifyTime;
            //Add to DataManager failed, release it
            QScopedPointer<VNoteItem> autoRelease(newNote);
            newNote = nullptr;

            //We don't need rollback the maxnoteid here
            //because data aready in the database.
            //folder->maxNoteIdRef()--
            //Should never reach here
        }
    } else {
        qCritical() << "New Note:" << newNote->noteId
                    << "Folder ID:" << newNote->folderId
                    << "Note type:" << newNote->noteType
                    << "Create time:" << newNote->createTime
                    << "Modify time:" << newNote->modifyTime;

        QScopedPointer<VNoteItem> autoRelease(newNote);
        newNote = nullptr;

        //Rollback the id if fialed
        folder->maxNoteIdRef()--;
    }

    return newNote;
}

/**
 * @brief VNoteItemOper::getNote
 * @param folderId
 * @param noteId
 * @return 记事项数据
 */
VNoteItem *VNoteItemOper::getNote(qint64 folderId, qint32 noteId)
{
    return VNoteDataManager::instance()->getNote(folderId, noteId);
}

/**
 * @brief VNoteItemOper::getFolderNotes
 * @param folderId
 * @return 记事项数据
 */
VNOTE_ITEMS_MAP *VNoteItemOper::getFolderNotes(qint64 folderId)
{
    return VNoteDataManager::instance()->getFolderNotes(folderId);
}

/**
 * @brief VNoteItemOper::getDefaultNoteName
 * @param folderId
 * @return 记事项名称
 */
QString VNoteItemOper::getDefaultNoteName(qint64 folderId)
{
    VNoteFolder *folder = VNoteDataManager::instance()->getFolder(folderId);

    QString defaultNoteName = DApplication::translate("DefaultName", "Text");

    if (nullptr != folder && folder->maxNoteIdRef() != 0) {
        defaultNoteName += QString("%1").arg(folder->maxNoteIdRef());
    }

    return defaultNoteName;
}

/**
 * @brief VNoteItemOper::getDefaultVoiceName
 * @return 语音项名称
 */
QString VNoteItemOper::getDefaultVoiceName() const
{
    QString defaultVoiceName = DApplication::translate("DefaultName", "Voice");

    if (nullptr != m_note) {
        defaultVoiceName += QString("%1").arg(m_note->maxVoiceIdRef() + 1);
    }

    return defaultVoiceName;
}

/**
 * @brief VNoteItemOper::deleteNote
 * @return true 成功
 */
bool VNoteItemOper::deleteNote()
{
    bool delOK = false;

    if (nullptr != m_note) {
        VNoteFolder *folder = m_note->folder();

        if (nullptr == folder) {
            VNoteFolderOper folderOps;
            folder = folderOps.getFolder(m_note->folderId);
            m_note->setFolder(folder);
        }

        Q_ASSERT(nullptr != folder);

        //Reset the max note id when folder empty.
        int folderNoteCount = folder->getNotesCount();
        if (Q_UNLIKELY(folderNoteCount == 1)) {
            folder->maxNoteIdRef() = 0;
        }

        DelNoteDbVisitor delNoteVisitor(VNoteDbManager::instance()->getVNoteDb(), m_note, nullptr);

        if (Q_LIKELY(VNoteDbManager::instance()->deleteData(&delNoteVisitor))) {
            //Release note Object
            QScopedPointer<VNoteItem> autoRelease(VNoteDataManager::instance()->delNote(m_note->folderId, m_note->noteId));

            delOK = true;
        } else {
            //Update failed rollback.
            folder->maxNoteIdRef() = folderNoteCount;
        }

        return delOK;
    }

    return delOK;
}

/**
 * @brief VNoteItemOper::updateTop
 * @param value　0取消置顶，１置顶
 * @return true成功，false失败
 */
bool VNoteItemOper::updateTop(int value)
{
    bool updateOK = false;
    if (nullptr != m_note) {
        if (m_note->isTop == value) {
            return updateOK;
        }
        m_note->isTop = value;
        UpdateNoteTopDbVisitor updateNoteVisitor(VNoteDbManager::instance()->getVNoteDb(), m_note, nullptr);
        if (!Q_UNLIKELY(!VNoteDbManager::instance()->updateData(&updateNoteVisitor))) {
            updateOK = true;
        } else {
            m_note->isTop = !value;
        }
    }
    return updateOK;
}

/**
 * @brief VNoteItemOper::updateFolderId
 * @param data 需要更新的笔记
 * @return true成功，false失败
 */
bool VNoteItemOper::updateFolderId(VNoteItem *data)
{
    bool updateOK = false;
    if (nullptr != data) {
        UpdateNoteFolderIdDbVisitor updateNoteVisitor(VNoteDbManager::instance()->getVNoteDb(), data, nullptr);
        if (!Q_UNLIKELY(!VNoteDbManager::instance()->updateData(&updateNoteVisitor))) {
            updateOK = true;
        }
    }
    return updateOK;
}
