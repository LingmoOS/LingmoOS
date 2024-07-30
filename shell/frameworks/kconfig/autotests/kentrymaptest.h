/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2007 Thomas Braxton <kde.braxton@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KENTRYMAPTEST_H
#define KENTRYMAPTEST_H

#include "kconfigdata_p.h"
#include <QObject>

class KEntryMapTest : public QObject
{
    Q_OBJECT

public:
    typedef KEntryMap::EntryOptions EntryOptions;
    typedef KEntryMap::SearchFlags SearchFlags;

    typedef KEntryMap::SearchFlag SearchFlag;
    static const SearchFlag SearchLocalized = KEntryMap::SearchLocalized;
    static const SearchFlag SearchDefaults = KEntryMap::SearchDefaults;

    typedef KEntryMap::EntryOption EntryOption;
    static const EntryOption EntryDirty = KEntryMap::EntryDirty;
    static const EntryOption EntryGlobal = KEntryMap::EntryGlobal;
    static const EntryOption EntryImmutable = KEntryMap::EntryImmutable;
    static const EntryOption EntryDeleted = KEntryMap::EntryDeleted;
    static const EntryOption EntryExpansion = KEntryMap::EntryExpansion;
    static const EntryOption EntryDefault = KEntryMap::EntryDefault;
    static const EntryOption EntryLocalized = KEntryMap::EntryLocalized;

private:
    template<typename KeyTypeA, typename KeyTypeB>
    void testKeyOrder();

private Q_SLOTS:
    void testKeyAndKeyOrder();
    void testKeyAndKeyViewOrder();
    void testKeyViewAndKeyOrder();
    void testSimple();
    void testDirty();
    void testDefault();
    void testDelete();
    void testGlobal();
    void testImmutable();
    void testLocale();
};

#endif // KENTRYMAPTEST_H
