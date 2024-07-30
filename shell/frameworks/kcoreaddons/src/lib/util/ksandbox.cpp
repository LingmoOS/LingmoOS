// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>

#include "ksandbox.h"

#include <QDebug>
#include <QFileInfo>

#include <kcoreaddons_debug.h>

bool KSandbox::isInside()
{
    static const bool isInside = isFlatpak() || isSnap();
    return isInside;
}

bool KSandbox::isFlatpak()
{
    static const bool isFlatpak = QFileInfo::exists(QStringLiteral("/.flatpak-info"));
    return isFlatpak;
}

bool KSandbox::isSnap()
{
    static const bool isSnap = qEnvironmentVariableIsSet("SNAP");
    return isSnap;
}

bool checkHasFlatpakSpawnPrivileges()
{
    QFile f(QStringLiteral("/.flatpak-info"));
    if (!f.open(QIODevice::ReadOnly)) {
        return false;
    }

    return f.readAll().contains("\norg.freedesktop.Flatpak=talk\n");
}

KSandbox::ProcessContext KSandbox::makeHostContext(const QProcess &process)
{
    if (!KSandbox::isFlatpak()) {
        return {process.program(), process.arguments()};
    }

    static const bool hasFlatpakSpawnPrivileges = checkHasFlatpakSpawnPrivileges();
    if (!hasFlatpakSpawnPrivileges) {
        qCWarning(KCOREADDONS_DEBUG) << "Process execution expects 'org.freedesktop.Flatpak=talk'" << process.program();
        return {process.program(), process.arguments()};
    }

    QStringList args{QStringLiteral("--watch-bus"), QStringLiteral("--host"), QStringLiteral("--forward-fd=1"), QStringLiteral("--forward-fd=2")};
    if (!process.workingDirectory().isEmpty()) {
        args << QStringLiteral("--directory=%1").arg(process.workingDirectory());
    }
    const auto systemEnvironment = QProcessEnvironment::systemEnvironment().toStringList();
    const auto processEnvironment = process.processEnvironment().toStringList();
    for (const auto &variable : processEnvironment) {
        if (systemEnvironment.contains(variable)) {
            continue;
        }
        args << QStringLiteral("--env=%1").arg(variable);
    }
    if (!process.program().isEmpty()) { // some callers are cheeky and pass no program but put it into the arguments (e.g. konsole)
        args << process.program();
    }
    args += process.arguments();
    return {QStringLiteral("/usr/bin/flatpak-spawn"), args};
}

KCOREADDONS_EXPORT void KSandbox::startHostProcess(QProcess &process, QProcess::OpenMode mode)
{
    const auto context = makeHostContext(process);
    process.start(context.program, context.arguments, mode);
}
