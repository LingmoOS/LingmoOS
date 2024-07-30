/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1997 Matthias Kalle Dalheimer <kalle@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kconfigguitest.h"
#include <QtTestGui>

#include <QDir>
#include <QFont>
#include <QStandardPaths>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kconfigskeleton.h>

QTEST_MAIN(KConfigTest)

// clazy:excludeall=non-pod-global-static

static const QColor s_color_entry1(QLatin1String{"steelblue"});
static const QColor s_color_entry2(235, 235, 100, 125);
static const QColor s_color_entry3(234, 234, 127);

static QFont fontEntry()
{
    return QFont{QStringLiteral("Times"), 16, QFont::Normal};
}

void KConfigTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);

    // cheat the linker on windows to link against kconfiggui
    KConfigSkeleton foo;
    Q_UNUSED(foo);

    KConfig sc(QStringLiteral("kconfigtest"));

    KConfigGroup cg(&sc, QStringLiteral("ComplexTypes"));
    cg.writeEntry("colorEntry1", s_color_entry1);
    cg.writeEntry("colorEntry2", s_color_entry2);
    cg.writeEntry("colorEntry3", (QList<int>() << 234 << 234 << 127));
    cg.writeEntry("colorEntry4", (QList<int>() << 235 << 235 << 100 << 125));
    cg.writeEntry("fontEntry", fontEntry());
    QVERIFY(sc.sync());

    KConfig sc1(QStringLiteral("kdebugrc"));
    KConfigGroup sg0(&sc1, QStringLiteral("0"));
    sg0.writeEntry("AbortFatal", false);
    sg0.writeEntry("WarnOutput", 0);
    sg0.writeEntry("FatalOutput", 0);
    QVERIFY(sc1.sync());

    // Qt 5.8.0 would fail the fromString(toString) roundtrip in QFont
    // if the qApp font has a styleName set.
    // This is fixed by https://codereview.qt-project.org/181645
    // It's not in yet, and it depends on the app font, so rather than
    // a version check, let's do a runtime check.
    QFont orig(fontEntry());
    QFont f;
    f.fromString(orig.toString());
    m_fontFromStringBug = (f.toString() != orig.toString());
    if (m_fontFromStringBug) {
        qDebug() << "QFont::fromString serialization bug (Qt 5.8.0), the font test will be skipped" << f.toString() << "!=" << orig.toString();
    }
}

void KConfigTest::cleanupTestCase()
{
    QDir dir(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation));
    QVERIFY(dir.removeRecursively());
}

void KConfigTest::testComplex()
{
    KConfig sc2(QStringLiteral("kconfigtest"));
    KConfigGroup sc3(&sc2, QStringLiteral("ComplexTypes"));

    QCOMPARE(QVariant(sc3.readEntry("colorEntry1", QColor(Qt::black))).toString(), QVariant(s_color_entry1).toString());
    QCOMPARE(sc3.readEntry("colorEntry1", QColor()), s_color_entry1);
    QCOMPARE(sc3.readEntry("colorEntry2", QColor()), s_color_entry2);
    QCOMPARE(sc3.readEntry("colorEntry3", QColor()), s_color_entry3);
    QCOMPARE(sc3.readEntry("colorEntry4", QColor()), s_color_entry2);
    QCOMPARE(sc3.readEntry("defaultColorTest", QColor("black")), QColor("black"));
    if (m_fontFromStringBug) {
        QEXPECT_FAIL("", "QFont fromString bug from Qt 5.8.0", Continue);
    }
    QCOMPARE(sc3.readEntry("fontEntry", QFont()), fontEntry());
}

void KConfigTest::testInvalid()
{
    KConfig sc(QStringLiteral("kconfigtest"));

    // all of these should print a message to the kdebug.dbg file
    KConfigGroup sc3(&sc, QStringLiteral("InvalidTypes"));

    QList<int> list;

    // 1 element list
    list << 1;
    sc3.writeEntry(QStringLiteral("badList"), list);
    QVERIFY(sc.sync());

    QVERIFY(sc3.readEntry("badList", QColor()) == QColor());

    // 2 element list
    list << 2;
    sc3.writeEntry("badList", list);
    QVERIFY(sc.sync());

    QVERIFY(sc3.readEntry("badList", QColor()) == QColor());

    // 3 element list
    list << 303;
    sc3.writeEntry("badList", list);
    QVERIFY(sc.sync());

    QVERIFY(sc3.readEntry("badList", QColor()) == QColor()); // out of bounds

    // 4 element list
    list << 4;
    sc3.writeEntry("badList", list);
    QVERIFY(sc.sync());

    QVERIFY(sc3.readEntry("badList", QColor()) == QColor()); // out of bounds

    list[2] = -3;
    sc3.writeEntry("badList", list);
    QVERIFY(sc.sync());
    QVERIFY(sc3.readEntry("badList", QColor()) == QColor()); // out of bounds

    // 5 element list
    list[2] = 3;
    list << 5;
    sc3.writeEntry("badList", list);
    QVERIFY(sc.sync());

    QVERIFY(sc3.readEntry("badList", QColor()) == QColor());

    // 6 element list
    list << 6;
    sc3.writeEntry("badList", list);
    QVERIFY(sc.sync());

    QVERIFY(sc3.readEntry("badList", QColor()) == QColor());
}

#include "moc_kconfigguitest.cpp"
