// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "jobmanager.h"
#include "common/constant.h"
#include "ipc/proto/backend.h"
#include "ipc/bridge.h"
#include "service/ipc/sendipcservice.h"
#include "service/rpc/sendrpcservice.h"

#include "utils/config.h"

JobManager *JobManager::instance()
{
    static JobManager manager;
    return &manager;
}

JobManager::~JobManager()
{
}

bool JobManager::handleRemoteRequestJob(QString json, QString *targetAppName)
{
    co::Json info;
    if (!info.parse_from(json.toStdString())) {
        return false;
    }
    FileTransJob fsjob;
    fsjob.from_json(info);
    int32 jobId = fsjob.job_id;
    fastring savedir = fsjob.save_path;
    fastring appName = fsjob.app_who;
    fastring tarAppname = fsjob.targetAppname;
    SendRpcService::instance()->removePing(appName.c_str());
    tarAppname = tarAppname.empty() ? appName : tarAppname;
    if (targetAppName)
        *targetAppName = QString(tarAppname.c_str());
    SendRpcService::instance()->removePing(tarAppname.c_str());
    QSharedPointer<TransferJob> job(new TransferJob());
    job->initJob(fsjob.app_who, tarAppname, jobId, fsjob.path, fsjob.sub, savedir, fsjob.write);
    if (!job->initRpc(fsjob.ip.empty() ? _connected_target : fsjob.ip, UNI_RPC_PORT_TRANS) || !job->initSuccess()) {
        ELOG << "init job rpc error !! json = " << fsjob.as_json();
        return false;
    }
    connect(job.data(), &TransferJob::notifyFileTransStatus, this, &JobManager::handleFileTransStatus, Qt::QueuedConnection);
    connect(job.data(), &TransferJob::notifyJobResult, this, &JobManager::handleJobTransStatus, Qt::QueuedConnection);
    connect(job.data(), &TransferJob::notifyJobFinished, this, &JobManager::handleRemoveJob, Qt::QueuedConnection);

    QSharedPointer<TransferJob> _job { nullptr };
    if (fsjob.write) {
        DLOG << "(" << jobId << ")write job save to: " << savedir;
        {
            QReadLocker lk(&g_m);
            _job = _transjob_recvs.value(jobId);
        }
        if (!_job.isNull() && !_job->ended()) {
            QWriteLocker lk(&g_m);
            _transjob_break.insert(jobId, _job);
            _job->stop(false);
        }

        QWriteLocker lk(&g_m);
        _transjob_recvs.remove(jobId);
        _transjob_recvs.insert(jobId, job);
    } else {
        DLOG << "(" << jobId << ")read job save to: " << savedir;
        {
            QReadLocker lk(&g_m);
            _job = _transjob_sends.value(jobId);
        }

        if (!_job.isNull() && !_job->ended()) {
            QWriteLocker lk(&g_m);
            _transjob_break.insert(jobId, _job);
            _job->stop(false);
        }

        QWriteLocker lk(&g_m);
        _transjob_sends.remove(jobId);
        _transjob_sends.insert(jobId, job);
    }

    QUNIGO([job]() {
        // DLOG << ".........start job: sched: " << co::sched_id() << " co: " << co::coroutine_id();
        job->start();
    });

    return true;
}

bool JobManager::doJobAction(const uint action, const int jobid)
{
    bool result = false;

    if (BACK_CANCEL_JOB == action) {
        QSharedPointer<TransferJob> rjob { nullptr };
        {
            QReadLocker lk(&g_m);
            rjob = _transjob_recvs.value(jobid);
        }
        if (!rjob.isNull()) {
            rjob->cancel(true);
            result = true;
        }

        QSharedPointer<TransferJob> sjob { nullptr };
        {
            QReadLocker lk(&g_m);
            sjob = _transjob_sends.value(jobid);
        }
        if (!sjob.isNull()) {
            sjob->cancel(true);
            result = true;
        }
    } else if (BACK_RESUME_JOB == action) {
    }
    return result;
}

bool JobManager::handleFSData(const co::Json &info, fastring buf, FileTransResponse *reply)
{
    QSharedPointer<FSDataBlock> datablock(new FSDataBlock);
    datablock->from_json(info);
    datablock->data = buf;
    int32 jobId = datablock->job_id;

    if (reply) {
        reply->id = datablock->file_id;
        reply->name = datablock->filename;
    }
    QSharedPointer<TransferJob> job { nullptr };
    {
        QReadLocker lk(&g_m);
        job = _transjob_recvs.value(jobId);
    }

    if (!job.isNull()) {
        job->pushQueque(datablock);
        if (datablock->flags & JobTransFileOp::FILE_COUNTED && job->freeBytes() < datablock->data_size) {
            return false;
        } else if (datablock->flags & JobTransFileOp::FIlE_DIR_CREATE || datablock->flags & JobTransFileOp::FIlE_CREATE) {
            co::sleep(10);
            job = _transjob_recvs.value(jobId);
            if (job.isNull() || job->ended()) {
                LOG << "create dir/file failed.";
                return false;
            }
        }
    } else {
        return false;
    }

    return true;
}

