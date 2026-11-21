// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "scroller.h"

#include <QElapsedTimer>
#include <QGuiApplication>
#include <QHash>
#include <QScopeGuard>
#include <QScreen>
#include <QStyleHints>
#include <QWheelEvent>
#include <QWindow>

#include <cmath>
#include <limits>

namespace Lingmo {

static constexpr qreal friction = 4.5;
static constexpr qreal springFrequency = 18.0;
static constexpr qreal rubberBandResistance = 0.55;
static constexpr qreal minimumFlickSpeed = 40.0;
static constexpr qreal maximumFlickSpeed = 6000.0;
static constexpr qreal stopSpeed = 5.0;
static constexpr qreal velocityTimeout = 80;
// The flick velocity is a line fitted to the last samples of this window.
static constexpr qreal velocityWindow = 100;
static constexpr int velocitySamples = 10;
// Frames predict past the last sample only while samples arrive at a steady rate.
static constexpr qreal minimumSampleInterval = 2;
static constexpr qreal maximumSampleInterval = 20;
static constexpr qreal maximumPrediction = 8;
// A release faster than this turns the page however little it moved.
static constexpr qreal pageFlickSpeed = 400.0;
static constexpr qreal snapDuration = 180;
static constexpr qreal snapPageDuration = 220;
static constexpr qreal minimumSnapDuration = 120;
static constexpr int wheelStepDuration = 120;
static constexpr qreal never = -std::numeric_limits<qreal>::infinity();

static qreal monotonicClock()
{
    static QElapsedTimer timer;
    if (!timer.isValid())
        timer.start();
    return timer.nsecsElapsed() / 1e6;
}

static qreal (*currentClock)() = monotonicClock;
// Event timestamps come from the compositor's clock; its offset to ours is the
// smallest delivery delay seen.
static qreal eventClockOffset = 0;
static bool eventClockKnown = false;

static qreal bounded(qreal position, qreal minimum, qreal maximum)
{
    return qBound(minimum, position, maximum);
}

static QList<Scroller *> &allScrollers()
{
    static QList<Scroller *> scrollers;
    return scrollers;
}

// The frame clock of one window, shared by the scrollers shown in it.
class ScrollFrames : public QObject
{
public:
    static ScrollFrames *of(QWindow *window)
    {
        static QHash<QWindow *, ScrollFrames *> frames;
        ScrollFrames *&windowFrames = frames[window];
        if (!windowFrames) {
            windowFrames = new ScrollFrames(window);
            connect(windowFrames, &QObject::destroyed, [window] {
                frames.remove(window);
            });
        }
        return windowFrames;
    }

    bool takesUpdateRequests = false;

protected:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (event->type() != QEvent::UpdateRequest)
            return QObject::eventFilter(watched, event);

        QList<QPointer<Scroller>> shown;
        for (Scroller *scroller : std::as_const(allScrollers())) {
            if (scroller->m_window == watched && scroller->isActive())
                shown.append(scroller);
        }
        if (!shown.isEmpty()) {
            const qreal time = frameTime();
            for (const auto &scroller : std::as_const(shown)) {
                if (scroller && scroller->isActive())
                    scroller->advance(time);
            }
        }
        return takesUpdateRequests;
    }

private:
    explicit ScrollFrames(QWindow *window)
        : QObject(window)
        , m_window(window)
    {
        window->installEventFilter(this);
    }

    qreal frameTime()
    {
        const qreal now = Scroller::clock();
        const qreal rate = m_window->screen() ? m_window->screen()->refreshRate() : 0;
        const qreal interval = 1000.0 / (rate >= 1 ? rate : 60);
        // Update requests come early or late around a steady refresh, and a
        // pipelined one can come twice in one refresh: frames step by whole
        // refreshes and drift slowly towards the clock.
        if (now - m_frameTime > 3 * interval || m_frameTime - now > 2 * interval) {
            m_frameTime = now;
        } else {
            const qreal predicted = m_frameTime
                + interval * qMax(1.0, std::round((now - m_frameTime) / interval));
            m_frameTime = predicted + 0.1 * (now - predicted);
        }
        return m_frameTime;
    }

    QWindow *m_window;
    qreal m_frameTime = never;
};

