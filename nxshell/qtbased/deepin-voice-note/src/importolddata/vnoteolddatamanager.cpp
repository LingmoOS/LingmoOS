// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vnoteolddatamanager.h"
#include "olddataloadwokers.h"

#include <DLog>

#include <QThreadPool>

VNoteOldDataManager *VNoteOldDataManager::_instance = nullptr;
VNoteDbManager *VNoteOldDataManager::m_oldDbManger = nullptr;

/**
 * @brief VNoteOldDataManager::VNoteOldDataManager
 * @param parent
 */
VNoteOldDataManager::VNoteOldDataManager(QObject *parent)
    : QObject(parent)
{
}

/**
 * @brief VNoteOldDataManager::instance
 * @return 单例对象
 */
VNoteOldDataManager *VNoteOldDataManager::instance()
{
    if (nullptr == _instance) {
        _instance = new VNoteOldDataManager();
    }

    return _instance;
}

/**
 * @brief VNoteOldDataManager::releaseInstance
 */
void VNoteOldDataManager::releaseInstance()
{
    if (nullptr != _instance) {
        delete m_oldDbManger;
        m_oldDbManger = nullptr;

        delete _instance;
        _instance = nullptr;
    }
}

/**
 * @brief VNoteOldDataManager::folders
 * @return 记事本数据
 */
VNOTE_FOLDERS_MAP *VNoteOldDataManager::folders()
{
    return m_qspNoteFoldersMap.get();
}

/**
 * @brief VNoteOldDataManager::allNotes
 * @return 记事项数据
 */
VNOTE_ALL_NOTES_MAP *VNoteOldDataManager::allNotes()
{
    return m_qspAllNotes.get();
}

/**
 * @brief VNoteOldDataManager::initOldDb
 */
void VNoteOldDataManager::initOldDb()
{
    //Init old database when create data manager
    //Bcs data manager depends on db.
    m_oldDbManger = new VNoteDbManager(true, this);
}

/**
 * @brief VNoteOldDataManager::reqDatas
 */
void VNoteOldDataManager::reqDatas()
{
    OldDataLoadTask *pOldDataLoadTask = new OldDataLoadTask();
    pOldDataLoadTask->setAutoDelete(true);

    connect(pOldDataLoadTask, &OldDataLoadTask::finishLoad, this, &VNoteOldDataManager::onFinishLoad);

    QThreadPool::globalInstance()->start(pOldDataLoadTask);
}

/**
 * @brief VNoteOldDataManager::doUpgrade
 */
void VNoteOldDataManager::doUpgrade()
{
    OldDataUpgradeTask *pOldDataUpgradeTask = new OldDataUpgradeTask();
    pOldDataUpgradeTask->setAutoDelete(true);

    connect(pOldDataUpgradeTask, &OldDataUpgradeTask::finishUpgrade, this, &VNoteOldDataManager::onFinishUpgrade);
    connect(pOldDataUpgradeTask, &OldDataUpgradeTask::progressValue, this, &VNoteOldDataManager::onProgress);

    QThreadPool::globalInstance()->start(pOldDataUpgradeTask);
}

/**
 * @brief VNoteOldDataManager::onFinishLoad
 */
void VNoteOldDataManager::onFinishLoad()
{
    //Just notify data ready.
    emit dataReady();
}

/**
 * @brief VNoteOldDataManager::onFinishUpgrade
 */
void VNoteOldDataManager::onFinishUpgrade()
{
    emit upgradeFinish();
}

/**
 * @brief VNoteOldDataManager::onProgress
 * @param value
 */
void VNoteOldDataManager::onProgress(int value)
{
    emit progressValue(value);
}
