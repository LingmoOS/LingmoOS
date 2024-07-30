/*
    SPDX-FileCopyrightText: 2010-2018 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "JobModel.h"

#include "kcupslib_log.h"

#include <KCupsJob.h>
#include <KCupsPrinter.h>
#include <KCupsRequest.h>

#include <QDateTime>
#include <QIODevice>
#include <QMimeData>
#include <QPointer>

#include <KFormat>
#include <KLocalizedString>
#include <KMessageBox>
#include <KUser>

JobModel::JobModel(QObject *parent)
    : QStandardItemModel(parent)
{
    setHorizontalHeaderItem(ColStatus, new QStandardItem(i18n("Status")));
    setHorizontalHeaderItem(ColName, new QStandardItem(i18n("Name")));
    setHorizontalHeaderItem(ColUser, new QStandardItem(i18n("User")));
    setHorizontalHeaderItem(ColCreated, new QStandardItem(i18n("Created")));
    setHorizontalHeaderItem(ColCompleted, new QStandardItem(i18n("Completed")));
    setHorizontalHeaderItem(ColPages, new QStandardItem(i18n("Pages")));
    setHorizontalHeaderItem(ColProcessed, new QStandardItem(i18n("Processed")));
    setHorizontalHeaderItem(ColSize, new QStandardItem(i18n("Size")));
    setHorizontalHeaderItem(ColStatusMessage, new QStandardItem(i18n("Status Message")));
    setHorizontalHeaderItem(ColPrinter, new QStandardItem(i18n("Printer")));
    setHorizontalHeaderItem(ColFromHost, new QStandardItem(i18n("From Hostname")));

    m_roles = QStandardItemModel::roleNames();
    m_roles[RoleJobId] = "jobId";
    m_roles[RoleJobState] = "jobState";
    m_roles[RoleJobName] = "jobName";
    m_roles[RoleJobPages] = "jobPages";
    m_roles[RoleJobSize] = "jobSize";
    m_roles[RoleJobOwner] = "jobOwner";
    m_roles[RoleJobCreatedAt] = "jobCreatedAt";
    m_roles[RoleJobIconName] = "jobIconName";
    m_roles[RoleJobCancelEnabled] = "jobCancelEnabled";
    m_roles[RoleJobHoldEnabled] = "jobHoldEnabled";
    m_roles[RoleJobReleaseEnabled] = "jobReleaseEnabled";
    m_roles[RoleJobRestartEnabled] = "jobRestartEnabled";
    m_roles[RoleJobPrinter] = "jobPrinter";
    m_roles[RoleJobOriginatingHostName] = "jobFrom";

    // This is emitted when a job change it's state
    connect(KCupsConnection::global(), &KCupsConnection::jobState, this, &JobModel::insertUpdateJob);

    // This is emitted when a job is created
    connect(KCupsConnection::global(), &KCupsConnection::jobCreated, this, &JobModel::insertUpdateJob);

    // This is emitted when a job is stopped
    connect(KCupsConnection::global(), &KCupsConnection::jobStopped, this, &JobModel::insertUpdateJob);

    // This is emitted when a job has it's config changed
    connect(KCupsConnection::global(), &KCupsConnection::jobConfigChanged, this, &JobModel::insertUpdateJob);

    // This is emitted when a job change it's progress
    connect(KCupsConnection::global(), &KCupsConnection::jobProgress, this, &JobModel::insertUpdateJob);

    // This is emitted when a printer is removed
    connect(KCupsConnection::global(), &KCupsConnection::jobCompleted, this, &JobModel::jobCompleted);

    connect(KCupsConnection::global(), &KCupsConnection::serverAudit, this, &JobModel::getJobs);
    connect(KCupsConnection::global(), &KCupsConnection::serverStarted, this, &JobModel::getJobs);
    connect(KCupsConnection::global(), &KCupsConnection::serverStopped, this, &JobModel::getJobs);
    connect(KCupsConnection::global(), &KCupsConnection::serverRestarted, this, &JobModel::getJobs);
}

void JobModel::setParentWId(WId parentId)
{
    m_parentId = parentId;
}

void JobModel::init(const QString &destName)
{
    m_destName = destName;

    // Get all jobs
    getJobs();
}

void JobModel::hold(const QString &printerName, int jobId)
{
    QPointer<KCupsRequest> request = new KCupsRequest;
    request->holdJob(printerName, jobId);
    request->waitTillFinished();
    if (request) {
        request->deleteLater();
    }
}

void JobModel::release(const QString &printerName, int jobId)
{
    QPointer<KCupsRequest> request = new KCupsRequest;
    request->releaseJob(printerName, jobId);
    request->waitTillFinished();
    if (request) {
        request->deleteLater();
    }
}

void JobModel::cancel(const QString &printerName, int jobId)
{
    QPointer<KCupsRequest> request = new KCupsRequest;
    request->cancelJob(printerName, jobId);
    request->waitTillFinished();
    if (request) {
        request->deleteLater();
    }
}

void JobModel::move(const QString &printerName, int jobId, const QString &toPrinterName)
{
    QPointer<KCupsRequest> request = new KCupsRequest;
    request->moveJob(printerName, jobId, toPrinterName);
    request->waitTillFinished();
    if (request) {
        request->deleteLater();
    }
}

void JobModel::getJobs()
{
    if (m_jobRequest) {
        return;
    }

    m_jobRequest = new KCupsRequest;
    connect(m_jobRequest, &KCupsRequest::finished, this, &JobModel::getJobFinished);

    const static QStringList attrs({KCUPS_JOB_ID,
                                    KCUPS_JOB_NAME,
                                    KCUPS_JOB_K_OCTETS,
                                    KCUPS_JOB_K_OCTETS_PROCESSED,
                                    KCUPS_JOB_STATE,
                                    KCUPS_JOB_STATE_REASONS,
                                    KCUPS_JOB_HOLD_UNTIL,
                                    KCUPS_TIME_AT_COMPLETED,
                                    KCUPS_TIME_AT_CREATION,
                                    KCUPS_TIME_AT_PROCESSING,
                                    KCUPS_JOB_PRINTER_URI,
                                    KCUPS_JOB_ORIGINATING_USER_NAME,
                                    KCUPS_JOB_ORIGINATING_HOST_NAME,
                                    KCUPS_JOB_MEDIA_PROGRESS,
                                    KCUPS_JOB_MEDIA_SHEETS,
                                    KCUPS_JOB_MEDIA_SHEETS_COMPLETED,
                                    KCUPS_JOB_PRINTER_STATE_MESSAGE,
                                    KCUPS_JOB_PRESERVED});
    m_jobRequest->getJobs(m_destName, false, m_whichjobs, attrs);

    m_processingJob.clear();
}

static KCupsJobs sanitizeJobs(KCupsJobs jobs)
{
    // For some reason sometimes cups has broken job queues with jobs with duplicated id
    // our model doesn't like that at all so sanitize the job list before processing it
    QVector<int> seenIds;
    int i = 0;
    while (i < jobs.count()) {
        const int jobId = jobs.at(i).id();
        if (seenIds.contains(jobId)) {
            qCWarning(LIBKCUPS) << "Found job with duplicated id" << jobId;
            jobs.removeAt(i);
        } else {
            seenIds << jobId;
            ++i;
        }
    }
    return jobs;
}

void JobModel::getJobFinished(KCupsRequest *request)
{
    if (request) {
        if (request->hasError()) {
            // clear the model after so that the proper widget can be shown
            clear();
        } else {
            const KCupsJobs jobs = sanitizeJobs(request->jobs());
            qCDebug(LIBKCUPS) << jobs.size();
            for (int i = 0; i < jobs.size(); ++i) {
                const KCupsJob job = jobs.at(i);
                if (job.state() == IPP_JOB_PROCESSING) {
                    m_processingJob = job.name();
                }

                // try to find the job row
                const int job_row = jobRow(job.id());
                if (job_row == -1) {
                    // not found, insert new one
                    insertJob(i, job);
                } else {
                    // update the job
                    updateJob(job_row, job);

                    if (job_row != i) {
                        // found at wrong position
                        // take it and insert on the right position
                        const QList<QStandardItem *> row = takeRow(job_row);
                        insertRow(i, row);
                    }
                }
            }

            // remove old jobs
            // The above code starts from 0 and make sure
            // dest == modelIndex(x) and if it's not the
            // case it either inserts or moves it.
            // so any item > num_jobs can be safely deleted
            while (rowCount() > jobs.size()) {
                removeRow(rowCount() - 1);
            }
        }
        request->deleteLater();
    } else {
        qCWarning(LIBKCUPS) << "Should not be called from a non KCupsRequest class" << sender();
    }
    m_jobRequest = nullptr;
}

void JobModel::jobCompleted(const QString &text,
                            const QString &printerUri,
                            const QString &printerName,
                            uint printerState,
                            const QString &printerStateReasons,
                            bool printerIsAcceptingJobs,
                            uint jobId,
                            uint jobState,
                            const QString &jobStateReasons,
                            const QString &jobName,
                            uint jobImpressionsCompleted)
{
    // REALLY? all these parameters just to say foo was deleted??
    Q_UNUSED(text)
    Q_UNUSED(printerUri)
    Q_UNUSED(printerName)
    Q_UNUSED(printerState)
    Q_UNUSED(printerStateReasons)
    Q_UNUSED(printerIsAcceptingJobs)
    Q_UNUSED(jobId)
    Q_UNUSED(jobState)
    Q_UNUSED(jobStateReasons)
    Q_UNUSED(jobName)
    Q_UNUSED(jobImpressionsCompleted)

    // We grab all jobs again
    getJobs();
}

void JobModel::insertUpdateJob(const QString &text,
                               const QString &printerUri,
                               const QString &printerName,
                               uint printerState,
                               const QString &printerStateReasons,
                               bool printerIsAcceptingJobs,
                               uint jobId,
                               uint jobState,
                               const QString &jobStateReasons,
                               const QString &jobName,
                               uint jobImpressionsCompleted)
{
    // REALLY? all these parameters just to say foo was created??
    Q_UNUSED(text)
    Q_UNUSED(printerUri)
    Q_UNUSED(printerName)
    Q_UNUSED(printerState)
    Q_UNUSED(printerStateReasons)
    Q_UNUSED(printerIsAcceptingJobs)
    Q_UNUSED(jobId)
    Q_UNUSED(jobState)
    Q_UNUSED(jobStateReasons)
    Q_UNUSED(jobName)
    Q_UNUSED(jobImpressionsCompleted)

    // We grab all jobs again
    getJobs();
}

void JobModel::insertJob(int pos, const KCupsJob &job)
{
    // insert the first column which has the job state and id
    QList<QStandardItem *> row;
    ipp_jstate_e jobState = job.state();
    auto statusItem = new QStandardItem(jobStatus(jobState));
    statusItem->setData(jobState, RoleJobState);
    statusItem->setData(job.id(), RoleJobId);
    statusItem->setData(job.name(), RoleJobName);
    statusItem->setData(job.originatingUserName(), RoleJobOwner);
    statusItem->setData(job.originatingHostName(), RoleJobOriginatingHostName);
    QString size = KFormat().formatByteSize(job.size());
    statusItem->setData(size, RoleJobSize);
    QString createdAt = QLocale().toString(job.createdAt());
    statusItem->setData(createdAt, RoleJobCreatedAt);

    // TODO move the update code before the insert and reuse some code...
    statusItem->setData(KCupsJob::iconName(jobState), RoleJobIconName);
    statusItem->setData(KCupsJob::cancelEnabled(jobState), RoleJobCancelEnabled);
    statusItem->setData(KCupsJob::holdEnabled(jobState), RoleJobHoldEnabled);
    statusItem->setData(KCupsJob::releaseEnabled(jobState), RoleJobReleaseEnabled);
    statusItem->setData(job.reprintEnabled(), RoleJobRestartEnabled);

    QString pages = QString::number(job.pages());
    if (job.processedPages()) {
        pages = QString::number(job.processedPages()) + QLatin1Char('/') + QString::number(job.processedPages());
    }
    if (statusItem->data(RoleJobPages) != pages) {
        statusItem->setData(pages, RoleJobPages);
    }

    row << statusItem;
    for (int i = ColName; i < LastColumn; i++) {
        // adds all Items to the model
        row << new QStandardItem;
    }

    // insert the whole row
    insertRow(pos, row);

    // update the items
    updateJob(pos, job);
}

void JobModel::updateJob(int pos, const KCupsJob &job)
{
    // Job Status & internal dataipp_jstate_e
    ipp_jstate_e jobState = job.state();
    QStandardItem *colStatus = item(pos, ColStatus);
    if (colStatus->data(RoleJobState).toInt() != jobState) {
        colStatus->setText(jobStatus(jobState));
        colStatus->setData(static_cast<int>(jobState), RoleJobState);

        colStatus->setData(KCupsJob::iconName(jobState), RoleJobIconName);
        colStatus->setData(KCupsJob::cancelEnabled(jobState), RoleJobCancelEnabled);
        colStatus->setData(KCupsJob::holdEnabled(jobState), RoleJobHoldEnabled);
        colStatus->setData(KCupsJob::releaseEnabled(jobState), RoleJobReleaseEnabled);
        colStatus->setData(job.reprintEnabled(), RoleJobRestartEnabled);
    }

    const QString pages =
        job.processedPages() ? QString::number(job.processedPages()) + QLatin1Char('/') + QString::number(job.processedPages()) : QString::number(job.pages());
    if (colStatus->data(RoleJobPages) != pages) {
        colStatus->setData(pages, RoleJobPages);
    }

    colStatus->setData(job.authenticationRequired(), RoleJobAuthenticationRequired);

    // internal dest name & column
    const QString destName = job.printer();
    if (colStatus->data(RoleJobPrinter).toString() != destName) {
        colStatus->setData(destName, RoleJobPrinter);
        // Column job printer Name
        item(pos, ColPrinter)->setText(destName);
    }

    // job name
    const QString jobName = job.name();
    if (item(pos, ColName)->text() != jobName) {
        colStatus->setData(jobName, RoleJobName);
        item(pos, ColName)->setText(jobName);
    }

    // owner of the job
    // try to get the full user name
    QString userString = job.originatingUserName();
    const KUser user(userString);
    if (user.isValid() && !user.property(KUser::FullName).toString().isEmpty()) {
        userString = user.property(KUser::FullName).toString();
    }

    // user name
    QStandardItem *colUser = item(pos, ColUser);
    if (colUser->text() != userString) {
        colUser->setText(userString);
    }

    // when it was created
    const QDateTime timeAtCreation = job.createdAt();
    QStandardItem *colCreated = item(pos, ColCreated);
    if (colCreated->data(Qt::DisplayRole).toDateTime() != timeAtCreation) {
        colCreated->setData(timeAtCreation, Qt::DisplayRole);
    }

    // when it was completed
    const QDateTime completedAt = job.completedAt();
    QStandardItem *colCompleted = item(pos, ColCompleted);
    if (colCompleted->data(Qt::DisplayRole).toDateTime() != completedAt) {
        if (!completedAt.isNull()) {
            colCompleted->setData(completedAt, Qt::DisplayRole);
        } else {
            // Clean the data might happen when the job is restarted
            colCompleted->setText(QString());
        }
    }

    // job pages
    const int completedPages = job.processedPages();
    QStandardItem *colPages = item(pos, ColPages);
    if (colPages->data(Qt::UserRole) != completedPages) {
        colPages->setData(completedPages, Qt::UserRole);
        colPages->setText(QString::number(completedPages));
    }

    // when it was processed
    const QDateTime timeAtProcessing = job.processedAt();
    QStandardItem *colProcessed = item(pos, ColProcessed);
    if (colProcessed->data(Qt::DisplayRole).toDateTime() != timeAtProcessing) {
        if (!timeAtProcessing.isNull()) {
            colProcessed->setData(timeAtProcessing, Qt::DisplayRole);
        } else {
            // Clean the data might happen when the job is restarted
            colCompleted->setText(QString());
        }
    }

    int jobSize = job.size();
    QStandardItem *colSize = item(pos, ColSize);
    if (colSize->data(Qt::UserRole) != jobSize) {
        colSize->setData(jobSize, Qt::UserRole);
        colSize->setText(KFormat().formatByteSize(jobSize));
    }

    // job printer state message
    const QString stateMessage = job.stateMsg();
    QStandardItem *colStatusMessage = item(pos, ColStatusMessage);
    if (colStatusMessage->text() != stateMessage) {
        colStatusMessage->setText(stateMessage);
    }

    // owner of the job
    // try to get the full user name
    const QString originatingHostName = job.originatingHostName();
    QStandardItem *colFromHost = item(pos, ColFromHost);
    if (colFromHost->text() != originatingHostName) {
        colFromHost->setText(originatingHostName);
    }
}

QStringList JobModel::mimeTypes() const
{
    return {QStringLiteral("application/x-cupsjobs")};
}

Qt::DropActions JobModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

QMimeData *JobModel::mimeData(const QModelIndexList &indexes) const
{
    auto mimeData = new QMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    for (const QModelIndex &index : indexes) {
        if (index.isValid() && index.column() == 0) {
            // serialize the jobId and fromDestName
            stream << data(index, RoleJobId).toInt() << data(index, RoleJobPrinter).toString() << item(index.row(), ColName)->text();
        }
    }

    mimeData->setData(QLatin1String("application/x-cupsjobs"), encodedData);
    return mimeData;
}

bool JobModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    Q_UNUSED(row)
    Q_UNUSED(column)
    Q_UNUSED(parent)
    if (action == Qt::IgnoreAction) {
        return true;
    }

    if (!data->hasFormat(QLatin1String("application/x-cupsjobs"))) {
        return false;
    }

    QByteArray encodedData = data->data(QLatin1String("application/x-cupsjobs"));
    QDataStream stream(&encodedData, QIODevice::ReadOnly);

    bool ret = false;
    while (!stream.atEnd()) {
        QString fromDestName, displayName;
        int jobId;
        // get the jobid and the from dest name
        stream >> jobId >> fromDestName >> displayName;
        if (fromDestName == m_destName) {
            continue;
        }

        QPointer<KCupsRequest> request = new KCupsRequest;
        request->moveJob(fromDestName, jobId, m_destName);
        request->waitTillFinished();
        if (request) {
            if (request->hasError()) {
                // failed to move one job
                // we return here to avoid more password tries
                KMessageBox::detailedErrorWId(m_parentId, i18n("Failed to move '%1' to '%2'", displayName, m_destName), request->errorMsg(), i18n("Failed"));
            }
            request->deleteLater();
            ret = !request->hasError();
        }
    }
    return ret;
}

QHash<int, QByteArray> JobModel::roleNames() const
{
    return m_roles;
}

KCupsRequest *JobModel::modifyJob(int row, JobAction action, const QString &newDestName, const QModelIndex &parent)
{
    Q_UNUSED(parent)

    if (row < 0 || row >= rowCount()) {
        qCWarning(LIBKCUPS) << "Row number is invalid:" << row;
        return nullptr;
    }

    QStandardItem *job = item(row, ColStatus);
    int jobId = job->data(RoleJobId).toInt();
    QString destName = job->data(RoleJobPrinter).toString();

    // ignore some jobs
    ipp_jstate_t state = static_cast<ipp_jstate_t>(job->data(RoleJobState).toInt());
    if ((state == IPP_JOB_HELD && action == Hold) || (state == IPP_JOB_CANCELED && action == Cancel) || (state != IPP_JOB_HELD && action == Release)) {
        return nullptr;
    }

    auto request = new KCupsRequest;
    switch (action) {
    case Cancel:
        request->cancelJob(destName, jobId);
        break;
    case Hold:
        request->holdJob(destName, jobId);
        break;
    case Release:
        request->releaseJob(destName, jobId);
        break;
    case Reprint:
        request->restartJob(destName, jobId);
        break;
    case Move:
        request->moveJob(destName, jobId, newDestName);
        break;
    default:
        qCWarning(LIBKCUPS) << "Unknown ACTION called!!!" << action;
        return nullptr;
    }

    return request;
}

int JobModel::jobRow(int jobId)
{
    // find the position of the jobId inside the model
    for (int i = 0; i < rowCount(); i++) {
        if (jobId == item(i)->data(RoleJobId).toInt()) {
            return i;
        }
    }
    // -1 if not found
    return -1;
}

QString JobModel::jobStatus(ipp_jstate_e job_state)
{
    switch (job_state) {
    case IPP_JOB_PENDING:
        return i18n("Pending");
    case IPP_JOB_HELD:
        return i18n("On hold");
    case IPP_JOB_PROCESSING:
        return QLatin1String("-");
    case IPP_JOB_STOPPED:
        return i18n("Stopped");
    case IPP_JOB_CANCELED:
        return i18n("Canceled");
    case IPP_JOB_ABORTED:
        return i18n("Aborted");
    case IPP_JOB_COMPLETED:
        return i18n("Completed");
    }
    return QLatin1String("-");
}

void JobModel::clear()
{
    removeRows(0, rowCount());
}

void JobModel::setWhichJobs(WhichJobs whichjobs)
{
    switch (whichjobs) {
    case WhichActive:
        m_whichjobs = CUPS_WHICHJOBS_ACTIVE;
        break;
    case WhichCompleted:
        m_whichjobs = CUPS_WHICHJOBS_COMPLETED;
        break;
    case WhichAll:
        m_whichjobs = CUPS_WHICHJOBS_ALL;
        break;
    }

    getJobs();
}

Qt::ItemFlags JobModel::flags(const QModelIndex &index) const
{
    if (index.isValid()) {
        ipp_jstate_t state = static_cast<ipp_jstate_t>(item(index.row(), ColStatus)->data(RoleJobState).toInt());
        if (state == IPP_JOB_PENDING || state == IPP_JOB_PROCESSING) {
            return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
        }
    }
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;
}

QString JobModel::processingJob() const
{
    return m_processingJob;
}

#include "moc_JobModel.cpp"
