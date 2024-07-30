// SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "kruntimeplatform.h"
#include <QObject>
#include <QTest>

class KRuntimePlatformTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testRuntimePlatform()
    {
        qputenv("LINGMO_PLATFORM", "mobile:bigscreen");
        QStringList expected{QStringLiteral("mobile"), QStringLiteral("bigscreen")};
        QCOMPARE(KRuntimePlatform::runtimePlatform(), expected);
    }
};

QTEST_GUILESS_MAIN(KRuntimePlatformTest)

#include "kruntimeplatformtest.moc"
