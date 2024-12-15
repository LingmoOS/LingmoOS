// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "olddbvisistors.h"
#include "common/vnoteforlder.h"
#include "common/vnoteitem.h"
#include "globaldef.h"

#include <DLog>
#include <DApplication>

/**
 * @brief OldFolderQryDbVisitor::OldFolderQryDbVisitor
 * @param db
 * @param inParam 参数
 * @param result 结果
 */
OldFolderQryDbVisitor::OldFolderQryDbVisitor(QSqlDatabase &db, const void *inParam, void *result)
    : DbVisitor(db, inParam, result)
{
}

/**
 * @brief OldFolderQryDbVisitor::visitorData
 * @return true 成功
 */
bool OldFolderQryDbVisitor::visitorData()
{
    bool isOK = false;

    if (nullptr != results.folders) {
        isOK = true;

        enum OldFolder {
            id = 0,
            name,
            create_time,
        };

        while (m_sqlQuery->next()) {
            VNoteFolder *folder = new VNoteFolder();

            //Only three fields available in old db.
            folder->id = m_sqlQuery->value(OldFolder::id).toInt();
            folder->name = m_sqlQuery->value(OldFolder::name).toString();
            folder->createTime =
                m_sqlQuery->value(OldFolder::create_time).toDateTime();

            //Need init other fields by ourself
            folder->category = 0;
            folder->defaultIcon = 0;
            folder->iconPath = "";
            folder->folder_state = 0;
            folder->maxNoteIdRef() = 0;
            folder->modifyTime = folder->createTime;
            folder->deleteTime = folder->createTime;

#ifdef QT_QML_DEBUG
            qInfo() << "" << (*folder);
#endif

            results.folders->folders.insert(folder->id, folder);
        }
    }

    return isOK;
}

/**
 * @brief OldFolderQryDbVisitor::prepareSqls
 * @return true 成功
 */
bool OldFolderQryDbVisitor::prepareSqls()
{
    //Query old folder data;
    QString querySql("SELECT id, name, create_time FROM folder;");

    m_dbvSqls.append(querySql);

    return true;
}

/**
 * @brief OldNoteQryDbVisitor::OldNoteQryDbVisitor
 * @param db
 * @param inParam
 * @param result
 */
OldNoteQryDbVisitor::OldNoteQryDbVisitor(QSqlDatabase &db, const void *inParam, void *result)
    : DbVisitor(db, inParam, result)
{
}

/**
 * @brief OldNoteQryDbVisitor::visitorData
 * @return true 成功
 */
bool OldNoteQryDbVisitor::visitorData()
{
    bool isOK = false;

    if (nullptr != results.notes) {
        isOK = true;

        enum OldNote {
            id = 0,
            folder_id,
            note_type,
            content_text,
            content_path,
            voice_time,
            create_time,
        };

        while (m_sqlQuery->next()) {
            VNoteItem *note = new VNoteItem();

            note->noteId = m_sqlQuery->value(OldNote::id).toInt();
            note->folderId = m_sqlQuery->value(OldNote::folder_id).toInt();
            note->noteType = m_sqlQuery->value(OldNote::note_type).toInt();

            QString text = m_sqlQuery->value(OldNote::content_text).toString();
            QString voicePath = m_sqlQuery->value(OldNote::content_path).toString();
            qint64 voiceSize = m_sqlQuery->value(OldNote::voice_time).toLongLong();

            note->createTime =
                m_sqlQuery->value(OldNote::create_time).toDateTime();

            VNoteBlock *ptrBlock = nullptr;

            if (note->noteType == Voice) {
                //Voice note need two TextBlock at voice
                //sides
                ptrBlock = new VNTextBlock();
                note->addBlock(ptrBlock);

                QString defaultVoiceName = DApplication::translate("DefaultName", "Voice");
                ptrBlock = new VNVoiceBlock();
                ptrBlock->ptrVoice->voicePath = voicePath;
                ptrBlock->ptrVoice->voiceSize = voiceSize;
                ptrBlock->ptrVoice->voiceTitle = defaultVoiceName + "1";
                ptrBlock->ptrVoice->createTime = note->createTime;
                ptrBlock->ptrVoice->blockText = text;
                note->addBlock(ptrBlock);

                ptrBlock = new VNTextBlock();
                note->addBlock(ptrBlock);
            } else {
                ptrBlock = new VNTextBlock();
                ptrBlock->blockText = text;
                note->addBlock(ptrBlock);
            }

            note->noteTitle = "";
            note->noteState = 0;

            note->modifyTime = note->createTime;
            note->deleteTime = note->createTime;

            VNOTE_ALL_NOTES_DATA_MAP::iterator it =
                results.notes->notes.find(note->folderId);
#ifdef QT_QML_DEBUG
            qInfo() << "" << (*note);
#endif
            //TODO
            //    If find the folder add note to it, or need create
            //folder items map first;
            if (it != results.notes->notes.end()) {
                (*it)->folderNotes.insert(note->noteId, note);
            } else {
                VNOTE_ITEMS_MAP *folderNotes = new VNOTE_ITEMS_MAP();

                //DataManager data should set autoRelease flag
                folderNotes->autoRelease = true;

                folderNotes->folderNotes.insert(note->noteId, note);
                results.notes->notes.insert(note->folderId, folderNotes);
            }
        }
    }

    return isOK;
}

/**
 * @brief OldNoteQryDbVisitor::prepareSqls
 * @return true 成功
 */
bool OldNoteQryDbVisitor::prepareSqls()
{
    //Query old notes data
    QString querySql("SELECT id, folder_id, note_type, content_text, content_path, voice_time, create_time FROM note;");

    m_dbvSqls.append(querySql);

    return true;
}
