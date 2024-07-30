/*
    SPDX-FileCopyrightText: 2009 George Kiagiadakis <gkiagia@users.sourceforge.net>
    SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef DRKONQI_H
#define DRKONQI_H

#include <QString>

class QWidget;

class SystemInformation;
class DebuggerManager;
class CrashedApplication;
class AbstractDrKonqiBackend;

class DrKonqi
{
public:
    static bool init();

    static SystemInformation *systemInformation();
    static DebuggerManager *debuggerManager();
    static CrashedApplication *crashedApplication();

    static void saveReport(const QString &reportText, QWidget *parent = nullptr);
    static void setSignal(int signal);
    static void setAppName(const QString &appName);
    static void setAppPath(const QString &appPath);
    static void setAppVersion(const QString &appVersion);
    static void setBugAddress(const QString &bugAddress);
    static void setProgramName(const QString &programName);
    static void setProductName(const QString &productName);
    static void setPid(int pid);
    static void setKdeinit(bool kdeinit);
    static void setSafer(bool safer);
    static void setRestarted(bool restarted);
    static void setKeepRunning(bool keepRunning);
    static void setThread(int thread);
    static void setStartupId(const QString &startupId);

    static int signal();
    static const QString &appName();
    static const QString &appPath();
    static const QString &appVersion();
    static const QString &bugAddress();
    static const QString &programName();
    static const QString &productName();
    static int pid();
    static bool isKdeinit();
    static bool isSafer();
    static bool isRestarted();
    static bool isKeepRunning();
    static int thread();
    static bool ignoreQuality();
    // Whether bugstest.kde.org is used.
    static bool isTestingBugzilla();
    static const QString &kdeBugzillaURL();
    static const QString &startupId();
    static QString backendClassName();

    // An ephemeral crash is one that cannot be restarted at a later point.
    // e.g. KCrashBackend is ephemeral, CoredumpBackend is not.
    static bool isEphemeralCrash();

    // Clean before quitting. This is not meant to ever get called if the quitting isn't the direct result of
    // an intentional quit. The primary effect of this function is that the backend will clean up persistent
    // backing data, such as coredumpd metadata files. We only want this to happen when we are certain
    // that the user has seen the crash but chosen to ignore it (e.g. closed the dialog window)
    static void cleanupBeforeQuit();

    // Whether to use the minimalistic UI or not. Defaults to false.
    static bool minimalMode();

    static DrKonqi *instance();
    QString m_glRenderer;
    QString m_exceptionName;
    QString m_exceptionWhat;

private:
    DrKonqi();
    ~DrKonqi();

    SystemInformation *m_systemInformation = nullptr;
    AbstractDrKonqiBackend *m_backend = nullptr;

    int m_signal;
    QString m_appName;
    QString m_appPath;
    QString m_appVersion;
    QString m_bugAddress;
    QString m_programName;
    QString m_productName;
    int m_pid;
    bool m_kdeinit;
    bool m_safer;
    bool m_restarted;
    bool m_keepRunning;
    int m_thread;
    QString m_startupId;
};

#endif
