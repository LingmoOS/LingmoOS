/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1997 Matthias Kalle Dalheimer <kalle@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kconfigtest.h"
#include "helper.h"

#include "config-kconfig.h"

#include <QSignalSpy>
#include <QStandardPaths>
#include <QTemporaryFile>
#include <QTest>
#include <kdesktopfile.h>
#include <qtemporarydir.h>

#include <kauthorized.h>
#include <kconfiggroup.h>
#include <kconfigwatcher.h>
#include <ksharedconfig.h>

#ifdef Q_OS_UNIX
#include <utime.h>
#endif
#ifndef Q_OS_WIN
#include <unistd.h> // getuid
#endif

KCONFIGGROUP_DECLARE_ENUM_QOBJECT(KConfigTest, Testing)
KCONFIGGROUP_DECLARE_FLAGS_QOBJECT(KConfigTest, Flags)

QTEST_MAIN(KConfigTest)

Q_DECLARE_METATYPE(KConfigGroup)

static QString homePath()
{
#ifdef Q_OS_WIN
    return QDir::homePath();
#else
    // Don't use QDir::homePath() on Unix, it removes any trailing slash, while KConfig uses $HOME.
    return QString::fromLocal8Bit(qgetenv("HOME"));
#endif
}

// clazy:excludeall=non-pod-global-static

static const bool s_bool_entry1 = true;
static const bool s_bool_entry2 = false;

static const QString s_string_entry1(QStringLiteral("hello"));
static const QString s_string_entry2(QStringLiteral(" hello"));
static const QString s_string_entry3(QStringLiteral("hello "));
static const QString s_string_entry4(QStringLiteral(" hello "));
static const QString s_string_entry5(QStringLiteral(" "));
static const QString s_string_entry6{};

static const char s_utf8bit_entry[] = "Hello äöü";
static const QString s_translated_string_entry1{QStringLiteral("bonjour")};
static const QByteArray s_bytearray_entry{"\x00\xff\x7f\x3c abc\x00\x00", 10};
static const char s_escapekey[] = " []\0017[]==]";
static const char s_escape_entry[] = "[]\170[]]=3=]\\] ";
static const double s_double_entry{123456.78912345};
static const float s_float_entry{123.567f};
static const QPoint s_point_entry{4351, 1235};
static const QSize s_size_entry{10, 20};
static const QRect s_rect_entry{10, 23, 5321, 13};
static const QDateTime s_date_time_entry{QDate{2002, 06, 23}, QTime{12, 55, 40}};
static const QDateTime s_date_time_with_ms_entry{QDate{2002, 06, 23}, QTime{12, 55, 40, 532}};
static const QStringList s_stringlist_entry{QStringLiteral("Hello,"), QStringLiteral("World")};
static const QStringList s_stringlist_empty_entry{};
static const QStringList s_stringlist_just_empty_element{QString{}};
static const QStringList s_stringlist_empty_trailing_element{QStringLiteral("Hi"), QString{}};
static const QStringList s_stringlist_escape_odd_entry{QStringLiteral("Hello\\\\\\"), QStringLiteral("World")};
static const QStringList s_stringlist_escape_even_entry{QStringLiteral("Hello\\\\\\\\"), QStringLiteral("World")};
static const QStringList s_stringlist_escape_comma_entry{QStringLiteral("Hel\\\\\\,\\\\,\\,\\\\\\\\,lo"), QStringLiteral("World")};
static const QList<int> s_int_listentry1{1, 2, 3, 4};
static const QList<QByteArray> s_bytearray_list_entry1{"", "1,2", "end"};
static const QVariantList s_variantlist_entry{true, false, QStringLiteral("joe"), 10023};
static const QVariantList s_variantlist_entry2{s_point_entry, s_size_entry};

static const QString s_homepath{homePath() + QLatin1String{"/foo"}};
static const QString s_homepath_escape{homePath() + QLatin1String("/foo/$HOME")};
static const QString s_canonical_homepath{QFileInfo(homePath()).canonicalFilePath() + QLatin1String("/foo")};
static const QString s_dollargroup{QStringLiteral("$i")};
static const QString s_test_subdir{QStringLiteral("kconfigtest_subdir/")};
static const QString s_kconfig_test_subdir(s_test_subdir + QLatin1String("kconfigtest"));
static const QString s_kconfig_test_illegal_object_path(s_test_subdir + QLatin1String("kconfig-test"));

#ifndef Q_OS_WIN
void initLocale()
{
    setenv("LC_ALL", "en_US.utf-8", 1);
    setenv("TZ", "UTC", 1);
}

Q_CONSTRUCTOR_FUNCTION(initLocale)
#endif

void KConfigTest::initTestCase()
{
    // ensure we don't use files in the real config directory
    QStandardPaths::setTestModeEnabled(true);

    qRegisterMetaType<KConfigGroup>();

    // These two need to be assigned here, after setTestModeEnabled(true), and before cleanupTestCase()
    m_testConfigDir = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QLatin1Char('/') + s_test_subdir;
    m_kdeGlobalsPath = QDir::cleanPath(m_testConfigDir + QLatin1String("..")) + QLatin1String("/kdeglobals");

    // to make sure all files from a previous failed run are deleted
    cleanupTestCase();

    KSharedConfigPtr mainConfig = KSharedConfig::openConfig();
    mainConfig->group(QStringLiteral("Main")).writeEntry("Key", "Value");
    mainConfig->sync();

    KConfig sc(s_kconfig_test_subdir);

    KConfigGroup cg(&sc, QStringLiteral("AAA")); // deleted later by testDelete
    cg.writeEntry("stringEntry1", s_string_entry1, KConfig::Persistent | KConfig::Global);

    cg = KConfigGroup(&sc, QStringLiteral("GlobalGroup"));
    cg.writeEntry("globalEntry", s_string_entry1, KConfig::Persistent | KConfig::Global);
    cg.deleteEntry("globalEntry2", KConfig::Global);

    cg = KConfigGroup(&sc, QStringLiteral("LocalGroupToBeDeleted")); // deleted later by testDelete
    cg.writeEntry("stringEntry1", s_string_entry1);

    cg = KConfigGroup(&sc, QStringLiteral("Hello"));
    cg.writeEntry("boolEntry1", s_bool_entry1);
    cg.writeEntry("boolEntry2", s_bool_entry2);

    QByteArray data(s_utf8bit_entry);
    QCOMPARE(data.size(), 12); // the source file is in utf8
    QCOMPARE(QString::fromUtf8(data).length(), 9);
    cg.writeEntry("Test", data);
    cg.writeEntry("bytearrayEntry", s_bytearray_entry);
    cg.writeEntry(s_escapekey, QString::fromLatin1(s_escape_entry));
    cg.writeEntry("emptyEntry", "");
    cg.writeEntry("stringEntry1", s_string_entry1);
    cg.writeEntry("stringEntry2", s_string_entry2);
    cg.writeEntry("stringEntry3", s_string_entry3);
    cg.writeEntry("stringEntry4", s_string_entry4);
    cg.writeEntry("stringEntry5", s_string_entry5);
    cg.writeEntry("urlEntry1", QUrl(QStringLiteral("http://qt-project.org")));
    cg.writeEntry("keywith=equalsign", s_string_entry1);
    cg.deleteEntry("stringEntry5");
    cg.deleteEntry("stringEntry6"); // deleting a nonexistent entry
    cg.writeEntry("byteArrayEntry1", s_string_entry1.toLatin1(), KConfig::Global | KConfig::Persistent);
    cg.writeEntry("doubleEntry1", s_double_entry);
    cg.writeEntry("floatEntry1", s_float_entry);

    sc.deleteGroup(QStringLiteral("deleteMe")); // deleting a nonexistent group

    cg = KConfigGroup(&sc, QStringLiteral("Complex Types"));
    cg.writeEntry("rectEntry", s_rect_entry);
    cg.writeEntry("pointEntry", s_point_entry);
    cg.writeEntry("sizeEntry", s_size_entry);
    cg.writeEntry("dateTimeEntry", s_date_time_entry);
    cg.writeEntry("dateEntry", s_date_time_entry.date());
    cg.writeEntry("dateTimeWithMSEntry", s_date_time_with_ms_entry);

    KConfigGroup ct = cg;
    cg = KConfigGroup(&ct, QStringLiteral("Nested Group 1"));
    cg.writeEntry("stringentry1", s_string_entry1);

    cg = KConfigGroup(&ct, QStringLiteral("Nested Group 2"));
    cg.writeEntry("stringEntry2", s_string_entry2);

    cg = KConfigGroup(&cg, QStringLiteral("Nested Group 2.1"));
    cg.writeEntry("stringEntry3", s_string_entry3);

    cg = KConfigGroup(&ct, QStringLiteral("Nested Group 3"));
    cg.writeEntry("stringEntry3", s_string_entry3);

    cg = KConfigGroup(&sc, QStringLiteral("List Types"));
    cg.writeEntry("listOfIntsEntry1", s_int_listentry1);
    cg.writeEntry("listOfByteArraysEntry1", s_bytearray_list_entry1);
    cg.writeEntry("stringListEntry", s_stringlist_entry);
    cg.writeEntry("stringListEmptyEntry", s_stringlist_empty_entry);
    cg.writeEntry("stringListJustEmptyElement", s_stringlist_just_empty_element);
    cg.writeEntry("stringListEmptyTrailingElement", s_stringlist_empty_trailing_element);
    cg.writeEntry("stringListEscapeOddEntry", s_stringlist_escape_odd_entry);
    cg.writeEntry("stringListEscapeEvenEntry", s_stringlist_escape_even_entry);
    cg.writeEntry("stringListEscapeCommaEntry", s_stringlist_escape_comma_entry);
    cg.writeEntry("variantListEntry", s_variantlist_entry);

    cg = KConfigGroup(&sc, QStringLiteral("Path Type"));
    cg.writePathEntry("homepath", s_homepath);
    cg.writePathEntry("homepathescape", s_homepath_escape);
    cg.writePathEntry("canonicalHomePath", s_canonical_homepath);

    cg = KConfigGroup(&sc, QStringLiteral("Enum Types"));
#if defined(_MSC_VER) && _MSC_VER == 1600
    cg.writeEntry("dummy", 42);
#else
    // Visual C++ 2010 throws an Internal Compiler Error here
    cg.writeEntry("enum-10", Tens);
    cg.writeEntry("enum-100", Hundreds);
    cg.writeEntry("flags-bit0", Flags(bit0));
    cg.writeEntry("flags-bit0-bit1", Flags(bit0 | bit1));
#endif

    cg = KConfigGroup(&sc, QStringLiteral("ParentGroup"));
    KConfigGroup cg1(&cg, QStringLiteral("SubGroup1"));
    cg1.writeEntry("somestring", "somevalue");
    cg.writeEntry("parentgrpstring", "somevalue");
    KConfigGroup cg2(&cg, QStringLiteral("SubGroup2"));
    cg2.writeEntry("substring", "somevalue");
    KConfigGroup cg3(&cg, QStringLiteral("SubGroup/3"));
    cg3.writeEntry("sub3string", "somevalue");

    QVERIFY(sc.isDirty());
    QVERIFY(sc.sync());
    QVERIFY(!sc.isDirty());

    QVERIFY2(QFile::exists(m_testConfigDir + QLatin1String("/kconfigtest")), qPrintable(m_testConfigDir + QLatin1String("/kconfigtest must exist")));
    QVERIFY2(QFile::exists(m_kdeGlobalsPath), qPrintable(m_kdeGlobalsPath + QStringLiteral(" must exist")));

    KConfig sc1(s_test_subdir + QLatin1String("kdebugrc"), KConfig::SimpleConfig);
    KConfigGroup sg0(&sc1, QStringLiteral("0"));
    sg0.writeEntry("AbortFatal", false);
    sg0.writeEntry("WarnOutput", 0);
    sg0.writeEntry("FatalOutput", 0);
    QVERIFY(sc1.sync());

    // Setup stuff to test KConfig::addConfigSources()
    KConfig devcfg(s_test_subdir + QLatin1String("specificrc"));
    KConfigGroup devonlygrp(&devcfg, QStringLiteral("Specific Only Group"));
    devonlygrp.writeEntry("ExistingEntry", "DevValue");
    KConfigGroup devandbasegrp(&devcfg, QStringLiteral("Shared Group"));
    devandbasegrp.writeEntry("SomeSharedEntry", "DevValue");
    devandbasegrp.writeEntry("SomeSpecificOnlyEntry", "DevValue");
    QVERIFY(devcfg.sync());
    KConfig basecfg(s_test_subdir + QLatin1String("baserc"));
    KConfigGroup basegrp(&basecfg, QStringLiteral("Base Only Group"));
    basegrp.writeEntry("ExistingEntry", "BaseValue");
    KConfigGroup baseanddevgrp(&basecfg, QStringLiteral("Shared Group"));
    baseanddevgrp.writeEntry("SomeSharedEntry", "BaseValue");
    baseanddevgrp.writeEntry("SomeBaseOnlyEntry", "BaseValue");
    QVERIFY(basecfg.sync());

    KConfig gecfg(s_test_subdir + QLatin1String("groupescapetest"), KConfig::SimpleConfig);
    cg = KConfigGroup(&gecfg, s_dollargroup);
    cg.writeEntry("entry", "doesntmatter");
}

