/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2010 Jacopo De Simoi <wilderkde@gmail.com>
    SPDX-FileCopyrightText: 2014 Lukáš Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2016 Kai Uwe Broulik <kde@privat.broulik.de>
    SPDX-FileCopyrightText: 2019 David Hallas <david@davidhallas.dk>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "klistopenfilesjob.h"

#include <QDir>
#include <QList>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>

class KListOpenFilesJobPrivate
{
public:
    KListOpenFilesJobPrivate(KListOpenFilesJob *Job, const QDir &Path)
        : job(Job)
        , path(Path)
    {
        QObject::connect(&lsofProcess, &QProcess::errorOccurred, job, [this](QProcess::ProcessError error) {
            lsofError(error);
        });

        QObject::connect(&lsofProcess, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), job, [this](int exitCode, QProcess::ExitStatus exitStatus) {
            lsofFinished(exitCode, exitStatus);
        });
    }

    void start()
    {
        if (!path.exists()) {
            emitResult(static_cast<int>(KListOpenFilesJob::Error::DoesNotExist), QObject::tr("Path %1 doesn't exist").arg(path.path()));
            return;
        }

        const QString lsofExec = QStandardPaths::findExecutable(QStringLiteral("lsof"));
        if (lsofExec.isEmpty()) {
            const QString envPath = QString::fromLocal8Bit(qgetenv("PATH"));
            emitResult(static_cast<int>(KListOpenFilesJob::Error::InternalError), QObject::tr("Could not find lsof executable in PATH:").arg(envPath));
            return;
        }

        lsofProcess.start(lsofExec, {QStringLiteral("-t"), QStringLiteral("+d"), path.path()});
    }

    void lsofError(QProcess::ProcessError processError)
    {
        emitResult(static_cast<int>(KListOpenFilesJob::Error::InternalError), QObject::tr("Failed to execute `lsof'. Error code %1").arg(processError));
    }

    void lsofFinished(int, QProcess::ExitStatus);
    void emitResult(int error, const QString &errorText);

    KListOpenFilesJob *job;
    const QDir path;
    bool hasEmittedResult = false;
    QProcess lsofProcess;

    KProcessList::KProcessInfoList processInfoList;
};

static KProcessList::KProcessInfo findInfoForPid(qint64 pid)
{
#ifdef HAVE_PROCSTAT
    // If HAVE_PROCSTAT is defined, then we're on a BSD, and there is a KProcessList implementation
    // that efficiently lists all processes, but KProcessList::processInfo() is slow because
    // it recalculates the list-of-all-processes on each iteration.
    const auto allProcesses = KProcessList::processInfoList();
    auto it = std::find_if(allProcesses.cbegin(), allProcesses.cend(), [pid](const KProcessList::KProcessInfo &info) {
        return info.pid() == pid;
    });
    return it != allProcesses.cend() ? *it : KProcessList::KProcessInfo{};
#else
    // Presumably Linux: processInfo(pid) is fine because it goes
    // straight to /proc/<pid> for information.
    return KProcessList::processInfo(pid);
#endif
}

void KListOpenFilesJobPrivate::lsofFinished(int, QProcess::ExitStatus)
{
    if (hasEmittedResult) {
        return;
    }
    const QString out(QString::fromLocal8Bit(lsofProcess.readAll()));

    const QRegularExpression re(QStringLiteral("\\s+"));
    const QList<QStringView> pidList = QStringView(out).split(re, Qt::SkipEmptyParts);

    for (const auto &pidStr : pidList) {
        const qint64 pid = pidStr.toLongLong();
        if (pid) {
            processInfoList << findInfoForPid(pid);
        }
    }
    job->emitResult();
}

void KListOpenFilesJobPrivate::emitResult(int error, const QString &errorText)
{
    if (hasEmittedResult) {
        return;
    }
    job->setError(error);
    job->setErrorText(errorText);
    job->emitResult();
    hasEmittedResult = true;
}

KListOpenFilesJob::KListOpenFilesJob(const QString &path)
    : d(new KListOpenFilesJobPrivate(this, path))
{
}

KListOpenFilesJob::~KListOpenFilesJob() = default;

void KListOpenFilesJob::start()
{
    d->start();
}

KProcessList::KProcessInfoList KListOpenFilesJob::processInfoList() const
{
    return d->processInfoList;
}

#include "moc_klistopenfilesjob.cpp"
