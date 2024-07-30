/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "kbusyindicatorwidget.h"

#include <QApplication>
#include <QIcon>
#include <QPainter>
#include <QResizeEvent>
#include <QStyle>
#include <QVariantAnimation>

class KBusyIndicatorWidgetPrivate
{
public:
    KBusyIndicatorWidgetPrivate(KBusyIndicatorWidget *parent)
        : q(parent)
    {
        animation.setLoopCount(-1);
        animation.setDuration(2000);
        animation.setStartValue(0);
        animation.setEndValue(360);
        QObject::connect(&animation, &QVariantAnimation::valueChanged, q, [this](QVariant value) {
            rotation = value.toReal();
            q->update(); // repaint new rotation
        });
    }

    KBusyIndicatorWidget *const q;
    QVariantAnimation animation;
    QIcon icon = QIcon::fromTheme(QStringLiteral("view-refresh"));
    qreal rotation = 0;
    QPointF paintCenter;
};

KBusyIndicatorWidget::KBusyIndicatorWidget(QWidget *parent)
    : QWidget(parent)
    , d(new KBusyIndicatorWidgetPrivate(this))
{
}

KBusyIndicatorWidget::~KBusyIndicatorWidget() = default;

QSize KBusyIndicatorWidget::minimumSizeHint() const
{
    const auto extent = QApplication::style()->pixelMetric(QStyle::PM_SmallIconSize);
    return QSize(extent, extent);
}

void KBusyIndicatorWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    d->animation.start();
}

void KBusyIndicatorWidget::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
    if (d->animation.state() == QAbstractAnimation::Running) // avoid warning if never started yet
        d->animation.pause();
}

void KBusyIndicatorWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    d->paintCenter = QPointF(event->size().width() / 2.0, //
                             event->size().height() / 2.0);
}

void KBusyIndicatorWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    // Rotate around the center and then reset back to origin for icon painting.
    painter.translate(d->paintCenter);
    painter.rotate(d->rotation);
    painter.translate(-d->paintCenter);

    d->icon.paint(&painter, rect());
}

bool KBusyIndicatorWidget::event(QEvent *event)
{
    // Only overridden to be flexible WRT binary compatible in the future.
    // Overriding later has potential to change the call going through
    // the vtable or not.
    return QWidget::event(event);
}

#include "moc_kbusyindicatorwidget.cpp"