void KConfigTest::cleanupTestCase()
{
    // ensure we don't delete the real directory
    QDir localConfig(m_testConfigDir);
    // qDebug() << "Erasing" << localConfig;
    if (localConfig.exists()) {
        QVERIFY(localConfig.removeRecursively());
    }
    QVERIFY(!localConfig.exists());
    if (QFile::exists(m_kdeGlobalsPath)) {
        QVERIFY(QFile::remove(m_kdeGlobalsPath));
    }
}

static QList<QByteArray> readLinesFrom(const QString &path)
{
    QFile file(path);
    const bool opened = file.open(QIODevice::ReadOnly | QIODevice::Text);
    QList<QByteArray> lines;
    if (!opened) {
        qWarning() << "Failed to open" << path;
        return lines;
    }
    QByteArray line;
    do {
        line = file.readLine();
        if (!line.isEmpty()) {
            lines.append(line);
        }
    } while (!line.isEmpty());
    return lines;
}

static const QString s_defaultArg = s_test_subdir + QLatin1String("kconfigtest");
static QList<QByteArray> readLines(const QString &fileName = s_defaultArg)
{
    const QString path = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    Q_ASSERT(!path.isEmpty());
    return readLinesFrom(path + QLatin1Char('/') + fileName);
}

// see also testDefaults, which tests reverting with a defaults (global) file available
void KConfigTest::testDirtyAfterRevert()
{
    KConfig sc(s_test_subdir + QLatin1String("kconfigtest_revert"));

    KConfigGroup cg(&sc, QStringLiteral("Hello"));
    cg.revertToDefault("does_not_exist");
    QVERIFY(!sc.isDirty());
    cg.writeEntry("Test", "Correct");
    QVERIFY(sc.isDirty());
    sc.sync();
    QVERIFY(!sc.isDirty());

    cg.revertToDefault("Test");
    QVERIFY(sc.isDirty());
    QVERIFY(sc.sync());
    QVERIFY(!sc.isDirty());

    cg.revertToDefault("Test");
    QVERIFY(!sc.isDirty());
}

void KConfigTest::testRevertAllEntries()
{
    // this tests the case were we revert (delete) all entries in a file,
    // leaving a blank file
    {
        KConfig sc(s_test_subdir + QLatin1String("konfigtest2"), KConfig::SimpleConfig);
        KConfigGroup cg(&sc, QStringLiteral("Hello"));
        cg.writeEntry("Test", "Correct");
    }

    {
        KConfig sc(s_test_subdir + QLatin1String("konfigtest2"), KConfig::SimpleConfig);
        KConfigGroup cg(&sc, QStringLiteral("Hello"));
        QCOMPARE(cg.readEntry("Test", "Default"), QStringLiteral("Correct"));
        cg.revertToDefault("Test");
    }

    KConfig sc(s_test_subdir + QLatin1String("konfigtest2"), KConfig::SimpleConfig);
    KConfigGroup cg(&sc, QStringLiteral("Hello"));
    QCOMPARE(cg.readEntry("Test", "Default"), QStringLiteral("Default"));
}

void KConfigTest::testSimple()
{
    // kdeglobals (which was created in initTestCase) must be found this way:
    const QStringList kdeglobals = QStandardPaths::locateAll(QStandardPaths::GenericConfigLocation, QStringLiteral("kdeglobals"));
    QVERIFY(!kdeglobals.isEmpty());

    KConfig sc2(s_kconfig_test_subdir);
    QCOMPARE(sc2.name(), s_test_subdir + QLatin1String{"kconfigtest"});

    // make sure groupList() isn't returning something it shouldn't
    const QStringList lstGroup = sc2.groupList();
    for (const QString &group : lstGroup) {
        QVERIFY(!group.isEmpty() && group != QLatin1String("<default>"));
        QVERIFY(!group.contains(QChar(0x1d)));
    }

    KConfigGroup sc3(&sc2, QStringLiteral("GlobalGroup"));

    QVERIFY(sc3.hasKey("globalEntry")); // from kdeglobals
    QVERIFY(!sc3.isEntryImmutable("globalEntry"));
    QCOMPARE(sc3.readEntry("globalEntry"), s_string_entry1);

    QVERIFY(!sc3.hasKey("globalEntry2"));
    QCOMPARE(sc3.readEntry("globalEntry2", QStringLiteral("bla")), QStringLiteral("bla"));

    QVERIFY(!sc3.hasDefault("globalEntry"));

    sc3 = KConfigGroup(&sc2, QStringLiteral("Hello"));
    QCOMPARE(sc3.readEntry("Test", QByteArray()), QByteArray(s_utf8bit_entry));
    QCOMPARE(sc3.readEntry("bytearrayEntry", QByteArray()), s_bytearray_entry);
    QCOMPARE(sc3.readEntry(s_escapekey), QString::fromLatin1(s_escape_entry));
    QCOMPARE(sc3.readEntry("Test", QString{}), QString::fromUtf8(s_utf8bit_entry));
    QCOMPARE(sc3.readEntry("emptyEntry" /*, QString("Fietsbel")*/), QLatin1String(""));
    QCOMPARE(sc3.readEntry("emptyEntry", QStringLiteral("Fietsbel")).isEmpty(), true);
    QCOMPARE(sc3.readEntry("stringEntry1"), s_string_entry1);
    QCOMPARE(sc3.readEntry("stringEntry2"), s_string_entry2);
    QCOMPARE(sc3.readEntry("stringEntry3"), s_string_entry3);
    QCOMPARE(sc3.readEntry("stringEntry4"), s_string_entry4);
    QVERIFY(!sc3.hasKey("stringEntry5"));
    QCOMPARE(sc3.readEntry("stringEntry5", QStringLiteral("test")), QStringLiteral("test"));
    QVERIFY(!sc3.hasKey("stringEntry6"));
    QCOMPARE(sc3.readEntry("stringEntry6", QStringLiteral("foo")), QStringLiteral("foo"));
    QCOMPARE(sc3.readEntry("urlEntry1", QUrl{}), QUrl(QStringLiteral("http://qt-project.org")));
    QCOMPARE(sc3.readEntry("boolEntry1", s_bool_entry1), s_bool_entry1);
    QCOMPARE(sc3.readEntry("boolEntry2", false), s_bool_entry2);
    QCOMPARE(sc3.readEntry("keywith=equalsign", QStringLiteral("wrong")), s_string_entry1);
    QCOMPARE(sc3.readEntry("byteArrayEntry1", QByteArray{}), s_string_entry1.toLatin1());
    QCOMPARE(sc3.readEntry("doubleEntry1", 0.0), s_double_entry);
    QCOMPARE(sc3.readEntry("floatEntry1", 0.0f), s_float_entry);
}

void KConfigTest::testDefaults()
{
    KConfig config(s_test_subdir + QLatin1String("defaulttest"), KConfig::NoGlobals);
    const QString defaultsFile = s_test_subdir + QLatin1String("defaulttest.defaults");
    KConfig defaults(defaultsFile, KConfig::SimpleConfig);
    const QString defaultsFilePath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QLatin1Char('/') + defaultsFile;

    const QString Default(QStringLiteral("Default"));
    const QString NotDefault(QStringLiteral("Not Default"));
    const QString Value1(s_string_entry1);
    const QString Value2(s_string_entry2);

    KConfigGroup group = defaults.group(QStringLiteral("any group"));
    group.writeEntry("entry1", Default);
    QVERIFY(group.sync());

    group = config.group(QStringLiteral("any group"));
    group.writeEntry("entry1", Value1);
    group.writeEntry("entry2", Value2);
    QVERIFY(group.sync());

    config.addConfigSources(QStringList{defaultsFilePath});

    config.setReadDefaults(true);
    QCOMPARE(group.readEntry("entry1", QString()), Default);
    QCOMPARE(group.readEntry("entry2", NotDefault), NotDefault); // no default for entry2

    config.setReadDefaults(false);
    QCOMPARE(group.readEntry("entry1", Default), Value1);
    QCOMPARE(group.readEntry("entry2", NotDefault), Value2);

    group.revertToDefault("entry1");
    QCOMPARE(group.readEntry("entry1", QString()), Default);
    group.revertToDefault("entry2");
    QCOMPARE(group.readEntry("entry2", QString()), QString());

    // TODO test reverting localized entries

    Q_ASSERT(config.isDirty());
    group.sync();

    // Check that everything is OK on disk, too
    KConfig reader(s_test_subdir + QLatin1String("defaulttest"), KConfig::NoGlobals);
    reader.addConfigSources(QStringList{defaultsFilePath});
    KConfigGroup readerGroup = reader.group(QStringLiteral("any group"));
    QCOMPARE(readerGroup.readEntry("entry1", QString()), Default);
    QCOMPARE(readerGroup.readEntry("entry2", QString()), QString());
}

void KConfigTest::testLocale()
{
    KConfig config(s_test_subdir + QLatin1String("kconfigtest.locales"), KConfig::SimpleConfig);
    const QString Translated(s_translated_string_entry1);
    const QString Untranslated(s_string_entry1);

    KConfigGroup group = config.group(QStringLiteral("Hello"));
    group.writeEntry("stringEntry1", Untranslated);
    config.setLocale(QStringLiteral("fr"));
    group.writeEntry("stringEntry1", Translated, KConfig::Localized | KConfig::Persistent);
    QVERIFY(config.sync());

    QCOMPARE(group.readEntry("stringEntry1", QString()), Translated);
    QCOMPARE(group.readEntryUntranslated("stringEntry1"), Untranslated);

    config.setLocale(QStringLiteral("C")); // strings written in the "C" locale are written as nonlocalized
    group.writeEntry("stringEntry1", Untranslated, KConfig::Localized | KConfig::Persistent);
    QVERIFY(config.sync());

    QCOMPARE(group.readEntry("stringEntry1", QString()), Untranslated);
}

void KConfigTest::testEncoding()
{
    const QString groupstr = QString::fromUtf8("UTF-8:\xc3\xb6l");

    const QString path = s_test_subdir + QLatin1String("kconfigtestencodings");
    KConfig c(path);
    KConfigGroup cg(&c, groupstr);
    cg.writeEntry("key", "value");
    QVERIFY(c.sync());

    const QList<QByteArray> lines = readLines(path);
    QCOMPARE(lines.count(), 2);
    QCOMPARE(lines.first(), QByteArray("[UTF-8:\xc3\xb6l]\n"));

    KConfig c2(path);
    KConfigGroup cg2(&c2, groupstr);
    QCOMPARE(cg2.readEntry("key"), QStringLiteral("value"));

    QVERIFY(c2.groupList().contains(groupstr));
}

void KConfigTest::testLists()
{
    KConfig sc2(s_kconfig_test_subdir);
    KConfigGroup sc3(&sc2, QStringLiteral("List Types"));

    QCOMPARE(sc3.readEntry("stringListEntry", QStringList{}), s_stringlist_entry);

    QCOMPARE(sc3.readEntry(QStringLiteral("stringListEmptyEntry"), QStringList(QStringLiteral("wrong"))), s_stringlist_empty_entry);

    QCOMPARE(sc3.readEntry(QStringLiteral("stringListJustEmptyElement"), QStringList()), s_stringlist_just_empty_element);

    QCOMPARE(sc3.readEntry(QStringLiteral("stringListEmptyTrailingElement"), QStringList()), s_stringlist_empty_trailing_element);

    QCOMPARE(sc3.readEntry(QStringLiteral("stringListEscapeOddEntry"), QStringList()), s_stringlist_escape_odd_entry);

    QCOMPARE(sc3.readEntry(QStringLiteral("stringListEscapeEvenEntry"), QStringList()), s_stringlist_escape_even_entry);

    QCOMPARE(sc3.readEntry(QStringLiteral("stringListEscapeCommaEntry"), QStringList()), s_stringlist_escape_comma_entry);

    QCOMPARE(sc3.readEntry("listOfIntsEntry1"), QString::fromLatin1("1,2,3,4"));
    QList<int> expectedIntList = s_int_listentry1;
    QVERIFY(sc3.readEntry("listOfIntsEntry1", QList<int>()) == expectedIntList);

    QCOMPARE(QVariant(sc3.readEntry("variantListEntry", s_variantlist_entry)).toStringList(), QVariant(s_variantlist_entry).toStringList());

    QCOMPARE(sc3.readEntry("listOfByteArraysEntry1", QList<QByteArray>()), s_bytearray_list_entry1);
}

