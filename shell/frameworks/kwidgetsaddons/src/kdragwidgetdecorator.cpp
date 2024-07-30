/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2000 Carsten Pfeiffer <pfeiffer@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kdragwidgetdecorator.h"

#include <QApplication>
#include <QDrag>
#include <QMouseEvent>
#include <QWidget>

class KDragWidgetDecoratorBasePrivate
{
public:
    bool dragEnabled = true;
    QWidget *decoratedWidget = nullptr;
    QPoint startPos;
};

KDragWidgetDecoratorBase::KDragWidgetDecoratorBase(QWidget *parent)
    : QObject(parent)
    , d(new KDragWidgetDecoratorBasePrivate)
{
    parent->installEventFilter(this);
    d->decoratedWidget = parent;
}

KDragWidgetDecoratorBase::~KDragWidgetDecoratorBase() = default;

bool KDragWidgetDecoratorBase::isDragEnabled() const
{
    return d->dragEnabled;
}

void KDragWidgetDecoratorBase::setDragEnabled(bool enable)
{
    d->dragEnabled = enable;
}

bool KDragWidgetDecoratorBase::eventFilter(QObject *watched, QEvent *event)
{
    Q_UNUSED(watched) // except in Q_ASSERT
    Q_ASSERT(watched == d->decoratedWidget);

    if (!d->dragEnabled) {
        return false;
    }

    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *e = static_cast<QMouseEvent *>(event);
        d->startPos = e->pos();

    } else if (event->type() == QEvent::MouseMove) {
        QMouseEvent *e = static_cast<QMouseEvent *>(event);
        if ((e->buttons() & Qt::LeftButton) && (e->pos() - d->startPos).manhattanLength() > QApplication::startDragDistance()) {
            startDrag();
            d->decoratedWidget->setProperty("down", false);
            return true;
        }
    }

    return false;
}

QWidget *KDragWidgetDecoratorBase::decoratedWidget() const
{
    return d->decoratedWidget;
}

QDrag *KDragWidgetDecoratorBase::dragObject()
{
    return nullptr;
}

void KDragWidgetDecoratorBase::startDrag()
{
    QDrag *drag = dragObject();
    if (drag) {
        drag->exec(Qt::CopyAction);
    }
}

#include "moc_kdragwidgetdecorator.cpp"
