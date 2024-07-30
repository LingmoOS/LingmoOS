/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2021 Steffen Hartleib <steffenhartleib@t-online.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

// Self
#include "ktwofingertap.h"

// Qt
#include <QApplication>
#include <QGraphicsWidget>
#include <QTouchEvent>

class KTwoFingerTapRecognizerPrivate
{
public:
    KTwoFingerTapRecognizerPrivate(KTwoFingerTapRecognizer *parent)
        : q(parent)
    {
    }
    KTwoFingerTapRecognizer *const q;
    bool mGestureTriggered = false;
    int mTapRadius = 40;  // same as Qt::TapGesture
    Qt::GestureState mLastState = Qt::NoGesture;
};

KTwoFingerTapRecognizer::KTwoFingerTapRecognizer()
    : QGestureRecognizer()
    , d(new KTwoFingerTapRecognizerPrivate(this))
{
}

KTwoFingerTapRecognizer::~KTwoFingerTapRecognizer()
{
}

QGesture* KTwoFingerTapRecognizer::create(QObject *target)
{
    Q_UNUSED(target)
    return static_cast<QGesture *>(new KTwoFingerTap());
}

QGestureRecognizer::Result KTwoFingerTapRecognizer::recognize(QGesture *gesture, QObject *watched, QEvent *event)
{
    Q_UNUSED(watched)

    KTwoFingerTap *ktwoFingerTap = static_cast<KTwoFingerTap *>(gesture);
    const QTouchEvent *touchEvent = static_cast<const QTouchEvent *>(event);

    switch (event->type()) {
    case QEvent::TouchBegin: {
        const QTouchEvent::TouchPoint tp = touchEvent->points().first();
        ktwoFingerTap->setHotSpot(tp.globalPressPosition());
        ktwoFingerTap->setPos(tp.pressPosition());
        ktwoFingerTap->setScreenPos(tp.globalPressPosition());
        ktwoFingerTap->setScenePos(tp.scenePressPosition());
        d->mLastState = Qt::NoGesture;
        return MayBeGesture;
    }

    case QEvent::TouchUpdate: {
        // The gesture was already canceled or finished, so we will ignore this event.
        if (d->mLastState == Qt::GestureCanceled || d->mLastState == Qt::GestureFinished) {
            return Ignore;
        }

        const int touchPointSize = touchEvent->points().size();

        if (touchPointSize > 2) {
            d->mLastState = Qt::GestureCanceled;
            return CancelGesture;
        }

        if (touchPointSize == 2) {
            if ((touchEvent->points().first().pressPosition() - touchEvent->points().first().position()).manhattanLength() > d->mTapRadius) {
                d->mLastState = Qt::GestureCanceled;
                return CancelGesture;
            }
            if ((touchEvent->points().at(1).pressPosition() - touchEvent->points().at(1).position()).manhattanLength() > d->mTapRadius) {
                d->mLastState = Qt::GestureCanceled;
                return CancelGesture;
            }
            if (touchEvent->touchPointStates() & Qt::TouchPointPressed) {
                d->mLastState = gesture->state() == Qt::NoGesture ? Qt::GestureStarted : Qt::GestureUpdated;
                return TriggerGesture;
            }
            if (touchEvent->touchPointStates() & Qt::TouchPointMoved) {
                d->mLastState = gesture->state() == Qt::NoGesture ? Qt::GestureStarted : Qt::GestureUpdated;
                return TriggerGesture;
            }
            if (touchEvent->touchPointStates() & Qt::TouchPointReleased) {
                d->mLastState = Qt::GestureFinished;
                return FinishGesture;
            }
        }
        break;
    }

    case QEvent::TouchEnd: {
        if (d->mLastState == Qt::GestureStarted || d->mLastState == Qt::GestureUpdated) {
            return FinishGesture;
        }
        break;
    }

    default:
        return Ignore;
    }
    return Ignore;
}

void KTwoFingerTapRecognizer::setTapRadius(int i)
{
    if (i < 0) {
        i = 0;
    }

    d->mTapRadius = i;
}

int KTwoFingerTapRecognizer::tapRadius() const
{
    return d->mTapRadius;
}

class KTwoFingerTapPrivate
{
public:
    KTwoFingerTapPrivate(KTwoFingerTap *parent)
        : q(parent)
    {
    }
    KTwoFingerTap *const q;
    QPointF mPos = QPointF(-1, -1);
    QPointF mScreenPos = QPointF(-1, -1);
    QPointF mScenePos = QPointF(-1, -1);
};

KTwoFingerTap::KTwoFingerTap(QObject *parent)
    : QGesture(parent)
    , d(new KTwoFingerTapPrivate(this))
{
}

KTwoFingerTap::~KTwoFingerTap()
{
}

QPointF KTwoFingerTap::pos() const
{
    return d->mPos;
}

void KTwoFingerTap::setPos(QPointF _pos)
{
    d->mPos = _pos;
}

QPointF KTwoFingerTap::screenPos() const
{
    return d->mScreenPos;
}

void KTwoFingerTap::setScreenPos(QPointF _screenPos)
{
    d->mScreenPos = _screenPos;
}

QPointF KTwoFingerTap::scenePos() const
{
    return d->mScenePos;
}

void KTwoFingerTap::setScenePos(QPointF _scenePos)
{
    d->mScenePos = _scenePos;
}

#include "moc_ktwofingertap.cpp"
