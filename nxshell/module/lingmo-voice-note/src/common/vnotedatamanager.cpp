// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vnotedatamanager.h"
#include "db/vnotedbmanager.h"
#include "task/loadfolderworker.h"
#include "task/loadnoteitemsworker.h"
#include "task/loadiconsworker.h"
#include "vnoteforlder.h"
#include "vnoteitem.h"

#include <DLog>

#include <QThreadPool>

DCORE_USE_NAMESPACE

VNoteDataManager *VNoteDataManager::_instance = nullptr;

/**
 * @brief VNoteDataManager::VNoteDataManager
 * @param parent
 */
VNoteDataManager::VNoteDataManager(QObject *parent)
    : QObject(parent)
{
}

/**
 * @brief VNoteDataManager::instance
 * @return 单例对象
 */
VNoteDataManager *VNoteDataManager::instance()
{
    if (nullptr == _instance) {
        _instance = new VNoteDataManager();

        //Init dabase when create data manager
        //Bcs data manager depends on db
        VNoteDbManager::instance();
    }

    return _instance;
}

/**
 * @brief VNoteDataManager::getNoteFolders
 * @return 记事本数据
 */
VNOTE_FOLDERS_MAP *VNoteDataManager::getNoteFolders()
{
    return m_qspNoteFoldersMap.get();
}

/**
 * @brief VNoteDataManager::addFolder
 * @param folder
 * @return 处理后完整的数据
 */
VNoteFolder *VNoteDataManager::addFolder(VNoteFolder *folder)
{
    VNoteFolder *retFlder = nullptr;

    if (nullptr != folder) {
        m_qspNoteFoldersMap->lock.lockForWrite();

        VNOTE_FOLDERS_DATA_MAP::iterator it = m_qspNoteFoldersMap->folders.find(folder->id);

        if (it == m_qspNoteFoldersMap->folders.end()) {
            m_qspNoteFoldersMap->folders.insert(folder->id, folder);
        } else {
            QScopedPointer<VNoteFolder> release(*it);

            m_qspNoteFoldersMap->folders.remove(folder->id);
            m_qspNoteFoldersMap->folders.insert(folder->id, folder);
        }

        m_qspNoteFoldersMap->lock.unlock();

        retFlder = folder;
    }

    return retFlder;
}

/**
 * @brief VNoteDataManager::getFolder
 * @param folderId
 * @return  记事本数据
 */
VNoteFolder *VNoteDataManager::getFolder(qint64 folderId)
{
    VNoteFolder *retFlder = nullptr;

    m_qspNoteFoldersMap->lock.lockForRead();

    VNOTE_FOLDERS_DATA_MAP::iterator it = m_qspNoteFoldersMap->folders.find(folderId);

    if (it != m_qspNoteFoldersMap->folders.end()) {
        retFlder = *it;
    }

    m_qspNoteFoldersMap->lock.unlock();

    return retFlder;
}

/**
 * @brief VNoteDataManager::delFolder
 * @param folderId
 * @return 移除的记事本数据
 */
VNoteFolder *VNoteDataManager::delFolder(qint64 folderId)
{
    VNoteFolder *retFlder = nullptr;

    m_qspNoteFoldersMap->lock.lockForWrite();

    //Find the folder first. if can't find, may some goes to wrong
    VNOTE_FOLDERS_DATA_MAP::iterator itFolder = m_qspNoteFoldersMap->folders.find(folderId);

    if (itFolder != m_qspNoteFoldersMap->folders.end()) {
        //Get notes lock ,release all notes in the folder
        m_qspAllNotesMap->lock.lockForWrite();

        VNOTE_ALL_NOTES_DATA_MAP::iterator itNote = m_qspAllNotesMap->notes.find(folderId);

        if (itNote != m_qspAllNotesMap->notes.end()) {
            VNOTE_ITEMS_MAP *itemsMap = itNote.value();
            m_qspAllNotesMap->notes.erase(itNote);
            QScopedPointer<VNOTE_ITEMS_MAP> foldersMap(itemsMap);

            //Remove voice files in the folder
            for (auto it : foldersMap->folderNotes) {
                it->delNoteData();
            }
        }

        //All note released. unlock
        m_qspAllNotesMap->lock.unlock();

        retFlder = *itFolder;
        m_qspNoteFoldersMap->folders.erase(itFolder);
    }

    m_qspNoteFoldersMap->lock.unlock();

    return retFlder;
}

