/*
    SPDX-FileCopyrightText: 2009 Sebastian Trueg <trueg@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "kpixmapsequencewidget.h"
#include "kpixmapsequence.h"
#include "kpixmapsequenceoverlaypainter.h"

class KPixmapSequenceWidgetPrivate
{
public:
    KPixmapSequenceOverlayPainter *m_painter = nullptr;
};

KPixmapSequenceWidget::KPixmapSequenceWidget(QWidget *parent)
    : QWidget(parent)
    , d(new KPixmapSequenceWidgetPrivate)
{
    d->m_painter = new KPixmapSequenceOverlayPainter(this);
    d->m_painter->setWidget(this);
}

KPixmapSequenceWidget::KPixmapSequenceWidget(const KPixmapSequence &seq, QWidget *parent)
    : QWidget(parent)
    , d(new KPixmapSequenceWidgetPrivate)
{
    d->m_painter = new KPixmapSequenceOverlayPainter(seq);
    d->m_painter->setWidget(this);
    if (seq.isValid()) {
        d->m_painter->start();
    }
}

KPixmapSequenceWidget::~KPixmapSequenceWidget()
{
    delete d->m_painter;
}

KPixmapSequence KPixmapSequenceWidget::sequence() const
{
    return d->m_painter->sequence();
}

int KPixmapSequenceWidget::interval() const
{
    return d->m_painter->interval();
}

QSize KPixmapSequenceWidget::sizeHint() const
{
    if (d->m_painter->sequence().isValid()) {
        return d->m_painter->sequence().frameSize();
    } else {
        return QWidget::sizeHint();
    }
}

void KPixmapSequenceWidget::setSequence(const KPixmapSequence &seq)
{
    d->m_painter->setSequence(seq);
    if (seq.isValid()) {
        setFixedSize(seq.frameSize());
        d->m_painter->start();
    } else {
        d->m_painter->stop();
    }
}

void KPixmapSequenceWidget::setInterval(int msecs)
{
    d->m_painter->setInterval(msecs);
}

#include "moc_kpixmapsequencewidget.cpp"
