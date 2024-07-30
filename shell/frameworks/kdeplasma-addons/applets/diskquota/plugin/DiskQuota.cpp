/*
 * SPDX-FileCopyrightText: 2015 Dominik Haumann <dhaumann@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include "DiskQuota.h"
#include "QuotaItem.h"

#include <KFormat>
#include <KLocalizedString>

#include <QRegularExpression>
#include <QStandardPaths>
#include <QTimer>
// #include <QDebug>

DiskQuota::DiskQuota(QObject *parent)
    : QObject(parent)
    , m_timer(new QTimer(this))
    , m_quotaProcess(new QProcess(this))
    , m_model(new QuotaListModel(this))
{
    connect(m_timer, &QTimer::timeout, this, &DiskQuota::updateQuota);
    m_timer->start(2 * 60 * 1000); // check every 2 minutes

    connect(m_quotaProcess, (void (QProcess::*)(int, QProcess::ExitStatus)) & QProcess::finished, this, &DiskQuota::quotaProcessFinished);

    updateQuota();
}

bool DiskQuota::quotaInstalled() const
{
    return m_quotaInstalled;
}

void DiskQuota::setQuotaInstalled(bool installed)
{
    if (m_quotaInstalled != installed) {
        m_quotaInstalled = installed;

        if (!installed) {
            m_model->clear();
            setStatus(PassiveStatus);
            setToolTip(i18n("Disk Quota"));
            setSubToolTip(i18n("Please install 'quota'"));
        }

        Q_EMIT quotaInstalledChanged();
    }
}

bool DiskQuota::cleanUpToolInstalled() const
{
    return m_cleanUpToolInstalled;
}

void DiskQuota::setCleanUpToolInstalled(bool installed)
{
    if (m_cleanUpToolInstalled != installed) {
        m_cleanUpToolInstalled = installed;
        Q_EMIT cleanUpToolInstalledChanged();
    }
}

DiskQuota::TrayStatus DiskQuota::status() const
{
    return m_status;
}

void DiskQuota::setStatus(TrayStatus status)
{
    if (m_status != status) {
        m_status = status;
        Q_EMIT statusChanged();
    }
}

QString DiskQuota::iconName() const
{
    return m_iconName;
}

void DiskQuota::setIconName(const QString &name)
{
    if (m_iconName != name) {
        m_iconName = name;
        Q_EMIT iconNameChanged();
    }
}

QString DiskQuota::toolTip() const
{
    return m_toolTip;
}

void DiskQuota::setToolTip(const QString &toolTip)
{
    if (m_toolTip != toolTip) {
        m_toolTip = toolTip;
        Q_EMIT toolTipChanged();
    }
}

QString DiskQuota::subToolTip() const
{
    return m_subToolTip;
}

void DiskQuota::setSubToolTip(const QString &subToolTip)
{
    if (m_subToolTip != subToolTip) {
        m_subToolTip = subToolTip;
        Q_EMIT subToolTipChanged();
    }
}

static QString iconNameForQuota(int quota)
{
    if (quota < 50) {
        return QStringLiteral("disk-quota");
    } else if (quota < 75) {
        return QStringLiteral("disk-quota-low");
    } else if (quota < 90) {
        return QStringLiteral("disk-quota-high");
    }

    // quota >= 90%
    return QStringLiteral("disk-quota-critical");
}

void DiskQuota::updateQuota()
{
    const bool quotaFound = !QStandardPaths::findExecutable(QStringLiteral("quota")).isEmpty();
    setQuotaInstalled(quotaFound);
    if (!quotaFound) {
        return;
    }

    // for now, only filelight is supported
    setCleanUpToolInstalled(!QStandardPaths::findExecutable(QStringLiteral("filelight")).isEmpty());

    // kill running process in case it hanged for whatever reason
    if (m_quotaProcess->state() != QProcess::NotRunning) {
        m_quotaProcess->kill();
    }

    // Try to run 'quota'
    const QStringList args{
        QStringLiteral("--show-mntpoint"), // second entry is e.g. '/home'
        QStringLiteral("--hide-device"), // hide e.g. /dev/sda3
        QStringLiteral("--no-mixed-pathnames"), // trim leading slashes from NFSv4 mountpoints
        QStringLiteral("--all-nfs"), // show all mount points
        QStringLiteral("--no-wrap"), // do not wrap long lines
        QStringLiteral("--quiet-refuse"), // no not print error message when NFS server does not respond
    };

    m_quotaProcess->start(QStringLiteral("quota"), args, QIODevice::ReadOnly);
}

void DiskQuota::quotaProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode)

    if (exitStatus != QProcess::NormalExit) {
        m_model->clear();
        setToolTip(i18n("Disk Quota"));
        setSubToolTip(i18n("Running quota failed"));
        return;
    }

    // get quota output
    const QString rawData = QString::fromLocal8Bit(m_quotaProcess->readAllStandardOutput());
    //     qDebug() << rawData;

    const QStringList lines = rawData.split(QRegularExpression(QStringLiteral("[\r\n]")), Qt::SkipEmptyParts);
    // Testing
    //     QStringList lines = QStringList()
    //         << QStringLiteral("/home/peterpan 3975379*  5000000 7000000           57602 0       0")
    //         << QStringLiteral("/home/archive 2263536  6000000 5100000            3932 0       0")
    //         << QStringLiteral("/home/shared 4271196*  10000000 7000000           57602 0       0");
    //         << QStringLiteral("/home/peterpan %1*  5000000 7000000           57602 0       0").arg(qrand() % 5000000)
    //         << QStringLiteral("/home/archive %1  5000000 5100000            3932 0       0").arg(qrand() % 5000000)
    //         << QStringLiteral("/home/shared %1*  5000000 7000000           57602 0       0").arg(qrand() % 5000000);
    //     lines.removeAt(qrand() % lines.size());

    // format class needed for GiB/MiB/KiB formatting
    KFormat fmt;
    int maxQuota = 0;
    QList<QuotaItem> items;

    // assumption: Filesystem starts with slash
    for (const QString &line : lines) {
        if (!line.contains(QLatin1Char('/'))) {
            continue;
        }

        QStringList parts = line.split(QLatin1Char(' '), Qt::SkipEmptyParts);
        // valid lines range from 7 to 9 parts (grace not always there):
        // Disk quotas for user dh (uid 1000):
        //      Filesystem   blocks  quota    limit     grace   files    quota   limit   grace
        //       /home     16296500  50000000 60000000          389155       0       0
        //       /home     16296500* 50000000 60000000      6   389155       0       0
        //       /home     16296500* 50000000 60000000      4   389155       0       0       5
        //       ^...........we want these...........^
        // NOTE: In case of a soft limit violation, a '*' is added in the used blocks.
        //       Hence, the star is removed below, if applicable

        if (parts.size() < 4) {
            continue;
        }

        // 'quota' uses kilo bytes -> factor 1024
        // NOTE: int is not large enough, hence qint64
        const qint64 used = parts[1].remove(QLatin1Char('*')).toLongLong() * 1024;
        qint64 softLimit = parts[2].toLongLong() * 1024;
        const qint64 hardLimit = parts[3].toLongLong() * 1024;
        if (softLimit == 0) { // softLimit might be unused (0)
            softLimit = hardLimit;
        }
        const qint64 freeSize = softLimit - used;
        const int percent = qMin(100, qMax(0, qRound(used * 100.0 / softLimit)));

        QuotaItem item;
        item.setIconName(iconNameForQuota(percent));
        item.setMountPoint(parts[0]);
        item.setUsage(percent);
        item.setMountString(i18nc("usage of quota, e.g.: '/home/bla: 38% used'", "%1: %2% used", parts[0], percent));
        item.setUsedString(i18nc("e.g.: 12 GiB of 20 GiB", "%1 of %2", fmt.formatByteSize(used), fmt.formatByteSize(softLimit)));
        item.setFreeString(i18nc("e.g.: 8 GiB free", "%1 free", fmt.formatByteSize(qMax(qint64(0), freeSize))));

        items.append(item);

        maxQuota = qMax(maxQuota, percent);
    }

    //     qDebug() << "QUOTAS:" << quotas;

    // make sure max quota is 100. Could be more, due to the
    // hard limit > soft limit, and we take soft limit as 100%
    maxQuota = qMin(100, maxQuota);

    // update icon in panel
    setIconName(iconNameForQuota(maxQuota));

    // update status
    setStatus(maxQuota < 50 ? PassiveStatus : maxQuota < 98 ? ActiveStatus : NeedsAttentionStatus);

    if (!items.isEmpty()) {
        setToolTip(i18nc("example: Quota: 83% used", "Quota: %1% used", maxQuota));
        setSubToolTip(QString());
    } else {
        setToolTip(i18n("Disk Quota"));
        setSubToolTip(i18n("No quota restrictions found."));
    }

    // merge new items, add new ones, remove old ones
    m_model->updateItems(items);
}

QuotaListModel *DiskQuota::model() const
{
    return m_model;
}

void DiskQuota::openCleanUpTool(const QString &mountPoint)
{
    if (!cleanUpToolInstalled()) {
        return;
    }

    QProcess::startDetached(QStringLiteral("filelight"), {mountPoint});
}
