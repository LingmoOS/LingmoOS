// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LINGMO_SCROLLER_H
#define LINGMO_SCROLLER_H

#include <QList>
#include <QObject>
#include <QPointF>
#include <QPointer>
#include <QTimer>

class QWheelEvent;
class QWindow;

namespace Lingmo {

class ScrollFrames;

// Touchpad scrolling of one view. Finger samples are resampled to each frame
// of the view's window and the coast runs on the same frame clock; without an
// exposed window, samples apply at once and advance() drives the coast.
// A paged view moves along one axis and settles on a viewport-sized page.
// Subclasses connect it to a kind of view.
class Scroller : public QObject
{
    Q_OBJECT

public:
    // A frame shows the fingers this far in the past, so most frames interpolate.
    static constexpr qreal resampleLatency = 5;

    ~Scroller() override;

    bool handleWheel(QWheelEvent *event, bool elasticOvershoot, bool paged = false);
    // A paged view lands on the page it was heading for.
    void cancel(bool restoreBounds = true);
    // Ends a coast where it is shown; an overscrolled view still settles.
    void stopCoast();
    // Shows the frame for frameTime, in clock() milliseconds.
    void advance(qreal frameTime);
    bool isActive() const;
    // From the first movement until the view comes to rest; a wheel step counts
    // for a moment, as it moves the view at once.
    bool isScrolling() const;

    static qreal clock();
    static void setClock(qreal (*clock)());
    // Fingers resting on the touchpad over the window.
    static void stopCoasts(QWindow *window);

signals:
    void scrollingChanged();

protected:
    struct Axis {
        qreal minimum;
        qreal maximum;
        qreal viewport;
        bool enabled;
    };

    explicit Scroller(QObject *parent = nullptr);

    virtual Axis horizontalAxis() const = 0;
    virtual Axis verticalAxis() const = 0;
    virtual QPointF position() const = 0;
    // A position past the bounds shows the view stretched over its edge.
    virtual void applyPosition(const QPointF &position) = 0;
    virtual void reportMoving(bool moving, Qt::Orientations axes);
    // Stops the view's own animation, such as a mouse flick.
    virtual void stopViewMotion();
    // False once the view is being destroyed and can no longer be scrolled.
    virtual bool isViewAlive() const;

    // Frames come from this window. A widget window repaints in full on every
    // update request, so a widget view takes the requests instead.
    void setWindow(QWindow *window, bool takesUpdateRequests = false);
    // Something else moved the view.
    void viewMoved();

    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    struct Sample {
        qreal time;
        QPointF position;
    };

    void setPosition(const QPointF &position);
    void setMoving(bool moving);
    bool isFrameDriven() const;
    void scheduleFrame();
    qreal eventTime(const QWheelEvent *event) const;
    void reset(bool restoreBounds);
    void settle();
    void begin();
    void addSample(qreal time);
    void release(qreal time);
    void startCoast(const QPointF &position, qreal time);
    QPointF contentPosition(const QPointF &dragPosition) const;
    QPointF resampled(qreal time, bool extrapolate, bool *extrapolated = nullptr) const;
    QPointF flickVelocity() const;
    qreal dragPosition(qreal position, const Axis &axis) const;
    qreal unboundedPosition(qreal position, const Axis &axis) const;
    void advanceAxis(qreal &position, qreal &velocity, const Axis &axis, qreal seconds);
    qreal pageTarget(qreal position, qreal velocity, const Axis &axis) const;
    qreal nearestPage(qreal position, const Axis &axis) const;
    void updateScrolling();

    QPointer<QWindow> m_window;
    QTimer m_wheelStep;
    QList<Sample> m_samples;
    QPointF m_dragPosition;
    QPointF m_shownDragPosition;
    QPointF m_coastPosition;
    QPointF m_velocity;
    QPointF m_snapTarget;
    QPointF m_snapDuration;
    QPointF m_snapVelocity;
    QPointF m_accumulatedDelta;
    qreal m_shownTime;
    qreal m_coastTime = 0;
    Qt::Orientations m_axes = Qt::Horizontal | Qt::Vertical;
    Qt::Orientations m_dragAxes;
    bool m_axisChosen = false;
    bool m_gesture = false;
    bool m_coasting = false;
    bool m_extrapolated = false;
    bool m_consumed = false;
    bool m_nativeMomentum = false;
    bool m_nativeBlockedX = false;
    bool m_nativeBlockedY = false;
    bool m_cancelled = false;
    bool m_writing = false;
    bool m_elastic = true;
    bool m_paged = false;
    bool m_scrolling = false;

    friend class ScrollFrames;
};

} // namespace Lingmo

#endif // LINGMO_SCROLLER_H