Scroller::Scroller(QObject *parent)
    : QObject(parent)
    , m_shownTime(never)
{
    allScrollers().append(this);
    m_wheelStep.setSingleShot(true);
    m_wheelStep.setInterval(wheelStepDuration);
    connect(&m_wheelStep, &QTimer::timeout, this, &Scroller::updateScrolling);
}

Scroller::~Scroller()
{
    allScrollers().removeOne(this);
}

qreal Scroller::clock()
{
    return currentClock();
}

void Scroller::setClock(qreal (*clock)())
{
    currentClock = clock ? clock : monotonicClock;
    eventClockKnown = false;
}

void Scroller::stopCoasts(QWindow *window)
{
    QList<QPointer<Scroller>> shown;
    for (Scroller *scroller : std::as_const(allScrollers())) {
        if (scroller->m_window == window)
            shown.append(scroller);
    }
    for (const auto &scroller : std::as_const(shown)) {
        if (scroller)
            scroller->stopCoast();
    }
}

void Scroller::reportMoving(bool, Qt::Orientations)
{
}

void Scroller::stopViewMotion()
{
}

bool Scroller::isViewAlive() const
{
    return true;
}

void Scroller::setPosition(const QPointF &position)
{
    m_writing = true;
    applyPosition(position);
    m_writing = false;
}

void Scroller::viewMoved()
{
    if (!m_writing)
        cancel(false);
}

void Scroller::setMoving(bool moving)
{
    if (!moving) {
        reportMoving(false, {});
        return;
    }

    Qt::Orientations axes;
    if (horizontalAxis().enabled && (!m_gesture || m_axes.testFlag(Qt::Horizontal)))
        axes |= Qt::Horizontal;
    if (verticalAxis().enabled && (!m_gesture || m_axes.testFlag(Qt::Vertical)))
        axes |= Qt::Vertical;
    reportMoving(true, axes);
}

void Scroller::setWindow(QWindow *window, bool takesUpdateRequests)
{
    if (window && takesUpdateRequests)
        ScrollFrames::of(window)->takesUpdateRequests = true;
    if (window == m_window)
        return;

    cancel();
    if (m_window)
        m_window->removeEventFilter(this);
    m_window = window;
    if (m_window) {
        ScrollFrames::of(m_window);
        m_window->installEventFilter(this);
    }
}

bool Scroller::isFrameDriven() const
{
    return m_window && m_window->isExposed();
}

void Scroller::scheduleFrame()
{
    if (isFrameDriven())
        m_window->requestUpdate();
}

qreal Scroller::eventTime(const QWheelEvent *event) const
{
    const qreal now = clock();
    if (!event->timestamp())
        return now;

    const qreal timestamp = event->timestamp();
    const qreal offset = now - timestamp;
    // A jump the other way is a new clock or a wrapped 32-bit timestamp.
    if (!eventClockKnown || offset < eventClockOffset || offset > eventClockOffset + 1000) {
        eventClockOffset = offset;
        eventClockKnown = true;
    }
    return timestamp + eventClockOffset;
}

bool Scroller::eventFilter(QObject *watched, QEvent *event)
{
    switch (event->type()) {
    case QEvent::MouseButtonPress:
    case QEvent::TouchBegin:
    case QEvent::KeyPress:
    case QEvent::WindowDeactivate:
    case QEvent::Hide:
        cancel();
        break;
    case QEvent::Wheel:
        // A new contact stops every coast in the window. An untouched
        // overscrolled view still settles; the touched view takes over.
        if (static_cast<QWheelEvent *>(event)->phase() == Qt::ScrollBegin && isActive())
            settle();
        break;
    default:
        break;
    }
    return QObject::eventFilter(watched, event);
}


bool Scroller::isActive() const
{
    return m_gesture || m_coasting;
}

bool Scroller::isScrolling() const
{
    return m_scrolling;
}

void Scroller::updateScrolling()
{
    const bool scrolling = (m_gesture && m_consumed) || m_coasting || m_wheelStep.isActive();
    if (scrolling == m_scrolling)
        return;

    m_scrolling = scrolling;
    if (isViewAlive())
        setMoving(scrolling);
    emit scrollingChanged();
}

void Scroller::cancel(bool restoreBounds)
{
    if (!isViewAlive()) {
        reset(false);
        return;
    }

    reset(restoreBounds);
    updateScrolling();
}

