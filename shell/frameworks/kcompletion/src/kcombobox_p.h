/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2000, 2001 Dawit Alemayehu <adawit@kde.org>
    SPDX-FileCopyrightText: 2000, 2001 Carsten Pfeiffer <pfeiffer@kde.org>
    SPDX-FileCopyrightText: 2000 Stefan Schimanski <1Stein@gmx.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KCOMBOBOX_P_H
#define KCOMBOBOX_P_H

#include "kcombobox.h"

#include "klineedit.h"

#include <QMenu>
#include <QPointer>

class KComboBoxPrivate
{
    Q_DECLARE_PUBLIC(KComboBox)

public:
    explicit KComboBoxPrivate(KComboBox *q)
        : q_ptr(q)
    {
    }
    virtual ~KComboBoxPrivate() = default;

    /**
     * Initializes the variables upon construction.
     */
    void init();

    void slotLineEditDeleted(QLineEdit *sender);

    KComboBox *const q_ptr;

    KLineEdit *klineEdit = nullptr;
    bool trapReturnKey = false;
    QPointer<QMenu> contextMenu;
    QMetaObject::Connection m_klineEditConnection;
};

#endif
