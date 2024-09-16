// SPDX-FileCopyrightText: 2019 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "viewapplication.h"

#include <QDebug>
#include <QFile>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QThread>
#include <QCommandLineParser>
#include <QSharedMemory>
#include <QBuffer>
#include <QDataStream>
#include <QLoggingCategory>

#include <iostream>
#include<signal.h>

#ifdef QT_DEBUG
Q_LOGGING_CATEGORY(logViewApp, "org.deepin.log.viewer.application.view")
#else
Q_LOGGING_CATEGORY(logViewApp, "org.deepin.log.viewer.application.view", QtInfoMsg)
#endif

ViewApplication::ViewApplication(int &argc, char **argv): QCoreApplication(argc, argv),m_commondM(new QSharedMemory())
{
    QCommandLineParser parser;
    parser.process(*this);
    const QStringList fileList = parser.positionalArguments();
    if (fileList.count() < 2) {
        qCCritical(logViewApp) << "cmd param count less than 2";
        return ;
    }
    bool useFinishedSignal = false;
    bool onlyExec = false;
    QStringList arg;
    if (fileList[0] == "dmesg") {
        arg << "-c"
            << "dmesg -r";
    } else if(fileList[0] == "coredumpctl-list"){
        arg << "-c"
            << "coredumpctl list --no-pager";
        useFinishedSignal = true;
    } else if(fileList[0] == "coredumpctl-info"){
        arg << "-c"
            << QString("coredumpctl info %1").arg(fileList[1]);
    } else if(fileList[0] == "coredumpctl-dump" && fileList.count() > 2){
        arg << "-c"
            << QString("coredumpctl dump %1 -o %2").arg(fileList[1]).arg(fileList[2]);
        onlyExec = true;
    } else if(fileList[0] == "readelf" && fileList.count() > 1){
        arg << "-c"
            << QString("readelf -n %1").arg(fileList[1]);
        useFinishedSignal = true;
    } else {
        arg << "-c" << QString("cat %1").arg(fileList[0]);
    }
    m_proc = new QProcess(this);

    m_commondM->setKey(fileList[1]);
    m_commondM->create(sizeof(ShareMemoryInfo));
    if (m_commondM->isAttached())      //检测程序当前是否关联共享内存
        m_commondM->detach();          //解除关联
    m_commondM->attach(QSharedMemory::ReadOnly);

    if (useFinishedSignal) {
        // 为读取崩溃日志全部信息，需要使用finished信号，使用readyReadStandardOutput信号，获取的日志信息不全
        connect(m_proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [ = ](int exitCode, QProcess::ExitStatus exitStatus) {
            Q_UNUSED(exitCode)
            Q_UNUSED(exitStatus)
            if (!getControlInfo().isStart)
            {
                m_proc->kill();
                releaseMemery();
                exit(0);
            }

            QByteArray byte =   m_proc->readAll();
            QStringList strList = QString(byte.replace('\u0000', "").replace("\x01", "")).split('\n', QString::SkipEmptyParts);
            std::cout << byte.replace('\u0000', "").data();
        });
    } else if (!onlyExec) {
        connect(m_proc, &QProcess::readyReadStandardOutput, this, [ = ] {
            if (!getControlInfo().isStart)
            {
                m_proc->kill();
                releaseMemery();
                exit(0);
            }
            QByteArray byte =   m_proc->readAll();
            std::cout << byte.replace('\u0000', "").data();
        });
    }

    m_proc->start("/bin/bash", arg);
    m_proc->waitForFinished(-1);
    m_proc->close();
}

ViewApplication::~ViewApplication()
{
    releaseMemery();
}

void ViewApplication::releaseMemery()
{
    if (m_commondM) {
        //  m_commondM->unlock();
        if (m_commondM->isAttached())      //检测程序当前是否关联共享内存
            m_commondM->detach();          //解除关联
        m_commondM->deleteLater();
    }
}

ViewApplication::ShareMemoryInfo ViewApplication::getControlInfo()
{
    ShareMemoryInfo defaultInfo;
    defaultInfo.isStart = false;
    if (m_commondM && m_commondM->isAttached()) {
        m_commondM->lock();
        ShareMemoryInfo   *m_pShareMemoryInfo = static_cast<ShareMemoryInfo *>(m_commondM->data());
        defaultInfo = m_pShareMemoryInfo ? *m_pShareMemoryInfo : defaultInfo;
        m_commondM->unlock();
        return defaultInfo;

    }
    return defaultInfo;
}
