/*
    This file is part of the KDE Frameworks

    SPDX-FileCopyrightText: 2011 Nokia Corporation and/or its subsidiary(-ies).
    SPDX-FileCopyrightText: 2019 David Hallas <david@davidhallas.dk>

    SPDX-License-Identifier: LGPL-2.1-only WITH Qt-LGPL-exception-1.1 OR LicenseRef-Qt-Commercial
*/

/*
 * Implementation notes:
 *
 * This file implements KProcessInfo and KProcessInfoList via Linux /proc
 * **or** via ps(1). If there's no /proc, it falls back to ps(1), usually.
 *
 * Although the code contains #ifdefs for FreeBSD (e.g. for ps(1) command-
 * line arguments), FreeBSD should never use this code, only the
 * procstat-based code in `kprocesslist_unix_procstat.cpp`.
 */

#include "kcoreaddons_debug.h"
#include "kprocesslist.h"

#include <QDebug>
#include <QDir>
#include <QProcess>

#ifdef Q_OS_FREEBSD
#error This KProcessInfo implementation is not supported on FreeBSD (use procstat)
#endif

using namespace KProcessList;

namespace
{
bool isUnixProcessId(const QString &procname)
{
    return std::none_of(procname.cbegin(), procname.cend(), [](const QChar ch) {
        return !ch.isDigit();
    });
}

// Determine UNIX processes by running ps
KProcessInfoList unixProcessListPS()
{
    KProcessInfoList rc;
    QProcess psProcess;
    const QStringList args{
#ifdef Q_OS_OPENBSD
        QStringLiteral("-ww"),
        QStringLiteral("-x"),
#endif
        QStringLiteral("-e"),
        QStringLiteral("-o"),
#ifdef Q_OS_MAC
        // command goes last, otherwise it is cut off
        QStringLiteral("pid state user comm command"),
#elif defined(Q_OS_OPENBSD)
        // On OpenBSD "login" is user who started the process in difference to
        // Linux where it is the effective user "ename" name.
        QStringLiteral("pid,state,login,comm,args"),
#else
        QStringLiteral("pid,state,user,comm,cmd"),
#endif
    };
    psProcess.start(QStringLiteral("ps"), args);
    if (!psProcess.waitForStarted()) {
        qCWarning(KCOREADDONS_DEBUG) << "Failed to execute ps" << args;
        return rc;
    }
    psProcess.waitForFinished();
    const QByteArray output = psProcess.readAllStandardOutput();
    const QByteArray errorOutput = psProcess.readAllStandardError();
    if (!errorOutput.isEmpty()) {
        qCWarning(KCOREADDONS_DEBUG) << "ps said" << errorOutput;
    }
    // Split "457 S+   /Users/foo.app"
    const QStringList lines = QString::fromLocal8Bit(output).split(QLatin1Char('\n'));
    const int lineCount = lines.size();
    const QChar blank = QLatin1Char(' ');
    for (int l = 1; l < lineCount; l++) { // Skip header
        const QString line = lines.at(l).simplified();
        // we can't just split on blank as the process name might
        // contain them
        const int endOfPid = line.indexOf(blank);
        const int endOfState = line.indexOf(blank, endOfPid + 1);
        const int endOfUser = line.indexOf(blank, endOfState + 1);
        const int endOfName = line.indexOf(blank, endOfUser + 1);

        if (endOfPid >= 0 && endOfState >= 0 && endOfUser >= 0) {
            const qint64 pid = QStringView(line).left(endOfPid).toUInt();

            QString user = line.mid(endOfState + 1, endOfUser - endOfState - 1);
            QString name = line.mid(endOfUser + 1, endOfName - endOfUser - 1);
            QString command = line.right(line.size() - endOfName - 1);
            rc.push_back(KProcessInfo(pid, command, name, user));
        }
    }

    return rc;
}

bool getProcessInfo(const QString &procId, KProcessInfo &processInfo)
{
    if (!isUnixProcessId(procId)) {
        return false;
    }
    QString statusFileName(QStringLiteral("/stat"));
    QString filename = QStringLiteral("/proc/");
    filename += procId;
    filename += statusFileName;
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        return false; // process may have exited
    }

    const QStringList data = QString::fromLocal8Bit(file.readAll()).split(QLatin1Char(' '));
    if (data.length() < 2) {
        return false;
    }
    qint64 pid = procId.toUInt();
    QString name = data.at(1);
    if (name.startsWith(QLatin1Char('(')) && name.endsWith(QLatin1Char(')'))) {
        name.chop(1);
        name.remove(0, 1);
    }
    // State is element 2
    // PPID is element 3
    QString user = QFileInfo(file).owner();
    file.close();

    QString command = name;

    QFile cmdFile(QLatin1String("/proc/") + procId + QLatin1String("/cmdline"));
    if (cmdFile.open(QFile::ReadOnly)) {
        QByteArray cmd = cmdFile.readAll();

        if (!cmd.isEmpty()) {
            // extract non-truncated name from cmdline
            int zeroIndex = cmd.indexOf('\0');
            int processNameStart = cmd.lastIndexOf('/', zeroIndex);
            if (processNameStart == -1) {
                processNameStart = 0;
            } else {
                processNameStart++;
            }
            name = QString::fromLocal8Bit(cmd.mid(processNameStart, zeroIndex - processNameStart));

            cmd.replace('\0', ' ');
            command = QString::fromLocal8Bit(cmd).trimmed();
        }
    }
    cmdFile.close();
    processInfo = KProcessInfo(pid, command, name, user);
    return true;
}

} // unnamed namespace

// Determine UNIX processes by reading "/proc". Default to ps if
// it does not exist
KProcessInfoList KProcessList::processInfoList()
{
    const QDir procDir(QStringLiteral("/proc/"));
    if (!procDir.exists()) {
        return unixProcessListPS();
    }
    const QStringList procIds = procDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    KProcessInfoList rc;
    rc.reserve(procIds.size());
    for (const QString &procId : procIds) {
        KProcessInfo processInfo;
        if (getProcessInfo(procId, processInfo)) {
            rc.push_back(processInfo);
        }
    }
    return rc;
}

// Determine UNIX process by reading "/proc".
//
// TODO: Use ps if "/proc" does not exist or is bogus; use code
//       from unixProcessListPS() but add a `-p pid` argument.
//
KProcessInfo KProcessList::processInfo(qint64 pid)
{
    KProcessInfo processInfo;
    getProcessInfo(QString::number(pid), processInfo);
    return processInfo;
}
