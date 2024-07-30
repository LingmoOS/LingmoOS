/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2006, 2007 Thomas Braxton <kde.braxton@gmail.com>
    SPDX-FileCopyrightText: 1999-2000 Preston Brown <pbrown@kde.org>
    SPDX-FileCopyrightText: 1996-2000 Matthias Kalle Dalheimer <kalle@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kconfigdata_p.h"

QDebug operator<<(QDebug dbg, const KEntryKey &key)
{
    dbg.nospace() << "[" << key.mGroup << ", " << key.mKey << (key.bLocal ? " localized" : "") << (key.bDefault ? " default" : "") << (key.bRaw ? " raw" : "")
                  << "]";
    return dbg.space();
}

QDebug operator<<(QDebug dbg, const KEntry &entry)
{
    dbg.nospace() << "[" << entry.mValue << (entry.bDirty ? " dirty" : "") << (entry.bGlobal ? " global" : "")
                  << (entry.bOverridesGlobal ? " overrides global" : "") << (entry.bImmutable ? " immutable" : "") << (entry.bDeleted ? " deleted" : "")
                  << (entry.bReverted ? " reverted" : "") << (entry.bExpand ? " expand" : "") << "]";

    return dbg.space();
}

KEntryMapIterator KEntryMap::findExactEntry(const QString &group, QAnyStringView key, KEntryMap::SearchFlags flags)
{
    const KEntryKeyView theKey(group, key, bool(flags & SearchLocalized), bool(flags & SearchDefaults));
    return find(theKey);
}

KEntryMapIterator KEntryMap::findEntry(const QString &group, QAnyStringView key, KEntryMap::SearchFlags flags)
{
    KEntryKeyView theKey(group, key, false, bool(flags & SearchDefaults));

    // try the localized key first
    if (flags & SearchLocalized) {
        theKey.bLocal = true;

        iterator it = find(theKey);
        if (it != end()) {
            return it;
        }

        theKey.bLocal = false;
    }
    return find(theKey);
}

KEntryMapConstIterator KEntryMap::constFindEntry(const QString &group, QAnyStringView key, SearchFlags flags) const
{
    KEntryKeyView theKey(group, key, false, bool(flags & SearchDefaults));

    // try the localized key first
    if (flags & SearchLocalized) {
        theKey.bLocal = true;

        auto it = find(theKey);
        if (it != cend()) {
            return it;
        }

        theKey.bLocal = false;
    }

    return find(theKey);
}

bool KEntryMap::setEntry(const QString &group, const QByteArray &key, const QByteArray &value, KEntryMap::EntryOptions options)
{
    KEntryKey k;
    KEntry e;
    bool newKey = false;

    const iterator it = findExactEntry(group, key, SearchFlags(options >> 16));

    if (key.isEmpty()) { // inserting a group marker
        k.mGroup = group;
        e.bImmutable = (options & EntryImmutable);
        if (options & EntryDeleted) {
            qWarning("Internal KConfig error: cannot mark groups as deleted");
        }
        if (it == end()) {
            insert_or_assign(k, e);
            return true;
        } else if (it->second == e) {
            return false;
        }

        it->second = e;
        return true;
    }

    if (it != end()) {
        if (it->second.bImmutable) {
            return false; // we cannot change this entry. Inherits group immutability.
        }
        k = it->first;
        e = it->second;
        // qDebug() << "found existing entry for key" << k;
        // If overridden entry is global and not default. And it's overridden by a non global
        if (e.bGlobal && !(options & EntryGlobal) && !k.bDefault) {
            e.bOverridesGlobal = true;
        }
    } else {
        // make sure the group marker is in the map
        KEntryMap const *that = this;
        auto cit = that->constFindEntry(group);
        if (cit == cend()) {
            insert_or_assign(KEntryKey(group), KEntry());
        } else if (cit->second.bImmutable) {
            return false; // this group is immutable, so we cannot change this entry.
        }

        k = KEntryKey(group, key);
        newKey = true;
    }

    // set these here, since we may be changing the type of key from the one we found
    k.bLocal = (options & EntryLocalized);
    k.bDefault = (options & EntryDefault);
    k.bRaw = (options & EntryRawKey);

    e.mValue = value;
    e.bDirty = e.bDirty || (options & EntryDirty);
    e.bNotify = e.bNotify || (options & EntryNotify);

    e.bGlobal = (options & EntryGlobal); // we can't use || here, because changes to entries in
    // kdeglobals would be written to kdeglobals instead
    // of the local config file, regardless of the globals flag
    e.bImmutable = e.bImmutable || (options & EntryImmutable);
    if (value.isNull()) {
        e.bDeleted = e.bDeleted || (options & EntryDeleted);
    } else {
        e.bDeleted = false; // setting a value to a previously deleted entry
    }
    e.bExpand = (options & EntryExpansion);
    e.bReverted = false;
    if (options & EntryLocalized) {
        e.bLocalizedCountry = (options & EntryLocalizedCountry);
    } else {
        e.bLocalizedCountry = false;
    }

    if (newKey) {
        // qDebug() << "inserting" << k << "=" << value;
        insert_or_assign(k, e);
        if (k.bDefault) {
            k.bDefault = false;
            // qDebug() << "also inserting" << k << "=" << value;
            insert_or_assign(k, e);
        }
        // TODO check for presence of unlocalized key
        return true;
    }

    // KEntry e2 = it->second;
    if (options & EntryLocalized) {
        // fast exit checks for cases where the existing entry is more specific
        const KEntry &e2 = it->second;
        if (e2.bLocalizedCountry && !e.bLocalizedCountry) {
            // lang_COUNTRY > lang
            return false;
        }
    }

    if (it->second != e) {
        // qDebug() << "changing" << k << "from" << it->second.mValue << "to" << value << e;
        it->second = e;
        if (k.bDefault) {
            KEntryKey nonDefaultKey(k);
            nonDefaultKey.bDefault = false;
            insert_or_assign(nonDefaultKey, e);
        }
        if (!(options & EntryLocalized)) {
            KEntryKey theKey(group, key, true, false);
            // qDebug() << "non-localized entry, remove localized one:" << theKey;
            erase(theKey);
            if (k.bDefault) {
                theKey.bDefault = true;
                erase(theKey);
            }
        }
        return true;
    }

    // qDebug() << k << "was already set to" << e.mValue;
    if (!(options & EntryLocalized)) {
        // qDebug() << "unchanged non-localized entry, remove localized one.";
        KEntryKey theKey(group, key, true, false);
        bool ret = false;
        iterator cit = find(theKey);
        if (cit != end()) {
            erase(cit);
            ret = true;
        }
        if (k.bDefault) {
            theKey.bDefault = true;
            iterator cit = find(theKey);
            if (cit != end()) {
                erase(cit);
                return true;
            }
        }
        return ret;
    }

    // qDebug() << "localized entry, unchanged, return false";
    // When we are writing a default, we know that the non-
    // default is the same as the default, so we can simply
    // use the same branch.
    return false;
}

