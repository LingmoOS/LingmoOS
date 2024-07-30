/* -*- C++ -*-
    This file is part of ThreadWeaver.

    SPDX-FileCopyrightText: 2005-2014 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <ThreadWeaver/ThreadWeaver>

#include <QCoreApplication>

using namespace ThreadWeaver;

//@@snippet_begin(sample-helloworldraw-class)
class QDebugJob : public Job
{
public:
    QDebugJob(const char *message = nullptr)
        : m_message(message)
    {
    }

protected:
    void run(JobPointer, Thread *) override
    {
        qDebug() << m_message;
    }

private:
    const char *m_message;
};
//@@snippet_end

//@@snippet_begin(sample-helloworldraw-main)
int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    // Allocate jobs as local variables:
    QDebugJob j1("Hello");
    QDebugJob j2("World!");
    JobPointer j3(new QDebugJob("This is..."));
    Job *j4 = new QDebugJob("ThreadWeaver!");
    // Queue the Job using the default Queue stream:
    stream() << j1 << j2 // local variables
             << j3 // a shared pointer
             << j4; // a raw pointer
    // Wait for finish(), because job is destroyed
    // before the global queue:
    Queue::instance()->finish();
}
//@@snippet_end
