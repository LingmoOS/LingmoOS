/*
    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>
*/

#ifndef SHORTCUTINHIBITION_H
#define SHORTCUTINHIBITION_H

class QWindow;

class ShortcutInhibition
{
public:
    virtual ~ShortcutInhibition()
    {
    }
    virtual void enableInhibition() = 0;
    virtual void disableInhibition() = 0;
    virtual bool shortcutsAreInhibited() const = 0;
};

#endif
