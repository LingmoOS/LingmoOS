/*
    SPDX-FileCopyrightText: 2022 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "kstandardshortcutwatcher.h"
#include "kconfiggroup.h"
#include "ksharedconfig.h"
#include "kstandardshortcut_p.h"

#include <QSignalSpy>
#include <QStandardPaths>
#include <QTest>

class KStandardShortcutWatcherTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void init();
    void testSignal();
    void testDataUpdated();
};

Q_DECLARE_METATYPE(KStandardShortcut::StandardShortcut)

const QList<QKeySequence> newShortcut = {Qt::CTRL | Qt::Key_Adiaeresis};

void KStandardShortcutWatcherTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    qRegisterMetaType<KStandardShortcut::StandardShortcut>();
    QVERIFY(KStandardShortcut::hardcodedDefaultShortcut(KStandardShortcut::Open) != newShortcut);
}

void KStandardShortcutWatcherTest::init()
{
    KConfigGroup group(KSharedConfig::openConfig(), QStringLiteral("Shortcuts"));
    group.writeEntry("Open", QKeySequence::listToString(KStandardShortcut::hardcodedDefaultShortcut(KStandardShortcut::Open)), KConfig::Global);
    group.sync();
    KStandardShortcut::initialize(KStandardShortcut::Open);
}

void KStandardShortcutWatcherTest::testSignal()
{
#ifdef Q_OS_WIN
    QSKIP("KConfig is built without DBus on Windows");
#endif
    QSignalSpy signalSpy(KStandardShortcut::shortcutWatcher(), &KStandardShortcut::StandardShortcutWatcher::shortcutChanged);
    KStandardShortcut::saveShortcut(KStandardShortcut::Open, newShortcut);
    QTRY_COMPARE(signalSpy.count(), 1);
    const QList<QVariant> arguments = signalSpy.takeFirst();
    QCOMPARE(arguments[0].toInt(), KStandardShortcut::Open);
    QCOMPARE(arguments[1].value<QList<QKeySequence>>(), newShortcut);
}

void KStandardShortcutWatcherTest::testDataUpdated()
{
#ifdef Q_OS_WIN
    QSKIP("KConfig is built without DBus on Windows");
#endif
    QSignalSpy signalSpy(KStandardShortcut::shortcutWatcher(), &KStandardShortcut::StandardShortcutWatcher::shortcutChanged);
    // Writing manually to forego automatic update in saveShortcut()
    KConfigGroup group(KSharedConfig::openConfig(), QStringLiteral("Shortcuts"));
    group.writeEntry("Open", QKeySequence::listToString(newShortcut), KConfig::Global | KConfig::Notify);
    group.sync();
    QTRY_COMPARE(signalSpy.count(), 1);
    QCOMPARE(KStandardShortcut::open(), newShortcut);
}

QTEST_MAIN(KStandardShortcutWatcherTest)
#include "kstandardshortcutwatchertest.moc"