void Scroller::reset(bool restoreBounds)
{
    const bool wasActive = isActive();
    const bool snapping = m_paged && m_coasting;
    const bool moved = m_paged && m_gesture && m_consumed;
    m_gesture = false;
    m_coasting = false;
    m_extrapolated = false;
    m_consumed = false;
    m_velocity = {};
    m_samples.clear();
    m_cancelled |= wasActive;
    if (!restoreBounds || !wasActive || !isViewAlive())
        return;

    const auto x = horizontalAxis();
    const auto y = verticalAxis();
    const QPointF current = position();
    if (snapping)
        setPosition(m_snapTarget);
    else if (moved)
        setPosition({nearestPage(current.x(), x), nearestPage(current.y(), y)});
    else if (!m_paged)
        setPosition({bounded(current.x(), x.minimum, x.maximum),
                     bounded(current.y(), y.minimum, y.maximum)});
}

void Scroller::settle()
{
    // A page snap is how a paged view settles.
    if (m_paged && m_coasting)
        return;

    const qreal time = m_shownTime == never ? clock() - resampleLatency : m_shownTime;
    reset(false);
    startCoast(position(), time);
    updateScrolling();
}

void Scroller::stopCoast()
{
    if (m_coasting || (m_gesture && m_nativeMomentum))
        settle();
}

qreal Scroller::dragPosition(qreal position, const Axis &axis) const
{
    const qreal edge = bounded(position, axis.minimum, axis.maximum);
    if (!m_elastic || axis.viewport <= 0)
        return edge;

    const qreal distance = position - edge;
    if (m_paged) {
        // Each pixel past an end gives less, up to a quarter of a page.
        const qreal limit = axis.viewport / 4;
        return edge + limit * distance / (std::abs(distance) + limit);
    }
    return edge + distance * rubberBandResistance
        / (1.0 + std::abs(distance) * rubberBandResistance / axis.viewport);
}

qreal Scroller::unboundedPosition(qreal position, const Axis &axis) const
{
    const qreal edge = bounded(position, axis.minimum, axis.maximum);
    const qreal distance = position - edge;
    if (!m_elastic || axis.viewport <= 0)
        return edge;

    if (m_paged) {
        const qreal limit = axis.viewport / 4;
        return edge + limit * distance / qMax(1.0, limit - std::abs(distance));
    }
    return edge + distance / (rubberBandResistance
        * qMax(0.01, 1.0 - std::abs(distance) / axis.viewport));
}

qreal Scroller::pageTarget(qreal position, qreal velocity, const Axis &axis) const
{
    if (!axis.enabled || axis.viewport <= 0)
        return position;

    const qreal page = (position - axis.minimum) / axis.viewport;
    qreal target = std::round(page);
    if (velocity > pageFlickSpeed)
        target = std::floor(page) + 1;
    else if (velocity < -pageFlickSpeed)
        target = std::ceil(page) - 1;
    return bounded(axis.minimum + target * axis.viewport, axis.minimum, axis.maximum);
}

qreal Scroller::nearestPage(qreal position, const Axis &axis) const
{
    return pageTarget(position, 0, axis);
}

QPointF Scroller::contentPosition(const QPointF &dragPosition) const
{
    QPointF content = position();
    if (m_dragAxes.testFlag(Qt::Horizontal))
        content.setX(this->dragPosition(dragPosition.x(), horizontalAxis()));
    if (m_dragAxes.testFlag(Qt::Vertical))
        content.setY(this->dragPosition(dragPosition.y(), verticalAxis()));
    return content;
}

void Scroller::begin()
{
    stopViewMotion();
    m_gesture = true;
    // A page snap runs on until the fingers move.
    m_coasting = m_paged && m_coasting;
    m_extrapolated = false;
    m_consumed = false;
    m_nativeMomentum = false;
    m_nativeBlockedX = false;
    m_nativeBlockedY = false;
    m_cancelled = false;
    m_velocity = {};
    m_accumulatedDelta = {};
    m_samples.clear();
    m_axes = Qt::Horizontal | Qt::Vertical;
    m_dragAxes = {};
    m_axisChosen = false;
    m_shownTime = never;
    const QPointF current = position();
    m_dragPosition = {unboundedPosition(current.x(), horizontalAxis()),
                      unboundedPosition(current.y(), verticalAxis())};
    m_shownDragPosition = m_dragPosition;
}