void KConfigTest::testPath()
{
    KConfig sc2(s_kconfig_test_subdir);
    KConfigGroup sc3(&sc2, QStringLiteral("Path Type"));
    QCOMPARE(sc3.readPathEntry(QStringLiteral("homepath"), QString{}), s_homepath);
    QCOMPARE(sc3.readPathEntry(QStringLiteral("homepathescape"), QString{}), s_homepath_escape);
    QCOMPARE(sc3.readPathEntry(QStringLiteral("canonicalHomePath"), QString{}), s_canonical_homepath);
    QCOMPARE(sc3.entryMap().value(QStringLiteral("homepath")), s_homepath);

    qputenv("WITHSLASH", "/a/");
    {
        QFile file(m_testConfigDir + QLatin1String("/pathtest"));
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream out(&file);
        out << "[Test Group]\n"
            << "homePath=$HOME/foo\n"
            << "homePath2=file://$HOME/foo\n"
            << "withSlash=$WITHSLASH/foo\n"
            << "withSlash2=$WITHSLASH\n"
            << "withBraces[$e]=file://${HOME}/foo\n"
            << "URL[$e]=file://${HOME}/foo\n"
            << "hostname[$e]=$(hostname)\n"
            << "escapes=aaa,bb/b,ccc\\,ccc\n"
            << "noeol=foo" // no EOL
            ;
    }
    KConfig cf2(s_test_subdir + QLatin1String("pathtest"));
    KConfigGroup group = cf2.group(QStringLiteral("Test Group"));
    QVERIFY(group.hasKey("homePath"));
    QCOMPARE(group.readPathEntry("homePath", QString{}), s_homepath);
    QVERIFY(group.hasKey("homePath2"));
    QCOMPARE(group.readPathEntry("homePath2", QString{}), QLatin1String("file://") + s_homepath);
    QVERIFY(group.hasKey("withSlash"));
    QCOMPARE(group.readPathEntry("withSlash", QString{}), QStringLiteral("/a//foo"));
    QVERIFY(group.hasKey("withSlash2"));
    QCOMPARE(group.readPathEntry("withSlash2", QString{}), QStringLiteral("/a/"));
    QVERIFY(group.hasKey("withBraces"));
    QCOMPARE(group.readPathEntry("withBraces", QString{}), QLatin1String("file://") + s_homepath);
    QVERIFY(group.hasKey("URL"));
    QCOMPARE(group.readEntry("URL", QString{}), QLatin1String("file://") + s_homepath);
    QVERIFY(group.hasKey("hostname"));
    QCOMPARE(group.readEntry("hostname", QString{}), QStringLiteral("(hostname)")); // the $ got removed because empty var name
    QVERIFY(group.hasKey("noeol"));
    QCOMPARE(group.readEntry("noeol", QString{}), QStringLiteral("foo"));

    const auto val = QStringList{QStringLiteral("aaa"), QStringLiteral("bb/b"), QStringLiteral("ccc,ccc")};
    QCOMPARE(group.readPathEntry(QStringLiteral("escapes"), QStringList()), val);
}

void KConfigTest::testPersistenceOfExpandFlagForPath()
{
    // This test checks that a path entry starting with $HOME is still flagged
    // with the expand flag after the config was altered without rewriting the
    // path entry.

    // 1st step: Open the config, add a new dummy entry and then sync the config
    // back to the storage.
    {
        KConfig sc2(s_kconfig_test_subdir);
        KConfigGroup sc3(&sc2, QStringLiteral("Path Type"));
        sc3.writeEntry("dummy", "dummy");
        QVERIFY(sc2.sync());
    }

    // 2nd step: Call testPath() again. Rewriting the config must not break
    // the testPath() test.
    testPath();
}

void KConfigTest::testPathQtHome()
{
    {
        QFile file(m_testConfigDir + QLatin1String("/pathtest"));
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream out(&file);
        out << "[Test Group]\n"
            << "dataDir[$e]=$QT_DATA_HOME/kconfigtest\n"
            << "cacheDir[$e]=$QT_CACHE_HOME/kconfigtest\n"
            << "configDir[$e]=$QT_CONFIG_HOME/kconfigtest\n";
    }
    KConfig cf2(s_test_subdir + QLatin1String("pathtest"));
    KConfigGroup group = cf2.group(QStringLiteral("Test Group"));
    qunsetenv("QT_DATA_HOME");
    qunsetenv("QT_CACHE_HOME");
    qunsetenv("QT_CONFIG_HOME");
    QVERIFY(group.hasKey("dataDir"));
    QCOMPARE(group.readEntry("dataDir", QString{}),
             QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation).append(QStringLiteral("/kconfigtest")));
    QVERIFY(group.hasKey("cacheDir"));
    QCOMPARE(group.readEntry("cacheDir", QString{}),
             QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation).append(QStringLiteral("/kconfigtest")));
    QVERIFY(group.hasKey("configDir"));
    QCOMPARE(group.readEntry("configDir", QString{}),
             QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation).append(QStringLiteral("/kconfigtest")));
    qputenv("QT_DATA_HOME", "/1");
    qputenv("QT_CACHE_HOME", "/2");
    qputenv("QT_CONFIG_HOME", "/3");
    QVERIFY(group.hasKey("dataDir"));
    QCOMPARE(group.readEntry("dataDir", QString{}), QStringLiteral("/1/kconfigtest"));
    QVERIFY(group.hasKey("cacheDir"));
    QCOMPARE(group.readEntry("cacheDir", QString{}), QStringLiteral("/2/kconfigtest"));
    QVERIFY(group.hasKey("configDir"));
    QCOMPARE(group.readEntry("configDir", QString{}), QStringLiteral("/3/kconfigtest"));
}

void KConfigTest::testComplex()
{
    KConfig sc2(s_kconfig_test_subdir);
    KConfigGroup sc3(&sc2, QStringLiteral("Complex Types"));

    QCOMPARE(sc3.readEntry("pointEntry", QPoint()), s_point_entry);
    QCOMPARE(sc3.readEntry("sizeEntry", s_size_entry), s_size_entry);
    QCOMPARE(sc3.readEntry("rectEntry", QRect(1, 2, 3, 4)), s_rect_entry);
    QCOMPARE(sc3.readEntry("dateTimeEntry", QDateTime()).toString(Qt::ISODateWithMs), s_date_time_entry.toString(Qt::ISODateWithMs));
    QCOMPARE(sc3.readEntry("dateEntry", QDate()).toString(Qt::ISODate), s_date_time_entry.date().toString(Qt::ISODate));
    QCOMPARE(sc3.readEntry("dateTimeWithMSEntry", QDateTime()).toString(Qt::ISODateWithMs), s_date_time_with_ms_entry.toString(Qt::ISODateWithMs));
    QCOMPARE(sc3.readEntry("dateTimeEntry", QDate()), s_date_time_entry.date());
}

void KConfigTest::testEnums()
{
    // Visual C++ 2010 (compiler version 16.0) throws an Internal Compiler Error
    // when compiling the code in initTestCase that creates these KConfig entries,
    // so we can't run this test
#if defined(_MSC_VER) && _MSC_VER == 1600
    QSKIP("Visual C++ 2010 can't compile this test");
#endif
    KConfig sc(s_kconfig_test_subdir);
    KConfigGroup sc3(&sc, QStringLiteral("Enum Types"));

    QCOMPARE(sc3.readEntry("enum-10"), QStringLiteral("Tens"));
    QVERIFY(sc3.readEntry("enum-100", Ones) != Ones);
    QVERIFY(sc3.readEntry("enum-100", Ones) != Tens);

    QCOMPARE(sc3.readEntry("flags-bit0"), QStringLiteral("bit0"));
    QVERIFY(sc3.readEntry("flags-bit0", Flags()) == bit0);

    int eid = staticMetaObject.indexOfEnumerator("Flags");
    QVERIFY(eid != -1);
    QMetaEnum me = staticMetaObject.enumerator(eid);
    Flags bitfield = bit0 | bit1;

    QCOMPARE(sc3.readEntry("flags-bit0-bit1"), QString::fromLatin1(me.valueToKeys(bitfield)));
    QVERIFY(sc3.readEntry("flags-bit0-bit1", Flags()) == bitfield);
}

void KConfigTest::testEntryMap()
{
    KConfig sc(s_kconfig_test_subdir);
    KConfigGroup cg(&sc, QStringLiteral("Hello"));
    QMap<QString, QString> entryMap = cg.entryMap();
    qDebug() << entryMap.keys();
    QCOMPARE(entryMap.value(QStringLiteral("stringEntry1")), s_string_entry1);
    QCOMPARE(entryMap.value(QStringLiteral("stringEntry2")), s_string_entry2);
    QCOMPARE(entryMap.value(QStringLiteral("stringEntry3")), s_string_entry3);
    QCOMPARE(entryMap.value(QStringLiteral("stringEntry4")), s_string_entry4);
    QVERIFY(!entryMap.contains(QStringLiteral("stringEntry5")));
    QVERIFY(!entryMap.contains(QStringLiteral("stringEntry6")));
    QCOMPARE(entryMap.value(QStringLiteral("Test")), QString::fromUtf8(s_utf8bit_entry));
    QCOMPARE(entryMap.value(QStringLiteral("bytearrayEntry")), QString::fromUtf8(s_bytearray_entry.constData()));
    QCOMPARE(entryMap.value(QStringLiteral("emptyEntry")), QString{});
    QVERIFY(entryMap.contains(QStringLiteral("emptyEntry")));
    QCOMPARE(entryMap.value(QStringLiteral("boolEntry1")), s_bool_entry1 ? QStringLiteral("true") : QStringLiteral("false"));
    QCOMPARE(entryMap.value(QStringLiteral("boolEntry2")), s_bool_entry2 ? QStringLiteral("true") : QStringLiteral("false"));
    QCOMPARE(entryMap.value(QStringLiteral("keywith=equalsign")), s_string_entry1);
    QCOMPARE(entryMap.value(QStringLiteral("byteArrayEntry1")), s_string_entry1);
    QCOMPARE(entryMap.value(QStringLiteral("doubleEntry1")).toDouble(), s_double_entry);
    QCOMPARE(entryMap.value(QStringLiteral("floatEntry1")).toFloat(), s_float_entry);
}

void KConfigTest::testInvalid()
{
    KConfig sc(s_kconfig_test_subdir);

    // all of these should print a message to the kdebug.dbg file
    KConfigGroup sc3(&sc, QStringLiteral("Invalid Types"));
    sc3.writeEntry("badList", s_variantlist_entry2);

    QList<int> list;

    // 1 element list
    list << 1;
    sc3.writeEntry(QStringLiteral("badList"), list);

    QVERIFY(sc3.readEntry("badList", QPoint()) == QPoint());
    QVERIFY(sc3.readEntry("badList", QRect()) == QRect());
    QVERIFY(sc3.readEntry("badList", QSize()) == QSize());
    QVERIFY(sc3.readEntry("badList", QDate()) == QDate());
    QVERIFY(sc3.readEntry("badList", QDateTime()) == QDateTime());

    // 2 element list
    list << 2;
    sc3.writeEntry("badList", list);

    QVERIFY(sc3.readEntry("badList", QRect()) == QRect());
    QVERIFY(sc3.readEntry("badList", QDate()) == QDate());
    QVERIFY(sc3.readEntry("badList", QDateTime()) == QDateTime());

    // 3 element list
    list << 303;
    sc3.writeEntry("badList", list);

    QVERIFY(sc3.readEntry("badList", QPoint()) == QPoint());
    QVERIFY(sc3.readEntry("badList", QRect()) == QRect());
    QVERIFY(sc3.readEntry("badList", QSize()) == QSize());
    QVERIFY(sc3.readEntry("badList", QDate()) == QDate()); // out of bounds
    QVERIFY(sc3.readEntry("badList", QDateTime()) == QDateTime());

    // 4 element list
    list << 4;
    sc3.writeEntry("badList", list);

    QVERIFY(sc3.readEntry("badList", QPoint()) == QPoint());
    QVERIFY(sc3.readEntry("badList", QSize()) == QSize());
    QVERIFY(sc3.readEntry("badList", QDate()) == QDate());
    QVERIFY(sc3.readEntry("badList", QDateTime()) == QDateTime());

    // 5 element list
    list[2] = 3;
    list << 5;
    sc3.writeEntry("badList", list);

    QVERIFY(sc3.readEntry("badList", QPoint()) == QPoint());
    QVERIFY(sc3.readEntry("badList", QRect()) == QRect());
    QVERIFY(sc3.readEntry("badList", QSize()) == QSize());
    QVERIFY(sc3.readEntry("badList", QDate()) == QDate());
    QVERIFY(sc3.readEntry("badList", QDateTime()) == QDateTime());

    // 6 element list
    list << 6;
    sc3.writeEntry("badList", list);

    QVERIFY(sc3.readEntry("badList", QPoint()) == QPoint());
    QVERIFY(sc3.readEntry("badList", QRect()) == QRect());
    QVERIFY(sc3.readEntry("badList", QSize()) == QSize());
}

void KConfigTest::testChangeGroup()
{
    KConfig sc(s_kconfig_test_subdir);
    KConfigGroup sc3(&sc, QStringLiteral("Hello"));
    QCOMPARE(sc3.name(), QStringLiteral("Hello"));
    KConfigGroup newGroup(sc3);

    KConfigGroup rootGroup(sc.group(QString()));
    QCOMPARE(rootGroup.name(), QStringLiteral("<default>"));
    KConfigGroup sc32(rootGroup.group(QStringLiteral("Hello")));
    QCOMPARE(sc32.name(), QStringLiteral("Hello"));
    KConfigGroup newGroup2(sc32);
}

// Simple test for deleteEntry
void KConfigTest::testDeleteEntry()
{
    const QString configFile = s_test_subdir + QLatin1String("kconfigdeletetest");

    {
        KConfig conf(configFile);
        conf.group(QStringLiteral("Hello")).writeEntry("DelKey", "ToBeDeleted");
    }
    const QList<QByteArray> lines = readLines(configFile);
    Q_ASSERT(lines.contains("[Hello]\n"));
    Q_ASSERT(lines.contains("DelKey=ToBeDeleted\n"));

    KConfig sc(configFile);
    KConfigGroup group(&sc, QStringLiteral("Hello"));

    group.deleteEntry("DelKey");
    QCOMPARE(group.readEntry("DelKey", QStringLiteral("Fietsbel")), QStringLiteral("Fietsbel"));

    QVERIFY(group.sync());
    Q_ASSERT(!readLines(configFile).contains("DelKey=ToBeDeleted\n"));
    QCOMPARE(group.readEntry("DelKey", QStringLiteral("still deleted")), QStringLiteral("still deleted"));
}

