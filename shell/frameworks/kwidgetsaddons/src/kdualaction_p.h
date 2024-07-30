/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2010 Aurélien Gâteau <agateau@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/
#ifndef KDUALACTION_P_H
#define KDUALACTION_P_H

#include "kguiitem.h"

class KDualActionPrivate
{
public:
    KDualAction *q;

    KGuiItem items[2];
    bool autoToggle;
    bool isActive;

    void init(KDualAction *q_ptr);
    void updateFromCurrentState();
    KGuiItem &item(bool active)
    {
        return active ? items[1] : items[0];
    }
    void slotTriggered();

    void updatedItem(bool active)
    {
        if (active == isActive) {
            updateFromCurrentState();
        }
    }

    void setGuiItem(bool active, const KGuiItem &_item)
    {
        item(active) = _item;
        updatedItem(active);
    }

    void setIcon(bool active, const QIcon &icon)
    {
        item(active).setIcon(icon);
        updatedItem(active);
    }

    void setText(bool active, const QString &text)
    {
        item(active).setText(text);
        updatedItem(active);
    }

    void setToolTip(bool active, const QString &toolTip)
    {
        item(active).setToolTip(toolTip);
        updatedItem(active);
    }
};

#endif /* KDUALACTION_P_H */