void Scroller::addSample(qreal time)
{
    if (!m_samples.isEmpty())
        time = qMax(time, m_samples.last().time);
    m_samples.append({time, m_dragPosition});
    m_extrapolated = false;
    while (m_samples.first().time < time - 2 * velocityWindow)
        m_samples.removeFirst();

    if (isFrameDriven())
        scheduleFrame();
    else
        advance(time + resampleLatency);
}

QPointF Scroller::resampled(qreal time, bool extrapolate, bool *extrapolated) const
{
    if (extrapolated)
        *extrapolated = false;
    const Sample &last = m_samples.last();
    if (time >= last.time) {
        if (!extrapolate || time == last.time || m_samples.size() < 2)
            return last.position;
        const Sample &previous = m_samples.at(m_samples.size() - 2);
        const qreal interval = last.time - previous.time;
        if (interval < minimumSampleInterval || interval > maximumSampleInterval)
            return last.position;
        const qreal ahead = qMin(time - last.time, qMin(interval / 2, maximumPrediction));
        if (extrapolated)
            *extrapolated = true;
        return last.position + (last.position - previous.position) * (ahead / interval);
    }

    for (qsizetype i = m_samples.size() - 1; i > 0; --i) {
        const Sample &before = m_samples.at(i - 1);
        if (before.time > time)
            continue;
        const Sample &after = m_samples.at(i);
        const qreal fraction = (time - before.time) / (after.time - before.time);
        return before.position + (after.position - before.position) * fraction;
    }
    return m_samples.first().position;
}

QPointF Scroller::flickVelocity() const
{
    const qreal end = m_samples.last().time;
    qsizetype first = m_samples.size() - 1;
    while (first > 0 && m_samples.size() - first < velocitySamples
           && end - m_samples.at(first - 1).time <= velocityWindow) {
        --first;
    }

    // A least-squares line is steadier than the last interval with
    // millisecond timestamps and uneven event delivery.
    const qreal count = m_samples.size() - first;
    qreal meanTime = 0;
    QPointF meanPosition;
    for (qsizetype i = first; i < m_samples.size(); ++i) {
        meanTime += m_samples.at(i).time / count;
        meanPosition += contentPosition(m_samples.at(i).position) / count;
    }
    qreal timeVariance = 0;
    QPointF covariance;
    for (qsizetype i = first; i < m_samples.size(); ++i) {
        const qreal time = m_samples.at(i).time - meanTime;
        timeVariance += time * time;
        covariance += (contentPosition(m_samples.at(i).position) - meanPosition) * time;
    }
    if (timeVariance <= 0)
        return {};

    const auto limit = [](qreal velocity) {
        return std::abs(velocity) < minimumFlickSpeed
            ? 0.0 : bounded(velocity, -maximumFlickSpeed, maximumFlickSpeed);
    };
    const QPointF velocity = covariance * (1000.0 / timeVariance);
    return {limit(velocity.x()), limit(velocity.y())};
}

