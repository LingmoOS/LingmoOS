/*
    SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include <common/surveytargetexpression.h>
#include <common/surveytargetexpressionevaluator.h>
#include <common/surveytargetexpressionparser.h>

#include <QDebug>
#include <QtTest/qtest.h>
#include <QObject>

using namespace KUserFeedback;

class SurveyTargetExpressionTest: public QObject, public SurveyTargetExpressionDataProvider
{
    Q_OBJECT
private:
    QHash<QString, QVariant> m_sourceData;
    QVariant sourceData(const QString &sourceName) const override
    {
        return m_sourceData.value(sourceName);
    }

private Q_SLOTS:
    void testValidParse_data()
    {
        QTest::addColumn<QString>("input");
        QTest::newRow("01") << QStringLiteral("true == 12.3");
        QTest::newRow("02") << QStringLiteral("42 >= false");
        QTest::newRow("03") << QStringLiteral("(42 >= 12.3)");
        QTest::newRow("04") << QStringLiteral("(42 >= 12.3) && (42 < 23)");
        QTest::newRow("05") << QStringLiteral("(42 >= 12.3 && 42 < 23) || true != false");
        QTest::newRow("06") << QStringLiteral("42 >= 12.3E-1 && 42 < 23.1e12 || true != false && -3.2 == .3");
        QTest::newRow("07") << QStringLiteral("\"a\" == \"\\\"b\\\"\"");
        QTest::newRow("08") << QStringLiteral("\"true\" == \"\" || \"&&\" != \"()\"");
        QTest::newRow("09") << QStringLiteral("usageTime.value > 3600");
        QTest::newRow("10") << QStringLiteral("3600 <= usageTime.value");
        QTest::newRow("11") << QStringLiteral("startCount.value != usageTime.value");
        QTest::newRow("12") << QStringLiteral("screens[0].dpi <= 240");
        QTest::newRow("13") << QStringLiteral("views[\"view1\"].ratio > 1");
    }

    void testValidParse()
    {
        QFETCH(QString, input);

        SurveyTargetExpressionParser p;
        QVERIFY(p.parse(input));
        QVERIFY(p.expression());
        qDebug() << p.expression();
    }

    void testInvalidParse_data()
    {
        QTest::addColumn<QString>("input");
        QTest::newRow("01") << QStringLiteral("true && 12.3");
        QTest::newRow("02") << QStringLiteral("42 || false");
        QTest::newRow("03") << QStringLiteral("(42)");
        QTest::newRow("04") << QStringLiteral("true == 12.3 >= 24 < (false)");
        QTest::newRow("05") << QStringLiteral("screen[0] == 4");
        QTest::newRow("06") << QStringLiteral("screen[false] == 4");
        QTest::newRow("07") << QStringLiteral("screen[false].dpi == 4");
        QTest::newRow("08") << QStringLiteral("screen[].dpi == 4");
        QTest::newRow("08") << QStringLiteral("screen[].dpi == 4");
        QTest::newRow("09") << QStringLiteral("screen[\"key\"] == 4");
        QTest::newRow("10") << QStringLiteral("\"\\\"\" == \"\\\"");
    }

    void testInvalidParse()
    {
        QFETCH(QString, input);
        SurveyTargetExpressionParser p;
        QVERIFY(!p.parse(input));
    }

    void testEval_data()
    {
        QTest::addColumn<QString>("input");
        QTest::addColumn<bool>("expected");
        QTest::newRow("01") << QStringLiteral("1 >= 2") << false;
        QTest::newRow("02") << QStringLiteral("1 < 2.0") << true;
        QTest::newRow("03") << QStringLiteral("true != false") << true;
        QTest::newRow("04") << QStringLiteral("\"abc\" == \"abc\"") << true;
        QTest::newRow("05") << QStringLiteral("1.0 > 1.5 || 1.0 <= 1.5") << true;
        QTest::newRow("06") << QStringLiteral("1.0 >= 1.5 && 1.0 < 1.5") << false;
        QTest::newRow("07") << QStringLiteral("3600 <= usageTime.value") << true;
        QTest::newRow("08") << QStringLiteral("3600 <= usageTime.value || 42 < lazy_eval.non_existing") << true;
        QTest::newRow("09") << QStringLiteral("screens[1].dpi <= 240") << true;
        QTest::newRow("10") << QStringLiteral("screens[2].dpi <= 240") << false;
        QTest::newRow("11") << QStringLiteral("screens.size == 2") << true;
        QTest::newRow("12") << QStringLiteral("1 == \"1\"") << false;
        QTest::newRow("13") << QStringLiteral("\"1\" == 1") << false;
        QTest::newRow("14") << QStringLiteral("screens[0].size == 1920") << true;
        QTest::newRow("15") << QStringLiteral("views[\"view2\"].ratio > 0.35") << true;
        QTest::newRow("16") << QStringLiteral("views[\"view3\"].size == \"???\"") << true;
        QTest::newRow("17") << QStringLiteral("views[\"view4\"].ratio > 0.35") << false;
        QTest::newRow("18") << QStringLiteral("views.size == 3") << true;
        QTest::newRow("19") << QStringLiteral("test.string == \"a\\\"b\\nc\\\\d\\te\\\\f\"") << true;
        QTest::newRow("20") << QStringLiteral("views[\"\"].ratio > 0.35") << false;
        QTest::newRow("21") << QStringLiteral("\"\" == \"\"") << true;
        QTest::newRow("22") << QStringLiteral("views.view1 == 2") << false;
        QTest::newRow("23") << QStringLiteral("views.non_existing == false") << false;
        QTest::newRow("24") << QStringLiteral("1 == 1 || 2 == 2 && 3 != 3") << true;
        QTest::newRow("25") << QStringLiteral("(1 == 1 || 2 == 2) && 3 != 3") << false;
        QTest::newRow("26") << QStringLiteral("1 == 1 && 2 == 2 || 3 != 3") << true;
        QTest::newRow("27") << QStringLiteral("1 == 1 && 2 == 2 || 3 == 3 &&  4!=4") << true;
    }

    void testEval()
    {
        QFETCH(QString, input);
        QFETCH(bool, expected);

        QVariantMap m;
        m.insert(QLatin1String("value"), 3600);
        m_sourceData.insert(QLatin1String("usageTime"), m);

        m.clear();
        m.insert(QLatin1String("dpi"), 200);
        m.insert(QLatin1String("size"), 1920);
        QVariantList l;
        l.push_back(m);
        l.push_back(m);
        m_sourceData.insert(QLatin1String("screens"), l);

        m.clear();
        m.insert(QLatin1String("ratio"), 0.4);
        m.insert(QLatin1String("size"), QLatin1String("???"));
        QVariantMap m2;
        m2.insert(QLatin1String("view1"), m);
        m2.insert(QLatin1String("view2"), m);
        m2.insert(QLatin1String("view3"), m);
        m_sourceData.insert(QLatin1String("views"), m2);

        m.clear();
        m.insert(QLatin1String("string"), QLatin1String("a\"b\nc\\d\te\\f"));
        m_sourceData.insert(QLatin1String("test"), m);

        SurveyTargetExpressionParser p;
        QVERIFY(p.parse(input));
        QVERIFY(p.expression());
        qDebug() << p.expression();
        SurveyTargetExpressionEvaluator eval;
        eval.setDataProvider(this);
        QCOMPARE(eval.evaluate(p.expression()), expected);
    }
};

QTEST_GUILESS_MAIN(SurveyTargetExpressionTest)

#include "surveytargetexpressiontest.moc"
