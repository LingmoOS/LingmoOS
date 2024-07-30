/* -*- C++ -*-
    Helper class for unit tests.

    SPDX-FileCopyrightText: 2005-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef APPENDCHARACTER_JOB
#define APPENDCHARACTER_JOB

#include <QMutex>
#include <QObject>

#include <ThreadWeaver/DebuggingAids>
#include <ThreadWeaver/Job>
#include <ThreadWeaver/JobPointer>

// define in test binary:

extern QMutex s_GlobalMutex;

class AppendCharacterJob : public ThreadWeaver::Job
{
public:
    AppendCharacterJob(QChar c = QChar(), QString *stringref = nullptr)
        : ThreadWeaver::Job()
    {
        setValues(c, stringref);
    }

    void setValues(QChar c, QString *stringref)
    {
        m_c = c;
        m_stringref = stringref;
    }

    void run(ThreadWeaver::JobPointer, ThreadWeaver::Thread *) override
    {
        QMutexLocker locker(&s_GlobalMutex);
        m_stringref->append(m_c);
        using namespace ThreadWeaver;
        TWDEBUG(3, "AppendCharacterJob::run: %c appended, result is %s.\n", m_c.toLatin1(), qPrintable(*m_stringref));
    }

    QChar character() const
    {
        return m_c;
    }

protected:
    QString *stringRef() const
    {
        return m_stringref;
    }

private:
    QChar m_c;
    QString *m_stringref;
};

class FailingAppendCharacterJob : public AppendCharacterJob
{
public:
    FailingAppendCharacterJob(QChar c = QChar(), QString *stringref = nullptr)
        : AppendCharacterJob(c, stringref)
    {
    }

    void run(ThreadWeaver::JobPointer job, ThreadWeaver::Thread *thread) override
    {
        AppendCharacterJob::run(job, thread);
        setStatus(Job::Status_Failed);
    }
};

class BusyJob : public ThreadWeaver::Job
{
public:
    BusyJob()
        : ThreadWeaver::Job()
    {
        using namespace ThreadWeaver;
        TWDEBUG(3, "BusyJob ctor\n");
    }

    ~BusyJob() override
    {
        using namespace ThreadWeaver;
        TWDEBUG(3, "~BusyJob\n");
    }

    void run(ThreadWeaver::JobPointer, ThreadWeaver::Thread *) override
    {
        using namespace ThreadWeaver;
        TWDEBUG(3, "BusyJob: entered run()\n");
        for (int i = 0; i < 100; ++i) {
            int k = (i << 3) + (i >> 4);
            Q_UNUSED(k);
        }
    }
};

#endif
