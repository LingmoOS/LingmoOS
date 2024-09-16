// SPDX-FileCopyrightText: 2019 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sharedmemorymanager.h"
#include <QSharedMemory>
#include <QDebug>
#include <QLoggingCategory>

#ifdef QT_DEBUG
Q_LOGGING_CATEGORY(logSM, "org.deepin.log.viewer.share.memory.manager")
#else
Q_LOGGING_CATEGORY(logSM, "org.deepin.log.viewer.share.memory.manager", QtInfoMsg)
#endif

#define LOG_POLIKIT_STOP_TAG "LOGAUTHCONTROL"
std::atomic<SharedMemoryManager *> SharedMemoryManager::m_instance;
std::mutex SharedMemoryManager::m_mutex;
SharedMemoryManager::SharedMemoryManager(QObject *parent)
    :  QObject(parent)
{
    init();
}

SharedMemoryManager *SharedMemoryManager::getInstance()
{
    return m_instance.load();
}

void SharedMemoryManager::setRunnableTag(ShareMemoryInfo iShareInfo)
{
    m_commondM->lock();
    qCDebug(logSM) << "ShareMemoryInfo.isStart:" << iShareInfo.isStart;
    m_pShareMemoryInfo = static_cast<ShareMemoryInfo *>(m_commondM->data());
    if (m_pShareMemoryInfo) {
        m_pShareMemoryInfo->isStart = iShareInfo.isStart;
    } else {
        //qCWarning(logSM) << "conntrol mem is Not Attech ";
    }
    m_commondM->unlock();

}

QString SharedMemoryManager::getRunnableKey()
{
    return  m_commondM->key();
}

bool SharedMemoryManager::isAttached()
{
    return (m_commondM && m_commondM->isAttached());
}

void SharedMemoryManager::releaseMemory()
{
    if (m_commondM) {
        //  m_commondM->unlock();
        qCDebug(logSM) << "shared memory error:" << m_commondM->error() << m_commondM->errorString();
        if (m_commondM->isAttached())      //检测程序当前是否关联共享内存
            m_commondM->detach();
        qCDebug(logSM) << "shared memory error:" << m_commondM->error() << m_commondM->errorString();

    }
}



void SharedMemoryManager::init()
{
    m_commondM = new QSharedMemory(this);
    QString tag = LOG_POLIKIT_STOP_TAG ;//+ QString::number(m_threadCount);
    m_commondM->setKey(tag);
    if (m_commondM->isAttached())      //检测程序当前是否关联共享内存
        m_commondM->detach();
    if (!m_commondM->create(sizeof(ShareMemoryInfo))) {     //创建共享内存，大小为size
        qCWarning(logSM) << "ShareMemory create error" << m_commondM->key() << QSharedMemory::SharedMemoryError(m_commondM->error()) << m_commondM->errorString();
        if (m_commondM->isAttached())      //检测程序当前是否关联共享内存
            m_commondM->detach();
        m_commondM->attach();

    } else {
        // 创建好以后，保持共享内存连接，防止释放。
        m_commondM->attach();
        // 主进程：首次赋值m_pShareMemoryInfo

    }
    ShareMemoryInfo info;
    info.isStart = true;

    setRunnableTag(info);
}
