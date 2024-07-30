/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2011 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QObject>

#include <QStandardPaths>
#include <QTest>
#include <ksharedconfig.h>

#include <kconfig.h>
#include <kconfiggroup.h>

class KConfigNoKdeHomeTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testNoKdeHome();
};

void KConfigNoKdeHomeTest::testNoKdeHome()
{
    QStandardPaths::setTestModeEnabled(true);
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    QDir configDir(configPath);
    QVERIFY(configDir.removeRecursively());
    QVERIFY(!QFile::exists(configPath));

    // Do what kf5-config does, and ensure the config directory doesn't get created (#233892)
    QVERIFY(!QFile::exists(configPath));
    KSharedConfig::openConfig();
    QVERIFY(!QFile::exists(configPath));

    // Now try to actually save something, see if it works.
    KConfigGroup group(KSharedConfig::openConfig(), QStringLiteral("Group"));
    group.writeEntry("Key", "Value");
    group.sync();
    QVERIFY(QFile::exists(configPath));
    const QString rcFile = QCoreApplication::applicationName() + QStringLiteral("rc");
    QVERIFY(QFile::exists(configPath + QLatin1Char('/') + rcFile));

    // Cleanup
    configDir.removeRecursively();
}

QTEST_MAIN(KConfigNoKdeHomeTest)

#include "kconfignokdehometest.moc"
