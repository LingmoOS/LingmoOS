// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "upgradedbutil.h"
#include "vnoteolddatamanager.h"
#include "db/vnotedbmanager.h"
#include "db/vnotefolderoper.h"
#include "db/vnoteitemoper.h"
#include "common/vnoteforlder.h"
#include "common/vnoteitem.h"
#include "globaldef.h"
#include "setting.h"

#include <DLog>

const QString UpgradeDbUtil::UPGRADE_STATE = "old.UpgradeDb/importOldDbState";

/**
 * @brief UpgradeDbUtil::UpgradeDbUtil
 */
UpgradeDbUtil::UpgradeDbUtil()
{
}

/**
 * @brief UpgradeDbUtil::saveUpgradeState
 * @param state 升级标志
 */
void UpgradeDbUtil::saveUpgradeState(int state)
{
    setting::instance()->setOption(UPGRADE_STATE, state);
}

/**
 * @brief UpgradeDbUtil::readUpgradeState
 * @return 升级标志
 */
int UpgradeDbUtil::readUpgradeState()
{
    int state = Invalid;

    state = setting::instance()->getOption(UPGRADE_STATE).toInt();

    return state;
}

/**
 * @brief UpgradeDbUtil::needUpdateOldDb
 * @param state
 * @return true 需要升级
 */
bool UpgradeDbUtil::needUpdateOldDb(int state)
{
    /*
     Only need update when both old exist and
     the update state is don't UpdateDone condition
     are arrived
    */
    bool fNeedUpdate = false;
    bool fHaveOldDb = VNoteDbManager::hasOldDataBase();

    if (fHaveOldDb) {
        VNoteOldDataManager::instance()->initOldDb();
    }

    if (fHaveOldDb && (state != UpdateDone)) {
        fNeedUpdate = true;
    }

    return fNeedUpdate;
}

/**
 * @brief UpgradeDbUtil::checkUpdateState
 * @param state
 */
void UpgradeDbUtil::checkUpdateState(int state)
{
    if (state == Processing) {
        qInfo() << "Upgrade old database exception dectected.";

        QString vnoteDatabasePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                                    + QDir::separator()
                                    + DEEPIN_VOICE_NOTE + QString(VNoteDbManager::DBVERSION) + QString(".db");

        QFileInfo dbFileInfo(vnoteDatabasePath);

        if (dbFileInfo.exists()) {
            QFile dbFile(vnoteDatabasePath);

            if (!dbFile.remove()) {
                qInfo() << "Remove exception db error:" << dbFile.errorString();
            }
        }
    }
}

/**
 * @brief UpgradeDbUtil::backUpOldDb
 */
void UpgradeDbUtil::backUpOldDb()
{
    QString vnoteDatabasePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                                + QDir::separator();

    QString oldDbName = DEEPIN_VOICE_NOTE + QString(".db");

    QFileInfo backupDir(vnoteDatabasePath + ".backup/");

    if (!backupDir.exists()) {
        QDir().mkdir(backupDir.filePath());
    }

    QFileInfo dbFileInfo(vnoteDatabasePath + oldDbName);

    if (dbFileInfo.exists()) {
        QDir(backupDir.filePath()).remove(backupDir.filePath() + QDir::separator() + oldDbName);

        QFile oldDBFile(dbFileInfo.filePath());

        if (!oldDBFile.rename(backupDir.filePath() + QDir::separator() + oldDbName)) {
            qInfo() << "Backup old database failed." << oldDBFile.errorString();
        }
    }
}

/**
 * @brief UpgradeDbUtil::clearVoices
 */
void UpgradeDbUtil::clearVoices()
{
    QDir oldVoiceDir(QStandardPaths::standardLocations(
                         QStandardPaths::DocumentsLocation)
                         .first()
                     + QDir::separator() + "voicenote/");

    if (oldVoiceDir.exists()) {
        if (!oldVoiceDir.removeRecursively()) {
            qInfo() << "Clear old voices failed!";
        } else {
            qInfo() << "Clear old voices done!";
        }
    }
}

/**
 * @brief UpgradeDbUtil::doFolderUpgrade
 * @param folder
 */
void UpgradeDbUtil::doFolderUpgrade(VNoteFolder *folder)
{
    qInfo() << "" << folder;

    if (nullptr != folder) {
        VNoteFolderOper folderOper;
        VNoteFolder *newVersionFolder = folderOper.addFolder(*folder);

        if (nullptr != newVersionFolder) {
            doFolderNoteUpgrade(newVersionFolder->id, folder->id);
        }
    }
}

/**
 * @brief UpgradeDbUtil::doFolderNoteUpgrade
 * @param newFolderId
 * @param oldFolderId
 */
void UpgradeDbUtil::doFolderNoteUpgrade(qint64 newFolderId, qint64 oldFolderId)
{
    VNOTE_ALL_NOTES_MAP *allNotes = VNoteOldDataManager::instance()->allNotes();

    QString appAudioPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QDir::separator() + "voicenote/";

    //Create audio dir if doesn't exist.
    if (!QFileInfo(appAudioPath).exists()) {
        QDir().mkdir(appAudioPath);
    }

    if (nullptr != allNotes) {
        auto folderNotes = allNotes->notes.find(oldFolderId);

        if (folderNotes != allNotes->notes.end()) {
            VNoteItemOper noteOper;

            for (auto note : folderNotes.value()->folderNotes) {
                //Change the old folder id to new folder id
                note->folderId = newFolderId;
                note->noteTitle = noteOper.getDefaultNoteName(newFolderId);

                if (note->haveVoice()) {
                    VNoteBlock *ptrBlock = nullptr;

                    for (auto it : note->datas.dataConstRef()) {
                        if (it->getType() == VNoteBlock::Voice) {
                            ptrBlock = it;
                            break;
                        }
                    }

                    if (nullptr != ptrBlock && !ptrBlock->ptrVoice->voicePath.isEmpty()) {
                        QFileInfo oldFileInfo(ptrBlock->ptrVoice->voicePath);

                        QString newVoiceName =
                            ptrBlock->ptrVoice->createTime.toString("yyyyMMddhhmmss") + QString(".mp3");

                        QString targetPath = appAudioPath + newVoiceName;

                        QFile oldFile(ptrBlock->ptrVoice->voicePath);

                        if (!oldFile.copy(targetPath)) {
                            qInfo() << "Copy file failed:" << targetPath
                                    << " error:" << oldFile.errorString();
                        } else {
                            ptrBlock->ptrVoice->voicePath = targetPath;
                        }
                    }
                }

                noteOper.addNote(*note);
            }
        }
    }
}
