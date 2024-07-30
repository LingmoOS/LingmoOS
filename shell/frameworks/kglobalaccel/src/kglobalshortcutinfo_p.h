/*
    SPDX-FileCopyrightText: 2008 Michael Jansen <kde@michael-jansen.biz>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KGLOBALSHORTCUTINFO_P_H
#define KGLOBALSHORTCUTINFO_P_H

/**
 * @internal
 */

static const int maxSequenceLength = 4;

#include "kglobalaccel.h"
#include "kglobalshortcutinfo.h"

class KGlobalShortcutInfoPrivate
{
public:
    QString contextUniqueName;
    QString contextFriendlyName;
    QString componentUniqueName;
    QString componentFriendlyName;
    QString uniqueName;
    QString friendlyName;
    QList<QKeySequence> keys;
    QList<QKeySequence> defaultKeys;
};

#endif /* #ifndef KGLOBALSHORTCUTINFO_P_H */
