// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "LoadInfoThread.h"
#include "GetInfoPool.h"
#include "GenerateDevicePool.h"
#include "DBusInterface.h"
#include "DeviceManager.h"

#include <DApplication>

#include <QLoggingCategory>
#include <QDateTime>
#include <QTimer>

#include<malloc.h>
#include <unistd.h>

DWIDGET_USE_NAMESPACE
static bool firstLoadFlag = true;

LoadInfoThread::LoadInfoThread()
    : mp_ReadFilePool()
    , mp_GenerateDevicePool()
    , m_Running(false)
    , m_FinishedReadFilePool(false)
    , m_Start(true)
{
    connect(&mp_ReadFilePool, &GetInfoPool::finishedAll, this, &LoadInfoThread::slotFinishedReadFilePool);
}

LoadInfoThread::~LoadInfoThread()
{
    long long begin = QDateTime::currentMSecsSinceEpoch();
    while (m_Running)
    {
        long long end = QDateTime::currentMSecsSinceEpoch();
        if (end - begin > 1000)
            _exit(0);
        usleep(100);
    }
    mp_ReadFilePool.deleteLater();
    mp_GenerateDevicePool.deleteLater();
}


void LoadInfoThread::run()
{
    // 判断后台是否正处理update状态
    QString info;
    DBusInterface::getInstance()->getInfo("is_server_running", info);
    // 请求后台更新信息
    m_Running = true;
    if (!info.toInt()) {
        m_Start = false;
        mp_ReadFilePool.getAllInfo();
        mp_ReadFilePool.waitForDone(-1);

        bool readDataFlag = false;
        int readNum = 0;
        while (!readDataFlag) {
            if (m_FinishedReadFilePool) {
                mp_ReadFilePool.getAllInfo();
                mp_ReadFilePool.waitForDone(-1);
            }
            // 为了保证上面那个线程池完全结束
            long long begin = QDateTime::currentMSecsSinceEpoch();
            while (true) {
                readDataFlag = !DeviceManager::instance()->cmdInfo("dmidecode4").isEmpty();
                if (readDataFlag && m_FinishedReadFilePool)
                    break;
                long long end = QDateTime::currentMSecsSinceEpoch();
                if (end - begin > 4000)
                    break;
                usleep(100);
            }
            if ((++readNum) > 3) {
                if (m_FinishedReadFilePool && !readDataFlag) {
                    mp_ReadFilePool.getAllInfo();
                    mp_ReadFilePool.waitForDone(-1);
                    begin = QDateTime::currentMSecsSinceEpoch();
                    while (true) {
                        if (m_FinishedReadFilePool)
                            break;
                        long long end = QDateTime::currentMSecsSinceEpoch();
                        if (end - begin > 4000)
                            break;
                        usleep(100);
                    }
                }
                break;
            }
        }

        m_FinishedReadFilePool = false;
        mp_GenerateDevicePool.generateDevice();
        mp_GenerateDevicePool.waitForDone(-1);
    }

    emit finished("finish");
    m_Running = false;
}

void LoadInfoThread::slotFinishedReadFilePool(const QString &)
{
    m_FinishedReadFilePool = true;
    // 首次加载刷新
    if (firstLoadFlag) {
        firstLoadFlag = false;
        emit finishedReadFilePool();
    }
}

void LoadInfoThread::setFramework(const QString &arch)
{
    // 设置架构
    mp_ReadFilePool.setFramework(arch);
}

