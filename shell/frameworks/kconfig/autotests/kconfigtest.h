/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1997 Matthias Kalle Dalheimer <kalle@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCONFIGTEST_H
#define KCONFIGTEST_H

#include <QObject>

class KConfigTest : public QObject
{
    Q_OBJECT

public:
    enum Testing {
        Ones = 1,
        Tens = 10,
        Hundreds = 100,
    };
    Q_ENUM(Testing)
    enum bits {
        bit0 = 1,
        bit1 = 2,
        bit2 = 4,
        bit3 = 8,
    };
    Q_DECLARE_FLAGS(Flags, bits)
    Q_FLAG(Flags)

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testSimple();
    void testDefaults();
    void testLists();
    void testLocale();
    void testEncoding();
    void testPath();
    void testPathQtHome();
    void testPersistenceOfExpandFlagForPath();
    void testComplex();
    void testEnums();
    void testEntryMap();
    void testInvalid();
    void testDeleteEntry();
    void testDelete();
    void testDeleteWhenLocalized();
    void testDefaultGroup();
    void testEmptyGroup();
    void testCascadingWithLocale();
    void testMerge();
    void testImmutable();
    void testGroupEscape();
    void testRevertAllEntries();
    void testChangeGroup();
    void testGroupCopyTo();
    void testConfigCopyTo();
    void testConfigCopyToSync();
    void testReparent();
    void testAnonymousConfig();
    void testQByteArrayUtf8();
    void testQStringUtf8_data();
    void testQStringUtf8();

    void testMoveValuesTo();
    void testMoveAllValuesTo();

    void testSubGroup();
    void testAddConfigSources();
    void testWriteOnSync();
    void testFailOnReadOnlyFileSync();
    void testDirtyOnEqual();
    void testDirtyOnEqualOverdo();
    void testCreateDir();
    void testSharedConfig();
    void testOptionOrder();
    void testLocaleConfig();
    void testDirtyAfterRevert();
    void testKdeGlobals();
    void testLocalDeletion();
    void testNewlines();
    void testXdgListEntry();
    void testNotify();
    void testNotifyIllegalObjectPath();
    void testKAuthorizeEnums();

    void testThreads();

    void testKdeglobalsVsDefault();

    // should be last
    void testSyncOnExit();

private:
    QString m_testConfigDir;
    QString m_kdeGlobalsPath;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(KConfigTest::Flags)

#endif /* KCONFIGTEST_H */
