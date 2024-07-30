/*
    SPDX-FileCopyrightText: 2010-2018 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "KCupsJob.h"
#include <QDateTime>

KCupsJob::KCupsJob()
    : m_jobId(0)
{
}

KCupsJob::KCupsJob(int jobId, const QString &printer)
    : m_jobId(jobId)
    , m_printer(printer)
{
    m_arguments[KCUPS_JOB_ID] = QString::number(jobId);
}

KCupsJob::KCupsJob(const QVariantMap &arguments)
    : m_arguments(arguments)
{
    m_jobId = arguments[KCUPS_JOB_ID].toInt();
    m_printer = arguments[KCUPS_JOB_PRINTER_URI].toString().section(QLatin1Char('/'), -1);
}

int KCupsJob::id() const
{
    return m_jobId;
}

QString KCupsJob::idStr() const
{
    return m_arguments[KCUPS_JOB_ID].toString();
}

QString KCupsJob::name() const
{
    return m_arguments[KCUPS_JOB_NAME].toString();
}

QString KCupsJob::originatingUserName() const
{
    return m_arguments[KCUPS_JOB_ORIGINATING_USER_NAME].toString();
}

QString KCupsJob::originatingHostName() const
{
    return m_arguments[KCUPS_JOB_ORIGINATING_HOST_NAME].toString();
}

QString KCupsJob::printer() const
{
    return m_printer;
}

QDateTime KCupsJob::createdAt() const
{
    QDateTime ret;
    const auto it = m_arguments.constFind(KCUPS_TIME_AT_CREATION);
    if (it != m_arguments.constEnd()) {
        ret.fromSecsSinceEpoch(it.value().toInt());
    }
    return ret;
}

QDateTime KCupsJob::completedAt() const
{
    QDateTime ret;
    const auto it = m_arguments.constFind(KCUPS_TIME_AT_COMPLETED);
    if (it != m_arguments.constEnd()) {
        ret.fromSecsSinceEpoch(it.value().toInt());
    }
    return ret;
}

QDateTime KCupsJob::processedAt() const
{
    QDateTime ret;
    const auto it = m_arguments.constFind(KCUPS_TIME_AT_PROCESSING);
    if (it != m_arguments.constEnd()) {
        ret.fromSecsSinceEpoch(it.value().toInt());
    }
    return ret;
}

int KCupsJob::pages() const
{
    return m_arguments[KCUPS_JOB_MEDIA_SHEETS].toInt();
}

int KCupsJob::processedPages() const
{
    return m_arguments[KCUPS_JOB_MEDIA_SHEETS_COMPLETED].toInt();
}

int KCupsJob::size() const
{
    int jobKOctets = m_arguments[KCUPS_JOB_K_OCTETS].toInt();
    jobKOctets *= 1024; // transform it to bytes
    return jobKOctets;
}

bool KCupsJob::preserved() const
{
    return m_arguments[KCUPS_JOB_PRESERVED].toInt();
}

QString KCupsJob::iconName(ipp_jstate_t state)
{
    QString ret;
    switch (state) {
    case IPP_JOB_PENDING:
        ret = QLatin1String("chronometer");
        break;
    case IPP_JOB_HELD:
        ret = QLatin1String("media-playback-pause");
        break;
    case IPP_JOB_PROCESSING:
        ret = QLatin1String("draw-arrow-forward");
        break;
    case IPP_JOB_STOPPED:
        ret = QLatin1String("draw-rectangle");
        break;
    case IPP_JOB_CANCELED:
        ret = QLatin1String("archive-remove");
        break;
    case IPP_JOB_ABORTED:
        ret = QLatin1String("task-attempt");
        break;
    case IPP_JOB_COMPLETED:
        ret = QLatin1String("task-complete");
        break;
    default:
        ret = QLatin1String("unknown");
    }
    return ret;
}

ipp_jstate_t KCupsJob::state() const
{
    return static_cast<ipp_jstate_t>(m_arguments[KCUPS_JOB_STATE].toUInt());
}

QString KCupsJob::stateMsg() const
{
    return m_arguments[KCUPS_JOB_PRINTER_STATE_MESSAGE].toString();
}

QString KCupsJob::stateReason() const
{
    return m_arguments[KCUPS_JOB_STATE_REASONS].toString();
}

QString KCupsJob::holdUntil() const
{
    return m_arguments[KCUPS_JOB_HOLD_UNTIL].toString();
}

bool KCupsJob::authenticationRequired() const
{
    return stateReason() == QStringLiteral("cups-held-for-authentication") || holdUntil() == QStringLiteral("auth-info-required");
}

bool KCupsJob::reprintEnabled() const
{
    if (state() >= IPP_JOB_STOPPED && preserved()) {
        return true;
    }
    return false;
}

bool KCupsJob::cancelEnabled(ipp_jstate_t state)
{
    switch (state) {
    case IPP_JOB_CANCELED:
    case IPP_JOB_COMPLETED:
    case IPP_JOB_ABORTED:
        return false;
    default:
        return true;
    }
}

bool KCupsJob::holdEnabled(ipp_jstate_t state)
{
    switch (state) {
    case IPP_JOB_CANCELED:
    case IPP_JOB_COMPLETED:
    case IPP_JOB_ABORTED:
    case IPP_JOB_HELD:
    case IPP_JOB_STOPPED:
        return false;
    default:
        return true;
    }
}

bool KCupsJob::releaseEnabled(ipp_jstate_t state)
{
    switch (state) {
    case IPP_JOB_HELD:
    case IPP_JOB_STOPPED:
        return true;
    default:
        return false;
    }
}
