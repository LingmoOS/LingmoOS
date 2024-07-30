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

#ifndef KTOGGLEACTION_P_H
#define KTOGGLEACTION_P_H

#include "kguiitem.h"
#include "ktoggleaction.h"

class KToggleActionPrivate
{
    Q_DECLARE_PUBLIC(KToggleAction)

public:
    explicit KToggleActionPrivate(KToggleAction *qq)
        : q_ptr(qq)
    {
    }

    virtual ~KToggleActionPrivate()
    {
        delete checkedGuiItem;
    }

    void init()
    {
        Q_Q(KToggleAction);

        q->setCheckable(true);
        QObject::connect(q, &QAction::toggled, q, &KToggleAction::slotToggled);
    }

    KToggleAction *const q_ptr;
    KGuiItem *checkedGuiItem = nullptr;
};

#endif