void KConfigTest::testDelete()
{
    KConfig sc(s_kconfig_test_subdir);

    KConfigGroup ct(&sc, QStringLiteral("Complex Types"));

    // First delete a nested group
    KConfigGroup delgr(&ct, QStringLiteral("Nested Group 3"));
    QVERIFY(delgr.exists());
    QVERIFY(ct.hasGroup(QStringLiteral("Nested Group 3")));
    QVERIFY(ct.groupList().contains(QStringLiteral("Nested Group 3")));
    delgr.deleteGroup();
    QVERIFY(!delgr.exists());
    QVERIFY(!ct.hasGroup(QStringLiteral("Nested Group 3")));
    QVERIFY(!ct.groupList().contains(QStringLiteral("Nested Group 3")));

    KConfigGroup ng(&ct, QStringLiteral("Nested Group 2"));
    QVERIFY(sc.hasGroup(QStringLiteral("Complex Types")));
    QVERIFY(sc.groupList().contains(QStringLiteral("Complex Types")));
    QVERIFY(!sc.hasGroup(QStringLiteral("Does not exist")));
    QVERIFY(ct.hasGroup(QStringLiteral("Nested Group 1")));
    QVERIFY(ct.groupList().contains(QStringLiteral("Nested Group 1")));
    sc.deleteGroup(QStringLiteral("Complex Types"));
    QCOMPARE(sc.group(QStringLiteral("Complex Types")).keyList().count(), 0);
    QVERIFY(!sc.hasGroup(QStringLiteral("Complex Types"))); // #192266
    QVERIFY(!sc.group(QStringLiteral("Complex Types")).exists());
    QVERIFY(!sc.groupList().contains(QStringLiteral("Complex Types")));
    QVERIFY(!ct.hasGroup(QStringLiteral("Nested Group 1")));
    QVERIFY(!ct.groupList().contains(QStringLiteral("Nested Group 1")));

    QCOMPARE(ct.group(QStringLiteral("Nested Group 1")).keyList().count(), 0);
    QCOMPARE(ct.group(QStringLiteral("Nested Group 2")).keyList().count(), 0);
    QCOMPARE(ng.group(QStringLiteral("Nested Group 2.1")).keyList().count(), 0);

    KConfigGroup cg(&sc, QStringLiteral("AAA"));
    cg.deleteGroup();
    QVERIFY(sc.entryMap(QStringLiteral("Complex Types")).isEmpty());
    QVERIFY(sc.entryMap(QStringLiteral("AAA")).isEmpty());
    QVERIFY(!sc.entryMap(QStringLiteral("Hello")).isEmpty()); // not deleted group
    QVERIFY(sc.entryMap(QStringLiteral("FooBar")).isEmpty()); // inexistent group

    KConfigGroup(&sc, QStringLiteral("LocalGroupToBeDeleted")).deleteGroup();

    QVERIFY(cg.sync());
    // Check what happens on disk
    const QList<QByteArray> lines = readLines();
    // qDebug() << lines;
    QVERIFY(!lines.contains("[Complex Types]\n"));
    QVERIFY(!lines.contains("[Complex Types][Nested Group 1]\n"));
    QVERIFY(!lines.contains("[Complex Types][Nested Group 2]\n"));
    QVERIFY(!lines.contains("[Complex Types][Nested Group 2.1]\n"));
    QVERIFY(!lines.contains("[LocalGroupToBeDeleted]\n"));
    QVERIFY(lines.contains("[AAA]\n")); // deleted from kconfigtest, but present in kdeglobals, so [$d]
    QVERIFY(lines.contains("[Hello]\n")); // a group that was not deleted

    // test for entries that are marked as deleted when there is no default
    KConfig cf(s_kconfig_test_subdir, KConfig::SimpleConfig); // make sure there are no defaults
    cg = cf.group(QStringLiteral("Portable Devices"));
    cg.writeEntry("devices|manual|(null)", "whatever");
    cg.writeEntry("devices|manual|/mnt/ipod", "/mnt/ipod");
    QVERIFY(cf.sync());

    int count = 0;
    const QList<QByteArray> listLines = readLines();
    for (const QByteArray &item : listLines) {
        if (item.startsWith("devices|")) { // krazy:exclude=strings
            ++count;
        }
    }
    QCOMPARE(count, 2);
    cg.deleteEntry("devices|manual|/mnt/ipod");
    QVERIFY(cf.sync());
    const QList<QByteArray> listLines2 = readLines();
    for (const QByteArray &item : listLines2) {
        QVERIFY(!item.contains("ipod"));
    }
}

void KConfigTest::testDefaultGroup()
{
    KConfig sc(s_kconfig_test_subdir);
    KConfigGroup defaultGroup(&sc, QStringLiteral("<default>"));
    QCOMPARE(defaultGroup.name(), QStringLiteral("<default>"));
    QVERIFY(!defaultGroup.exists());
    defaultGroup.writeEntry("TestKey", "defaultGroup");
    QVERIFY(defaultGroup.exists());
    QCOMPARE(defaultGroup.readEntry("TestKey", QString{}), QStringLiteral("defaultGroup"));
    QVERIFY(sc.sync());

    {
        // Test reading it
        KConfig sc2(s_kconfig_test_subdir);
        KConfigGroup defaultGroup2(&sc2, QStringLiteral("<default>"));
        QCOMPARE(defaultGroup2.name(), QStringLiteral("<default>"));
        QVERIFY(defaultGroup2.exists());
        QCOMPARE(defaultGroup2.readEntry("TestKey", QString{}), QStringLiteral("defaultGroup"));
    }
    {
        // Test reading it
        KConfig sc2(s_kconfig_test_subdir);
        KConfigGroup emptyGroup(&sc2, QString());
        QCOMPARE(emptyGroup.name(), QStringLiteral("<default>"));
        QVERIFY(emptyGroup.exists());
        QCOMPARE(emptyGroup.readEntry("TestKey", QString{}), QStringLiteral("defaultGroup"));
    }

    QList<QByteArray> lines = readLines();
    QVERIFY(!lines.contains("[]\n"));
    QVERIFY(!lines.isEmpty());
    QCOMPARE(lines.first(), QByteArray("TestKey=defaultGroup\n"));

    // Now that the group exists make sure it isn't returned from groupList()
    const QStringList groupList = sc.groupList();
    for (const QString &group : groupList) {
        QVERIFY(!group.isEmpty() && group != QLatin1String("<default>"));
    }

    defaultGroup.deleteGroup();
    QVERIFY(sc.sync());

    // Test if deleteGroup worked
    lines = readLines();
    QVERIFY(lines.first() != QByteArray("TestKey=defaultGroup\n"));
}

void KConfigTest::testEmptyGroup()
{
    KConfig sc(s_kconfig_test_subdir);
    KConfigGroup emptyGroup(&sc, QString());
    QCOMPARE(emptyGroup.name(), QStringLiteral("<default>")); // confusing, heh?
    QVERIFY(!emptyGroup.exists());
    emptyGroup.writeEntry("TestKey", "emptyGroup");
    QVERIFY(emptyGroup.exists());
    QCOMPARE(emptyGroup.readEntry("TestKey", QString{}), QStringLiteral("emptyGroup"));
    QVERIFY(sc.sync());

    {
        // Test reading it
        KConfig sc2(s_kconfig_test_subdir);
        KConfigGroup defaultGroup(&sc2, QStringLiteral("<default>"));
        QCOMPARE(defaultGroup.name(), QStringLiteral("<default>"));
        QVERIFY(defaultGroup.exists());
        QCOMPARE(defaultGroup.readEntry("TestKey", QString{}), QStringLiteral("emptyGroup"));
    }
    {
        // Test reading it
        KConfig sc2(s_kconfig_test_subdir);
        KConfigGroup emptyGroup2(&sc2, QString());
        QCOMPARE(emptyGroup2.name(), QStringLiteral("<default>"));
        QVERIFY(emptyGroup2.exists());
        QCOMPARE(emptyGroup2.readEntry("TestKey", QString{}), QStringLiteral("emptyGroup"));
    }

    QList<QByteArray> lines = readLines();
    QVERIFY(!lines.contains("[]\n")); // there's no support for the [] group, in fact.
    QCOMPARE(lines.first(), QByteArray("TestKey=emptyGroup\n"));

    // Now that the group exists make sure it isn't returned from groupList()
    const QStringList groupList = sc.groupList();
    for (const QString &group : groupList) {
        QVERIFY(!group.isEmpty() && group != QLatin1String("<default>"));
    }
    emptyGroup.deleteGroup();
    QVERIFY(sc.sync());

    // Test if deleteGroup worked
    lines = readLines();
    QVERIFY(lines.first() != QByteArray("TestKey=defaultGroup\n"));
}

#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC) && !defined(Q_OS_BLACKBERRY) && !defined(Q_OS_ANDROID)
#define Q_XDG_PLATFORM
#endif

void KConfigTest::testCascadingWithLocale()
{
    // This test relies on XDG_CONFIG_DIRS, which only has effect on Unix.
    // Cascading (more than two levels) isn't available at all on Windows.
#ifdef Q_XDG_PLATFORM
    QTemporaryDir middleDir;
    QTemporaryDir globalDir;
    const QByteArray oldConfigDirs = qgetenv("XDG_CONFIG_DIRS");
    qputenv("XDG_CONFIG_DIRS", qPrintable(middleDir.path() + QLatin1Char(':') + globalDir.path()));

    const QString globalConfigDir = globalDir.path() + QLatin1Char('/') + s_test_subdir;
    QVERIFY(QDir().mkpath(globalConfigDir));
    QFile global(globalConfigDir + QLatin1String("foo.desktop"));
    QVERIFY(global.open(QIODevice::WriteOnly | QIODevice::Text));
    QTextStream globalOut(&global);
    globalOut << "[Group]\n"
              << "FromGlobal=true\n"
              << "FromGlobal[fr]=vrai\n"
              << "Name=Testing\n"
              << "Name[fr]=FR\n"
              << "Other=Global\n"
              << "Other[fr]=Global_FR\n";
    global.close();

    const QString middleConfigDir = middleDir.path() + QLatin1Char('/') + s_test_subdir;
    QVERIFY(QDir().mkpath(middleConfigDir));
    QFile local(middleConfigDir + QLatin1String("foo.desktop"));
    QVERIFY(local.open(QIODevice::WriteOnly | QIODevice::Text));
    QTextStream out(&local);
    out << "[Group]\n"
        << "FromLocal=true\n"
        << "FromLocal[fr]=vrai\n"
        << "Name=Local Testing\n"
        << "Name[fr]=FR\n"
        << "Other=English Only\n";
    local.close();

    KConfig config(s_test_subdir + QLatin1String("foo.desktop"));
    KConfigGroup group = config.group(QStringLiteral("Group"));
    QCOMPARE(group.readEntry("FromGlobal"), QStringLiteral("true"));
    QCOMPARE(group.readEntry("FromLocal"), QStringLiteral("true"));
    QCOMPARE(group.readEntry("Name"), QStringLiteral("Local Testing"));
    config.setLocale(QStringLiteral("fr"));
    QCOMPARE(group.readEntry("FromGlobal"), QStringLiteral("vrai"));
    QCOMPARE(group.readEntry("FromLocal"), QStringLiteral("vrai"));
    QCOMPARE(group.readEntry("Name"), QStringLiteral("FR"));
    QCOMPARE(group.readEntry("Other"), QStringLiteral("English Only")); // Global_FR is locally overridden
    qputenv("XDG_CONFIG_DIRS", oldConfigDirs);
#endif
}

void KConfigTest::testMerge()
{
    DefaultLocale defaultLocale;
    QLocale::setDefault(QLocale::c());
    KConfig config(s_test_subdir + QLatin1String("mergetest"), KConfig::SimpleConfig);

    KConfigGroup cg = config.group(QStringLiteral("some group"));
    cg.writeEntry("entry", " random entry");
    cg.writeEntry("another entry", "blah blah blah");

    {
        // simulate writing by another process
        QFile file(m_testConfigDir + QLatin1String("/mergetest"));
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream out(&file);
        out << "[Merged Group]\n"
            << "entry1=Testing\n"
            << "entry2=More Testing\n"
            << "[some group]\n"
            << "entry[fr]=French\n"
            << "entry[es]=Spanish\n"
            << "entry[de]=German\n";
    }
    QVERIFY(config.sync());

    {
        QList<QByteArray> lines;
        // this is what the file should look like
        lines << "[Merged Group]\n"
              << "entry1=Testing\n"
              << "entry2=More Testing\n"
              << "\n"
              << "[some group]\n"
              << "another entry=blah blah blah\n"
              << "entry=\\srandom entry\n"
              << "entry[de]=German\n"
              << "entry[es]=Spanish\n"
              << "entry[fr]=French\n";
        QFile file(m_testConfigDir + QLatin1String("/mergetest"));
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        for (const QByteArray &line : std::as_const(lines)) {
            QCOMPARE(line, file.readLine());
        }
    }
}