QString KEntryMap::getEntry(const QString &group, QAnyStringView key, const QString &defaultValue, KEntryMap::SearchFlags flags, bool *expand) const
{
    const auto it = constFindEntry(group, key, flags);
    QString theValue = defaultValue;

    if (it != cend() && !it->second.bDeleted) {
        if (!it->second.mValue.isNull()) {
            const QByteArray data = it->second.mValue;
            theValue = QString::fromUtf8(data.constData(), data.length());
            if (expand) {
                *expand = it->second.bExpand;
            }
        }
    }

    return theValue;
}

bool KEntryMap::hasEntry(const QString &group, QAnyStringView key, KEntryMap::SearchFlags flags) const
{
    const auto it = constFindEntry(group, key, flags);
    if (it == cend()) {
        return false;
    }
    if (it->second.bDeleted) {
        return false;
    }
    if (key.isNull()) { // looking for group marker
        return it->second.mValue.isNull();
    }
    // if it->bReverted, we'll just return true; the real answer depends on lookup up with SearchDefaults, though.
    return true;
}

bool KEntryMap::getEntryOption(const KEntryMapConstIterator &it, KEntryMap::EntryOption option) const
{
    if (it == cend()) {
        return false;
    }

    switch (option) {
    case EntryDirty:
        return it->second.bDirty;
    case EntryLocalized:
        return it->first.bLocal;
    case EntryGlobal:
        return it->second.bGlobal;
    case EntryImmutable:
        return it->second.bImmutable;
    case EntryDeleted:
        return it->second.bDeleted;
    case EntryExpansion:
        return it->second.bExpand;
    case EntryNotify:
        return it->second.bNotify;
    default:
        return false;
    }
}

void KEntryMap::setEntryOption(KEntryMapIterator it, KEntryMap::EntryOption option, bool bf)
{
    if (it == end()) {
        return;
    }

    switch (option) {
    case EntryDirty:
        it->second.bDirty = bf;
        return;
    case EntryGlobal:
        it->second.bGlobal = bf;
        return;
    case EntryImmutable:
        it->second.bImmutable = bf;
        return;
    case EntryDeleted:
        it->second.bDeleted = bf;
        return;
    case EntryExpansion:
        it->second.bExpand = bf;
        return;
    case EntryNotify:
        it->second.bNotify = bf;
        return;
    default:
        return; // fall through
    }
}

bool KEntryMap::revertEntry(const QString &group, QAnyStringView key, KEntryMap::EntryOptions options, KEntryMap::SearchFlags flags)
{
    Q_ASSERT((flags & KEntryMap::SearchDefaults) == 0);
    iterator entry = findEntry(group, key, flags);
    if (entry == end()) {
        return false;
    }

    // qDebug() << "reverting" << entry->first << " = " << entry->mValue;
    if (entry->second.bReverted) { // already done before
        return false;
    }

    KEntryKey defaultKey(entry->first);
    defaultKey.bDefault = true;
    // qDebug() << "looking up default entry with key=" << defaultKey;
    const auto defaultEntry = find(defaultKey);
    if (defaultEntry != cend()) {
        Q_ASSERT(defaultEntry->first.bDefault);
        // qDebug() << "found, update entry";
        entry->second = defaultEntry->second; // copy default value, for subsequent lookups
    } else {
        entry->second.mValue = QByteArray();
    }
    entry->second.bNotify = entry->second.bNotify || (options & EntryNotify);
    entry->second.bDirty = true;
    entry->second.bReverted = true; // skip it when writing out to disk

    // qDebug() << "Here's what we have now:" << *this;
    return true;
}
