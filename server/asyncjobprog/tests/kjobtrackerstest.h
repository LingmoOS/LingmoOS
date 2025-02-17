/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2007 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KJOBTRACKERSTEST_H
#define KJOBTRACKERSTEST_H

#include <KJob>

#include <QTimer>

class KTestJob : public KJob
{
    Q_OBJECT

public:
    enum State {
        StatingDirs,
        CreatingDirs,
        CopyingFiles,
        Stopped,
    };

    // 10 files per directory
    // 1000 bytes per files
    KTestJob(int numberOfDirs = 5);
    ~KTestJob() override;

    void start() override;

private Q_SLOTS:
    void nextStep();

protected:
    void stateNextDir();
    void createNextDir();
    void copyNextFile();

    bool doSuspend() override;
    bool doResume() override;
    bool doKill() override;

private:
    qulonglong m_numberOfDirs;
    qulonglong m_currentSpeed;
    State m_state;
    QTimer m_timer;
};

#endif
