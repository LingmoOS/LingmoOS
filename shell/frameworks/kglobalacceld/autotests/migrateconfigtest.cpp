/*
    SPDX-FileCopyrightText: 2023 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QTest>

#include "globalshortcutsregistry.h"

class MigrateConfigTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void compareGroups(const KConfigGroup &a, const KConfigGroup &b)
    {
        for (auto [key, value] : a.entryMap().asKeyValueRange()) {
            QCOMPARE(value, b.readEntry(key));
        }

        for (auto [key, value] : b.entryMap().asKeyValueRange()) {
            QCOMPARE(value, a.readEntry(key));
        }
    }

    void compareGroupList(const KConfigBase &a, const KConfigBase &b)
    {
        QCOMPARE(a.groupList(), b.groupList());

        const QStringList groups = a.groupList();
        for (const QString &group : groups) {
            compareGroups(a.group(group), b.group(group));
        }
    }

    void testMigrate()
    {
        QStandardPaths::setTestModeEnabled(true);
        qunsetenv("XDG_DATA_DIRS");

        QDir configDir(QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation));
        configDir.mkpath(QStringLiteral("."));
        configDir.remove(QStringLiteral("kglobalshortcutsrc"));

        QDir dataDir(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation));
        dataDir.mkpath(QStringLiteral("kglobalaccel"));

        QFile::copy(QFINDTESTDATA("kglobalshortcutsrc"),
                    QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QLatin1String("/kglobalshortcutsrc"));

        QFile::copy(QFINDTESTDATA("org.kde.test.desktop"),
                    QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/kglobalaccel/org.kde.test.desktop"));

        // Creating the registry will migrate the shortcut config
        GlobalShortcutsRegistry registry;

        // Compare actual with expected config
        KConfig actual(QStringLiteral("kglobalshortcutsrc"));
        KConfig expected(QFINDTESTDATA("kglobalshortcutsrc.expected"));

        compareGroupList(actual, expected);
        compareGroupList(actual.group(QStringLiteral("services")), expected.group(QStringLiteral("services")));
    }
};

QTEST_MAIN(MigrateConfigTest)

#include "migrateconfigtest.moc"
