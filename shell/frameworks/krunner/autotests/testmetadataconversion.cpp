/*
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "kpluginmetadata_utils_p.h"
#include <QTest>

class TestMetaDataConversion : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testMetaDataConversion()
    {
        const KPluginMetaData data = parseMetaDataFromDesktopFile(QFINDTESTDATA("plugins/plasma-runner-testconversionfile.desktop"));
        QVERIFY(data.isValid());
        QCOMPARE(data.pluginId(), "testconversionfile");
        QCOMPARE(data.name(), "DBus runner test");
        QCOMPARE(data.description(), "Some Comment");
    }
};

QTEST_MAIN(TestMetaDataConversion)

#include "testmetadataconversion.moc"
