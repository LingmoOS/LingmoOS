// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ztaskinterface.h"

#include <QEventLoop>
#include <QTimer>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(TASK, "org.deepin.dde-printer.task")

static const char *g_taskText[] = {"null",
                                   "Refresh devices by backends",
                                   "Refresh devices by host",
                                   "Init ppds",
                                   "not use",
                                   "not use",
                                   "not use",
                                   "not use",
                                   "not use",
                                   "not use",
                                   "Toruble shoot",
                                   "Cups monitor",
                                   "max value"
                                  };

TaskInterface::TaskInterface(int id, QObject *parent)
    : QThread(parent)
{
    m_bQuit = false;
    m_iTaskId = id;
    m_errCode = 0;
}

TaskInterface::~TaskInterface()
{
    qCInfo(TASK) << g_taskText[m_iTaskId];
    stop();
}

void TaskInterface::stop()
{
    m_bQuit = true;

    qCInfo(TASK) << "Stop task " << g_taskText[m_iTaskId];
    this->disconnect();
    if (this->isRunning()) {
        this->quit();
        this->wait();
    }
}

QString TaskInterface::getErrorString()
{
    return m_strLastErr;
}

int TaskInterface::getErrCode()
{
    return m_errCode;
}

void TaskInterface::run()
{
    qCInfo(TASK) << "Task " << g_taskText[m_iTaskId] << " running...";
    int iRet = 0;
    emit signalStatus(m_iTaskId, TStat_Running);
    iRet = doWork();

    if (m_bQuit)
        return;

    //如果doWork中没有设置ErrCode，返回值作为errCode
    if (0 == getErrCode())
        m_errCode = iRet;

    qCInfo(TASK) << "Task " << g_taskText[m_iTaskId] << " finished " << iRet;
    if (0 == iRet)
        emit signalStatus(m_iTaskId, TStat_Suc);
    else
        emit signalStatus(m_iTaskId, TStat_Fail);
}