/**
 * @brief VNoteDataManager::folderCount
 * @return 记事本数量
 */
qint32 VNoteDataManager::folderCount()
{
    m_qspNoteFoldersMap->lock.lockForRead();
    int count = m_qspNoteFoldersMap->folders.size();
    m_qspNoteFoldersMap->lock.unlock();

    return count;
}

/**
 * @brief VNoteDataManager::addNote
 * @param note 记事项
 * @return 处理后完整的记事项数据
 */
VNoteItem *VNoteDataManager::addNote(VNoteItem *note)
{
    VNoteItem *retNote = nullptr;

    if (nullptr != note) {
        m_qspAllNotesMap->lock.lockForWrite();

        //Find the folder first. if can't find, may some goes to wrong
        VNOTE_ALL_NOTES_DATA_MAP::iterator it = m_qspAllNotesMap->notes.find(note->folderId);

        if (it != m_qspAllNotesMap->notes.end()) {
            VNOTE_ITEMS_MAP *notesInFolder = *it;

            Q_ASSERT(nullptr != notesInFolder);

            //TODO:
            //    Need lock the folder first,for muti-thread operation.
            //But new maybe not necessary
            notesInFolder->lock.lockForWrite();

            //Find if alreay exist same id note
            VNOTE_ITEMS_DATA_MAP::iterator noteIter = notesInFolder->folderNotes.find(note->noteId);

            if (noteIter == notesInFolder->folderNotes.end()) {
                notesInFolder->folderNotes.insert(note->noteId, note);
            } else {
                //Release old and insert new
                QScopedPointer<VNoteItem> release(*noteIter);

                notesInFolder->folderNotes.remove(note->noteId);
                notesInFolder->folderNotes.insert(note->noteId, note);
            }

            notesInFolder->lock.unlock();
        } else {
            qInfo() << __FUNCTION__ << "Add note failed: the folder don't exist:" << note->folderId;

            VNOTE_ITEMS_MAP *folderNotes = new VNOTE_ITEMS_MAP();

            //Set release flag true in data manager
            folderNotes->autoRelease = true;

            folderNotes->folderNotes.insert(note->noteId, note);
            m_qspAllNotesMap->notes.insert(note->folderId, folderNotes);
        }

        m_qspAllNotesMap->lock.unlock();

        retNote = note;
    }

    return retNote;
}

/**
 * @brief VNoteDataManager::getNote
 * @param folderId
 * @param noteId
 * @return 记事项数据
 */
VNoteItem *VNoteDataManager::getNote(qint64 folderId, qint32 noteId)
{
    VNoteItem *retNote = nullptr;

    m_qspAllNotesMap->lock.lockForRead();

    //Find the folder first. if can't find, may some goes to wrong
    VNOTE_ALL_NOTES_DATA_MAP::iterator it = m_qspAllNotesMap->notes.find(folderId);

    if (it != m_qspAllNotesMap->notes.end()) {
        VNOTE_ITEMS_MAP *notesInFolder = *it;

        Q_ASSERT(nullptr != notesInFolder);

        //TODO:
        //    Need lock the folder first,for muti-thread operation.
        //But new maybe not necessary
        notesInFolder->lock.lockForRead();

        //Find the note
        VNOTE_ITEMS_DATA_MAP::iterator noteIter = notesInFolder->folderNotes.find(noteId);

        if (noteIter != notesInFolder->folderNotes.end()) {
            retNote = *noteIter;
        }

        notesInFolder->lock.unlock();
    } else {
        qCritical() << __FUNCTION__ << "Get note failed: the folder don't exist:" << folderId;
    }

    m_qspAllNotesMap->lock.unlock();

    return retNote;
}

/**
 * @brief VNoteDataManager::delNote
 * @param folderId
 * @param noteId
 * @return 移除的记事项数据
 */
