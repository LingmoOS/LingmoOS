// SPDX-License-Identifier: LGPL-2.0-or-later
// SPDX-FileCopyrightText: 2023 Harald Sitter <sitter@kde.org>

#include "dbussanitizer_p.h"

#include <QTest>

class DBusSanitizerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void test()
    {
        QCOMPARE(kconfigDBusSanitizePath(QStringLiteral("/lingmo_org-kde!lingmo~desktop:appletsrc")),
                 QStringLiteral("/lingmo_org_kde_lingmo_desktop_appletsrc"));
    }
};

QTEST_GUILESS_MAIN(DBusSanitizerTest)
#include "dbussanitizertest.moc"
