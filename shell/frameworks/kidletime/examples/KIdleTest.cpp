/*
 * SPDX-FileCopyrightText: 2009 Dario Freddi <drf@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KIdleTest.h"
#include <QDebug>

#include "kidletime.h"

//@@snippet_begin(initialize)
KIdleTest::KIdleTest()
{
    // connect to idle events
    connect(KIdleTime::instance(), &KIdleTime::resumingFromIdle, this, &KIdleTest::resumeEvent);
    connect(KIdleTime::instance(), qOverload<int, int>(&KIdleTime::timeoutReached), this, &KIdleTest::timeoutReached);

    // register to get informed for the very next user event
    KIdleTime::instance()->catchNextResumeEvent();

    printf("Your idle time is %d\n", KIdleTime::instance()->idleTime());
    printf("Welcome!! Move your mouse or do something to start...\n");
}
//@@snippet_end

KIdleTest::~KIdleTest()
{
}

//@@snippet_begin(resumeEvent)
void KIdleTest::resumeEvent()
{
    KIdleTime::instance()->removeAllIdleTimeouts();

    printf("Great! Now stay idle for 5 seconds to get a nice message. From 10 seconds on, you can move your mouse to get back here.\n");
    printf("If you will stay idle for too long, I will simulate your activity after 25 seconds, and make everything start back\n");

    KIdleTime::instance()->addIdleTimeout(5000);
    KIdleTime::instance()->addIdleTimeout(10000);
    KIdleTime::instance()->addIdleTimeout(25000);
}
//@@snippet_end

//@@snippet_begin(timeoutReached)
void KIdleTest::timeoutReached(int id, int timeout)
{
    Q_UNUSED(id)

    if (timeout == 5000) {
        printf("5 seconds passed, stay still some more...\n");
    } else if (timeout == 10000) {
        KIdleTime::instance()->catchNextResumeEvent();
        printf("Cool. You can move your mouse to start over\n");
    } else if (timeout == 25000) {
        printf("Uff, you're annoying me. Let's start again. I'm simulating your activity now\n");
        KIdleTime::instance()->simulateUserActivity();
    } else {
        qDebug() << "OUCH";
    }
}
//@@snippet_end

#include "moc_KIdleTest.cpp"