bool Scroller::handleWheel(QWheelEvent *event, bool elasticOvershoot, bool paged)
{
    const auto phase = event->phase();
    // A wheel is left to the paged view's own paging.
    if (paged && phase == Qt::NoScrollPhase)
        return false;

    const auto notify = qScopeGuard([this] {
        updateScrolling();
    });
    m_elastic = elasticOvershoot;
    m_paged = paged;
    const auto x = horizontalAxis();
    const auto y = verticalAxis();
    const qreal time = eventTime(event);
    const bool precise = phase != Qt::NoScrollPhase || !event->pixelDelta().isNull();

    if (phase == Qt::ScrollBegin)
        begin();
    else if (phase == Qt::ScrollUpdate && !m_gesture && !m_cancelled)
        begin();

    if (phase == Qt::ScrollEnd) {
        if (!m_gesture)
            return false;
        // Some backends include the last movement in the end event.
        if (!event->pixelDelta().isNull()) {
            QWheelEvent last(event->position(), event->globalPosition(), event->pixelDelta(), {},
                             event->buttons(), event->modifiers(),
                             m_nativeMomentum ? Qt::ScrollMomentum : Qt::ScrollUpdate, event->inverted());
            last.setTimestamp(event->timestamp());
            handleWheel(&last, elasticOvershoot, paged);
        }
        const bool consumed = m_consumed;
        release(time);
        return consumed;
    }

    if (phase == Qt::ScrollMomentum) {
        if (m_cancelled)
            return true;
        if (m_paged)
            return m_consumed;
        if (!m_nativeMomentum) {
            m_coasting = false;
            m_samples.clear();
            m_velocity = {};
            m_nativeMomentum = true;
            m_gesture = true;
        }
    } else if (phase == Qt::ScrollUpdate && m_cancelled) {
        return true;
    }

    // A pixel vector is authoritative on both axes, including its zeroes.
    QPointF delta = precise ? QPointF(event->pixelDelta())
        : QPointF(event->angleDelta()) * (QGuiApplication::styleHints()->wheelScrollLines() / 8.0);
    if (phase == Qt::NoScrollPhase) {
        cancel();
        stopViewMotion();
    }
    if (!precise) {
        if ((event->modifiers() & Qt::ShiftModifier) || (!y.enabled && x.enabled && delta.x() == 0)) {
            delta.setX(delta.y());
            delta.setY(0);
        }
        if (event->modifiers() & Qt::ControlModifier) {
            delta.setX(delta.x() == 0 ? 0 : std::copysign(x.viewport, delta.x()));
            delta.setY(delta.y() == 0 ? 0 : std::copysign(y.viewport, delta.y()));
        }
    }

    if (m_gesture && !m_nativeMomentum && !m_axisChosen) {
        m_accumulatedDelta += delta;
        const qreal dx = std::abs(m_accumulatedDelta.x());
        const qreal dy = std::abs(m_accumulatedDelta.y());
        if (qMax(dx, dy) >= 6) {
            if (m_paged)
                m_axes = dx > dy ? Qt::Horizontal : Qt::Vertical;
            else if (dx > 2.5 * dy)
                m_axes = Qt::Horizontal;
            else if (dy > 2.5 * dx)
                m_axes = Qt::Vertical;
            m_axisChosen = true;
            // A pager moves only once the direction is known, then by all of it.
            if (m_paged) {
                delta = m_accumulatedDelta;
                // The seed took the first movement on both axes; keep the pager's.
                if (!m_samples.isEmpty()) {
                    QPointF &seed = m_samples.first().position;
                    if (!x.enabled || m_axes != Qt::Horizontal)
                        seed.setX(m_dragPosition.x());
                    if (!y.enabled || m_axes != Qt::Vertical)
                        seed.setY(m_dragPosition.y());
                }
            }
        } else if (m_paged) {
            // Frames then ease into the held-back movement instead of jumping to it.
            if (m_samples.isEmpty() && !m_coasting && !delta.isNull())
                m_samples.append({time, m_dragPosition - delta});
            return false;
        }
    }
    if (!x.enabled || (m_gesture && !m_axes.testFlag(Qt::Horizontal)))
        delta.setX(0);
    if (!y.enabled || (m_gesture && !m_axes.testFlag(Qt::Vertical)))
        delta.setY(0);

    if (delta.isNull())
        return m_gesture && m_consumed;

    if (m_gesture && !m_nativeMomentum) {
        if (m_coasting) {
            // Moving fingers catch a page snap where it is shown.
            m_coasting = false;
            const QPointF current = position();
            m_dragPosition = {unboundedPosition(current.x(), x), unboundedPosition(current.y(), y)};
            m_shownDragPosition = m_dragPosition;
        }
        // After a pause, movement continues from the predicted position on
        // screen rather than stepping back to the last sample.
        if (m_extrapolated && time - m_samples.last().time > maximumSampleInterval) {
            m_dragPosition = m_shownDragPosition;
            m_samples.append({m_shownTime, m_shownDragPosition});
            m_extrapolated = false;
        }
        if (delta.x() != 0)
            m_dragAxes |= Qt::Horizontal;
        if (delta.y() != 0)
            m_dragAxes |= Qt::Vertical;
        const QPointF previous = contentPosition(m_dragPosition);
        m_dragPosition -= delta;
        if (!m_elastic) {
            m_dragPosition = {bounded(m_dragPosition.x(), x.minimum, x.maximum),
                              bounded(m_dragPosition.y(), y.minimum, y.maximum)};
        }
        const QPointF next = contentPosition(m_dragPosition);
        m_consumed |= next != previous;
        addSample(time);
        return next != previous || m_consumed;
    }

    const QPointF previous = position();
    QPointF next = previous;
    if (phase == Qt::ScrollMomentum) {
        if (!m_nativeBlockedX)
            next.setX(dragPosition(previous.x() - delta.x(), x));
        if (!m_nativeBlockedY)
            next.setY(dragPosition(previous.y() - delta.y(), y));
        m_nativeBlockedX |= next.x() <= x.minimum || next.x() >= x.maximum;
        m_nativeBlockedY |= next.y() <= y.minimum || next.y() >= y.maximum;
    } else {
        next = {bounded(previous.x() - delta.x(), x.minimum, x.maximum),
                bounded(previous.y() - delta.y(), y.minimum, y.maximum)};
    }
    setPosition(next);
    if (phase == Qt::NoScrollPhase && next != previous)
        m_wheelStep.start();
    if (m_coasting)
        m_coastPosition = next;
    else if (phase == Qt::ScrollMomentum)
        startCoast(next, time);
    m_consumed |= next != previous;
    return next != previous || (m_gesture && m_consumed);
}

