/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2008 Fredrik Höglund <fredrik@kde.org>
    SPDX-FileCopyrightText: 2008 Konstantin Heil <konst.heil@stud.uni-heidelberg.de>
    SPDX-FileCopyrightText: 2009 Peter Penz <peter.penz@gmx.at>
    SPDX-FileCopyrightText: 2017 Elvis Angelaccio <elvis.angelaccio@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "ktooltipwidget.h"

#include <QPaintEvent>
#include <QScreen>
#include <QStyleOptionFrame>
#include <QStylePainter>
#include <QTimer>
#include <QVBoxLayout>
#include <QWindow>

class KToolTipWidgetPrivate
{
public:
    KToolTipWidgetPrivate(KToolTipWidget *parent)
        : q(parent)
    {
    }

    void init();
    void addWidget(QWidget *widget);
    void removeWidget();
    void show(const QPoint &pos, QWindow *transientParent);
    void storeParent();
    void restoreParent();
    QPoint centerBelow(const QRect &rect, QScreen *screen) const;

    KToolTipWidget *const q;
    QTimer hideTimer;
    QVBoxLayout *layout = nullptr;
    QWidget *content = nullptr;
    QWidget *contentParent = nullptr;
};

void KToolTipWidgetPrivate::init()
{
    layout = new QVBoxLayout(q);

    hideTimer.setSingleShot(true);
    hideTimer.setInterval(500);

    QObject::connect(&hideTimer, &QTimer::timeout, q, &QWidget::hide);

    q->setAttribute(Qt::WA_TranslucentBackground);
    q->setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
}

void KToolTipWidgetPrivate::addWidget(QWidget *widget)
{
    removeWidget();
    content = widget;
    storeParent();
    layout->addWidget(content);
    QObject::connect(content, &QWidget::destroyed, q, &QWidget::hide);
}

void KToolTipWidgetPrivate::removeWidget()
{
    layout->removeWidget(content);
    restoreParent();
}

void KToolTipWidgetPrivate::show(const QPoint &pos, QWindow *transientParent)
{
    if (pos.isNull()) {
        return;
    }

    q->move(pos);
    q->createWinId();
    q->windowHandle()->setProperty("ENABLE_BLUR_BEHIND_HINT", true);
    q->windowHandle()->setTransientParent(transientParent);

    QObject::connect(transientParent, &QWindow::activeChanged, q, &QWidget::hide);

    q->show();
}

void KToolTipWidgetPrivate::storeParent()
{
    if (!content) {
        return;
    }

    contentParent = qobject_cast<QWidget *>(content->parent());
}

void KToolTipWidgetPrivate::restoreParent()
{
    if (!content || !contentParent) {
        return;
    }

    content->setParent(contentParent);
}

QPoint KToolTipWidgetPrivate::centerBelow(const QRect &rect, QScreen *screen) const
{
    // It must be assured that:
    // - the content is fully visible
    // - the content is not drawn inside rect

    const QSize size = q->sizeHint();
    const int margin = q->style()->pixelMetric(QStyle::PM_ToolTipLabelFrameWidth);
    const QRect screenGeometry = screen->geometry();

    const bool hasRoomToLeft = (rect.left() - size.width() - margin >= screenGeometry.left());
    const bool hasRoomToRight = (rect.right() + size.width() + margin <= screenGeometry.right());
    const bool hasRoomAbove = (rect.top() - size.height() - margin >= screenGeometry.top());
    const bool hasRoomBelow = (rect.bottom() + size.height() + margin <= screenGeometry.bottom());
    if (!hasRoomAbove && !hasRoomBelow && !hasRoomToLeft && !hasRoomToRight) {
        return QPoint();
    }

    int x = 0;
    int y = 0;
    if (hasRoomBelow || hasRoomAbove) {
        x = qMax(screenGeometry.left(), rect.center().x() - size.width() / 2);
        if (x + size.width() >= screenGeometry.right()) {
            x = screenGeometry.right() - size.width() + 1;
        }
        Q_ASSERT(x >= 0);
        if (hasRoomBelow) {
            y = rect.bottom() + margin;
        } else {
            y = rect.top() - size.height() - margin + 1;
        }
    } else {
        Q_ASSERT(hasRoomToLeft || hasRoomToRight);
        if (hasRoomToRight) {
            x = rect.right() + margin;
        } else {
            x = rect.left() - size.width() - margin + 1;
        }
        // Put the tooltip at the bottom of the screen. The x-coordinate has already
        // been adjusted, so that no overlapping with rect occurs.
        y = screenGeometry.bottom() - size.height() + 1;
    }

    return QPoint(x, y);
}

KToolTipWidget::KToolTipWidget(QWidget *parent)
    : QWidget(parent)
    , d(new KToolTipWidgetPrivate(this))
{
    d->init();
}

KToolTipWidget::~KToolTipWidget()
{
    d->restoreParent();
}

void KToolTipWidget::showAt(const QPoint &pos, QWidget *content, QWindow *transientParent)
{
    d->addWidget(content);
    d->show(pos, transientParent);
}

void KToolTipWidget::showBelow(const QRect &rect, QWidget *content, QWindow *transientParent)
{
    d->addWidget(content);

    const auto contentMargins = layout()->contentsMargins();
    const QSize screenSize = transientParent->screen()->geometry().size();

    content->setMaximumSize(screenSize.shrunkBy(contentMargins));

    d->show(d->centerBelow(rect, transientParent->screen()), transientParent);
}

int KToolTipWidget::hideDelay() const
{
    return d->hideTimer.interval();
}

void KToolTipWidget::hideLater()
{
    if (!isVisible()) {
        return;
    }

    if (hideDelay() > 0) {
        d->hideTimer.start();
    } else {
        hide();
    }
}

void KToolTipWidget::setHideDelay(int delay)
{
    d->hideTimer.setInterval(delay);
}

void KToolTipWidget::enterEvent(QEnterEvent *)
{
    // Ignore hide delay and leave tooltip visible.
    if (hideDelay() > 0) {
        d->hideTimer.stop();
    } else {
        hide();
    }
}

void KToolTipWidget::hideEvent(QHideEvent *)
{
    d->removeWidget();

    QObject::disconnect(windowHandle()->transientParent(), &QWindow::activeChanged, this, &QWidget::hide);

    // Give time to the content widget to get his own hide event.
    QTimer::singleShot(0, this, &KToolTipWidget::hidden);
}

void KToolTipWidget::leaveEvent(QEvent *)
{
    // Don't bother starting the hide timer, we are done.
    hide();
}

void KToolTipWidget::paintEvent(QPaintEvent *event)
{
    QStylePainter painter(this);
    painter.setClipRegion(event->region());
    QStyleOptionFrame option;
    option.initFrom(this);
    painter.drawPrimitive(QStyle::PE_PanelTipLabel, option);
    painter.end();

    QWidget::paintEvent(event);
}

#include "moc_ktooltipwidget.cpp"
