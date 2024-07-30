/*
    SPDX-FileCopyrightText: 2009 Sebastian Trueg <trueg@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef _K_PIXMAPSEQUENCE_WIDGET_H_
#define _K_PIXMAPSEQUENCE_WIDGET_H_

#include <QWidget>
#include <memory>

#include <kwidgetsaddons_export.h>

class KPixmapSequence;

/**
 * \class KPixmapSequenceWidget kpixmapsequencewidget.h KPixmapSequenceWidget
 *
 * \brief A simple widget showing a fixed size pixmap sequence.
 *
 * The KPixmapSequenceWidget uses the KPixmapSequenceOverlayPainter to show a
 * sequence of pixmaps. It is intended as a simple wrapper around the
 * KPixmapSequenceOverlayPainter in case a widget is more appropriate than
 * an event filter.
 *
 * \author Sebastian Trueg <trueg@kde.org>
 *
 * \since 4.4
 */
class KWIDGETSADDONS_EXPORT KPixmapSequenceWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int interval READ interval WRITE setInterval)

public:
    /**
     * Constructor
     */
    explicit KPixmapSequenceWidget(QWidget *parent = nullptr);
    KPixmapSequenceWidget(const KPixmapSequence &seq, QWidget *parent = nullptr);

    /**
     * Destructor
     */
    ~KPixmapSequenceWidget() override;

    /**
     * The sequence used to draw the overlay.
     *
     * \sa setSequence
     */
    KPixmapSequence sequence() const;

    /**
     * The interval between frames.
     *
     * \sa setInterval, KPixmapSequenceOverlayPainter::interval
     */
    int interval() const;

    /**
     * \reimpl
     */
    QSize sizeHint() const override;

public Q_SLOTS:
    /**
     * Set the sequence to be used. By default the KDE busy sequence is used.
     */
    void setSequence(const KPixmapSequence &seq);

    /**
     * Set the interval between frames. The default is 200.
     * \sa interval, KPixmapSequenceOverlayPainter::setInterval
     */
    void setInterval(int msecs);

private:
    std::unique_ptr<class KPixmapSequenceWidgetPrivate> const d;
};

#endif
