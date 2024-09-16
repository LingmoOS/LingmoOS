// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "zjobmanager.h"
#include "common.h"
#include "config.h"
#include "cupsconnection.h"
#include "qtconvert.h"
#include "cupsconnectionfactory.h"


#include <QMap>
#include <QVariant>
#include <QFile>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusReply>

static const char *g_whichs[] = {"all", "not-completed", "completed"};
static const char *jattrs[] = /* Attributes we need for jobs... */
{
    JOB_ATTR_ID,
    JOB_ATTR_SIZE,
    JOB_ATTR_NAME,
    JOB_ATTR_USER,
    JOB_ATTR_STATE,
    JOB_ATTR_STATE_MEG,
    JOB_ATTR_STATE_RES,
    JOB_ATTR_URI,
    JOB_ATTR_STATE_STR,
    JOB_ATTR_TIME_ADD,
    JOB_ATTR_TIME_END,
    JOB_ATTR_PRIORITY,
    JOB_ATTR_DOC_NUM,
    nullptr
};

int JobManager::getJobs(map<int, map<string, string>> &jobs, int which, int myJobs)
{
    vector<string> requst;

    qCDebug(COMMONMOUDLE) << which << myJobs;

    for (int i = 0; jattrs[i]; i++) {
        requst.push_back(jattrs[i]);
    }

    try {
        auto conPtr = CupsConnectionFactory::createConnectionBySettings();
        if (conPtr)
            jobs = conPtr->getJobs(g_whichs[which], myJobs, 0, 0, &requst);
    } catch (const std::exception &ex) {
        qCWarning(COMMONMOUDLE) << "Got execpt: " << QString::fromUtf8(ex.what());
        return -1;
    }

    qCDebug(COMMONMOUDLE) << "Got jobs count: " << jobs.size();
    map<int, map<string, string>>::iterator itJobs = jobs.begin();
    QDBusInterface interface(SERVICE_INTERFACE_NAME, SERVICE_INTERFACE_PATH, SERVICE_INTERFACE_NAME, QDBusConnection::sessionBus());

    while (itJobs != jobs.end()) {
        map<string, string> info = itJobs->second;
        qCDebug(COMMONMOUDLE) << JOB_ATTR_ID << itJobs->first;
        if (interface.isValid()) {
            QDBusReply<bool> result = interface.call("isJobPurged", itJobs->first);
            if (result.isValid() && result.value()) {
                qCInfo(COMMONMOUDLE) << itJobs->first << "is purged";
                itJobs = jobs.erase(itJobs);
            } else
                itJobs++;
        } else
            itJobs++;
        dumpStdMapValue(info);
    }
    return 0;
}

int JobManager::getJobById(map<string, string> &job, int jobId, int myJob)
{
    map<int, map<string, string>> jobs;
    map<int, map<string, string>>::iterator itJobs;
    vector<string> requst;

    for (int i = 0; jattrs[i]; i++) {
        requst.push_back(jattrs[i]);
    }

    try {
        auto conPtr = CupsConnectionFactory::createConnectionBySettings();
        if (conPtr)
            jobs = conPtr->getJobs(g_whichs[WHICH_JOB_ALL], myJob, 1, jobId, &requst);
    } catch (const std::exception &ex) {
        qCWarning(COMMONMOUDLE) << "Got execpt: " << QString::fromUtf8(ex.what());
        return -1;
    }

    for (itJobs = jobs.begin(); itJobs != jobs.end(); itJobs++) {
        dumpStdMapValue(itJobs->second);
        if (itJobs->first == jobId) {
            job = itJobs->second;
            return 0;
        }
    }

    qCDebug(COMMONMOUDLE) << "Not found " << jobId;
    return -2;
}

int JobManager::cancelJob(int job_id)
{
    qCInfo(COMMONMOUDLE) << "Job: " << job_id;

    try {
        auto conPtr = CupsConnectionFactory::createConnectionBySettings();
        if (conPtr)
            conPtr->cancelJob(job_id, 0);
    } catch (const std::exception &ex) {
        qCWarning(COMMONMOUDLE) << "Got execpt: " << QString::fromUtf8(ex.what());
        return -1;
    }

    return 0;
}

int JobManager::deleteJob(int job_id, const char *dest)
{
    qCInfo(COMMONMOUDLE) << "Job: " << job_id;

    try {
        auto conPtr = CupsConnectionFactory::createConnectionBySettings();
        if (!conPtr)
            return -1;
        if (dest) {
            conPtr->cancelAllJobs(dest, nullptr, 1, 1);
        } else {
            conPtr->cancelJob(job_id, 1);
        }
    } catch (const std::exception &ex) {
        qCWarning(COMMONMOUDLE) << "Got execpt: " << QString::fromUtf8(ex.what());
        return -1;
    }

    return 0;
}

int JobManager::holdJob(int job_id)
{
    qCInfo(COMMONMOUDLE) << "Job: " << job_id;

    try {
        auto conPtr = CupsConnectionFactory::createConnectionBySettings();
        if (conPtr)
            conPtr->holdJob(job_id);
    } catch (const std::exception &ex) {
        qCWarning(COMMONMOUDLE) << "Got execpt: " << QString::fromUtf8(ex.what());
        return -1;
    }

    return 0;
}

