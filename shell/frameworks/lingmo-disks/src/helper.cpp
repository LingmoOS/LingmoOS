// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

#include "helper.h"

#include <QDebug>
#include <QFileInfo>
#include <QProcess>

#include <chrono>

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// Append name to /dev/ and ensure it is a trustable block device.
static QString nameToPath(const QString &name)
{
    if (name.isEmpty()) {
        return {};
    }

    // This also excludes relative path shenanigans as they'd all need to contain a separator.
    if (name.contains(QLatin1Char('/'))) {
        qWarning() << "Device names must not contain slashes";
        return {};
    }

    const QString path = QStringLiteral("/dev/%1").arg(name);

    struct stat sb;
    QByteArray fileName(QFile::encodeName(path));
    if (lstat(fileName.constData(), &sb) == -1) {
        const int err = errno;
        qWarning() << "Failed to stat block device" << name << strerror(err);
        return {};
    }

#ifdef Q_OS_FREEBSD // There are only character devices
    if (!S_ISCHR(sb.st_mode)) {
#else // On others assume they are block devices (e.g. linux)
    if (!S_ISBLK(sb.st_mode)) {
#endif
        qWarning() << "Device is not actually a block device" << name;
        return {};
    }

    if (sb.st_uid != 0) {
        qWarning() << "Device is not owned by root" << name;
        return {};
    }

    return path;
}

ActionReply SMARTHelper::smartctl(const QVariantMap &args)
{
    // For security reasons we only accept fully resolved device names which
    // we use to construct the final /dev/$name path.
    const QString name = args.value(QStringLiteral("deviceName")).toString();
    const QString devicePath = nameToPath(name);
    if (devicePath.isEmpty()) {
        return ActionReply::HelperErrorReply();
    }

    // PATH is super minimal when invoked through dbus
    setenv("PATH", "/usr/sbin:/sbin:/usr/local/sbin", 1);

    const QString command = QStringLiteral("smartctl");
    const QString all = QStringLiteral("--all");

    // JSON output.
    QProcess jsonProcess;
    // json=c is badly documented and means "gimme json but don't pretty print"
    jsonProcess.start(command, {all, QStringLiteral("--json=c"), devicePath}, QProcess::ReadOnly);

    // Diagnostic CLI output for advanced users.
    QProcess cliProcess;
    cliProcess.start(command, {all, devicePath}, QProcess::ReadOnly);

    // Wait for 120 seconds + 5 seconds leeway.
    // This allows us to ideally let smartctl time out internally and still
    // construct a json blob if possible. The kernel ioctl timeout for nvme
    // apparently is 60 seconds and internal smartctl timeouts also largely
    // appear to be in that range but there may be more than one :|
    // https://bugs.kde.org/show_bug.cgi?id=428844
    using namespace std::chrono_literals;
    if (!jsonProcess.waitForFinished(std::chrono::milliseconds(125s).count())) {
        qDebug() << "jsonProcess has not finished in time";
    }
    // CLI was also running. Give it a bit of extra time still in case there was resource contention.
    if (!cliProcess.waitForFinished(std::chrono::milliseconds(10s).count())) {
        qDebug() << "cliProcess has not finished in time";
    }

    ActionReply reply;
    reply.addData(QStringLiteral("exitCode"), jsonProcess.exitCode());
    reply.addData(QStringLiteral("data"), jsonProcess.readAllStandardOutput());
    reply.addData(QStringLiteral("cliExitCode"), cliProcess.exitCode());
    reply.addData(QStringLiteral("cliData"), cliProcess.readAllStandardOutput());
    return reply;
}

KAUTH_HELPER_MAIN("org.kde.kded.smart", SMARTHelper)

#include "moc_helper.cpp"