void KConfigTest::testImmutable()
{
    {
        QFile file(m_testConfigDir + QLatin1String("/immutabletest"));
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream out(&file);
        out << "[$i]\n"
            << "entry1=Testing\n"
            << "[group][$i]\n"
            << "[group][subgroup][$i]\n";
    }

    KConfig config(s_test_subdir + QLatin1String("immutabletest"), KConfig::SimpleConfig);
    QVERIFY(config.isGroupImmutable(QString()));
    KConfigGroup cg = config.group(QString());
    QVERIFY(cg.isEntryImmutable("entry1"));
    KConfigGroup cg1 = config.group(QStringLiteral("group"));
    QVERIFY(cg1.isImmutable());
    KConfigGroup cg1a = cg.group(QStringLiteral("group"));
    QVERIFY(cg1a.isImmutable());
    KConfigGroup cg2 = cg1.group(QStringLiteral("subgroup"));
    QVERIFY(cg2.isImmutable());
}

void KConfigTest::testOptionOrder()
{
    {
        QFile file(m_testConfigDir + QLatin1String("/doubleattrtest"));
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream out(&file);
        out << "[group3]\n"
            << "entry2=unlocalized\n"
            << "entry2[$i][de_DE]=t2\n";
    }
    KConfig config(s_test_subdir + QLatin1String("doubleattrtest"), KConfig::SimpleConfig);
    config.setLocale(QStringLiteral("de_DE"));
    KConfigGroup cg3 = config.group(QStringLiteral("group3"));
    QVERIFY(!cg3.isImmutable());
    QCOMPARE(cg3.readEntry("entry2", ""), QStringLiteral("t2"));
    QVERIFY(cg3.isEntryImmutable("entry2"));
    config.setLocale(QStringLiteral("C"));
    QCOMPARE(cg3.readEntry("entry2", ""), QStringLiteral("unlocalized"));
    QVERIFY(!cg3.isEntryImmutable("entry2"));
    cg3.writeEntry("entry2", "modified");
    QVERIFY(config.sync());

    {
        QList<QByteArray> lines;
        // this is what the file should look like
        lines << "[group3]\n"
              << "entry2=modified\n"
              << "entry2[de_DE][$i]=t2\n";

        QFile file(m_testConfigDir + QLatin1String("/doubleattrtest"));
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        for (const QByteArray &line : std::as_const(lines)) {
            QCOMPARE(line, file.readLine());
        }
    }
}

void KConfigTest::testGroupEscape()
{
    KConfig config(s_test_subdir + QLatin1String("groupescapetest"), KConfig::SimpleConfig);
    QVERIFY(config.group(s_dollargroup).exists());
}

void KConfigTest::testSubGroup()
{
    KConfig sc(s_kconfig_test_subdir);
    KConfigGroup cg(&sc, QStringLiteral("ParentGroup"));
    QCOMPARE(cg.readEntry("parentgrpstring", ""), QStringLiteral("somevalue"));
    KConfigGroup subcg1(&cg, QStringLiteral("SubGroup1"));
    QCOMPARE(subcg1.name(), QStringLiteral("SubGroup1"));
    QCOMPARE(subcg1.readEntry("somestring", ""), QStringLiteral("somevalue"));
    KConfigGroup subcg2(&cg, QStringLiteral("SubGroup2"));
    QCOMPARE(subcg2.name(), QStringLiteral("SubGroup2"));
    QCOMPARE(subcg2.readEntry("substring", ""), QStringLiteral("somevalue"));
    KConfigGroup subcg3(&cg, QStringLiteral("SubGroup/3"));
    QCOMPARE(subcg3.readEntry("sub3string", ""), QStringLiteral("somevalue"));
    QCOMPARE(subcg3.name(), QStringLiteral("SubGroup/3"));
    KConfigGroup rcg(&sc, QString());
    KConfigGroup srcg(&rcg, QStringLiteral("ParentGroup"));
    QCOMPARE(srcg.readEntry("parentgrpstring", ""), QStringLiteral("somevalue"));

    QStringList groupList = cg.groupList();
    groupList.sort(); // comes from QSet, so order is undefined
    QCOMPARE(groupList, (QStringList{QStringLiteral("SubGroup/3"), QStringLiteral("SubGroup1"), QStringLiteral("SubGroup2")}));

    const QStringList expectedSubgroup3Keys{QStringLiteral("sub3string")};
    QCOMPARE(subcg3.keyList(), expectedSubgroup3Keys);
    const QStringList expectedParentGroupKeys{QStringLiteral("parentgrpstring")};

    QCOMPARE(cg.keyList(), expectedParentGroupKeys);

    QCOMPARE(QStringList(cg.entryMap().keys()), expectedParentGroupKeys);
    QCOMPARE(QStringList(subcg3.entryMap().keys()), expectedSubgroup3Keys);

    // Create A group containing only other groups. We want to make sure it
    // shows up in groupList of sc
    KConfigGroup neg(&sc, QStringLiteral("NoEntryGroup"));
    KConfigGroup negsub1(&neg, QStringLiteral("NEG Child1"));
    negsub1.writeEntry("entry", "somevalue");
    KConfigGroup negsub2(&neg, QStringLiteral("NEG Child2"));
    KConfigGroup negsub3(&neg, QStringLiteral("NEG Child3"));
    KConfigGroup negsub31(&negsub3, QStringLiteral("NEG Child3-1"));
    KConfigGroup negsub4(&neg, QStringLiteral("NEG Child4"));
    KConfigGroup negsub41(&negsub4, QStringLiteral("NEG Child4-1"));
    negsub41.writeEntry("entry", "somevalue");

    // A group exists if it has content
    QVERIFY(negsub1.exists());

    // But it doesn't exist if it has no content
    // Ossi and David say: this is how it's supposed to work.
    // However you could add a dummy entry for now, or we could add a "Persist" feature to kconfig groups
    // which would make it written out, much like "immutable" already makes them persistent.
    QVERIFY(!negsub2.exists());

    // A subgroup does not qualify as content if it is also empty
    QVERIFY(!negsub3.exists());

    // A subgroup with content is ok
    QVERIFY(negsub4.exists());

    // Only subgroups with content show up in groupList()
    // QEXPECT_FAIL("", "Empty subgroups do not show up in groupList()", Continue);
    // QCOMPARE(neg.groupList(), QStringList() << "NEG Child1" << "NEG Child2" << "NEG Child3" << "NEG Child4");
    // This is what happens
    QStringList groups = neg.groupList();
    groups.sort(); // Qt5 made the ordering unreliable, due to QHash
    QCOMPARE(groups, (QStringList{QStringLiteral("NEG Child1"), QStringLiteral("NEG Child4")}));

    // make sure groupList() isn't returning something it shouldn't
    const QStringList listGroup = sc.groupList();
    for (const QString &group : listGroup) {
        QVERIFY(!group.isEmpty() && group != QLatin1String("<default>"));
        QVERIFY(!group.contains(QChar(0x1d)));
        QVERIFY(!group.contains(QLatin1String("subgroup")));
        QVERIFY(!group.contains(QLatin1String("SubGroup")));
    }

    QVERIFY(sc.sync());

    // Check that the empty groups are not written out.
    const QList<QByteArray> lines = readLines();
    QVERIFY(lines.contains("[NoEntryGroup][NEG Child1]\n"));
    QVERIFY(!lines.contains("[NoEntryGroup][NEG Child2]\n"));
    QVERIFY(!lines.contains("[NoEntryGroup][NEG Child3]\n"));
    QVERIFY(!lines.contains("[NoEntryGroup][NEG Child4]\n")); // implicit group, not written out
    QVERIFY(lines.contains("[NoEntryGroup][NEG Child4][NEG Child4-1]\n"));
}

void KConfigTest::testAddConfigSources()
{
    KConfig cf(s_test_subdir + QLatin1String("specificrc"));

    cf.addConfigSources(QStringList{m_testConfigDir + QLatin1String("/baserc")});
    cf.reparseConfiguration();

    KConfigGroup specificgrp(&cf, QStringLiteral("Specific Only Group"));
    QCOMPARE(specificgrp.readEntry("ExistingEntry", ""), QStringLiteral("DevValue"));

    KConfigGroup sharedgrp(&cf, QStringLiteral("Shared Group"));
    QCOMPARE(sharedgrp.readEntry("SomeSpecificOnlyEntry", ""), QStringLiteral("DevValue"));
    QCOMPARE(sharedgrp.readEntry("SomeBaseOnlyEntry", ""), QStringLiteral("BaseValue"));
    QCOMPARE(sharedgrp.readEntry("SomeSharedEntry", ""), QStringLiteral("DevValue"));

    KConfigGroup basegrp(&cf, QStringLiteral("Base Only Group"));
    QCOMPARE(basegrp.readEntry("ExistingEntry", ""), QStringLiteral("BaseValue"));
    basegrp.writeEntry("New Entry Base Only", "SomeValue");

    KConfigGroup newgrp(&cf, QStringLiteral("New Group"));
    newgrp.writeEntry("New Entry", "SomeValue");

    QVERIFY(cf.sync());

    KConfig plaincfg(s_test_subdir + QLatin1String("specificrc"));

    KConfigGroup newgrp2(&plaincfg, QStringLiteral("New Group"));
    QCOMPARE(newgrp2.readEntry("New Entry", ""), QStringLiteral("SomeValue"));

    KConfigGroup basegrp2(&plaincfg, QStringLiteral("Base Only Group"));
    QCOMPARE(basegrp2.readEntry("New Entry Base Only", ""), QStringLiteral("SomeValue"));
}

void KConfigTest::testGroupCopyTo()
{
    KConfig cf1(s_kconfig_test_subdir);
    KConfigGroup original = cf1.group(QStringLiteral("Enum Types"));

    KConfigGroup copy = cf1.group(QStringLiteral("Enum Types Copy"));
    original.copyTo(&copy); // copy from one group to another
    QCOMPARE(copy.entryMap(), original.entryMap());

    KConfig cf2(s_test_subdir + QLatin1String("copy_of_kconfigtest"), KConfig::SimpleConfig);
    QVERIFY(!cf2.hasGroup(original.name()));
    QVERIFY(!cf2.hasGroup(copy.name()));

    KConfigGroup newGroup = cf2.group(original.name());
    original.copyTo(&newGroup); // copy from one file to another
    QVERIFY(cf2.hasGroup(original.name()));
    QVERIFY(!cf2.hasGroup(copy.name())); // make sure we didn't copy more than we wanted
    QCOMPARE(newGroup.entryMap(), original.entryMap());
}

void KConfigTest::testConfigCopyToSync()
{
    KConfig cf1(s_kconfig_test_subdir);
    // Prepare source file
    KConfigGroup group(&cf1, QStringLiteral("CopyToTest"));
    group.writeEntry("Type", "Test");
    QVERIFY(cf1.sync());

    // Copy to "destination"
    const QString destination = m_testConfigDir + QLatin1String("/kconfigcopytotest");
    QFile::remove(destination);

    KConfig cf2(s_test_subdir + QLatin1String("kconfigcopytotest"));
    KConfigGroup group2(&cf2, QStringLiteral("CopyToTest"));

    group.copyTo(&group2);

    QString testVal = group2.readEntry("Type");
    QCOMPARE(testVal, QStringLiteral("Test"));
    // should write to disk the copied data from group
    QVERIFY(cf2.sync());
    QVERIFY(QFile::exists(destination));
}

void KConfigTest::testConfigCopyTo()
{
    KConfig cf1(s_kconfig_test_subdir);
    {
        // Prepare source file
        KConfigGroup group(&cf1, QStringLiteral("CopyToTest"));
        group.writeEntry("Type", "Test");
        QVERIFY(cf1.sync());
    }

    {
        // Copy to "destination"
        const QString destination = m_testConfigDir + QLatin1String("/kconfigcopytotest");
        QFile::remove(destination);
        KConfig cf2;
        cf1.copyTo(destination, &cf2);
        KConfigGroup group2(&cf2, QStringLiteral("CopyToTest"));
        QString testVal = group2.readEntry("Type");
        QCOMPARE(testVal, QStringLiteral("Test"));
        QVERIFY(cf2.sync());
        QVERIFY(QFile::exists(destination));
    }

    // Check copied config file on disk
    KConfig cf3(s_test_subdir + QLatin1String("kconfigcopytotest"));
    KConfigGroup group3(&cf3, QStringLiteral("CopyToTest"));
    QString testVal = group3.readEntry("Type");
    QCOMPARE(testVal, QStringLiteral("Test"));
}

void KConfigTest::testReparent()
{
    KConfig cf(s_kconfig_test_subdir);
    const QString name(QStringLiteral("Enum Types"));
    KConfigGroup group = cf.group(name);
    const QMap<QString, QString> originalMap = group.entryMap();
    KConfigGroup parent = cf.group(QStringLiteral("Parent Group"));

    QVERIFY(!parent.hasGroup(name));

    QVERIFY(group.entryMap() == originalMap);

    group.reparent(&parent); // see if it can be made a sub-group of another group
    QVERIFY(parent.hasGroup(name));
    QCOMPARE(group.entryMap(), originalMap);

    group.reparent(&cf); // see if it can make it a top-level group again
    //    QVERIFY(!parent.hasGroup(name));
    QCOMPARE(group.entryMap(), originalMap);
}

