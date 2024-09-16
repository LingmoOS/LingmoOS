// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "processopenthread.h"
#include "kprocess.h"

ProcessOpenThread::ProcessOpenThread(QObject *parent)
    : QThread(parent)
{
    connect(this, &ProcessOpenThread::finished, this, &QObject::deleteLater);
}

void ProcessOpenThread::setProgramPath(const QString &strProgramPath)
{
    m_strProgramPath = strProgramPath;
}

void ProcessOpenThread::setArguments(const QStringList &listArguments)
{
    m_listArguments = listArguments;
}

void ProcessOpenThread::run()
{
    // 启动外部进程
    KProcess *cmdprocess = new KProcess;
    cmdprocess->setOutputChannelMode(KProcess::MergedChannels);
    cmdprocess->setNextOpenMode(QIODevice::ReadWrite | QIODevice::Unbuffered | QIODevice::Text);
    cmdprocess->setProgram(m_strProgramPath, m_listArguments);
    cmdprocess->start();
    cmdprocess->waitForFinished(-1);
    delete  cmdprocess;
}
