/*
    This file is part of the KDE Frameworks

    SPDX-FileCopyrightText: 2011 Nokia Corporation and/or its subsidiary(-ies).
    SPDX-FileCopyrightText: 2019 David Hallas <david@davidhallas.dk>

    SPDX-License-Identifier: LGPL-2.1-only WITH Qt-LGPL-exception-1.1 OR LicenseRef-Qt-Commercial
*/

#include "kprocesslist.h"

#include <QLibrary>
#include <algorithm>

// Enable Win API of XP SP1 and later
#ifdef Q_OS_WIN
#if !defined(_WIN32_WINNT)
#define _WIN32_WINNT 0x0502
#endif
#include <qt_windows.h>
#if !defined(PROCESS_SUSPEND_RESUME) // Check flag for MinGW
#define PROCESS_SUSPEND_RESUME (0x0800)
#endif // PROCESS_SUSPEND_RESUME
#endif // Q_OS_WIN

#include <psapi.h>
#include <tlhelp32.h>

using namespace KProcessList;

// Resolve QueryFullProcessImageNameW out of kernel32.dll due
// to incomplete MinGW import libs and it not being present
// on Windows XP.
static inline BOOL queryFullProcessImageName(HANDLE h, DWORD flags, LPWSTR buffer, DWORD *size)
{
    // Resolve required symbols from the kernel32.dll
    typedef BOOL(WINAPI * QueryFullProcessImageNameWProtoType)(HANDLE, DWORD, LPWSTR, PDWORD);
    static QueryFullProcessImageNameWProtoType queryFullProcessImageNameW = nullptr;
    if (!queryFullProcessImageNameW) {
        QLibrary kernel32Lib(QLatin1String("kernel32.dll"), 0);
        if (kernel32Lib.isLoaded() || kernel32Lib.load()) {
            queryFullProcessImageNameW = (QueryFullProcessImageNameWProtoType)kernel32Lib.resolve("QueryFullProcessImageNameW");
        }
    }
    if (!queryFullProcessImageNameW)
        return FALSE;
    // Read out process
    return (*queryFullProcessImageNameW)(h, flags, buffer, size);
}

struct ProcessInfo {
    QString processOwner;
    QString fullPath;
};

static inline ProcessInfo winProcessInfo(DWORD processId)
{
    ProcessInfo pi;
    HANDLE handle = OpenProcess(PROCESS_QUERY_INFORMATION, TOKEN_READ, processId);
    if (handle == INVALID_HANDLE_VALUE)
        return pi;
    HANDLE processTokenHandle = nullptr;
    if (!OpenProcessToken(handle, TOKEN_READ, &processTokenHandle) || !processTokenHandle)
        return pi;

    TCHAR fullProcessPath[MAX_PATH] = {0};
    DWORD fullProcessPathLength = MAX_PATH;
    if (queryFullProcessImageName(handle, 0, fullProcessPath, &fullProcessPathLength)) {
        pi.fullPath = QString::fromUtf16(reinterpret_cast<const char16_t *>(fullProcessPath));
    }

    DWORD size = 0;
    GetTokenInformation(processTokenHandle, TokenUser, nullptr, 0, &size);

    if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
        QByteArray buf;
        buf.resize(size);
        PTOKEN_USER userToken = reinterpret_cast<PTOKEN_USER>(buf.data());
        if (userToken && GetTokenInformation(processTokenHandle, TokenUser, userToken, size, &size)) {
            SID_NAME_USE sidNameUse;
            TCHAR user[MAX_PATH] = {0};
            DWORD userNameLength = MAX_PATH;
            TCHAR domain[MAX_PATH] = {0};
            DWORD domainNameLength = MAX_PATH;

            if (LookupAccountSid(nullptr, userToken->User.Sid, user, &userNameLength, domain, &domainNameLength,
                &sidNameUse))
                pi.processOwner = QString::fromUtf16(reinterpret_cast<const char16_t *>(user));
        }
    }

    CloseHandle(processTokenHandle);
    CloseHandle(handle);
    return pi;
}

KProcessInfoList KProcessList::processInfoList()
{
    KProcessInfoList rc;

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE)
        return rc;

    for (bool hasNext = Process32First(snapshot, &pe); hasNext; hasNext = Process32Next(snapshot, &pe)) {
        const ProcessInfo processInf = winProcessInfo(pe.th32ProcessID);
        const QString commandName = QString::fromUtf16(reinterpret_cast<char16_t *>(pe.szExeFile));
        if (processInf.fullPath.isEmpty()) {
            rc.push_back(KProcessInfo(pe.th32ProcessID, commandName, processInf.processOwner));
        } else {
            rc.push_back(KProcessInfo(pe.th32ProcessID, processInf.fullPath, commandName, processInf.processOwner));
        }
    }
    CloseHandle(snapshot);
    return rc;
}

KProcessInfo KProcessList::processInfo(qint64 pid)
{
    KProcessInfoList processInfoList = KProcessList::processInfoList();
    auto testProcessIterator = std::find_if(processInfoList.begin(), processInfoList.end(), [pid](const KProcessList::KProcessInfo &info) {
        return info.pid() == pid;
    });
    if (testProcessIterator != processInfoList.end()) {
        return *testProcessIterator;
    }
    return KProcessInfo();
}