static void ageTimeStamp(const QString &path, int nsec)
{
#ifdef Q_OS_UNIX
    QDateTime mtime = QFileInfo(path).lastModified().addSecs(-nsec);
    struct utimbuf utbuf;
    utbuf.actime = mtime.toSecsSinceEpoch();
    utbuf.modtime = utbuf.actime;
    utime(QFile::encodeName(path).constData(), &utbuf);
#else
    QTest::qSleep(nsec * 1000);
#endif
}

void KConfigTest::testWriteOnSync()
{
    QDateTime oldStamp;
    QDateTime newStamp;
    KConfig sc(s_kconfig_test_subdir, KConfig::IncludeGlobals);

    // Age the timestamp of global config file a few sec, and collect it.
    QString globFile = m_kdeGlobalsPath;
    ageTimeStamp(globFile, 2); // age 2 sec
    oldStamp = QFileInfo(globFile).lastModified();

    // Add a local entry and sync the config.
    // Should not rewrite the global config file.
    KConfigGroup cgLocal(&sc, QStringLiteral("Locals"));
    cgLocal.writeEntry("someLocalString", "whatever");
    QVERIFY(sc.sync());

    // Verify that the timestamp of global config file didn't change.
    newStamp = QFileInfo(globFile).lastModified();
    QCOMPARE(newStamp, oldStamp);

    // Age the timestamp of local config file a few sec, and collect it.
    QString locFile = m_testConfigDir + QLatin1String("/kconfigtest");
    ageTimeStamp(locFile, 2); // age 2 sec
    oldStamp = QFileInfo(locFile).lastModified();

    // Add a global entry and sync the config.
    // Should not rewrite the local config file.
    KConfigGroup cgGlobal(&sc, QStringLiteral("Globals"));
    cgGlobal.writeEntry("someGlobalString", "whatever", KConfig::Persistent | KConfig::Global);
    QVERIFY(sc.sync());

    // Verify that the timestamp of local config file didn't change.
    newStamp = QFileInfo(locFile).lastModified();
    QCOMPARE(newStamp, oldStamp);
}

void KConfigTest::testFailOnReadOnlyFileSync()
{
    KConfig sc(s_test_subdir + QLatin1String("kconfigfailonreadonlytest"));
    KConfigGroup cgLocal(&sc, QStringLiteral("Locals"));

    cgLocal.writeEntry("someLocalString", "whatever");
    QVERIFY(cgLocal.sync());

    QFile f(m_testConfigDir + QLatin1String("kconfigfailonreadonlytest"));
    QVERIFY(f.exists());
    QVERIFY(f.setPermissions(QFileDevice::ReadOwner));

#ifndef Q_OS_WIN
    if (::getuid() == 0) {
        QSKIP("Root can write to read-only files");
    }
#endif
    cgLocal.writeEntry("someLocalString", "whatever2");
    QVERIFY(!cgLocal.sync());

    QVERIFY(f.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner));
    QVERIFY(f.remove());
}

void KConfigTest::testDirtyOnEqual()
{
    QDateTime oldStamp;
    QDateTime newStamp;
    KConfig sc(s_kconfig_test_subdir);

    // Initialize value
    KConfigGroup cgLocal(&sc, QStringLiteral("random"));
    cgLocal.writeEntry("theKey", "whatever");
    QVERIFY(sc.sync());

    // Age the timestamp of local config file a few sec, and collect it.
    QString locFile = m_testConfigDir + QLatin1String("/kconfigtest");
    ageTimeStamp(locFile, 2); // age 2 sec
    oldStamp = QFileInfo(locFile).lastModified();

    // Write exactly the same again
    cgLocal.writeEntry("theKey", "whatever");
    // This should be a no-op
    QVERIFY(sc.sync());

    // Verify that the timestamp of local config file didn't change.
    newStamp = QFileInfo(locFile).lastModified();
    QCOMPARE(newStamp, oldStamp);
}

void KConfigTest::testDirtyOnEqualOverdo()
{
    QByteArray val1(
        "\0"
        "one",
        4);
    QByteArray val2(
        "\0"
        "two",
        4);
    QByteArray defvalr;

    KConfig sc(s_kconfig_test_subdir);
    KConfigGroup cgLocal(&sc, QStringLiteral("random"));
    cgLocal.writeEntry("someKey", val1);
    QCOMPARE(cgLocal.readEntry("someKey", defvalr), val1);
    cgLocal.writeEntry("someKey", val2);
    QCOMPARE(cgLocal.readEntry("someKey", defvalr), val2);
}

void KConfigTest::testCreateDir()
{
    // Test auto-creating the parent directory when needed (KConfigIniBackend::createEnclosing)
    QTemporaryDir tmpDir(QDir::tempPath() + QStringLiteral("/fakehome_XXXXXX"));
    QVERIFY(tmpDir.isValid());
    const QString kdehome = tmpDir.path() + QLatin1String("/.kde-unit-test");
    const QString subdir = kdehome + QLatin1String("/newsubdir");
    const QString file = subdir + QLatin1String("/foo.desktop");
    QFile::remove(file);
    QDir().rmdir(subdir);
    QVERIFY(!QDir().exists(subdir));
    KDesktopFile desktopFile(file);
    desktopFile.desktopGroup().writeEntry("key", "value");
    QVERIFY(desktopFile.sync());
    QVERIFY(QFile::exists(file));

    // Cleanup
    QFile::remove(file);
    QDir().rmdir(subdir);
}

void KConfigTest::testSyncOnExit()
{
    // Often, the KGlobalPrivate global static's destructor ends up calling ~KConfig ->
    // KConfig::sync ... and if that code triggers KGlobal code again then things could crash.
    // So here's a test for modifying KSharedConfig::openConfig() and not syncing, the process exit will sync.
    KConfigGroup grp(KSharedConfig::openConfig(s_test_subdir + QLatin1String("syncOnExitRc")), QStringLiteral("syncOnExit"));
    grp.writeEntry("key", "value");
}

void KConfigTest::testSharedConfig()
{
    // Can I use a KConfigGroup even after the KSharedConfigPtr goes out of scope?
    KConfigGroup myConfigGroup;
    {
        KSharedConfigPtr config = KSharedConfig::openConfig(s_kconfig_test_subdir);
        myConfigGroup = KConfigGroup(config, QStringLiteral("Hello"));
    }
    QCOMPARE(myConfigGroup.readEntry("stringEntry1"), s_string_entry1);

    // Get the main config
    KSharedConfigPtr mainConfig = KSharedConfig::openConfig();
    KConfigGroup mainGroup(mainConfig, QStringLiteral("Main"));
    QCOMPARE(mainGroup.readEntry("Key", QString{}), QStringLiteral("Value"));
}

void KConfigTest::testLocaleConfig()
{
    // Initialize the testdata
    QDir().mkpath(m_testConfigDir);
    const QString file = m_testConfigDir + QLatin1String("/localized.test");
    QFile::remove(file);
    QFile f(file);
    QVERIFY(f.open(QIODevice::WriteOnly));
    QTextStream ts(&f);
    ts << "[Test_Wrong]\n";
    ts << "foo[ca]=5\n";
    ts << "foostring[ca]=nice\n";
    ts << "foobool[ca]=true\n";
    ts << "[Test_Right]\n";
    ts << "foo=5\n";
    ts << "foo[ca]=5\n";
    ts << "foostring=primary\n";
    ts << "foostring[ca]=nice\n";
    ts << "foobool=primary\n";
    ts << "foobool[ca]=true\n";
    f.close();

    // Load the testdata
    QVERIFY(QFile::exists(file));
    KConfig config(file);
    config.setLocale(QStringLiteral("ca"));

    // This group has only localized values. That is not supported. The values
    // should be dropped on loading.
    KConfigGroup cg(&config, QStringLiteral("Test_Wrong"));
    QEXPECT_FAIL("", "The localized values are not dropped", Continue);
    QVERIFY(!cg.hasKey("foo"));
    QEXPECT_FAIL("", "The localized values are not dropped", Continue);
    QVERIFY(!cg.hasKey("foostring"));
    QEXPECT_FAIL("", "The localized values are not dropped", Continue);
    QVERIFY(!cg.hasKey("foobool"));

    // Now check the correct config group
    KConfigGroup cg2(&config, QStringLiteral("Test_Right"));
    QCOMPARE(cg2.readEntry("foo"), QStringLiteral("5"));
    QCOMPARE(cg2.readEntry("foo", 3), 5);
    QCOMPARE(cg2.readEntry("foostring"), QStringLiteral("nice"));
    QCOMPARE(cg2.readEntry("foostring", "ugly"), QStringLiteral("nice"));
    QCOMPARE(cg2.readEntry("foobool"), QStringLiteral("true"));
    QCOMPARE(cg2.readEntry("foobool", false), true);

    // Clean up after the testcase
    QFile::remove(file);
}

void KConfigTest::testDeleteWhenLocalized()
{
    // Initialize the testdata
    QTemporaryDir tmpDir(QDir::tempPath() + QStringLiteral("/fakehome_XXXXXX"));
    QVERIFY(tmpDir.isValid());
    const QString subdir = tmpDir.path() + QLatin1String("/.kde-unit-test/");
    QDir().mkpath(subdir);
    const QString file = subdir + QLatin1String("/localized_delete.test");
    QFile::remove(file);
    QFile f(file);
    QVERIFY(f.open(QIODevice::WriteOnly));
    QTextStream ts(&f);
    ts << "[Test4711]\n";
    ts << "foo=3\n";
    ts << "foo[ca]=5\n";
    ts << "foo[de]=7\n";
    ts << "foostring=ugly\n";
    ts << "foostring[ca]=nice\n";
    ts << "foostring[de]=schoen\n";
    ts << "foobool=false\n";
    ts << "foobool[ca]=true\n";
    ts << "foobool[de]=true\n";
    f.close();

    // Load the testdata. We start in locale "ca".
    QVERIFY(QFile::exists(file));
    KConfig config(file);
    config.setLocale(QStringLiteral("ca"));
    KConfigGroup cg(&config, QStringLiteral("Test4711"));

    // Delete a value. Once with localized, once with Normal
    cg.deleteEntry("foostring", KConfigBase::Persistent | KConfigBase::Localized);
    cg.deleteEntry("foobool");
    QVERIFY(config.sync());

    // The value is now gone. The others are still there. Everything correct
    // here.
    QVERIFY(!cg.hasKey("foostring"));
    QVERIFY(!cg.hasKey("foobool"));
    QVERIFY(cg.hasKey("foo"));

    // The current state is: (Just return before this comment.)
    // [...]
    // foobool[ca]=true
    // foobool[de]=wahr
    // foostring=ugly
    // foostring[de]=schoen

    // Now switch the locale to "de" and repeat the checks. Results should be
    // the same. But they currently are not. The localized value are
    // independent of each other. All values are still there in "de".
    config.setLocale(QStringLiteral("de"));
    QEXPECT_FAIL("", "Currently localized values are not deleted correctly", Continue);
    QVERIFY(!cg.hasKey("foostring"));
    QEXPECT_FAIL("", "Currently localized values are not deleted correctly", Continue);
    QVERIFY(!cg.hasKey("foobool"));
    QVERIFY(cg.hasKey("foo"));
    // Check where the wrong values come from.
    // We get the "de" value.
    QCOMPARE(cg.readEntry("foostring", "nothing"), QStringLiteral("schoen"));
    // We get the "de" value.
    QCOMPARE(cg.readEntry("foobool", false), true);

    // Now switch the locale back "ca" and repeat the checks. Results are
    // again different.
    config.setLocale(QStringLiteral("ca"));
    // This line worked above. But now it fails.
    QEXPECT_FAIL("", "Currently localized values are not deleted correctly", Continue);
    QVERIFY(!cg.hasKey("foostring"));
    // This line worked above too.
    QEXPECT_FAIL("", "Currently localized values are not deleted correctly", Continue);
    QVERIFY(!cg.hasKey("foobool"));
    QVERIFY(cg.hasKey("foo"));
    // Check where the wrong values come from.
    // We get the primary value because the "ca" value was deleted.
    QCOMPARE(cg.readEntry("foostring", "nothing"), QStringLiteral("ugly"));
    // We get the "ca" value.
    QCOMPARE(cg.readEntry("foobool", false), true);

    // Now test the deletion of a group.
    cg.deleteGroup();
    QVERIFY(config.sync());

    // Current state: [ca] and [de] entries left... oops.
    // qDebug() << readLinesFrom(file);

    // Bug: The group still exists [because of the localized entries]...
    QVERIFY(cg.exists());
    QVERIFY(!cg.hasKey("foo"));
    QVERIFY(!cg.hasKey("foostring"));
    QEXPECT_FAIL("", "Currently localized values are not deleted correctly", Continue);
    QVERIFY(!cg.hasKey("foobool"));

    // Now switch the locale to "de" and repeat the checks. All values
    // still here because only the primary values are deleted.
    config.setLocale(QStringLiteral("de"));
    QEXPECT_FAIL("", "Currently localized values are not deleted correctly", Continue);
    QVERIFY(!cg.hasKey("foo"));
    QEXPECT_FAIL("", "Currently localized values are not deleted correctly", Continue);
    QVERIFY(!cg.hasKey("foostring"));
    QEXPECT_FAIL("", "Currently localized values are not deleted correctly", Continue);
    QVERIFY(!cg.hasKey("foobool"));
    // Check where the wrong values come from.
    // We get the "de" value.
    QCOMPARE(cg.readEntry("foostring", "nothing"), QStringLiteral("schoen"));
    // We get the "de" value.
    QCOMPARE(cg.readEntry("foobool", false), true);
    // We get the "de" value.
    QCOMPARE(cg.readEntry("foo", 0), 7);

    // Now switch the locale to "ca" and repeat the checks
    // "foostring" is now really gone because both the primary value and the
    // "ca" value are deleted.
    config.setLocale(QStringLiteral("ca"));
    QEXPECT_FAIL("", "Currently localized values are not deleted correctly", Continue);
    QVERIFY(!cg.hasKey("foo"));
    QVERIFY(!cg.hasKey("foostring"));
    QEXPECT_FAIL("", "Currently localized values are not deleted correctly", Continue);
    QVERIFY(!cg.hasKey("foobool"));
    // Check where the wrong values come from.
    // We get the "ca" value.
    QCOMPARE(cg.readEntry("foobool", false), true);
    // We get the "ca" value.
    QCOMPARE(cg.readEntry("foo", 0), 5);

    // Cleanup
    QFile::remove(file);
}

