/*
    SPDX-FileCopyrightText: 2009 George Kiagiadakis <gkiagia@users.sourceforge.net>
    SPDX-FileCopyrightText: 2021-2022 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef CRASHEDAPPLICATION_H
#define CRASHEDAPPLICATION_H

#include <QDateTime>
#include <QFileInfo>
#include <QObject>

#include "bugreportaddress.h"

class KCrashBackend;

using EntriesHash = QHash<QByteArray, QByteArray>;

class CrashedApplication : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QFileInfo executable READ executable CONSTANT)
    Q_PROPERTY(QString exectuableAbsoluteFilePath READ exectuableAbsoluteFilePath CONSTANT)
    Q_PROPERTY(QString fakeExecutableBaseName READ fakeExecutableBaseName CONSTANT)
    Q_PROPERTY(QString version READ version CONSTANT)
    Q_PROPERTY(QString bugReportAddress READ bugReportAddress CONSTANT)
    Q_PROPERTY(QString productName READ productName CONSTANT)
    Q_PROPERTY(int pid READ pid CONSTANT)
    Q_PROPERTY(int signalNumber READ signalNumber CONSTANT)
    Q_PROPERTY(QString signalName READ signalName CONSTANT)
    Q_PROPERTY(bool hasBeenRestarted READ hasBeenRestarted NOTIFY restarted)
    Q_PROPERTY(QDateTime datetime READ datetime CONSTANT)
    Q_PROPERTY(bool hasDeletedFiles READ hasDeletedFiles CONSTANT)
    Q_PROPERTY(bool wasNotResponding READ wasNotResponding CONSTANT)
public:
    CrashedApplication(int pid,
                       int thread,
                       int signalNumber,
                       const QFileInfo &executable,
                       const QString &version,
                       const BugReportAddress &reportAddress,
                       const QString &name = QString(),
                       const QString &productName = QString(),
                       const QDateTime &datetime = QDateTime::currentDateTime(),
                       bool restarted = false,
                       bool hasDeletedFiles = false,
                       bool applicationNotResponding = false,
                       const QString &fakeBaseName = QString(),
                       QObject *parent = nullptr);

    ~CrashedApplication() override;

    /** Returns the crashed program's name, possibly translated (ex. "The KDE Crash Handler") */
    QString name() const;

    /** Returns a QFileInfo with information about the executable that crashed */
    QFileInfo executable() const;

    /** convenience wrapper to make access from QML easier */
    QString exectuableAbsoluteFilePath() const
    {
        return m_executable.absoluteFilePath();
    }

    /** When an application is run via kdeinit, the executable() method returns kdeinit4, but
     * we still need a way to know which is the application that was loaded by kdeinit. So,
     * this method returns the base name of the executable that would have been launched if
     * the app had not been loaded by kdeinit (ex. "lingmo-desktop"). If the application was
     * not launched via kdeinit, this method returns executable().baseName();
     */
    QString fakeExecutableBaseName() const;

    /** Returns the version of the crashed program */
    QString version() const;

    /** Returns the address where the bug report for this application should go */
    BugReportAddress bugReportAddress() const;

    /** Bugzilla product name, if the crashed application has explicitly specified that. */
    QString productName() const;

    /** Returns the pid of the crashed program */
    int pid() const;

    /** Returns the signal number that the crashed program received */
    int signalNumber() const;

    /** Returns the name of the signal (ex. SIGSEGV) */
    QString signalName() const;

    bool hasBeenRestarted() const;

    int thread() const;

    const QDateTime &datetime() const;

    /** @returns whether mmap'd files have been deleted, e.g. updated since start of app */
    bool hasDeletedFiles() const;

    [[nodiscard]] bool wasNotResponding() const;

public Q_SLOTS:
    void restart();

Q_SIGNALS:
    void restarted(bool success);

protected:
    int m_pid;
    int m_signalNumber;
    QString m_name;
    QFileInfo m_executable;
    QString m_fakeBaseName;
    QString m_version;
    BugReportAddress m_reportAddress;
    QString m_productName;
    bool m_restarted;
    int m_thread;
    QDateTime m_datetime;
    bool m_hasDeletedFiles;
    bool m_applicationNotResponding;

public:
    // Only set for the 'coredumpd' backend. Path to on-disk core dump.
    QString m_coreFile;
    // Also only set for coredumpd backend. A bunch of log entries from journal.
    QList<EntriesHash> m_logs;
};

QString getSuggestedKCrashFilename(const CrashedApplication *app);

#endif // CRASHEDAPPLICATION_H
