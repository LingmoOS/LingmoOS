// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>

#ifndef KSANDBOX_H
#define KSANDBOX_H

#include <QProcess>

#include <kcoreaddons_export.h>

/**
 * @brief Utility functions for use inside application sandboxes such as flatpak or snap.
 * @since 5.95
 */
namespace KSandbox
{

/// @returns whether the application is inside one of the supported sandboxes
KCOREADDONS_EXPORT bool isInside();

/// @returns whether the application is inside a flatpak sandbox
KCOREADDONS_EXPORT bool isFlatpak();

/// @returns whether the application is inside a snap sandbox
KCOREADDONS_EXPORT bool isSnap();

/**
 * @brief Container for host process startup context
 * @since 5.97
 */
struct ProcessContext {
    const QString program;
    const QStringList arguments;
};

/**
 * @returns the actual program and arguments for running the QProcess on the host (e.g. a flatpak-spawn-wrapped argument list)
 * @since 5.97
 */
KCOREADDONS_EXPORT KSandbox::ProcessContext makeHostContext(const QProcess &process);

/**
 * Starts the QProcess on the host (if the current context is inside a sandbox, otherwise it simply runs QProcess::start)
 * @since 5.97
 */
KCOREADDONS_EXPORT void startHostProcess(QProcess &process, QProcess::OpenMode mode = QProcess::ReadWrite);

} // namespace KSandbox

#endif // KSANDBOX_H