int JobManager::holdjobs(const QString &printerName)
{
    map<int, map<string, string>> jobs;
    map<int, map<string, string>>::iterator itJobs;

    if (0 != getJobs(jobs, WHICH_JOB_RUNING))
        return -1;

    for (itJobs = jobs.begin(); itJobs != jobs.end(); itJobs++) {
        map<string, string> job = itJobs->second;
        QString uri = attrValueToQString(job[JOB_ATTR_URI]);

        if (printerName == getPrinterNameFromUri(uri)) {
            holdJob(itJobs->first);
        }
    }

    return true;
}

int JobManager::releaseJob(int job_id)
{
    qCInfo(COMMONMOUDLE) << "Job: " << job_id;

    try {
        auto conPtr = CupsConnectionFactory::createConnectionBySettings();
        if (conPtr)
            conPtr->releaseJob(job_id);
    } catch (const std::exception &ex) {
        qCWarning(COMMONMOUDLE) << "Got execpt: " << QString::fromUtf8(ex.what());
        return -1;
    }

    return 0;
}

int JobManager::restartJob(int job_id)
{
    qCInfo(COMMONMOUDLE) << "Job: " << job_id;

    try {
        auto conPtr = CupsConnectionFactory::createConnectionBySettings();
        if (conPtr)
            conPtr->restartJob(job_id, nullptr);
    } catch (const std::exception &ex) {
        qCWarning(COMMONMOUDLE) << "Got execpt: " << QString::fromUtf8(ex.what());
        return -1;
    }

    return 0;
}

int JobManager::moveJob(const char *destUri, int job_id, const char *srcUri)
{
    qCInfo(COMMONMOUDLE) << "Job: " << job_id;

    try {
        auto conPtr = CupsConnectionFactory::createConnectionBySettings();
        if (conPtr)
            conPtr->moveJob(destUri, job_id, srcUri);
    } catch (const std::exception &ex) {
        qCWarning(COMMONMOUDLE) << "Got execpt: " << QString::fromUtf8(ex.what());
        return -1;
    }

    return 0;
}

static int setJobPriority(int job_id, int iPriority)
{
    qCInfo(COMMONMOUDLE) << "Job: " << job_id << iPriority;

    try {
        auto conPtr = CupsConnectionFactory::createConnectionBySettings();
        if (conPtr)
            conPtr->setJobPriority(job_id, iPriority);
    } catch (const std::exception &ex) {
        qCWarning(COMMONMOUDLE) << "Got execpt: " << QString::fromUtf8(ex.what());
        return -1;
    }

    return 0;
}

bool JobManager::isCompletedState(int state)
{
    return (IPP_JSTATE_COMPLETED == state || IPP_JSTATE_ABORTED == state || IPP_JSTATE_CANCELED == state);
}

int JobManager::priorityJob(int job_id, int &iPriority)
{
    map<int, map<string, string>> jobs;
    map<int, map<string, string>>::const_iterator itmaps;

    qCInfo(COMMONMOUDLE) << job_id << iPriority;

    if (0 != getJobs(jobs))
        return -1;

    if (LOWEST_Priority > iPriority) {
        iPriority = DEFAULT_Priority;
        for (itmaps = jobs.begin(); itmaps != jobs.end(); itmaps++) {
            map<string, string> jobinfo = itmaps->second;
            int jobPriority = attrValueToQString(jobinfo[JOB_ATTR_PRIORITY]).toInt();
            qCDebug(COMMONMOUDLE) << "Got " << itmaps->first << jobPriority;

            if (jobPriority >= iPriority) {
                iPriority = jobPriority;
                if (++iPriority > HIGHEST_Priority) {
                    iPriority = HIGHEST_Priority;
                    qCDebug(COMMONMOUDLE) << "Change " << itmaps->first << "to" << HIGHEST_Priority - 1;
                    setJobPriority(itmaps->first, HIGHEST_Priority - 1);
                }
            }
        }
    }

    qCDebug(COMMONMOUDLE) << "Set" << job_id << iPriority;
    return setJobPriority(job_id, iPriority);
}

QString JobManager::printTestPage(const char *dest, int &jobId, const char *format)
{
    const char *testFile = "/usr/share/cups/data/testprint";

    if (!QFile::exists(testFile)) {
        qCWarning(COMMONMOUDLE) << "No test file: " << testFile;
        return testFile + tr(" not found");
    }

    try {
        auto conPtr = CupsConnectionFactory::createConnectionBySettings();
        if (conPtr)
            jobId = conPtr->printTestPage(dest, testFile, PrintTestTitle, format, nullptr);
    } catch (const std::exception &ex) {
        qCWarning(COMMONMOUDLE) << "Got execpt: " << QString::fromUtf8(ex.what());
        return QString::fromUtf8(ex.what());
    }

    return QString();
}

JobManager *JobManager::getInstance()
{
    static JobManager *instance = nullptr;

    if (!instance)
        instance = new JobManager();

    return instance;
}