void KConfigTest::testKdeGlobals()
{
    {
        KConfig glob(QStringLiteral("kdeglobals"));
        KConfigGroup general(&glob, QStringLiteral("General"));
        general.writeEntry("testKG", "1");
        QVERIFY(glob.sync());
    }

    KConfig globRead(QStringLiteral("kdeglobals"));
    const KConfigGroup general(&globRead, QStringLiteral("General"));
    QCOMPARE(general.readEntry("testKG"), QStringLiteral("1"));

    // Check we wrote into kdeglobals
    const QList<QByteArray> lines = readLines(QStringLiteral("kdeglobals"));
    QVERIFY(lines.contains("[General]\n"));
    QVERIFY(lines.contains("testKG=1\n"));

    // Writing using NoGlobals
    {
        KConfig glob(QStringLiteral("kdeglobals"), KConfig::NoGlobals);
        KConfigGroup general(&glob, QStringLiteral("General"));
        general.writeEntry("testKG", "2");
        QVERIFY(glob.sync());
    }
    globRead.reparseConfiguration();
    QCOMPARE(general.readEntry("testKG"), QStringLiteral("2"));

    // Reading using NoGlobals
    {
        KConfig globReadNoGlob(QStringLiteral("kdeglobals"), KConfig::NoGlobals);
        const KConfigGroup generalNoGlob(&globReadNoGlob, QStringLiteral("General"));
        QCOMPARE(generalNoGlob.readEntry("testKG"), QStringLiteral("2"));
    }
}

void KConfigTest::testLocalDeletion()
{
    // Prepare kdeglobals
    {
        KConfig glob(QStringLiteral("kdeglobals"));
        KConfigGroup general(&glob, QStringLiteral("OwnTestGroup"));
        general.writeEntry("GlobalKey", "DontTouchMe");
        QVERIFY(glob.sync());
    }

    QStringList expectedKeys{QStringLiteral("LocalKey")};
    expectedKeys.prepend(QStringLiteral("GlobalWrite"));

    // Write into kconfigtest, including deleting GlobalKey
    {
        KConfig mainConfig(s_kconfig_test_subdir);
        KConfigGroup mainGroup(&mainConfig, QStringLiteral("OwnTestGroup"));
        mainGroup.writeEntry("LocalKey", QStringLiteral("LocalValue"));
        mainGroup.writeEntry("GlobalWrite", QStringLiteral("GlobalValue"), KConfig::Persistent | KConfig::Global); // goes to kdeglobals
        QCOMPARE(mainGroup.readEntry("GlobalKey"), QStringLiteral("DontTouchMe"));
        mainGroup.deleteEntry("GlobalKey"); // local deletion ([$d]), kdeglobals is unchanged
        QCOMPARE(mainGroup.readEntry("GlobalKey", "Default"), QStringLiteral("Default")); // key is gone
        QCOMPARE(mainGroup.keyList(), expectedKeys);
    }

    // Check what ended up in kconfigtest
    const QList<QByteArray> lines = readLines();
    QVERIFY(lines.contains("[OwnTestGroup]\n"));
    QVERIFY(lines.contains("GlobalKey[$d]\n"));

    // Check what ended up in kdeglobals
    {
        KConfig globReadNoGlob(QStringLiteral("kdeglobals"), KConfig::NoGlobals);
        const KConfigGroup generalNoGlob(&globReadNoGlob, QStringLiteral("OwnTestGroup"));
        QCOMPARE(generalNoGlob.readEntry("GlobalKey"), QStringLiteral("DontTouchMe"));
        QCOMPARE(generalNoGlob.readEntry("GlobalWrite"), QStringLiteral("GlobalValue"));
        QVERIFY(!generalNoGlob.hasKey("LocalValue"));
        QStringList expectedGlobalKeys{QStringLiteral("GlobalKey")};
        expectedGlobalKeys.append(QStringLiteral("GlobalWrite"));
        QCOMPARE(generalNoGlob.keyList(), expectedGlobalKeys);
    }

    // Check what we see when re-reading the config file
    {
        KConfig mainConfig(s_kconfig_test_subdir);
        KConfigGroup mainGroup(&mainConfig, QStringLiteral("OwnTestGroup"));
        QCOMPARE(mainGroup.readEntry("GlobalKey", "Default"), QStringLiteral("Default")); // key is gone
        QCOMPARE(mainGroup.keyList(), expectedKeys);
    }
}

void KConfigTest::testAnonymousConfig()
{
    KConfig anonConfig(QString(), KConfig::SimpleConfig);
    KConfigGroup general(&anonConfig, QStringLiteral("General"));
    QCOMPARE(general.readEntry("testKG"), QString()); // no kdeglobals merging
    general.writeEntry("Foo", "Bar");
    QCOMPARE(general.readEntry("Foo"), QStringLiteral("Bar"));
}

void KConfigTest::testQByteArrayUtf8()
{
    QTemporaryFile file;
    QVERIFY(file.open());
    KConfig config(file.fileName(), KConfig::SimpleConfig);
    KConfigGroup general(&config, QStringLiteral("General"));
    QByteArray bytes(256, '\0');
    for (int i = 0; i < 256; i++) {
        bytes[i] = i;
    }
    general.writeEntry("Utf8", bytes);
    config.sync();
    file.flush();
    file.close();
    QFile readFile(file.fileName());
    QVERIFY(readFile.open(QFile::ReadOnly));
#define VALUE                                                                                                                                                  \
    "Utf8="                                                                                                                                                    \
    "\\x00\\x01\\x02\\x03\\x04\\x05\\x06\\x07\\x08\\t\\n\\x0b\\x0c\\r\\x0e\\x0f\\x10\\x11\\x12\\x13\\x14\\x15\\x16\\x17\\x18\\x19\\x1a\\x1b\\x1c\\x1d\\x1e\\x" \
    "1f "                                                                                                                                                      \
    "!\"#$%&'()*+,-./"                                                                                                                                         \
    "0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"                                                                       \
    "\\x7f\\x80\\x81\\x82\\x83\\x84\\x85\\x86\\x87\\x88\\x89\\x8a\\x8b\\x8c\\x8d\\x8e\\x8f\\x90\\x91\\x92\\x93\\x94\\x95\\x96\\x97\\x98\\x99\\x9a\\x9b\\x9c\\" \
    "x9d\\x9e\\x9f\\xa0\\xa1\\xa2\\xa3\\xa4\\xa5\\xa6\\xa7\\xa8\\xa9\\xaa\\xab\\xac\\xad\\xae\\xaf\\xb0\\xb1\\xb2\\xb3\\xb4\\xb5\\xb6\\xb7\\xb8\\xb9\\xba\\xb" \
    "b\\xbc\\xbd\\xbe\\xbf\\xc0\\xc1\\xc2\\xc3\\xc4\\xc5\\xc6\\xc7\\xc8\\xc9\\xca\\xcb\\xcc\\xcd\\xce\\xcf\\xd0\\xd1\\xd2\\xd3\\xd4\\xd5\\xd6\\xd7\\xd8\\xd9"  \
    "\\xda\\xdb\\xdc\\xdd\\xde\\xdf\\xe0\\xe1\\xe2\\xe3\\xe4\\xe5\\xe6\\xe7\\xe8\\xe9\\xea\\xeb\\xec\\xed\\xee\\xef\\xf0\\xf1\\xf2\\xf3\\xf4\\xf5\\xf6\\xf7\\" \
    "xf8\\xf9\\xfa\\xfb\\xfc\\xfd\\xfe\\xff"
    const QByteArray fileBytes = readFile.readAll();
#ifndef Q_OS_WIN
    QCOMPARE(fileBytes, QByteArrayLiteral("[General]\n" VALUE "\n"));
#else
    QCOMPARE(fileBytes, QByteArrayLiteral("[General]\r\n" VALUE "\r\n"));
#endif
#undef VALUE

    // check that reading works
    KConfig config2(file.fileName(), KConfig::SimpleConfig);
    KConfigGroup general2(&config2, QStringLiteral("General"));
    QCOMPARE(bytes, general2.readEntry("Utf8", QByteArray()));
}

void KConfigTest::testQStringUtf8_data()
{
    QTest::addColumn<QByteArray>("data");
    QTest::newRow("1") << QByteArray("Téléchargements\tTéléchargements");
    QTest::newRow("2") << QByteArray("$¢ह€𐍈\t$¢ह€𐍈");
    QTest::newRow("3") << QByteArray("\xc2\xe0\xa4\xf0\x90\x8d\t\\xc2\\xe0\\xa4\\xf0\\x90\\x8d");
    // 2 byte overlong
    QTest::newRow("4") << QByteArray("\xc1\xbf\t\\xc1\\xbf");
    // 3 byte overlong
    QTest::newRow("5") << QByteArray("\xe0\x9f\xbf\t\\xe0\\x9f\\xbf");
    // 4 byte overlong
    QTest::newRow("6") << QByteArray("\xf0\x8f\xbf\xbf\t\\xf0\\x8f\\xbf\\xbf");
    // outside unicode range
    QTest::newRow("7") << QByteArray("\xf4\x90\x80\x80\t\\xf4\\x90\\x80\\x80");
    // just within range
    QTest::newRow("8") << QByteArray("\xc2\x80\t\xc2\x80");
    QTest::newRow("9") << QByteArray("\xe0\xa0\x80\t\xe0\xa0\x80");
    QTest::newRow("10") << QByteArray("\xf0\x90\x80\x80\t\xf0\x90\x80\x80");
    QTest::newRow("11") << QByteArray("\xf4\x8f\xbf\xbf\t\xf4\x8f\xbf\xbf");
}

void KConfigTest::testQStringUtf8()
{
    QFETCH(QByteArray, data);
    const QList<QByteArray> d = data.split('\t');
    const QByteArray value = d[0];
    const QByteArray serialized = d[1];
    QTemporaryFile file;
    QVERIFY(file.open());
    KConfig config(file.fileName(), KConfig::SimpleConfig);
    KConfigGroup general(&config, QStringLiteral("General"));
    general.writeEntry("key", value);
    config.sync();
    file.flush();
    file.close();
    QFile readFile(file.fileName());
    QVERIFY(readFile.open(QFile::ReadOnly));
    QByteArray fileBytes = readFile.readAll();
#ifdef Q_OS_WIN
    fileBytes.replace(QByteArrayLiteral("\r\n"), QByteArrayLiteral("\n"));
#endif
    QCOMPARE(fileBytes, QByteArrayLiteral("[General]\nkey=") + serialized + QByteArrayLiteral("\n"));

    // check that reading works
    KConfig config2(file.fileName(), KConfig::SimpleConfig);
    KConfigGroup general2(&config2, QStringLiteral("General"));
    QCOMPARE(value, general2.readEntry("key", QByteArray()));
}

void KConfigTest::testNewlines()
{
    // test that kconfig always uses the native line endings
    QTemporaryFile file;
    QVERIFY(file.open());
    KConfig anonConfig(file.fileName(), KConfig::SimpleConfig);
    KConfigGroup general(&anonConfig, QStringLiteral("General"));
    general.writeEntry("Foo", "Bar");
    general.writeEntry("Bar", "Foo");
    anonConfig.sync();
    file.flush();
    file.close();
    QFile readFile(file.fileName());
    QVERIFY(readFile.open(QFile::ReadOnly));
#ifndef Q_OS_WIN
    QCOMPARE(readFile.readAll(), QByteArrayLiteral("[General]\nBar=Foo\nFoo=Bar\n"));
#else
    QCOMPARE(readFile.readAll(), QByteArrayLiteral("[General]\r\nBar=Foo\r\nFoo=Bar\r\n"));
#endif
}