void Scroller::release(qreal time)
{
    m_gesture = false;
    // Fingers that never moved a paged view leave it as it is, snapping or not.
    if (m_paged && !m_consumed) {
        if (!m_coasting)
            m_samples.clear();
        return;
    }
    if (m_nativeMomentum || m_samples.isEmpty()) {
        m_velocity = {};
        if (!m_coasting)
            startCoast(position(), time);
        return;
    }

    const Sample last = m_samples.last();
    m_velocity = time - last.time > velocityTimeout ? QPointF() : flickVelocity();
    if (!m_velocity.isNull()) {
        // The fingers left with the last sample; frames before it still show samples.
        startCoast(contentPosition(last.position), last.time);
    } else if (m_extrapolated) {
        m_samples.clear();
        startCoast(contentPosition(m_shownDragPosition), qMax(m_shownTime, time));
    } else {
        startCoast(contentPosition(last.position), qMax(last.time, time));
    }
}

void Scroller::startCoast(const QPointF &position, qreal time)
{
    const auto x = horizontalAxis();
    const auto y = verticalAxis();
    m_coastPosition = position;
    m_coastTime = time;
    if (m_paged) {
        m_snapTarget = {pageTarget(position.x(), m_velocity.x(), x),
                        pageTarget(position.y(), m_velocity.y(), y)};
        const auto snap = [this](qreal from, qreal to, qreal velocity, const Axis &axis,
                                 qreal &duration, qreal &startVelocity) {
            const qreal distance = std::abs(to - from);
            duration = snapDuration + snapPageDuration * qMin(1.0, distance / axis.viewport);
            if (axis.enabled && (to - from) * velocity < 0) {
                // Heading away, as past an end, the page turns round from the
                // speed it was shown at instead of jumping back.
                const qreal unbounded = unboundedPosition(from, axis);
                startVelocity = velocity * (dragPosition(unbounded + 0.5, axis)
                                            - dragPosition(unbounded - 0.5, axis));
                return;
            }
            // OutCubic starts at three times its average speed. Fingers faster
            // than that shorten it, so a quick release does not brake the page.
            if ((to - from) * velocity > 0)
                duration = qMax(minimumSnapDuration, qMin(duration, 3000 * distance / std::abs(velocity)));
            if (distance == 0)
                duration = 0;
            startVelocity = duration > 0 ? 3000 * (to - from) / duration : 0;
        };
        snap(position.x(), m_snapTarget.x(), m_velocity.x(), x, m_snapDuration.rx(), m_snapVelocity.rx());
        snap(position.y(), m_snapTarget.y(), m_velocity.y(), y, m_snapDuration.ry(), m_snapVelocity.ry());
        m_coasting = position != m_snapTarget || !m_snapVelocity.isNull() || position != this->position();
    } else {
        m_coasting = !m_velocity.isNull() || position != this->position()
            || position.x() != bounded(position.x(), x.minimum, x.maximum)
            || position.y() != bounded(position.y(), y.minimum, y.maximum);
    }
    if (m_coasting)
        scheduleFrame();
    else
        m_samples.clear();
}

