/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2024 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KConfig>
#include <KConfigGroup>

#include <QObject>
#include <QStandardPaths>
#include <QTest>

static const QString s_test_subdir{QStringLiteral("kconfigtest_subdir/")};
static const QString s_kconfig_test_subdir(s_test_subdir + QLatin1String("kconfigtest"));

static const QString s_string_entry1(QStringLiteral("hello"));

class KConfigBenchmark : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();

    void testHasKey();
    void testReadEntry();
    void testKConfigGroupKeyList();
};

void KConfigBenchmark::initTestCase()
{
    // ensure we don't use files in the real config directory
    QStandardPaths::setTestModeEnabled(true);

    KConfig sc(s_kconfig_test_subdir);
    KConfigGroup cg(&sc, QStringLiteral("Main"));
    cg.deleteGroup();
    cg.writeEntry("UsedEntry", s_string_entry1);
}

void KConfigBenchmark::testHasKey()
{
    bool hasUsedKey = false;
    bool hasNotUsedKey = true;

    KConfig sc(s_kconfig_test_subdir);
    KConfigGroup cg(&sc, QStringLiteral("Main"));

    QBENCHMARK {
        hasUsedKey = cg.hasKey("UsedEntry");
        hasNotUsedKey = cg.hasKey("NotUsedEntry");
    }

    QCOMPARE(hasUsedKey, true);
    QCOMPARE(hasNotUsedKey, false);
}

void KConfigBenchmark::testReadEntry()
{
    QString usedEntry;
    QString notUsedEntry;
    const QString defaultEntry = QStringLiteral("Default");

    KConfig sc(s_kconfig_test_subdir);
    KConfigGroup cg(&sc, QStringLiteral("Main"));

    QBENCHMARK {
        usedEntry = cg.readEntry("UsedEntry", defaultEntry);
        notUsedEntry = cg.readEntry("NotUsedEntry", defaultEntry);
    }

    QCOMPARE(usedEntry, s_string_entry1);
    QCOMPARE(notUsedEntry, defaultEntry);
}

void KConfigBenchmark::testKConfigGroupKeyList()
{
    QStringList keyList;
    const QString defaultEntry = QStringLiteral("Default");

    KConfig sc(s_kconfig_test_subdir);
    KConfigGroup cg(&sc, QStringLiteral("Main"));
    cg.writeEntry("Entry2", s_string_entry1);
    cg.writeEntry("Entry3", s_string_entry1);

    QBENCHMARK {
        keyList = cg.keyList();
    }

    const QStringList expectedKeyList{
        QStringLiteral("Entry2"),
        QStringLiteral("Entry3"),
        QStringLiteral("UsedEntry"),
    };
    QCOMPARE(keyList, expectedKeyList);
}

QTEST_GUILESS_MAIN(KConfigBenchmark)

#include "kconfig_benchmark.moc"
