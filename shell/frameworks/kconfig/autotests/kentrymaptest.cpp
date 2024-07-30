/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2007 Thomas Braxton <kde.braxton@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kentrymaptest.h"

#include <QTest>

// clazy:excludeall=non-pod-global-static

static const QString group1{QStringLiteral("A Group")};
static const QByteArray key1{"A Key"};
static const QByteArray key2{"Another Key"};
static const QByteArray value1{"A value"};
static const QByteArray value2{"A different value"};

QTEST_MAIN(KEntryMapTest)

template<typename KeyTypeA, typename KeyTypeB>
void KEntryMapTest::testKeyOrder()
{
    const KeyTypeA groupMarkerA(group1, {});
    const KeyTypeA entryA(group1, key1);
    const KeyTypeA localizedA(group1, key1, true, false);
    const KeyTypeA localizedDefaultA(group1, key1, true, true);

    const KeyTypeB entryB(group1, key1);
    const KeyTypeB localizedB(group1, key1, true, false);
    const KeyTypeB localizedDefaultB(group1, key1, true, true);
    const KeyTypeB defaultEntryB(group1, key1, false, true);

    // group marker should come before all entries
    QVERIFY(groupMarkerA < entryB);
    QVERIFY(groupMarkerA < defaultEntryB);
    QVERIFY(groupMarkerA < localizedB);
    QVERIFY(groupMarkerA < localizedDefaultB);

    // localized should come before entry
    QVERIFY(localizedA < entryB);

    // localized-default should come after localized entry
    QVERIFY(localizedA < localizedDefaultB);

    // localized-default should come before non-localized entry
    QVERIFY(localizedDefaultA < entryB);

    // default should come after entry
    QVERIFY(entryA < defaultEntryB);
}

void KEntryMapTest::testKeyAndKeyOrder()
{
    testKeyOrder<KEntryKey, KEntryKey>();
}

void KEntryMapTest::testKeyAndKeyViewOrder()
{
    testKeyOrder<KEntryKey, KEntryKeyView>();
}

void KEntryMapTest::testKeyViewAndKeyOrder()
{
    testKeyOrder<KEntryKeyView, KEntryKey>();
}

void KEntryMapTest::testSimple()
{
    KEntryMap map;

    map.setEntry(group1, key1, value1, EntryOptions());
    QCOMPARE(map.size(), 2); // the group marker & 1 key
    map.setEntry(group1, key2, value2, EntryOptions());
    QCOMPARE(map.size(), 3); // the group marker & 2 keys

    QVERIFY(map.constFindEntry(group1) != map.cend());
    QCOMPARE(map.constFindEntry(group1.toLower()), map.cend());

    QVERIFY(map.constFindEntry(group1, key1) != map.cend());
    QCOMPARE(map.constFindEntry(group1, key1.toLower()), map.cend());
    QVERIFY(map.constFindEntry(group1, key2) != map.cend());
    QCOMPARE(map.constFindEntry(group1, key2.toUpper()), map.cend());

    QByteArray found = map.constFindEntry(group1, key1)->second.mValue;
    QCOMPARE(found, value1);
    QVERIFY(found != value2);

    found = map.constFindEntry(group1, key2)->second.mValue;
    QVERIFY(found != value1);
    QCOMPARE(found, value2);
}

void KEntryMapTest::testDirty()
{
    KEntryMap map;
    bool ret = map.setEntry(group1, key1, value1, EntryDefault);
    QCOMPARE(ret, true);
    ret = map.setEntry(group1, key1, value1, EntryDefault);
    QCOMPARE(ret, false);
    ret = map.setEntry(group1, key2, value2, EntryOptions());
    QCOMPARE(ret, true);
    ret = map.setEntry(group1, key2, value2, EntryOptions());
    QCOMPARE(ret, false);
}

