/*
    SPDX-FileCopyrightText: 2009 Sebastian Trueg <trueg@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "kpixmapsequenceoverlaypainter.h"
#include "kpixmapsequence.h"

#include <QCoreApplication>
#include <QEvent>
#include <QPainter>
#include <QPointer>
#include <QRect>
#include <QTimer>
#include <QWidget>

class KPixmapSequenceOverlayPainterPrivate
{
public:
    void init(KPixmapSequenceOverlayPainter *p);
    void timeout();
    void paintFrame();

    KPixmapSequence &sequence();

    QRect pixmapRect();

    KPixmapSequence m_sequence;
    QPointer<QWidget> m_widget;
    Qt::Alignment m_alignment;
    QPoint m_offset;
    QRect m_rect;

    QTimer m_timer;
    int m_counter;

    bool m_started;

    KPixmapSequenceOverlayPainter *q;
};

void KPixmapSequenceOverlayPainterPrivate::init(KPixmapSequenceOverlayPainter *p)
{
    q = p;
    m_widget = nullptr;
    m_alignment = Qt::AlignCenter;
    m_started = false;
    q->setInterval(200);
    QObject::connect(&m_timer, &QTimer::timeout, q, [this]() {
        timeout();
    });
}

void KPixmapSequenceOverlayPainterPrivate::timeout()
{
    if (sequence().isEmpty()) {
        return;
    }
    ++m_counter;
    m_counter %= sequence().frameCount();
    if (m_widget) {
        m_widget->update(pixmapRect());
    }
}

void KPixmapSequenceOverlayPainterPrivate::paintFrame()
{
    if (m_counter >= sequence().frameCount()) {
        return;
    }
    QPainter p(m_widget);
    p.drawPixmap(pixmapRect(), sequence().frameAt(m_counter), QRect(QPoint(0, 0), sequence().frameSize()));
}

KPixmapSequence &KPixmapSequenceOverlayPainterPrivate::sequence()
{
    return m_sequence;
}

QRect KPixmapSequenceOverlayPainterPrivate::pixmapRect()
{
    QRect rect(m_rect);
    if (!rect.isValid()) {
        rect = m_widget->rect();
    }

    QPoint pos(rect.topLeft());
    if (m_alignment & Qt::AlignHCenter) {
        pos.setX(rect.center().x() - (sequence().frameSize().width() / 2));
    } else if (m_alignment & Qt::AlignRight) {
        pos.setX(rect.right() - sequence().frameSize().width());
    }

    if (m_alignment & Qt::AlignVCenter) {
        pos.setY(rect.center().y() - (sequence().frameSize().height() / 2));
    } else if (m_alignment & Qt::AlignBottom) {
        pos.setY(rect.bottom() - sequence().frameSize().height());
    }

    pos += m_offset;

    return QRect(pos, sequence().frameSize());
}

KPixmapSequenceOverlayPainter::KPixmapSequenceOverlayPainter(QObject *parent)
    : QObject(parent)
    , d(new KPixmapSequenceOverlayPainterPrivate)
{
    d->init(this);
}

KPixmapSequenceOverlayPainter::KPixmapSequenceOverlayPainter(const KPixmapSequence &seq, QObject *parent)
    : QObject(parent)
    , d(new KPixmapSequenceOverlayPainterPrivate)
{
    d->init(this);
    d->m_sequence = seq;
}

KPixmapSequenceOverlayPainter::~KPixmapSequenceOverlayPainter()
{
    stop();
}

KPixmapSequence KPixmapSequenceOverlayPainter::sequence() const
{
    return d->sequence();
}

int KPixmapSequenceOverlayPainter::interval() const
{
    return d->m_timer.interval();
}

QRect KPixmapSequenceOverlayPainter::rect() const
{
    if (d->m_rect.isValid()) {
        return d->m_rect;
    } else if (d->m_widget) {
        return d->m_widget->rect();
    } else {
        return QRect();
    }
}

Qt::Alignment KPixmapSequenceOverlayPainter::alignment() const
{
    return d->m_alignment;
}

QPoint KPixmapSequenceOverlayPainter::offset() const
{
    return d->m_offset;
}

void KPixmapSequenceOverlayPainter::setSequence(const KPixmapSequence &seq)
{
    bool restart = d->m_started;
    stop();
    d->m_sequence = seq;
    if (restart) {
        start();
    }
}

void KPixmapSequenceOverlayPainter::setInterval(int msecs)
{
    d->m_timer.setInterval(msecs);
}

void KPixmapSequenceOverlayPainter::setWidget(QWidget *w)
{
    stop();
    d->m_widget = w;
}

void KPixmapSequenceOverlayPainter::setRect(const QRect &rect)
{
    bool restart = d->m_started;
    stop();
    d->m_rect = rect;
    if (restart) {
        start();
    }
}

void KPixmapSequenceOverlayPainter::setAlignment(Qt::Alignment align)
{
    bool restart = d->m_started;
    stop();
    d->m_alignment = align;
    if (restart) {
        start();
    }
}

void KPixmapSequenceOverlayPainter::setOffset(const QPoint &offset)
{
    bool restart = d->m_started;
    stop();
    d->m_offset = offset;
    if (restart) {
        start();
    }
}

void KPixmapSequenceOverlayPainter::start()
{
    if (d->m_widget) {
        stop();

        d->m_counter = 0;
        d->m_started = true;
        d->m_widget->installEventFilter(this);
        if (d->m_widget->isVisible()) {
            d->m_timer.start();
            d->m_widget->update(d->pixmapRect());
        }
    }
}

void KPixmapSequenceOverlayPainter::stop()
{
    d->m_timer.stop();
    if (d->m_widget && d->m_started) {
        d->m_started = false;
        d->m_widget->removeEventFilter(this);
        d->m_widget->update(d->pixmapRect());
    }
}

bool KPixmapSequenceOverlayPainter::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == d->m_widget) {
        switch (event->type()) {
        case QEvent::Paint:
            // make sure we paint after everyone else including other event filters
            obj->removeEventFilter(this); // don't recourse...
            QCoreApplication::sendEvent(obj, event);
            d->paintFrame();
            obj->installEventFilter(this); // catch on...
            return true;
            break;
        case QEvent::Hide:
            d->m_timer.stop();
            break;
        case QEvent::Show:
            if (d->m_started) {
                d->m_timer.start();
                d->m_widget->update(d->pixmapRect());
            }
            break;
        default:
            break;
        }
    }

    return false;
}

#include "moc_kpixmapsequenceoverlaypainter.cpp"
