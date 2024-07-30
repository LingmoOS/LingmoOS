/*
    SPDX-FileCopyrightText: 2016 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef WINUTILS_P_H
#define WINUTILS_P_H

#include "windevicemanager.h" // for qGetLastError

#include <qt_windows.h>

/**
 * @brief RAII class for blocking Windows from reporting errors
 *
 * Internally calls SetThreadErrorMode. This class stores the
 * current error mode on construction and disables certain error reportings
 * during the life time of this object
 *
 * @code
 * {
 *     WinErrorBlocker blocker; // custom error mode is set
 *     // Your Windows API calls...
 * } // end of scope: original error mode is restored
 * @endcode
 *
 * Note that qstorageinfo_win.cpp (qtbase) does something similar to silence errors
 *
 * See MSDN documentation: https://msdn.microsoft.com/en-us/library/windows/desktop/dd553630(v=vs.85).aspx
 */
class WinErrorBlocker
{
public:
    WinErrorBlocker()
    {
        // SetThreadErrorMode only available since Windows 7
        // see: https://msdn.microsoft.com/en-us/library/windows/desktop/ms680621(v=vs.85).aspx
#if _WIN32_WINNT < 0x0601
        m_oldmode = ::SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);
#else
        if (!::SetThreadErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX, &m_oldmode)) {
            qWarning() << "Failed to call SetThreadErrorMode:" << qGetLastError();
        }
#endif
    }
    ~WinErrorBlocker()
    {
#if _WIN32_WINNT < 0x0601
        ::SetErrorMode(m_oldmode);
#else
        if (!::SetThreadErrorMode(m_oldmode, NULL)) {
            qWarning() << "Failed to call SetThreadErrorMode:" << qGetLastError();
        }
#endif
    }

private:
    Q_DISABLE_COPY(WinErrorBlocker)
#if _WIN32_WINNT < 0x0601
    UINT m_oldmode;
#else
    DWORD m_oldmode;
#endif
};

#endif