void KEntryMapTest::testDefault()
{
    KEntryMap map;

    map.setEntry(group1, key1, value1, EntryDefault);
    QCOMPARE(map.size(), 3); // group marker, default, entry
    map.setEntry(group1, key2, value2, EntryOptions());
    QCOMPARE(map.size(), 4); // group marker, default1, entry1, entry2

    const auto defaultEntry(map.constFindEntry(group1, key1, SearchDefaults));
    const auto entry1(map.constFindEntry(group1, key1));
    const auto entry2(map.constFindEntry(group1, key2));

    // default set for entry1
    QVERIFY(defaultEntry != map.cend());
    QCOMPARE(defaultEntry->second.mValue, entry1->second.mValue);

    // no default set for entry2
    QCOMPARE(map.constFindEntry(group1, key2, SearchDefaults), map.cend());

    // change from default
    map.setEntry(group1, key1, value2, EntryOptions());
    QVERIFY(defaultEntry->second.mValue != entry1->second.mValue);
    QVERIFY(entry1 != entry2);
    QCOMPARE(entry1->second.mValue, entry2->second.mValue);

    // revert entry1
    map.revertEntry(group1, key1, EntryOptions());
    QCOMPARE(defaultEntry->second.mValue, entry1->second.mValue);

    // revert entry2, no default --> should be marked as deleted
    map.revertEntry(group1, key2, EntryOptions());
    QCOMPARE(entry2->second.mValue, QByteArray());
    QVERIFY(entry2->second.bDirty);
    QVERIFY(entry2->second.bReverted);
}

void KEntryMapTest::testDelete()
{
    KEntryMap map;

    map.setEntry(group1, key1, value1, EntryDefault);
    map.setEntry(group1, key2, value2, EntryDefault);
    QCOMPARE(map.size(), 5);

    map.setEntry(group1, key2, QByteArray(), EntryDeleted | EntryDirty);
    QCOMPARE(map.size(), 5); // entry should still be in map, so it can override merged entries later
    QCOMPARE(map.constFindEntry(group1, key2)->second.mValue, QByteArray());
}

void KEntryMapTest::testGlobal()
{
    KEntryMap map;

    map.setEntry(group1, key1, value1, EntryGlobal);
    QCOMPARE(map.constFindEntry(group1, key1)->second.bGlobal, true);

    // this should create a new key that is not "global"
    map.setEntry(group1, key1, value2, EntryOptions());
    QCOMPARE(map.constFindEntry(group1, key1)->second.bGlobal, false);
}

void KEntryMapTest::testImmutable()
{
    KEntryMap map;

    map.setEntry(group1, key1, value1, EntryImmutable);
    QCOMPARE(map.constFindEntry(group1, key1)->second.bImmutable, true); // verify the immutable bit was set

    map.setEntry(group1, key1, value2, EntryOptions());
    QCOMPARE(map.constFindEntry(group1, key1)->second.mValue, value1); // verify the value didn't change

    map.clear();

    map.setEntry(group1, QByteArray(), QByteArray(), EntryImmutable);
    QCOMPARE(map.constFindEntry(group1)->second.bImmutable, true); // verify the group is immutable

    map.setEntry(group1, key1, value1, EntryOptions()); // should be ignored since the group is immutable
    QCOMPARE(map.constFindEntry(group1, key1), map.cend());
}

void KEntryMapTest::testLocale()
{
    const QByteArray translatedDefault("hola");
    const QByteArray translated("bonjour");
    const QByteArray untranslated("hello");
    KEntryMap map;

    map.setEntry(group1, key1, untranslated, EntryDefault);
    QCOMPARE(map.constFindEntry(group1, key1)->second.mValue, untranslated);
    QCOMPARE(map.constFindEntry(group1, key1, SearchLocalized)->second.mValue, untranslated); // no localized value yet

    map.setEntry(group1, key1, translated, EntryLocalized);

    QCOMPARE(map.constFindEntry(group1, key1, SearchLocalized)->second.mValue, translated); // has localized value now
    QVERIFY(map.constFindEntry(group1, key1, SearchLocalized)->second.mValue != map.constFindEntry(group1, key1)->second.mValue);
    QCOMPARE(map.constFindEntry(group1, key1, SearchDefaults | SearchLocalized)->second.mValue, untranslated); // default should still be untranslated

    map.setEntry(group1, key1, translatedDefault, EntryDefault | EntryLocalized);
    QCOMPARE(map.constFindEntry(group1, key1, SearchLocalized)->second.mValue, translatedDefault);
    map.setEntry(group1, key1, translated, EntryLocalized); // set the translated entry to a different locale
    QCOMPARE(map.constFindEntry(group1, key1, SearchLocalized)->second.mValue, translated);
}

#include "moc_kentrymaptest.cpp"
