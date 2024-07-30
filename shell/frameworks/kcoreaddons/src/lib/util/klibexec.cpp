/*
    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
    SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>
*/

#include "klibexec.h"
#include <config-util.h>

#if HAVE_DLADDR
#include <dlfcn.h>
#elif defined(Q_OS_WIN)
#include <windows.h>

#include <QVarLengthArray>
#endif

#include <QCoreApplication>
#include <QDir>
#include <QLibraryInfo>

#include <kcoreaddons_debug.h>

static QString libraryPathFromAddress(void *address)
{
#if HAVE_DLADDR
    Dl_info info{};
    if (dladdr(address, &info) == 0) {
        qCWarning(KCOREADDONS_DEBUG) << "Failed to match address to shared object.";
        // Do not call dlerror. It's only expected to return something useful on freebsd!
        return {};
    }
    return QFile::decodeName(info.dli_fname);
#elif defined(Q_OS_WIN)
    HMODULE hModule = nullptr;
    if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, static_cast<LPWSTR>(address), &hModule)) {
        qCWarning(KCOREADDONS_DEBUG) << "Failed to GetModuleHandleExW" << GetLastError();
        return {};
    }
    if (!hModule) {
        qCWarning(KCOREADDONS_DEBUG) << "hModule null unexpectedly";
        return {};
    }

    QVarLengthArray<wchar_t, MAX_PATH> pathArray;
    DWORD pathSize = pathArray.size();
    while (pathSize == pathArray.size()) { // pathSize doesn't include the null byte on success, so this only ever true if we need to grow
        pathArray.resize(pathArray.size() + MAX_PATH);
        pathSize = GetModuleFileNameW(hModule, pathArray.data(), pathArray.size());
        if (pathSize == 0) {
            qCWarning(KCOREADDONS_DEBUG) << "Failed to GetModuleFileNameW" << GetLastError();
            return {};
        }
    }
    return QDir::fromNativeSeparators(QString::fromWCharArray(pathArray.data()));
#else // unsupported
    return {};
#endif
}

QString KLibexec::pathFromAddress(const QString &relativePath, void *address)
{
    const QString libraryPath = libraryPathFromAddress(address);
    const QString absoluteDirPath = QFileInfo(libraryPath).absolutePath();
    const QString libexecPath = QFileInfo(absoluteDirPath + QLatin1Char('/') + relativePath).absoluteFilePath();
    return libexecPath;
}

QStringList KLibexec::pathCandidates(const QString &relativePath)
{
    const QString qLibexec = QLibraryInfo::path(QLibraryInfo::LibraryExecutablesPath);
    const QString qLibexecKF6 = qLibexec + QLatin1String("/kf6");

    return {
        QCoreApplication::applicationDirPath(), // look where our application binary is located
        qLibexec, // look where libexec path is (can be set in qt.conf)
        qLibexecKF6, // on !win32 we use a kf6 suffix
        relativePath,
    };
}
