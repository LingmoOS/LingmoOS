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

#include "ktoggleaction.h"
#include "ktoggleaction_p.h"

KToggleAction::KToggleAction(QObject *parent)
    : KToggleAction(*new KToggleActionPrivate(this), parent)
{
}

KToggleAction::KToggleAction(KToggleActionPrivate &dd, QObject *parent)
    : QAction(parent)
    , d_ptr(&dd)
{
    Q_D(KToggleAction);

    d->init();
}

KToggleAction::KToggleAction(const QString &text, QObject *parent)
    : QAction(parent)
    , d_ptr(new KToggleActionPrivate(this))
{
    Q_D(KToggleAction);

    setText(text);
    d->init();
}

KToggleAction::KToggleAction(const QIcon &icon, const QString &text, QObject *parent)
    : QAction(parent)
    , d_ptr(new KToggleActionPrivate(this))
{
    Q_D(KToggleAction);

    setIcon(icon);
    setText(text);
    d->init();
}

KToggleAction::~KToggleAction() = default;

void KToggleAction::setCheckedState(const KGuiItem &checkedItem)
{
    Q_D(KToggleAction);

    delete d->checkedGuiItem;
    d->checkedGuiItem = new KGuiItem(checkedItem);
}

void KToggleAction::slotToggled(bool)
{
    Q_D(KToggleAction);

    if (d->checkedGuiItem) {
        QString string = d->checkedGuiItem->text();
        d->checkedGuiItem->setText(text());
        setText(string);

        string = d->checkedGuiItem->toolTip();
        d->checkedGuiItem->setToolTip(toolTip());
        setToolTip(string);

        if (d->checkedGuiItem->hasIcon()) {
            QIcon icon = d->checkedGuiItem->icon();
            d->checkedGuiItem->setIcon(this->icon());
            QAction::setIcon(icon);
        }
    }
}

#include "moc_ktoggleaction.cpp"
