// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DealDependThread.h"
#include "model/deblistmodel.h"
#include "utils/hierarchicalverify.h"

DealDependThread::DealDependThread(QObject *parent)
{
    Q_UNUSED(parent);
    proc = new QProcess(this);
    connect(proc, static_cast<void (QProcess::*)(int)>(&QProcess::finished), this, &DealDependThread::slotInstallFinished);
    connect(proc, &QProcess::readyReadStandardOutput, this, &DealDependThread::slotReadOutput);
}

DealDependThread::~DealDependThread()
{
    delete proc;
}

void DealDependThread::setDependsList(const QStringList &dependList, int index)
{
    m_index = index;
    m_dependsList = dependList;
}

void DealDependThread::setBrokenDepend(const QString &dependName)
{
    m_brokenDepend = dependName;
}

void DealDependThread::slotReadOutput()
{
    QString tmp = proc->readAllStandardOutput().data();

    if (tmp.contains("StartInstallDeepinwine")) {
        emit signalDependResult(DebListModel::AuthConfirm, m_index, m_brokenDepend);
        return;
    }
    if (tmp.contains("Not authorized")) {
        bDependsStatusErr = true;
        qWarning() << qPrintable("install Wine dependency Not authorized");
        emit signalDependResult(DebListModel::CancelAuth, m_index, m_brokenDepend);
    }

    // 检测是否包含分级验签错误信息
    if (HierarchicalVerify::instance()->checkTransactionError(m_brokenDepend, tmp)) {
        bVerifyStatusErr = true;
        qWarning() << QString("[Hierarchical] Install Wine dependency not verify, [output]: %1").arg(tmp);
        // 结束后统一发送信号
    }
}

void DealDependThread::run()
{
    proc->setProcessChannelMode(QProcess::MergedChannels);
    msleep(100);

    bDependsStatusErr = false;
    bVerifyStatusErr = false;

    emit signalDependResult(DebListModel::AuthBefore, m_index, m_brokenDepend);
    proc->start("pkexec",
                QStringList() << "deepin-deb-installer-dependsInstall"
                              << "InstallDeepinWine" << m_dependsList);
    emit signalEnableCloseButton(false);
}

void DealDependThread::slotInstallFinished(int num = -1)
{
    if (bDependsStatusErr) {
        emit signalDependResult(DebListModel::AnalysisErr, m_index, m_brokenDepend);
        bDependsStatusErr = false;
        return;
    }

    if (bVerifyStatusErr) {
        emit signalDependResult(DebListModel::VerifyDependsErr, m_index, m_brokenDepend);
        bVerifyStatusErr = false;
        return;
    }

    if (num == 0) {
        emit signalDependResult(DebListModel::AuthDependsSuccess, m_index, m_brokenDepend);
    } else {
        qWarning() << m_dependsList << "install error" << num;
        emit signalDependResult(DebListModel::AuthDependsErr, m_index, m_brokenDepend);
    }
    emit signalEnableCloseButton(true);

    qInfo() << qPrintable("Process(apt) install finished, num: ") << num << qPrintable("Depends list:") << m_dependsList;
}
