// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "reportlogmanager.h"
#include "reportlogworker.h"

#include <QThread>
#include <QUrl>
#include <QDebug>

using namespace deepin_cross;

ReportLogManager *ReportLogManager::instance()
{
    static ReportLogManager ins;
    return &ins;
}

ReportLogManager::ReportLogManager(QObject *parent)
    : QObject (parent)
{
}

ReportLogManager::~ReportLogManager()
{
    if (reportWorkThread) {
        qInfo() << "Log thread start to quit";
        reportWorkThread->quit();
        reportWorkThread->wait(2000);
        qInfo() << "Log thread quited.";
    }
}

void ReportLogManager::init()
{
    reportWorker = new ReportLogWorker();
    if (!reportWorker->init()) {
        reportWorker->deleteLater();
        return;
    }

    reportWorkThread = new QThread();
    connect(reportWorkThread, &QThread::finished, [&]() {
        reportWorker->deleteLater();
    });
    reportWorker->moveToThread(reportWorkThread);

    initConnection();

    reportWorkThread->start();
}

void ReportLogManager::commit(const QString &type, const QVariantMap &args)
{
    Q_EMIT requestCommitLog(type, args);
}

void ReportLogManager::initConnection()
{
    connect(this, &ReportLogManager::requestCommitLog, reportWorker, &ReportLogWorker::commitLog, Qt::QueuedConnection);
}