void Scroller::advance(qreal frameTime)
{
    const qreal time = frameTime - resampleLatency;
    if (!isViewAlive() || !isActive())
        return;
    // A frame clock that ran ahead and was set back shows nothing new until it
    // catches up, but must not stop asking for frames.
    if (time < m_shownTime) {
        scheduleFrame();
        return;
    }

    QPointF next;
    bool pending = false;
    if (m_gesture && !m_nativeMomentum && !m_coasting) {
        if (m_samples.isEmpty())
            return;
        m_shownDragPosition = resampled(time, true, &m_extrapolated);
        next = contentPosition(m_shownDragPosition);
        pending = time < m_samples.last().time;
    } else if (m_coasting && time < m_coastTime) {
        next = m_samples.isEmpty() ? position() : contentPosition(resampled(time, false));
        pending = true;
    } else if (m_coasting && m_paged) {
        const qreal elapsed = time - m_coastTime;
        // A cubic Hermite from the start velocity to rest on the target.
        const auto eased = [elapsed](qreal from, qreal to, qreal velocity, qreal duration) {
            if (elapsed >= duration)
                return to;
            const qreal progress = elapsed / duration;
            const qreal remaining = 1.0 - progress;
            return to + (from - to) * (1.0 + 2.0 * progress) * remaining * remaining
                + velocity * duration / 1000 * progress * remaining * remaining;
        };
        next = {eased(m_coastPosition.x(), m_snapTarget.x(), m_snapVelocity.x(), m_snapDuration.x()),
                eased(m_coastPosition.y(), m_snapTarget.y(), m_snapVelocity.y(), m_snapDuration.y())};
        if (elapsed >= qMax(m_snapDuration.x(), m_snapDuration.y())) {
            m_coasting = false;
            m_samples.clear();
        }
        pending = m_coasting;
    } else if (m_coasting) {
        const auto x = horizontalAxis();
        const auto y = verticalAxis();
        const qreal seconds = (time - m_coastTime) / 1000.0;
        advanceAxis(m_coastPosition.rx(), m_velocity.rx(), x, seconds);
        advanceAxis(m_coastPosition.ry(), m_velocity.ry(), y, seconds);
        m_coastTime = time;
        next = m_coastPosition;
        if (m_velocity.isNull() && next.x() >= x.minimum && next.x() <= x.maximum
            && next.y() >= y.minimum && next.y() <= y.maximum) {
            m_coasting = false;
            m_samples.clear();
        }
        pending = m_coasting;
    } else {
        return;
    }

    m_shownTime = time;
    const bool changed = next != position();
    setPosition(next);
    if (changed || pending)
        scheduleFrame();
    updateScrolling();
}

void Scroller::advanceAxis(qreal &position, qreal &velocity, const Axis &axis, qreal seconds)
{
    const qreal edge = bounded(position, axis.minimum, axis.maximum);
    if (!axis.enabled || (!m_elastic && edge != position)) {
        position = edge;
        velocity = 0;
        return;
    }

    if (edge != position) {
        const qreal offset = position - edge;
        const qreal coefficient = velocity + springFrequency * offset;
        const qreal decay = std::exp(-springFrequency * seconds);
        position = edge + (offset + coefficient * seconds) * decay;
        velocity = (velocity - springFrequency * coefficient * seconds) * decay;
        if (std::abs(position - edge) < 0.5 && std::abs(velocity) < stopSpeed) {
            position = edge;
            velocity = 0;
        }
    } else {
        const qreal initialPosition = position;
        const qreal initialVelocity = velocity;
        const qreal decay = std::exp(-friction * seconds);
        position += velocity * (1.0 - decay) / friction;
        velocity *= decay;
        const qreal limit = bounded(position, axis.minimum, axis.maximum);
        if (limit != position) {
            if (m_elastic) {
                // Split the frame at the edge so rebound does not depend on refresh rate.
                const qreal crossing = -std::log1p(-(limit - initialPosition) * friction / initialVelocity)
                    / friction;
                const qreal remaining = qMax(0.0, seconds - crossing);
                const qreal edgeVelocity = initialVelocity * std::exp(-friction * crossing);
                const qreal springDecay = std::exp(-springFrequency * remaining);
                position = limit + edgeVelocity * remaining * springDecay;
                velocity = edgeVelocity * (1.0 - springFrequency * remaining) * springDecay;
            } else {
                position = limit;
                velocity = 0;
            }
        } else if (std::abs(velocity) < stopSpeed) {
            velocity = 0;
        }
    }
}

} // namespace Lingmo
