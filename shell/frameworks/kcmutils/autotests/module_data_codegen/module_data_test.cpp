/*
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include <QObject>
#include <QTest>

#include "cursorthemedata.h"
#include "cursorthemesettings.h"

class ModuleDataTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testReadingDefaults();
};

void ModuleDataTest::testReadingDefaults()
{
    Test::CursorThemeData data;
    Test::CursorThemeSettings *settings = data.settings();
    QVERIFY(settings);
    QCOMPARE(settings->cursorTheme(), QStringLiteral("breeze_cursors"));
    QCOMPARE(settings->cursorSize(), 24);
}

QTEST_MAIN(ModuleDataTest)

#include "module_data_test.moc"
