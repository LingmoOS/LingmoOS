/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KLazyLocalizedString>
#include <KLocalizedString>

#include <QTest>

#include <cstring>

class KLazyLocalizedStringTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testImplicitConversionToLocalizedString()
    {
        // this has to compile
        KLocalizedString s = kli18n("message");
        s = kli18nc("context", "message");
        s = kli18np("singular", "plural");
        s = kli18ncp("context", "singular", "plural");
        s = klxi18n("message");
        s = klxi18nc("context", "message");
        s = klxi18np("singular", "plural");
        s = klxi18ncp("context", "singular", "plural");

        // this should not compile
        // s = kli18n(QStringLiteral("message").toUtf8().constData());
        // auto s2 = new KLazyLocalizedString("bla", "blub", "foo", false);
    }

    void testEmpty()
    {
        KLazyLocalizedString ls;
        QVERIFY(ls.isEmpty());

        KLocalizedString s = ls;
        QVERIFY(s.isEmpty());
    }

    void testStaticMessageTable()
    {
        struct {
            int someProperty;
            KLazyLocalizedString msg;
        } static constexpr const msg_table[] = {
            {0, kli18n("message")},
            {1, kli18nc("context", "message")},
            {2, kli18np("singular", "plural")},
            {3, kli18ncp("context", "singular", "plural")},
            {4, klxi18n("message")},
            {5, klxi18nc("context", "message")},
            {6, klxi18np("singular", "plural")},
            {7, klxi18ncp("context", "singular", "plural")},
        };

        // direct access
        for (const auto &entry : msg_table) {
            QVERIFY(!entry.msg.toString().isEmpty());
        }

        // storing in a local variable
        int max = 0;
        KLazyLocalizedString ls;
        for (const auto &entry : msg_table) {
            if (entry.someProperty > max) {
                max = entry.someProperty;
                ls = entry.msg;
            }
        }
        QVERIFY(!ls.toString().isEmpty());
        QCOMPARE(std::strcmp(ls.untranslatedText(), "singular"), 0);
    }
};

QTEST_GUILESS_MAIN(KLazyLocalizedStringTest)

#include "klazylocalizedstringtest.moc"
