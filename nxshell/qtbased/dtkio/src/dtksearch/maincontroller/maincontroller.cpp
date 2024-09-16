// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "maincontroller.h"

#include <QDebug>

DSEARCH_BEGIN_NAMESPACE

MainController::MainController(QObject *parent)
    : QObject(parent)
{
}

MainController::~MainController()
{
    stop();
}

bool MainController::doSearchTask(const QVariantMap &options)
{
    if (task)
        stop();

    task = new TaskCommander(options, this);

    connect(task, &TaskCommander::matched, this, &MainController::matched, Qt::DirectConnection);
    connect(task, &TaskCommander::finished, this, &MainController::completed, Qt::DirectConnection);

    if (task->start())
        return true;

    qWarning() << "fail to start task " << task;
    task->deleteSelf();
    return false;
}

void MainController::stop()
{
    if (task) {
        disconnect(task);
        task->stop();
        task->deleteSelf();
        task = nullptr;
    }
}

QStringList MainController::getResults()
{
    if (task)
        return task->getResults();

    return {};
}

void MainController::onFinished()
{
    stop();
    emit completed();
}

DSEARCH_END_NAMESPACE