VNoteItem *VNoteDataManager::delNote(qint64 folderId, qint32 noteId)
{
    VNoteItem *retNote = nullptr;

    m_qspAllNotesMap->lock.lockForWrite();

    //Find the folder first. if can't find, may some goes to wrong
    VNOTE_ALL_NOTES_DATA_MAP::iterator it = m_qspAllNotesMap->notes.find(folderId);

    if (it != m_qspAllNotesMap->notes.end()) {
        VNOTE_ITEMS_MAP *notesInFolder = *it;

        Q_ASSERT(nullptr != notesInFolder);

        //TODO:
        //    Need lock the folder first,for muti-thread operation.
        //But new maybe not necessary
        notesInFolder->lock.lockForWrite();

        //Find the note
        VNOTE_ITEMS_DATA_MAP::iterator noteIter = notesInFolder->folderNotes.find(noteId);

        if (noteIter != notesInFolder->folderNotes.end()) {
            retNote = *noteIter;
            notesInFolder->folderNotes.erase(noteIter);

            //Remove voice file of voice note
            retNote->delNoteData();
        }

        notesInFolder->lock.unlock();
    } else {
        qCritical() << __FUNCTION__ << "Delete note failed: the folder don't exist:" << folderId;
    }

    m_qspAllNotesMap->lock.unlock();

    return retNote;
}

/**
 * @brief VNoteDataManager::folderNotesCount
 * @param folderId
 * @return 记事项数量
 */
qint32 VNoteDataManager::folderNotesCount(qint64 folderId)
{
    qint32 notesCount = 0;

    VNOTE_ITEMS_MAP *folderNotes = getFolderNotes(folderId);

    if (nullptr != folderNotes) {
        folderNotes->lock.lockForRead();
        notesCount = folderNotes->folderNotes.count();
        folderNotes->lock.unlock();
    }

    return notesCount;
}

/**
 * @brief VNoteDataManager::getFolderNotes
 * @param folderId
 * @return 记事本所有的记事项数据
 */
VNOTE_ITEMS_MAP *VNoteDataManager::getFolderNotes(qint64 folderId)
{
    VNOTE_ITEMS_MAP *folderNotes = nullptr;

    m_qspAllNotesMap->lock.lockForRead();

    //Find the folder first. if can't find, may some goes to wrong
    VNOTE_ALL_NOTES_DATA_MAP::iterator it = m_qspAllNotesMap->notes.find(folderId);

    if (it != m_qspAllNotesMap->notes.end()) {
        folderNotes = *it;
    } else {
        //       qInfo() << __FUNCTION__ << "Get note failed: the folder don't exist:" << folderId;
        folderNotes = new VNOTE_ITEMS_MAP();
        folderNotes->autoRelease = true;
        m_qspAllNotesMap->notes.insert(folderId, folderNotes);
    }

    m_qspAllNotesMap->lock.unlock();

    return folderNotes;
}

/**
 * @brief VNoteDataManager::getDefaultIcon
 * @param index
 * @param type
 * @return 图标
 */
QPixmap VNoteDataManager::getDefaultIcon(qint32 index, IconsType type)
{
    m_iconLock.lockForRead();
    if (index < 1 || index > m_defaultIcons[type].size()) {
        index = 0;
    }

    QPixmap icon = m_defaultIcons[type].at(index);

    m_iconLock.unlock();

    return icon;
}

/**
 * @brief VNoteDataManager::isAllDatasReady
 * @return true 所有数据加载完成
 */
bool VNoteDataManager::isAllDatasReady() const
{
    qInfo() << "m_fDataState:" << m_fDataState;

    if ((m_fDataState & DataState::FolderDataReady)
        && (m_fDataState & DataState::NotesDataReady)) {
        return true;
    } else {
        return false;
    }
}

/**
 * @brief VNoteDataManager::reqNoteDefIcons
 */
void VNoteDataManager::reqNoteDefIcons()
{
    LoadIconsWorker *iconLoadWorker = new LoadIconsWorker(); //
    iconLoadWorker->setAutoDelete(true);

    QThreadPool::globalInstance()->start(iconLoadWorker, QThread::TimeCriticalPriority);
}

