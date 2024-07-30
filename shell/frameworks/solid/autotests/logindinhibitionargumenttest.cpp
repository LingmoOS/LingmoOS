/*
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "../src/solid/power/backends/freedesktop/logindinhibitionargument.h"
#include <QTest>

using namespace Solid;
class LoginInhibitionArgumentTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testFromPowerStates();
};

void LoginInhibitionArgumentTest::testFromPowerStates()
{
    QString arg = LogindInhibitionArgument::fromPowerState(Power::InhibitionTypes(Power::Shutdown & Power::Sleep));
    QCOMPARE(arg, QStringLiteral("sleep:shutdown"));
}

QTEST_GUILESS_MAIN(LoginInhibitionArgumentTest)

#include "logindinhibitionargumenttest.moc"
