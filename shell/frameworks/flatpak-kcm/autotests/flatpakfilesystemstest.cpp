// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

#include <QDebug>
#include <QMetaEnum>
#include <QTest>

#include "flatpakpermission.h"

class FlatpakFilesystemTest : public QObject
{
    Q_OBJECT

    using FilesystemPrefix = FlatpakFilesystemsEntry::FilesystemPrefix;
    using AccessMode = FlatpakFilesystemsEntry::AccessMode;

private Q_SLOTS:

    void testRoundtrip_data()
    {
        QTest::addColumn<QString>("rawInput");
        QTest::addColumn<QString>("expectedOutput");

        QTest::newRow("/path is same")   << QStringLiteral("/path")   << QStringLiteral("/path");
        QTest::newRow("!/path is same")  << QStringLiteral("!/path")  << QStringLiteral("!/path");

        QTest::newRow("home is home")     << QStringLiteral("home")   << QStringLiteral("home");
        QTest::newRow("~ is home too")    << QStringLiteral("~")      << QStringLiteral("home");
        QTest::newRow("not home")         << QStringLiteral("!home")  << QStringLiteral("!home");
        QTest::newRow("~/path is ~/path") << QStringLiteral("~/path") << QStringLiteral("~/path");

        QTest::newRow("host is same")     << QStringLiteral("host")     << QStringLiteral("host");
        QTest::newRow("host-os is same")  << QStringLiteral("host-os")  << QStringLiteral("host-os");
        QTest::newRow("host-etc is same") << QStringLiteral("host-etc") << QStringLiteral("host-etc");

        QTest::newRow("xdg-desktop is same")           << QStringLiteral("xdg-desktop")           << QStringLiteral("xdg-desktop");
        QTest::newRow("xdg-desktop/path is same")      << QStringLiteral("xdg-desktop/path")      << QStringLiteral("xdg-desktop/path");
        QTest::newRow("xdg-documents is same")         << QStringLiteral("xdg-documents")         << QStringLiteral("xdg-documents");
        QTest::newRow("xdg-documents/path is same")    << QStringLiteral("xdg-documents/path")    << QStringLiteral("xdg-documents/path");
        QTest::newRow("xdg-download is same")          << QStringLiteral("xdg-download")          << QStringLiteral("xdg-download");
        QTest::newRow("xdg-download/path is same")     << QStringLiteral("xdg-download/path")     << QStringLiteral("xdg-download/path");
        QTest::newRow("xdg-music is same")             << QStringLiteral("xdg-music")             << QStringLiteral("xdg-music");
        QTest::newRow("xdg-music/path is same")        << QStringLiteral("xdg-music/path")        << QStringLiteral("xdg-music/path");
        QTest::newRow("xdg-pictures is same")          << QStringLiteral("xdg-pictures")          << QStringLiteral("xdg-pictures");
        QTest::newRow("xdg-pictures/path is same")     << QStringLiteral("xdg-pictures/path")     << QStringLiteral("xdg-pictures/path");
        QTest::newRow("xdg-public-share is same")      << QStringLiteral("xdg-public-share")      << QStringLiteral("xdg-public-share");
        QTest::newRow("xdg-public-share/path is same") << QStringLiteral("xdg-public-share/path") << QStringLiteral("xdg-public-share/path");
        QTest::newRow("xdg-videos is same")            << QStringLiteral("xdg-videos")            << QStringLiteral("xdg-videos");
        QTest::newRow("xdg-videos/path is same")       << QStringLiteral("xdg-videos/path")       << QStringLiteral("xdg-videos/path");
        QTest::newRow("xdg-templates is same")         << QStringLiteral("xdg-templates")         << QStringLiteral("xdg-templates");
        QTest::newRow("xdg-templates/path is same")    << QStringLiteral("xdg-templates/path")    << QStringLiteral("xdg-templates/path");

        QTest::newRow("xdg-cache is same")             << QStringLiteral("xdg-cache")             << QStringLiteral("xdg-cache");
        QTest::newRow("xdg-cache/path is same")        << QStringLiteral("xdg-cache/path")        << QStringLiteral("xdg-cache/path");
        QTest::newRow("xdg-config is same")            << QStringLiteral("xdg-config")            << QStringLiteral("xdg-config");
        QTest::newRow("xdg-config/path is same")       << QStringLiteral("xdg-config/path")       << QStringLiteral("xdg-config/path");
        QTest::newRow("xdg-data is same")              << QStringLiteral("xdg-data")              << QStringLiteral("xdg-data");
        QTest::newRow("xdg-data/path is same")         << QStringLiteral("xdg-data/path")         << QStringLiteral("xdg-data/path");

        QTest::newRow("xdg-run/path is same")          << QStringLiteral("xdg-run/path")          << QStringLiteral("xdg-run/path");

        QTest::newRow(":ro is same")        << QStringLiteral("/path:ro")     << QStringLiteral("/path:ro");
        QTest::newRow(":rw is default")     << QStringLiteral("/path:rw")     << QStringLiteral("/path");
        QTest::newRow(":create is default") << QStringLiteral("/path:create") << QStringLiteral("/path:create");
    }

