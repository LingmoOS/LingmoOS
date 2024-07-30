/*
    SPDX-FileCopyrightText: 2022 Ahmad Samir <a.samirh78@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KCountry>
#include <KCountrySubdivision>

#include <QObject>
#include <QStandardPaths>
#include <QTest>

class KCatalogTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);
    }
    void testLookup()
    {
        QByteArray language = "fr_CH";
        for (int i = 0; i < 15; ++i) {
            language += ":fr_CH";
        }
        const int len = language.size();
        QVERIFY(len > 64);

        // Set LANGUAGE env var to a very long string
        qputenv("LANGUAGE", language);
        const QByteArray before = qgetenv("LANGUAGE");
        QCOMPARE(before.size(), len);

        // This goes through KCatalog::translate(), which will get the value of the
        // LANGUAGE env var and copy it to to a char[64] array
        auto c = KCountry::fromAlpha2(u"NZ");
        QVERIFY(c.isValid());
        QCOMPARE(c.name(), QStringLiteral("Nouvelle-Zélande"));

        QByteArray after = qgetenv("LANGUAGE");
        // LANGUAGE env var value was truncated
        QCOMPARE(after.size(), 64 - strlen("LANGUAGE=") - 1);
    }
};

QTEST_GUILESS_MAIN(KCatalogTest)

#include "kcatalogtest.moc"
