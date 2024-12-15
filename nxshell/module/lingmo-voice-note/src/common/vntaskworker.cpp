// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vntaskworker.h"
#include "task/vntask.h"

#include <DLog>

/**
 * @brief VNTaskWorker::VNTaskWorker
 * @param parent
 */
VNTaskWorker::VNTaskWorker(QObject *parent)
    : QThread(parent)
{
}

/**
 * @brief VNTaskWorker::addTask
 * @param task 任务线程
 */
void VNTaskWorker::addTask(VNTask *task)
{
    m_taskLock.lock();
    m_safetyTaskQueue.push_back(task);
    m_taskLock.unlock();

    m_taskCondition.wakeAll();
}

/**
 * @brief VNTaskWorker::setWorkerName
 * @param worker 任务名称
 */
void VNTaskWorker::setWorkerName(const QString &worker)
{
    m_workerName = worker;
}

/**
 * @brief VNTaskWorker::quitWorker
 */
void VNTaskWorker::quitWorker()
{
    m_fQuit = true;
    m_taskLock.lock();
    m_taskCondition.wakeAll();
    m_taskLock.unlock();
}

/**
 * @brief VNTaskWorker::run
 */
void VNTaskWorker::run()
{
    do {
        VNTask *task = nullptr;

        m_taskLock.lock();

        if (m_fQuit) {
            qInfo() << m_workerName << "-->Going to quit!";
            m_taskLock.unlock();
            break;
        }

        if (m_safetyTaskQueue.size() == 0) {
            qInfo() << m_workerName << "-->No task to do,waitting!";

            m_taskCondition.wait(&m_taskLock);
        } else {
            task = m_safetyTaskQueue.front();
            m_safetyTaskQueue.pop_front();
        }

        m_taskLock.unlock();

        if (nullptr != task) {
            task->run();

            if (task->autoDelete()) {
                task->deleteLater();
            }
        }
    } while (1);
}