void KConfigTest::testMoveValuesTo()
{
    QTemporaryFile file;
    QVERIFY(file.open());
    // Prepare kdeglobals
    {
        KConfig glob(QStringLiteral("kdeglobals"));
        KConfigGroup general(&glob, QStringLiteral("TestGroup"));
        general.writeEntry("GlobalKey", "PlsDeleteMe");
        QVERIFY(glob.sync());
    }

    KConfigGroup grp = KSharedConfig::openConfig(file.fileName())->group(QStringLiteral("TestGroup"));

    grp.writeEntry("test1", "first_value");
    grp.writeEntry("test_empty", "");
    grp.writeEntry("other", "other_value");
    grp.writePathEntry("my_path", QStringLiteral("~/somepath"));
    // because this key is from the global file it should be explicitly deleted
    grp.deleteEntry("GlobalKey");

    QTemporaryFile targetFile;
    QVERIFY(targetFile.open());
    targetFile.close();
    KConfigGroup targetGroup = KSharedConfig::openConfig(targetFile.fileName(), KConfig::SimpleConfig)->group(QStringLiteral("MoveToGroup"));

    grp.moveValuesTo({"test1", "test_empty", "does_not_exist", "my_path", "GlobalKey"}, targetGroup);
    QVERIFY(grp.config()->isDirty());
    QVERIFY(targetGroup.config()->isDirty());

    QCOMPARE(grp.keyList(), QStringList{QStringLiteral("other")});
    QStringList expectedKeyList{QStringLiteral("my_path"), QStringLiteral("test1"), QStringLiteral("test_empty")};
    QCOMPARE(targetGroup.keyList(), expectedKeyList);
    QCOMPARE(targetGroup.readEntry("test1"), QStringLiteral("first_value"));

    targetGroup.sync();
    QFile targetReadFile(targetFile.fileName());
    targetReadFile.open(QFile::ReadOnly);
    QVERIFY(targetReadFile.readAll().contains(QByteArray("my_path[$e]=~/somepath")));
}

void KConfigTest::testMoveAllValuesTo()
{
    QTemporaryFile file;
    QVERIFY(file.open());
    // Prepare kdeglobals
    {
        KConfig glob(QStringLiteral("kdeglobals"));
        KConfigGroup general(&glob, QStringLiteral("TestGroup"));
        general.writeEntry("GlobalKey", "PlsDeleteMe");
        QVERIFY(glob.sync());
    }

    KConfigGroup grp = KSharedConfig::openConfig(file.fileName())->group(QStringLiteral("TestGroup"));

    grp.writeEntry("test1", "first_value");
    grp.writeEntry("test_empty", "");
    grp.writePathEntry("my_path", QStringLiteral("~/somepath"));
    // because this key is from the global file it should be explicitly deleted
    grp.deleteEntry("GlobalKey");

    QTemporaryFile targetFile;
    QVERIFY(targetFile.open());
    targetFile.close();
    KConfigGroup targetGroup = KSharedConfig::openConfig(targetFile.fileName(), KConfig::SimpleConfig)->group(QStringLiteral("MoveToGroup"));

    grp.moveValuesTo(targetGroup);
    QVERIFY(grp.config()->isDirty());
    QVERIFY(targetGroup.config()->isDirty());

    QStringList expectedKeyList{QStringLiteral("my_path"), QStringLiteral("test1"), QStringLiteral("test_empty")};
    QCOMPARE(targetGroup.keyList(), expectedKeyList);
    QCOMPARE(targetGroup.readEntry("test1"), QStringLiteral("first_value"));

    targetGroup.sync();
    QFile targetReadFile(targetFile.fileName());
    targetReadFile.open(QFile::ReadOnly);
    QVERIFY(targetReadFile.readAll().contains(QByteArray("my_path[$e]=~/somepath")));
}

void KConfigTest::testXdgListEntry()
{
    QTemporaryFile file;
    QVERIFY(file.open());
    QTextStream out(&file);
    out << "[General]\n"
        << "Key1=\n" // empty list
        // emtpty entries
        << "Key2=;\n"
        << "Key3=;;\n"
        << "Key4=;;;\n"
        << "Key5=\\;\n"
        << "Key6=1;2\\;3;;\n";
    out.flush();
    file.close();
    KConfig anonConfig(file.fileName(), KConfig::SimpleConfig);
    KConfigGroup grp = anonConfig.group(QStringLiteral("General"));
    QStringList invalidList; // use this as a default when an empty list is expected
    invalidList << QStringLiteral("Error! Default value read!");
    QCOMPARE(grp.readXdgListEntry("Key1", invalidList), (QStringList{}));
    QCOMPARE(grp.readXdgListEntry("Key2", invalidList), (QStringList{QString{}}));
    QCOMPARE(grp.readXdgListEntry("Key3", invalidList), (QStringList{QString{}, QString{}}));
    QCOMPARE(grp.readXdgListEntry("Key4", invalidList), (QStringList{QString{}, QString{}, QString{}}));
    QCOMPARE(grp.readXdgListEntry("Key5", invalidList), (QStringList{QStringLiteral(";")}));
    QCOMPARE(grp.readXdgListEntry("Key6", invalidList), (QStringList{QStringLiteral("1"), QStringLiteral("2;3"), QString{}}));
}

#include <QThreadPool>
#include <qtconcurrentrun.h>

// To find multithreading bugs: valgrind --tool=helgrind --track-lockorders=no ./kconfigtest testThreads
void KConfigTest::testThreads()
{
    QThreadPool::globalInstance()->setMaxThreadCount(6);
    // Run in parallel some tests that work on different config files,
    // otherwise unexpected things might indeed happen.
    const QList<QFuture<void>> futures = {
        QtConcurrent::run(&KConfigTest::testAddConfigSources, this),
        QtConcurrent::run(&KConfigTest::testSimple, this),
        QtConcurrent::run(&KConfigTest::testDefaults, this),
        QtConcurrent::run(&KConfigTest::testSharedConfig, this),
        QtConcurrent::run(&KConfigTest::testSharedConfig, this),
    };

    // QEXPECT_FAIL triggers race conditions, it should be fixed to use QThreadStorage...
    // futures << QtConcurrent::run(this, &KConfigTest::testDeleteWhenLocalized);
    // futures << QtConcurrent::run(this, &KConfigTest::testEntryMap);
    for (QFuture<void> f : futures) {
        f.waitForFinished();
    }
}

void KConfigTest::testNotify()
{
#if !KCONFIG_USE_DBUS
    QSKIP("KConfig notification requires DBus");
#endif

    KConfig config(s_kconfig_test_subdir);
    auto myConfigGroup = KConfigGroup(&config, QStringLiteral("TopLevelGroup"));

    // mimics a config in another process, which is watching for events
    auto remoteConfig = KSharedConfig::openConfig(s_kconfig_test_subdir);
    KConfigWatcher::Ptr watcher = KConfigWatcher::create(remoteConfig);

    // some random config that shouldn't be changing when kconfigtest changes, only on kdeglobals
    auto otherRemoteConfig = KSharedConfig::openConfig(s_test_subdir + QLatin1String("kconfigtest2"));
    KConfigWatcher::Ptr otherWatcher = KConfigWatcher::create(otherRemoteConfig);

    QSignalSpy watcherSpy(watcher.data(), &KConfigWatcher::configChanged);
    QSignalSpy otherWatcherSpy(otherWatcher.data(), &KConfigWatcher::configChanged);

    // write entries in a group and subgroup
    myConfigGroup.writeEntry("entryA", "foo", KConfig::Persistent | KConfig::Notify);
    auto subGroup = myConfigGroup.group(QStringLiteral("aSubGroup"));
    subGroup.writeEntry("entry1", "foo", KConfig::Persistent | KConfig::Notify);
    subGroup.writeEntry("entry2", "foo", KConfig::Persistent | KConfig::Notify);
    config.sync();
    watcherSpy.wait();
    QCOMPARE(watcherSpy.count(), 2);

    std::sort(watcherSpy.begin(), watcherSpy.end(), [](QList<QVariant> a, QList<QVariant> b) {
        return a[0].value<KConfigGroup>().name() < b[0].value<KConfigGroup>().name();
    });

    QCOMPARE(watcherSpy[0][0].value<KConfigGroup>().name(), QStringLiteral("TopLevelGroup"));
    QCOMPARE(watcherSpy[0][1].value<QByteArrayList>(), QByteArrayList({"entryA"}));

    QCOMPARE(watcherSpy[1][0].value<KConfigGroup>().name(), QStringLiteral("aSubGroup"));
    QCOMPARE(watcherSpy[1][0].value<KConfigGroup>().parent().name(), QStringLiteral("TopLevelGroup"));
    QCOMPARE(watcherSpy[1][1].value<QByteArrayList>(), QByteArrayList({"entry1", "entry2"}));

    // delete an entry
    watcherSpy.clear();
    myConfigGroup.deleteEntry("entryA", KConfig::Persistent | KConfig::Notify);
    config.sync();
    watcherSpy.wait();
    QCOMPARE(watcherSpy.count(), 1);
    QCOMPARE(watcherSpy[0][0].value<KConfigGroup>().name(), QStringLiteral("TopLevelGroup"));
    QCOMPARE(watcherSpy[0][1].value<QByteArrayList>(), QByteArrayList({"entryA"}));

    // revert to default an entry
    watcherSpy.clear();
    myConfigGroup.revertToDefault("entryA", KConfig::Persistent | KConfig::Notify);
    config.sync();
    watcherSpy.wait();
    QCOMPARE(watcherSpy.count(), 1);
    QCOMPARE(watcherSpy[0][0].value<KConfigGroup>().name(), QStringLiteral("TopLevelGroup"));
    QCOMPARE(watcherSpy[0][1].value<QByteArrayList>(), QByteArrayList({"entryA"}));

    // deleting a group, should notify that every entry in that group has changed
    watcherSpy.clear();
    myConfigGroup.deleteGroup(QStringLiteral("aSubGroup"), KConfig::Persistent | KConfig::Notify);
    config.sync();
    watcherSpy.wait();
    QCOMPARE(watcherSpy.count(), 1);
    QCOMPARE(watcherSpy[0][0].value<KConfigGroup>().name(), QStringLiteral("aSubGroup"));
    QCOMPARE(watcherSpy[0][1].value<QByteArrayList>(), QByteArrayList({"entry1", "entry2"}));

    // global write still triggers our notification
    watcherSpy.clear();
    myConfigGroup.writeEntry("someGlobalEntry", "foo", KConfig::Persistent | KConfig::Notify | KConfig::Global);
    config.sync();
    watcherSpy.wait();
    QCOMPARE(watcherSpy.count(), 1);
    QCOMPARE(watcherSpy[0][0].value<KConfigGroup>().name(), QStringLiteral("TopLevelGroup"));
    QCOMPARE(watcherSpy[0][1].value<QByteArrayList>(), QByteArrayList({"someGlobalEntry"}));

    // watching another file should have only triggered from the kdeglobals change
    QCOMPARE(otherWatcherSpy.count(), 1);
    QCOMPARE(otherWatcherSpy[0][0].value<KConfigGroup>().name(), QStringLiteral("TopLevelGroup"));
    QCOMPARE(otherWatcherSpy[0][1].value<QByteArrayList>(), QByteArrayList({"someGlobalEntry"}));
}

void KConfigTest::testNotifyIllegalObjectPath()
{
#if !KCONFIG_USE_DBUS
    QSKIP("KConfig notification requires DBus");
#endif

    KConfig config(s_kconfig_test_illegal_object_path);
    auto myConfigGroup = KConfigGroup(&config, QStringLiteral("General"));

    // mimics a config in another process, which is watching for events
    auto remoteConfig = KSharedConfig::openConfig(s_kconfig_test_illegal_object_path);
    KConfigWatcher::Ptr watcher = KConfigWatcher::create(remoteConfig);

    QSignalSpy watcherSpy(watcher.data(), &KConfigWatcher::configChanged);

    // write entries in a group and subgroup
    myConfigGroup.writeEntry("entryA", "foo", KConfig::Persistent | KConfig::Notify);
    config.sync();
    watcherSpy.wait();
    QCOMPARE(watcherSpy.size(), 1);
}

void KConfigTest::testKAuthorizeEnums()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup actionRestrictions = config->group(QStringLiteral("KDE Action Restrictions"));
    actionRestrictions.writeEntry("shell_access", false);
    actionRestrictions.writeEntry("action/open_with", false);

    QVERIFY(!KAuthorized::authorize(KAuthorized::SHELL_ACCESS));
    QVERIFY(!KAuthorized::authorizeAction(KAuthorized::OPEN_WITH));
    actionRestrictions.deleteGroup();

    QVERIFY(!KAuthorized::authorize((KAuthorized::GenericRestriction)0));
    QVERIFY(!KAuthorized::authorizeAction((KAuthorized::GenericAction)0));
}

void KConfigTest::testKdeglobalsVsDefault()
{
    // Add testRestore key with global value in kdeglobals
    KConfig glob(QStringLiteral("kdeglobals"));
    KConfigGroup generalGlob(&glob, QStringLiteral("General"));
    generalGlob.writeEntry("testRestore", "global");
    QVERIFY(glob.sync());

    KConfig local(s_test_subdir + QLatin1String("restorerc"));
    KConfigGroup generalLocal(&local, QStringLiteral("General"));
    // Check if we get global and not the default value from cpp (defaultcpp) when reading data from restorerc
    QCOMPARE(generalLocal.readEntry("testRestore", "defaultcpp"), QStringLiteral("global"));

    // Add test restore key with restore value in restorerc file
    generalLocal.writeEntry("testRestore", "restore");
    QVERIFY(local.sync());
    local.reparseConfiguration();
    // We expect to get the value from restorerc file
    QCOMPARE(generalLocal.readEntry("testRestore", "defaultcpp"), QStringLiteral("restore"));

    // Revert to default testRestore key and we expect to get default value and not the global one
    generalLocal.revertToDefault("testRestore");
    local.sync();
    local.reparseConfiguration();
    QCOMPARE(generalLocal.readEntry("testRestore", "defaultcpp"), QStringLiteral("defaultcpp"));
}

#include "moc_kconfigtest.cpp"
