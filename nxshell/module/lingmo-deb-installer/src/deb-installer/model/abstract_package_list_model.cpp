// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "abstract_package_list_model.h"

/**
   @class AbstractPackageListModel
   @brief Interface for the package data model and package installation control.
 */
AbstractPackageListModel::AbstractPackageListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

AbstractPackageListModel::WorkerStatus AbstractPackageListModel::getWorkerStatus() const
{
    return m_workerStatus;
}

/**
 * @brief Set the worker `status` and emit the corresponding signal.
 */
void AbstractPackageListModel::setWorkerStatus(WorkerStatus status)
{
    m_workerStatus = status;

    switch (status) {
        case WorkerProcessing:
            Q_FALLTHROUGH();
        case WorkerUnInstall:
            Q_EMIT signalWorkerStart();
            break;
        case WorkerFinished:
            Q_EMIT signalWorkerFinished();
            break;
        default:
            break;
    }
}
