/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2004 Felix Berger <felixberger@beldesign.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "ktoolbarlabelaction.h"

#include <QApplication>
#include <QLabel>
#include <QPointer>
#include <QToolBar>

class KToolBarLabelActionPrivate
{
public:
    QPointer<QAction> buddy;
    QPointer<QLabel> label;
};

KToolBarLabelAction::KToolBarLabelAction(const QString &text, QObject *parent)
    : QWidgetAction(parent)
    , d(new KToolBarLabelActionPrivate)
{
    setText(text);
    d->label = nullptr;
}

KToolBarLabelAction::KToolBarLabelAction(QAction *buddy, const QString &text, QObject *parent)
    : QWidgetAction(parent)
    , d(new KToolBarLabelActionPrivate)
{
    setBuddy(buddy);
    setText(text);

    d->label = nullptr;
}

KToolBarLabelAction::~KToolBarLabelAction() = default;

void KToolBarLabelAction::setBuddy(QAction *buddy)
{
    d->buddy = buddy;

    QList<QLabel *> labels;
    const auto associatedWidgets = this->associatedObjects();
    for (auto *widget : associatedWidgets) {
        if (QToolBar *toolBar = qobject_cast<QToolBar *>(widget)) {
            if (QLabel *label = qobject_cast<QLabel *>(toolBar->widgetForAction(this))) {
                labels.append(label);
            }
        }
    }
    const auto buddysAssociatedWidgets = buddy->associatedObjects();
    for (auto *widget : buddysAssociatedWidgets) {
        if (QToolBar *toolBar = qobject_cast<QToolBar *>(widget)) {
            QWidget *newBuddy = toolBar->widgetForAction(buddy);
            for (QLabel *label : std::as_const(labels)) {
                label->setBuddy(newBuddy);
            }
            return;
        }
    }
}

QAction *KToolBarLabelAction::buddy() const
{
    return d->buddy;
}

bool KToolBarLabelAction::event(QEvent *event)
{
    if (event->type() == QEvent::ActionChanged) {
        if (d->label && text() != d->label->text()) {
            Q_EMIT textChanged(text());
            d->label->setText(text());
        }
    }

    return QWidgetAction::event(event);
}

bool KToolBarLabelAction::eventFilter(QObject *watched, QEvent *event)
{
    if (d->label && d->buddy && event->type() == QEvent::PolishRequest && watched == d->label) {
        const auto buddysAssociatedWidgets = d->buddy->associatedObjects();
        for (auto *widget : buddysAssociatedWidgets) {
            if (QToolBar *toolBar = qobject_cast<QToolBar *>(widget)) {
                QWidget *newBuddy = toolBar->widgetForAction(d->buddy);
                d->label->setBuddy(newBuddy);
            }
        }
    }

    return QWidgetAction::eventFilter(watched, event);
}

QWidget *KToolBarLabelAction::createWidget(QWidget *_parent)
{
    QToolBar *parent = qobject_cast<QToolBar *>(_parent);
    if (!parent) {
        return QWidgetAction::createWidget(_parent);
    }
    if (!d->label) {
        d->label = new QLabel(parent);

        // These lines were copied from Konqueror's KonqDraggableLabel class in
        // konq_misc.cc
        d->label->setBackgroundRole(QPalette::Button);
        d->label->setAlignment((QApplication::isRightToLeft() ? Qt::AlignRight : Qt::AlignLeft) | Qt::AlignVCenter);
        d->label->adjustSize();
        d->label->setText(text());
        d->label->installEventFilter(this);
    }

    return d->label;
}

#include "moc_ktoolbarlabelaction.cpp"
