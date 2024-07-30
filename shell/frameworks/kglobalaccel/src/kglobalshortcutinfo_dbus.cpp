/*
    SPDX-FileCopyrightText: 2008 Michael Jansen <kde@michael-jansen.biz>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kglobalshortcutinfo.h"
#include "kglobalshortcutinfo_p.h"

QDBusArgument &operator<<(QDBusArgument &argument, const QKeySequence &sequence)
{
    argument.beginStructure();
    argument.beginArray(qMetaTypeId<int>());
    for (int i = 0; i < maxSequenceLength; i++) {
        argument << (i < sequence.count() ? sequence[i].toCombined() : 0);
    }
    argument.endArray();
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, QKeySequence &sequence)
{
    int s1;
    int s2;
    int s3;
    int s4;
    argument.beginStructure();
    argument.beginArray();
    argument >> s1 >> s2 >> s3 >> s4;
    sequence = QKeySequence(s1, s2, s3, s4);
    argument.endArray();
    argument.endStructure();
    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const KGlobalShortcutInfo &shortcut)
{
    argument.beginStructure();
    /* clang-format off */
    argument << shortcut.uniqueName()
             << shortcut.friendlyName()
             << shortcut.componentUniqueName()
             << shortcut.componentFriendlyName()
             << shortcut.contextUniqueName()
             << shortcut.contextFriendlyName();
    /* clang-format on */
    argument.beginArray(qMetaTypeId<int>());

    const QList<QKeySequence> keys = shortcut.keys();
    for (const QKeySequence &key : keys) {
        argument << key[0].toCombined();
    }
    argument.endArray();
    argument.beginArray(qMetaTypeId<int>());

    const QList<QKeySequence> defaultKeys = shortcut.defaultKeys();
    for (const QKeySequence &key : defaultKeys) {
        argument << key[0].toCombined();
    }
    argument.endArray();
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, KGlobalShortcutInfo &shortcut)
{
    argument.beginStructure();
    /* clang-format off */
    argument >> shortcut.d->uniqueName
             >> shortcut.d->friendlyName
             >> shortcut.d->componentUniqueName
             >> shortcut.d->componentFriendlyName
             >> shortcut.d->contextUniqueName
             >> shortcut.d->contextFriendlyName;
    /* clang-format on */

    argument.beginArray();
    while (!argument.atEnd()) {
        int key;
        argument >> key;
        shortcut.d->keys.append(QKeySequence(key));
    }
    argument.endArray();
    argument.beginArray();
    while (!argument.atEnd()) {
        int key;
        argument >> key;
        shortcut.d->defaultKeys.append(QKeySequence(key));
    }
    argument.endArray();
    argument.endStructure();
    return argument;
}
