/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2021 Steffen Hartleib <steffenhartleib@t-online.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

// Self
#include "ktwofingerswipe.h"

// Qt
#include <QGraphicsWidget>
#include <QLineF>
#include <QTouchEvent>
#include <QDebug>
class KTwoFingerSwipeRecognizerPrivate
{
public:
    KTwoFingerSwipeRecognizerPrivate(KTwoFingerSwipeRecognizer *parent)
        : q(parent)
    {
    }
    KTwoFingerSwipeRecognizer *const q;
    qint64 mTouchBeginnTimestamp = 0;
    bool mGestureAlreadyFinished = false;
    int mMaxSwipeTime = 90;
    int mMinSwipeDistance = 30;
};

KTwoFingerSwipeRecognizer::KTwoFingerSwipeRecognizer()
    : QGestureRecognizer()
    , d(new KTwoFingerSwipeRecognizerPrivate(this))
{
}

KTwoFingerSwipeRecognizer::~KTwoFingerSwipeRecognizer()
{
}

QGesture* KTwoFingerSwipeRecognizer::create(QObject *target)
{
    Q_UNUSED(target);
    return static_cast<QGesture *>(new KTwoFingerSwipe());
}

QGestureRecognizer::Result KTwoFingerSwipeRecognizer::recognize(QGesture *gesture, QObject *watched, QEvent *event)
{
    Q_UNUSED(watched)

    KTwoFingerSwipe *kTwoFingerSwipe = static_cast<KTwoFingerSwipe *>(gesture);
    const QTouchEvent *touchEvent = static_cast<const QTouchEvent *>(event);

    switch (event->type()) {
    case QEvent::TouchBegin: {
        d->mTouchBeginnTimestamp = touchEvent->timestamp();
        d->mGestureAlreadyFinished = false;
        const QTouchEvent::TouchPoint tp = touchEvent->points().first();
        kTwoFingerSwipe->setHotSpot(tp.globalPressPosition());
        kTwoFingerSwipe->setPos(tp.pressPosition());
        kTwoFingerSwipe->setScreenPos(tp.globalPressPosition());
        kTwoFingerSwipe->setScenePos(tp.scenePressPosition());
        kTwoFingerSwipe->setSwipeAngle(0.0);
        return MayBeGesture;
    }

    case QEvent::TouchUpdate: {
        // The gesture was already canceled or finished, so we will ignore this event.
        if (d->mGestureAlreadyFinished) {
            return Ignore;
        }

        const qint64 now = touchEvent->timestamp();
        const qint64 elapsedTime = now - d->mTouchBeginnTimestamp;
        const QLineF ql = QLineF(touchEvent->points().first().pressPosition(), touchEvent->points().first().position());
        const qreal length = ql.length();
        const int touchPointSize = touchEvent->points().size();

        kTwoFingerSwipe->setSwipeAngle(ql.angle());

        if (touchPointSize > 2) {
                d->mGestureAlreadyFinished = true;
                return CancelGesture;
        }

        if (touchPointSize == 2) {
            if (elapsedTime > d->mMaxSwipeTime) {
                d->mGestureAlreadyFinished = true;
                return CancelGesture;
            }
            if (length >= d->mMinSwipeDistance && elapsedTime <= d->mMaxSwipeTime && !d->mGestureAlreadyFinished) {
                d->mGestureAlreadyFinished = true;
                return FinishGesture;
            } else if (elapsedTime <= d->mMaxSwipeTime && !d->mGestureAlreadyFinished) {
                return TriggerGesture;
            }
        }
        break;
    }

    case QEvent::TouchEnd: {
        if (gesture->state() == Qt::GestureUpdated || gesture->state() == Qt::GestureStarted) {
            if (!d->mGestureAlreadyFinished) {
                return CancelGesture;
            }
        }
        break;
    }

    default:
        return Ignore;
    }
    return Ignore;
}

int KTwoFingerSwipeRecognizer::maxSwipeTime() const
{
    return d->mMaxSwipeTime;
}

void KTwoFingerSwipeRecognizer::setMaxSwipeTime(int i)
{
    if (i < 0) {
        i = 0;
    }

    d->mMaxSwipeTime = i;
}

int KTwoFingerSwipeRecognizer::minSswipeDistance() const
{
    return d->mMinSwipeDistance;
}

void KTwoFingerSwipeRecognizer::setSwipeDistance(int i)
{
    if (i < 0) {
        i = 0;
    }

    d->mMinSwipeDistance = i;
}



class KTwoFingerSwipePrivate
{
public:
    KTwoFingerSwipePrivate(KTwoFingerSwipe *parent)
        : q(parent)
    {
    }
    KTwoFingerSwipe *const q;
    QPointF mPos = QPointF(-1, -1);
    QPointF mScreenPos = QPointF(-1, -1);
    QPointF mScenePos = QPointF(-1, -1);
    qreal mSwipeAngle = 0.0;
};

KTwoFingerSwipe::KTwoFingerSwipe(QObject *parent)
    : QGesture(parent)
    , d(new KTwoFingerSwipePrivate(this))
{
}

KTwoFingerSwipe::~KTwoFingerSwipe()
{
}

QPointF KTwoFingerSwipe::pos() const
{
    return d->mPos;
}

void KTwoFingerSwipe::setPos(QPointF _pos)
{
    d->mPos = _pos;
}

QPointF KTwoFingerSwipe::screenPos() const
{
    return d->mScreenPos;
}

void KTwoFingerSwipe::setScreenPos(QPointF _screenPos)
{
    d->mScreenPos = _screenPos;
}

QPointF KTwoFingerSwipe::scenePos() const
{
    return d->mScenePos;
}

void KTwoFingerSwipe::setScenePos(QPointF _scenePos)
{
    d->mScenePos = _scenePos;
}

qreal KTwoFingerSwipe::swipeAngle() const
{
    return d->mSwipeAngle;
}
 void KTwoFingerSwipe::setSwipeAngle(qreal _swipeAngle)
{
    d->mSwipeAngle = _swipeAngle;
}

#include "moc_ktwofingerswipe.cpp"
