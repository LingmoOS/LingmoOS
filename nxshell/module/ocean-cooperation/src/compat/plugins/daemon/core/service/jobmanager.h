// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef JOBMANAGER_H
#define JOBMANAGER_H

#include <QObject>
#include <QMap>
#include <QSharedPointer>
#include <QReadWriteLock>

#include <service/job/transferjob.h>
#include "common/commonstruct.h"
#include "co/co.h"

class JobManager : public QObject
{
    Q_OBJECT
public:
    static JobManager *instance();
    ~JobManager();

public slots:
    bool handleRemoteRequestJob(QString json, QString *targetAppName);
    bool doJobAction(const uint action, const int jobid);
    bool handleFSData(const co::Json &info, fastring buf, FileTransResponse *reply);
    bool handleCancelJob(co::Json &info, FileTransResponse *reply);
    bool handleTransReport(co::Json &info, FileTransResponse *reply);

    void handleFileTransStatus(QString appname, int status, QString fileinfo);
    void handleJobTransStatus(QString appname, int jobid, int status, QString savedir);
    void handleRemoveJob(const int jobid);
    void handleOtherOffline(const QString &ip);
private:
    explicit JobManager(QObject *parent = nullptr);

private:
    QMap<int, QSharedPointer<TransferJob>> _transjob_sends;
    QMap<int, QSharedPointer<TransferJob>> _transjob_recvs;
    // record the send jobs which have error happend. that can be resumed by user.
    QMap<int, QSharedPointer<TransferJob>> _transjob_break;
    fastring _connected_target;
    QReadWriteLock g_m;
};

#endif // JOBMANAGER_H