    void testRoundtrip()
    {
        QFETCH(QString, rawInput);
        QFETCH(QString, expectedOutput);
        const auto entry = FlatpakFilesystemsEntry::parse(rawInput);
        QVERIFY(entry.has_value());
        const auto actual = entry.value().format();
        QCOMPARE(actual, expectedOutput);
    }

    void testInvalid_data()
    {
        QTest::addColumn<QString>("rawInput");

        QTest::newRow("empty is invalid") << QStringLiteral("");

        QTest::newRow("/ is invalid")  << QStringLiteral("/");
        QTest::newRow("!/ is invalid") << QStringLiteral("!/");

        QTest::newRow("xdg-run/ can't omit path") << QStringLiteral("xdg-run/");

        QTest::newRow("standalone :ro")     << QStringLiteral(":ro");
        QTest::newRow("standalone :rw")     << QStringLiteral(":rw");
        QTest::newRow("standalone :create") << QStringLiteral(":create");
        QTest::newRow("standalone !")       << QStringLiteral("!");

        QTest::newRow("combine !:ro")                << QStringLiteral("!:ro");
        QTest::newRow("combine !:rw")                << QStringLiteral("!:rw");
        QTest::newRow("combine !:create")            << QStringLiteral("!:create");
        QTest::newRow("combine !:create with /path") << QStringLiteral("!/path:create");
    }

    void testInvalid()
    {
        QFETCH(QString, rawInput);
        const auto entry = FlatpakFilesystemsEntry::parse(rawInput);
        QVERIFY(!entry.has_value());
    }

    // Technically this is already covered by testRoundtrip, but we also want to test equality operator.
    void testParts_data()
    {
        QTest::addColumn<QString>("rawInput");
        QTest::addColumn<FlatpakFilesystemsEntry>("expected");

        QTest::newRow("/path")    << QStringLiteral("/path")        << FlatpakFilesystemsEntry(FilesystemPrefix::Absolute, AccessMode::ReadWrite, QLatin1String("path"));
        QTest::newRow("not home") << QStringLiteral("!home")        << FlatpakFilesystemsEntry(FilesystemPrefix::Home, AccessMode::Deny, QLatin1String());
        QTest::newRow(":ro")      << QStringLiteral("/path:ro")     << FlatpakFilesystemsEntry(FilesystemPrefix::Absolute, AccessMode::ReadOnly, QLatin1String("path"));
        QTest::newRow(":create")  << QStringLiteral("/path:create") << FlatpakFilesystemsEntry(FilesystemPrefix::Absolute, AccessMode::Create, QLatin1String("path"));
        // Instead of treating entries as invalid, parse them as Unknown and preserve content.
        QTest::newRow("host can't have path")     << QStringLiteral("host/abcd")     << FlatpakFilesystemsEntry(FilesystemPrefix::Unknown, AccessMode::ReadWrite, QStringLiteral("host/abcd"));
        QTest::newRow("host-os can't have path")  << QStringLiteral("host-os/abcd")  << FlatpakFilesystemsEntry(FilesystemPrefix::Unknown, AccessMode::ReadWrite, QStringLiteral("host-os/abcd"));
        QTest::newRow("host-etc can't have path") << QStringLiteral("host-etc/abcd") << FlatpakFilesystemsEntry(FilesystemPrefix::Unknown, AccessMode::ReadWrite, QStringLiteral("host-etc/abcd"));

        QTest::newRow("not host still can't have path") << QStringLiteral("!host/abcd") << FlatpakFilesystemsEntry(FilesystemPrefix::Unknown, AccessMode::Deny, QStringLiteral("host/abcd"));

        QTest::newRow("xdg-run can't omit path") << QStringLiteral("xdg-run")
                                                 << FlatpakFilesystemsEntry(FilesystemPrefix::Unknown, AccessMode::ReadWrite, QStringLiteral("xdg-run"));
        ;
    }

    void testParts()
    {
        QFETCH(QString, rawInput);
        QFETCH(FlatpakFilesystemsEntry, expected);
        const auto entry = FlatpakFilesystemsEntry::parse(rawInput);
        QVERIFY(entry.has_value());
        const auto actual = entry.value();
        QCOMPARE(actual, expected);
        // Test != operator as well
        QCOMPARE(!(actual == expected), (actual != expected));
    }
};

QTEST_MAIN(FlatpakFilesystemTest)

#include "flatpakfilesystemstest.moc"