bool JobManager::handleCancelJob(co::Json &info, FileTransResponse *reply)
{
    FileTransJobAction obj;
    obj.from_json(info);
    int32 jobId = obj.job_id;
    bool result = false;
    if (reply) {
        reply->id = jobId;
        reply->name = obj.appname;
    }

    QSharedPointer<TransferJob> rjob { nullptr };
    {
        QReadLocker lk(&g_m);
        rjob = _transjob_recvs.value(jobId);
    }
    if (!rjob.isNull() && !rjob->ended()) {
        //disconnect(job, &TransferJob::notifyFileTransStatus, this, &ServiceManager::handleFileTransStatus);
        DLOG << "recv > remote canceled this job: " << jobId;
        rjob->cancel();
        QString name(rjob->getAppName().c_str());
        SendIpcService::instance()->handleRemoveJob(rjob->getAppName().c_str(), jobId);
        result = true;
    }

    QSharedPointer<TransferJob> sjob { nullptr };
    {
        QReadLocker lk(&g_m);
        sjob = _transjob_sends.value(jobId);
    }
    if (!sjob.isNull() && !sjob->ended()) {
        //disconnect(job, &TransferJob::notifyFileTransStatus, this, &ServiceManager::handleFileTransStatus);
        DLOG << "send > remote canceled this job: " << jobId;
        sjob->cancel();
        QString name(sjob->getAppName().c_str());
        SendIpcService::instance()->handleRemoveJob(sjob->getAppName().c_str(), jobId);
        result = true;
    }

    return result;
}

bool JobManager::handleTransReport(co::Json &info, FileTransResponse *reply)
{
    FSReport obj;
    obj.from_json(info);
    int32 jobId = obj.job_id;
    fastring path = obj.path;
    if (reply) {
        reply->id = jobId;
        reply->name = path;
    }

    switch (obj.result) {
    case IO_ERROR: {
        QSharedPointer<TransferJob> job { nullptr };
        {
            QReadLocker lk(&g_m);
            job = _transjob_sends.value(jobId);
        }
        if (!job.isNull()) {
            // move the job into breaks record map.
            _transjob_break.insert(jobId, _transjob_sends.take(jobId));
        }
        break;
    }
    case OK:
        break;
    case FINIASH: {
        QSharedPointer<TransferJob> job { nullptr };
        {
            QReadLocker lk(&g_m);
            job = _transjob_recvs.value(jobId);
        }
        if (!job.isNull()) {
            //disconnect(job, &TransferJob::notifyFileTransStatus, this, &ServiceManager::handleFileTransStatus);
            job->waitFinish();
            QSharedPointer<TransferJob> sjob { nullptr };
            {
                QWriteLocker lk(&g_m);
                _transjob_recvs.remove(jobId);
            }
            QString name(job->getAppName().c_str());
            SendIpcService::instance()->handleRemoveJob(job->getAppName().c_str(), jobId);
        } else {
            return false;
        }
        break;
    }
    default:
        DLOG << "unkown report: " << obj.result;
        break;
    }

    return true;
}

void JobManager::handleFileTransStatus(QString appname, int status, QString fileinfo)
{
    //DLOG << "notify file trans status to:" << appname.toStdString();
    co::Json infojson;
    infojson.parse_from(fileinfo.toStdString());
    FileInfo filejob;
    filejob.from_json(infojson);

    //notifyFileStatus {FileStatus}
    co::Json req = {
        { "job_id", filejob.job_id },
        { "file_id", filejob.file_id },
        { "name", filejob.name },
        { "status", status },
        { "total", filejob.total_size },
        { "current", filejob.current_size },
        { "millisec", filejob.time_spended },
    };

    QString jsonMsg = req.str().c_str();
    SendIpcService::instance()->handleSendToClient(appname, FRONT_NOTIFY_FILE_STATUS, jsonMsg);
}

void JobManager::handleJobTransStatus(QString appname, int jobid, int status, QString savedir)
{
    //DLOG << "notify file trans status to:" << appname.toStdString() << " jobid=" << jobid;
    //cbTransStatus {GenericResult}
    co::Json req = {
        { "id", jobid },
        { "result", status },
        { "msg", savedir.toStdString() },
        { "isself", false },
    };

    QString jsonMsg = req.str().c_str();
    SendIpcService::instance()->handleSendToClient(appname, FRONT_TRANS_STATUS_CB, jsonMsg);
}

void JobManager::handleRemoveJob(const int jobid)
{
    QWriteLocker lk(&g_m);
    _transjob_recvs.remove(jobid);
    _transjob_sends.remove(jobid);
    for (auto it = _transjob_break.begin(); it != _transjob_break.end();) {
        if (it.value()->ended()) {
            it = _transjob_break.erase(it);
        } else {
            it++;
        }
    }
}

void JobManager::handleOtherOffline(const QString &ip)
{
    QSharedPointer<TransferJob> job { nullptr };
    {
        QReadLocker lk(&g_m);
        if (_transjob_sends.isEmpty() && _transjob_recvs.isEmpty())
            return;
        job = _transjob_sends.isEmpty() ? _transjob_recvs.first() : _transjob_sends.first();
    }
    if (job.isNull())
        return;

    auto suc = job->offlineCancel(ip);
    if (!suc)
        return;
    QWriteLocker lk(&g_m);
    auto id = _transjob_sends.isEmpty() ? _transjob_recvs.key(job) : _transjob_sends.key(job);
    _transjob_sends.clear();
    _transjob_recvs.clear();
    _transjob_break.insert(id, job);
}

JobManager::JobManager(QObject *parent)
    : QObject(parent)
{
}
