/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2006 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KJOBTEST_H
#define KJOBTEST_H

#include "kjob.h"
#include "kjobuidelegate.h"
#include <QEventLoop>
#include <QMap>
#include <QObject>

class TestJob : public KJob
{
    Q_OBJECT
public:
    TestJob();
    ~TestJob() override;

    void start() override;
    using KJob::isFinished;
    using KJob::setProgressUnit;

protected:
    bool doKill() override;

public:
    void setError(int errorCode);
    void setErrorText(const QString &errorText);
    void setProcessedSize(qulonglong size);
    void setTotalSize(qulonglong size);
    void setProcessedFiles(qulonglong files);
    void setTotalFiles(qulonglong files);
    void setPercent(unsigned long percentage);
};

class TestJobUiDelegate : public KJobUiDelegate
{
    Q_OBJECT
protected:
    virtual void connectJob(KJob *job);
};

class WaitJob;

class KJobTest : public QObject
{
    Q_OBJECT
public:
    enum class Action {
        Start,
        KillQuietly,
        KillVerbosely,
    };
    Q_ENUM(Action)

    KJobTest();

public Q_SLOTS:

    // These slots need to be public, otherwise qtestlib calls them as part of the test
    void slotStartInnerJob();
    void slotFinishOuterJob();
    void slotFinishInnerJob();

private Q_SLOTS:
    void testEmitResult_data();
    void testEmitResult();
    void testProgressTracking();
    void testExec_data();
    void testExec();
    void testKill_data();
    void testKill();
    void testDestroy();
    void testEmitAtMostOnce_data();
    void testEmitAtMostOnce();
    void testDelegateUsage();
    void testNestedExec();

    void slotResult(KJob *job);
    void slotFinished(KJob *job);

private:
    TestJob *setupErrorResultFinished();

    QEventLoop loop;
    int m_lastError;
    QString m_lastErrorText;
    int m_resultCount;
    int m_finishedCount;

    WaitJob *m_outerJob;
    WaitJob *m_innerJob;
    QMap<KJob *, int> m_jobFinishCount;
};

class WaitJob : public KJob
{
    Q_OBJECT
public:
    void start() override;
    void makeItFinish();
};

#endif