/**
 * @brief VNoteDataManager::reqNoteFolders
 */
void VNoteDataManager::reqNoteFolders()
{
    if (m_pNotesLoadThread == nullptr) {
        m_pForldesLoadThread = new LoadFolderWorker();
        m_pForldesLoadThread->setAutoDelete(true);

        connect(m_pForldesLoadThread, &LoadFolderWorker::onFoldersLoaded,
                this, &VNoteDataManager::onFoldersLoaded);

        QThreadPool::globalInstance()->start(m_pForldesLoadThread);
    }
}

/**
 * @brief VNoteDataManager::reqNoteItems
 */
void VNoteDataManager::reqNoteItems()
{
    if (m_pNotesLoadThread == nullptr) {
        m_pNotesLoadThread = new LoadNoteItemsWorker();
        m_pNotesLoadThread->setAutoDelete(true);

        connect(m_pNotesLoadThread, &LoadNoteItemsWorker::onAllNotesLoaded,
                this, &VNoteDataManager::onAllNotesLoaded);

        QThreadPool::globalInstance()->start(m_pNotesLoadThread);
    }
}

/**
 * @brief VNoteDataManager::getAllNotesInFolder
 * @return 所有记事本的记事项数据
 */
VNOTE_ALL_NOTES_MAP *VNoteDataManager::getAllNotesInFolder()
{
    return m_qspAllNotesMap.get();
}

/**
 * @brief VNoteDataManager::onFoldersLoaded
 * @param foldesMap 记事本数据
 */
void VNoteDataManager::onFoldersLoaded(VNOTE_FOLDERS_MAP *foldesMap)
{
    //Release old data
    if (m_qspNoteFoldersMap != nullptr) {
        qInfo() << "Release old foldersMap:" << m_qspNoteFoldersMap.get()
                << " size:" << m_qspNoteFoldersMap->folders.size();

        m_qspNoteFoldersMap->lock.lockForWrite();

        for (auto it : m_qspNoteFoldersMap->folders) {
            delete it;
        }

        m_qspNoteFoldersMap->folders.clear();

        m_qspNoteFoldersMap->lock.unlock();
    }

    m_qspNoteFoldersMap.reset(foldesMap);

    qInfo() << "Loaded new foldersMap:" << m_qspNoteFoldersMap.get()
            << " size:" << m_qspNoteFoldersMap->folders.size();

    //Object is already deleted
    m_pForldesLoadThread = nullptr;

    //Set folder data ready flag
    m_fDataState |= DataState::FolderDataReady;

    emit onNoteFoldersLoaded();

    //Send data ready signal if data ready
    if (isAllDatasReady()) {
        emit onAllDatasReady();
    }
}

/**
 * @brief VNoteDataManager::onAllNotesLoaded
 * @param notesMap 笔记数据
 */
void VNoteDataManager::onAllNotesLoaded(VNOTE_ALL_NOTES_MAP *notesMap)
{
    //Release old data
    if (m_qspAllNotesMap != nullptr) {
        qInfo() << "Release old notesMap:" << m_qspAllNotesMap.get()
                << "All notes in folders:" << m_qspAllNotesMap->notes.size();

        m_qspAllNotesMap->lock.lockForWrite();

        for (auto folderNotes : m_qspAllNotesMap->notes) {
            for (auto note : folderNotes->folderNotes) {
                delete note;
            }

            folderNotes->folderNotes.clear();
            delete reinterpret_cast<VNOTE_ITEMS_MAP *>(folderNotes);
        }

        m_qspAllNotesMap->notes.clear();

        m_qspAllNotesMap->lock.unlock();
    }

    m_qspAllNotesMap.reset(notesMap);

    qInfo() << "Release old notesMap:" << m_qspAllNotesMap.get()
            << "All notes in folders:" << m_qspAllNotesMap->notes.size();

    //Object is already deleted
    m_pNotesLoadThread = nullptr;

    //Set folder data ready flag
    m_fDataState |= DataState::NotesDataReady;

    emit onNoteItemsLoaded();

    //Send data ready signal if data ready
    if (isAllDatasReady()) {
        emit onAllDatasReady();
    }
}
