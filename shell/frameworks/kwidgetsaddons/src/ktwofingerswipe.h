/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2021 Steffen Hartleib <steffenhartleib@t-online.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KTWOFINGERSWIPE_H
#define KTWOFINGERSWIPE_H

#include <kwidgetsaddons_export.h>

#include <QGesture>
#include <QGestureRecognizer>
#include <memory>

/**
 * @class KTwoFingerSwipe ktwofingerswipe.h KTwoFingerSwipe
 *
 * @short A two finger swipe gesture.
 *
 * Provides a class for a two finger swipe gesture.
 *
 * Note: The QGestureManager need a QMainwindow, to delivery the gesture.
 *
 * @since 5.83
 * @author Steffen Hartleib <steffenhartleib@t-online.de>
 */
class KWIDGETSADDONS_EXPORT KTwoFingerSwipe : public QGesture
{
    Q_OBJECT
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)
    Q_PROPERTY(QPointF screenPos READ screenPos WRITE setScreenPos)
    Q_PROPERTY(QPointF scenePos READ scenePos WRITE setScenePos)
    Q_PROPERTY(qreal swipeAngle READ swipeAngle WRITE setSwipeAngle)
public:
    /**
     * The constructor.
     */
    explicit KTwoFingerSwipe(QObject *parent = nullptr);

    /**
     * Destructor
     */
    ~KTwoFingerSwipe() override;

    /**
     * The start position of the gesture, relative to the widget that received the gesture.
     *
     * Note: This is not necessarily the same position as in the widget that grabGesture() uses.
     *
     * @return The start position of the gesture, relative to the widget.
     */
    Q_REQUIRED_RESULT QPointF pos() const;

    /**
     * Sets the position, relative to the widget.
     *
     * @param pos The position.
     */
    void setPos(QPointF pos);

    /**
     * @return The start screen position of the gesture.
     */
    Q_REQUIRED_RESULT QPointF screenPos() const;

    /**
     * Sets the screen position.
     *
     * @param screenPos The screen position.
     */
    void setScreenPos(QPointF screenPos);

    /**
     * @return The start scene position of the gesture.
     */
    Q_REQUIRED_RESULT QPointF scenePos() const;

    /**
     * Sets the scene position.
     *
     * @param scenePos The scene position, identical to the screen position for widgets.
     */
    void setScenePos(QPointF scenePos);

    /**
     * @return The angle of the swipe gesture.
     */
    Q_REQUIRED_RESULT qreal swipeAngle() const;

    /**
     * Sets the angle of the swipe gesture
     *
     * @param swipeAngle The angle.
     */
    void setSwipeAngle(qreal swipeAngle);
private:
    std::unique_ptr<class KTwoFingerSwipePrivate> const d;
};

/**
 * @class KTwoFingerSwipeRecognizer ktwofingerswiperecognizer.h KTwoFingerSwipeRecognizer
 *
 * @short The recognizer for a two finger swipe gesture.
 *
 * Provides the recognizer for a two finger swipe gesture. To adjust the maximum swipe time
 * and the minimum swipe distance, for a valid swipe gesture:
 * @see setMaxSwipeTime
 * @see setSwipeDistance
 *
 * @since 5.83
 * @author Steffen Hartleib <steffenhartleib@t-online.de>
 */
class KWIDGETSADDONS_EXPORT KTwoFingerSwipeRecognizer : public QGestureRecognizer
{
public:
    /**
     * The constructor.
     */
    KTwoFingerSwipeRecognizer();

    /**
     * Destructor
     */
    ~KTwoFingerSwipeRecognizer() override;

    /**
     * Qt called this member to create a new QGesture object.
     *
     * @param target The target for the gesture.
     *
     * @return The new QGesture object.
     */
    QGesture* create(QObject *target) override;

    /**
     * Handles the given event for the watched object and update the gesture object.
     *
     * @param gesture The gesture object.
     * @param watched The watched object.
     * @param event The event.
     *
     * @return The result reflects how much of the gesture has been recognized.
     */
    Result recognize(QGesture *gesture, QObject *watched, QEvent *event) override;

    /**
     * @return The maximum duration for the swipe gesture, in milliseconds.
     */
    Q_REQUIRED_RESULT int maxSwipeTime() const;

    /**
     * Set the maximum duration of the swipe gesture. If @param i is negative, it will be set to null.
     *
     * @param i The maximum duration in milliseconds.
     */
    void setMaxSwipeTime(int i);

    /**
     * @return The minimum distance for the swipe gesture.
     */
    Q_REQUIRED_RESULT int minSswipeDistance() const;

    /**
     * Set the minimum distance of the swipe gesture. If @param i is negative, it will be set to null.
     *
     * @param i The minimum distance.
     */
    void setSwipeDistance(int i);

private:
    std::unique_ptr<class KTwoFingerSwipeRecognizerPrivate> const d;
    Q_DISABLE_COPY(KTwoFingerSwipeRecognizer)
};

#endif
