/* This file is part of the KDE libraries
 * SPDX-FileCopyrightText: 2009 Dario Freddi <drf at kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "kwindowbasedidletimepoller_p.h"

#include <QEvent>
#include <QTimer>
#include <QWindow>

KWindowBasedIdleTimePoller::KWindowBasedIdleTimePoller(QObject *parent)
    : KAbstractIdleTimePoller(parent)
{
}

KWindowBasedIdleTimePoller::~KWindowBasedIdleTimePoller()
{
}

bool KWindowBasedIdleTimePoller::isAvailable()
{
    return true;
}

bool KWindowBasedIdleTimePoller::setUpPoller()
{
    m_pollTimer = new QTimer(this);

    // setup idle timer, with some smart polling
    connect(m_pollTimer, &QTimer::timeout, this, &KWindowBasedIdleTimePoller::poll);

    m_grabber = new QWindow();
    m_grabber->setFlag(Qt::X11BypassWindowManagerHint);
    m_grabber->setPosition(-1000, -1000);
    m_grabber->installEventFilter(this);
    m_grabber->setObjectName(QStringLiteral("KIdleGrabberWidget"));

    return additionalSetUp();
}

void KWindowBasedIdleTimePoller::unloadPoller()
{
    m_pollTimer->deleteLater();
    m_grabber->deleteLater();
}

QList<int> KWindowBasedIdleTimePoller::timeouts() const
{
    return m_timeouts;
}

void KWindowBasedIdleTimePoller::addTimeout(int nextTimeout)
{
    m_timeouts.append(nextTimeout);
    poll();
}

bool KWindowBasedIdleTimePoller::eventFilter(QObject *object, QEvent *event)
{
    if (object == m_grabber && (event->type() == QEvent::MouseMove || event->type() == QEvent::KeyPress)) {
        detectedActivity();
        return true;
    } else if (object != m_grabber) {
        // If it's not the grabber, fallback to default event filter
        return false;
    }

    // Otherwise, simply ignore it
    return false;
}

void KWindowBasedIdleTimePoller::waitForActivity()
{
    m_grabber->show();
    m_grabber->setMouseGrabEnabled(true);
    m_grabber->setKeyboardGrabEnabled(true);
}

void KWindowBasedIdleTimePoller::detectedActivity()
{
    stopCatchingIdleEvents();
    Q_EMIT resumingFromIdle();
}

void KWindowBasedIdleTimePoller::releaseInputLock()
{
    m_grabber->setMouseGrabEnabled(false);
    m_grabber->setKeyboardGrabEnabled(false);
    m_grabber->hide();
}

int KWindowBasedIdleTimePoller::poll()
{
    int idle = getIdleTime();

    // Check if we reached a timeout..
    for (int timeOut : std::as_const(m_timeouts)) {
        if ((timeOut - idle < 300 && timeOut >= idle) || (idle - timeOut < 300 && idle > timeOut)) {
            // Bingo!
            Q_EMIT timeoutReached(timeOut);
        }
    }

    // Let's check the timer now!
    int mintime = 0;

    for (int i : std::as_const(m_timeouts)) {
        if (i > idle && (i < mintime || mintime == 0)) {
            mintime = i;
        }
    }

    // qDebug() << "mintime " << mintime << "idle " << idle;

    if (mintime != 0) {
        m_pollTimer->start(mintime - idle);
    } else {
        m_pollTimer->stop();
    }

    return idle;
}

int KWindowBasedIdleTimePoller::forcePollRequest()
{
    return poll();
}

void KWindowBasedIdleTimePoller::removeTimeout(int timeout)
{
    m_timeouts.removeOne(timeout);
    poll();
}

void KWindowBasedIdleTimePoller::catchIdleEvent()
{
    waitForActivity();
}

void KWindowBasedIdleTimePoller::stopCatchingIdleEvents()
{
    releaseInputLock();
}

#include "moc_kwindowbasedidletimepoller_p.cpp"
