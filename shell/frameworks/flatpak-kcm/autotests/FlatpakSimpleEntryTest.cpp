// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

#include <QDebug>
#include <QMetaEnum>
#include <QTest>
#include <utility>

#include "flatpakcommon.h"
#include "flatpakpermission.h"

using namespace FlatpakStrings;

class FlatpakSimpleEntryTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    // This test intentionally does not verify contents of the parsing result.
    // In case if we ever switch from raw strings to enum values, this test
    // would remain constant.
    void testValidAndRoundtrip_data()
    {
        QTest::addColumn<QString>("rawInput");

        const std::array groups = {
            std::make_pair(QLatin1String(FLATPAK_METADATA_KEY_SHARED), flatpak_context_shares),
            std::make_pair(QLatin1String(FLATPAK_METADATA_KEY_SOCKETS), flatpak_context_sockets),
            std::make_pair(QLatin1String(FLATPAK_METADATA_KEY_DEVICES), flatpak_context_devices),
            std::make_pair(QLatin1String(FLATPAK_METADATA_KEY_FEATURES), flatpak_context_features),
        };

        for (const auto &[group, entries] : groups) {
            for (const char **entryPtr = entries; *entryPtr != nullptr; entryPtr++) {
                const auto entry = QString(*entryPtr);
                const auto disabledEntry = QLatin1String("!%1").arg(entry);
                const auto testRowName = QLatin1String("%1/%2").arg(group, entry);
                QTest::newRow(qUtf8Printable(testRowName)) << entry;
                const auto disabledTestRowName = QLatin1String("%1/!%2").arg(group, entry);
                QTest::newRow(qUtf8Printable(disabledTestRowName)) << disabledEntry;
            }
        }
    }

    void testValidAndRoundtrip()
    {
        QFETCH(QString, rawInput);
        const auto entry = FlatpakSimpleEntry::parse(rawInput);
        QVERIFY(entry.has_value());
        const auto actual = entry.value().format();
        QCOMPARE(actual, rawInput);
    }

    void testEnabled()
    {
        const auto entry = FlatpakSimpleEntry::parse(QStringLiteral("devel"));
        QVERIFY(entry.has_value());
        const auto &value = entry.value();
        QVERIFY(value.isEnabled());
        QCOMPARE(value.name(), QLatin1String("devel"));
    }

    void testDisabled()
    {
        const auto entry = FlatpakSimpleEntry::parse(QStringLiteral("!multiarch"));
        QVERIFY(entry.has_value());
        const auto &value = entry.value();
        QVERIFY(!value.isEnabled());
        QCOMPARE(value.name(), QLatin1String("multiarch"));
    }

    void testEquality()
    {
        const auto entryOn = FlatpakSimpleEntry(QStringLiteral("bluetooth"));
        const auto entryOff = FlatpakSimpleEntry(QStringLiteral("!bluetooth"));

        QVERIFY(entryOn.isEnabled());
        QVERIFY(entryOff.isEnabled());
        QCOMPARE(entryOn, entryOn);
        QCOMPARE(entryOff, entryOff);
        QVERIFY(entryOn != entryOff);

        const auto entryAll1 = FlatpakSimpleEntry(QStringLiteral("all"));
        const auto entryAll2 = FlatpakSimpleEntry(QStringLiteral("all"));

        QCOMPARE(entryAll1, entryAll2);
        QVERIFY(entryAll1 != entryOn);
        QVERIFY(entryAll1 != entryOff);
    }
};

QTEST_MAIN(FlatpakSimpleEntryTest)

#include "FlatpakSimpleEntryTest.moc"
