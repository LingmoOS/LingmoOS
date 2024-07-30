/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 Reginald Stadlbauer <reggie@kde.org>
    SPDX-FileCopyrightText: 1999 Simon Hausmann <hausmann@kde.org>
    SPDX-FileCopyrightText: 2000 Nicolas Hadacek <haadcek@kde.org>
    SPDX-FileCopyrightText: 2000 Kurt Granroth <granroth@kde.org>
    SPDX-FileCopyrightText: 2000 Michael Koch <koch@kde.org>
    SPDX-FileCopyrightText: 2001 Holger Freyther <freyther@kde.org>
    SPDX-FileCopyrightText: 2002 Ellis Whitehead <ellis@kde.org>
    SPDX-FileCopyrightText: 2002 Joseph Wenninger <jowenn@kde.org>
    SPDX-FileCopyrightText: 2003 Andras Mantia <amantia@kde.org>
    SPDX-FileCopyrightText: 2005-2006 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "ktogglefullscreenaction.h"

#include "ktoggleaction_p.h"

#include <QEvent>
#include <QWidget>

class KToggleFullScreenActionPrivate : public KToggleActionPrivate
{
    Q_DECLARE_PUBLIC(KToggleFullScreenAction)

public:
    KToggleFullScreenActionPrivate(KToggleFullScreenAction *qq)
        : KToggleActionPrivate(qq)
    {
    }

    void updateTextsAndIcon()
    {
        Q_Q(KToggleFullScreenAction);

        q->setText(KToggleFullScreenAction::tr("F&ull Screen Mode", "@action:inmenu"));
        q->setIconText(KToggleFullScreenAction::tr("Full Screen", "@action:intoolbar"));
        q->setToolTip(KToggleFullScreenAction::tr("Display the window in full screen", "@info:tooltip"));
        q->setIcon(QIcon::fromTheme(QStringLiteral("view-fullscreen")));
    }

    QWidget *window = nullptr;
};

KToggleFullScreenAction::KToggleFullScreenAction(QObject *parent)
    : KToggleAction(*new KToggleFullScreenActionPrivate(this), parent)
{
    Q_D(KToggleFullScreenAction);

    d->updateTextsAndIcon();
}

KToggleFullScreenAction::KToggleFullScreenAction(QWidget *window, QObject *parent)
    : KToggleAction(*new KToggleFullScreenActionPrivate(this), parent)
{
    Q_D(KToggleFullScreenAction);

    d->updateTextsAndIcon();
    setWindow(window);
}

KToggleFullScreenAction::~KToggleFullScreenAction() = default;

void KToggleFullScreenAction::setWindow(QWidget *window)
{
    Q_D(KToggleFullScreenAction);

    if (d->window) {
        d->window->removeEventFilter(this);
    }

    d->window = window;

    if (d->window) {
        d->window->installEventFilter(this);
    }
}

void KToggleFullScreenAction::slotToggled(bool checked)
{
    KToggleAction::slotToggled(checked);
}

bool KToggleFullScreenAction::eventFilter(QObject *object, QEvent *event)
{
    Q_D(KToggleFullScreenAction);

    if (object == d->window) {
        if (event->type() == QEvent::WindowStateChange) {
            if (d->window->isFullScreen() != isChecked()) {
                activate(QAction::Trigger);
            }
        }
    }

    return false;
}

void KToggleFullScreenAction::setFullScreen(QWidget *window, bool set)
{
    if (set) {
        window->setWindowState(window->windowState() | Qt::WindowFullScreen);
    } else {
        window->setWindowState(window->windowState() & ~Qt::WindowFullScreen);
    }
}

#include "moc_ktogglefullscreenaction.cpp"
