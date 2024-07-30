/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2006 Chusslove Illich <caslav.ilic@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>
#include <QTest>

#include <KLocalizedContext>
#include <QDebug>
class KI18nDeclarativeTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testLocalizedContext_data()
    {
        QTest::addColumn<QString>("propertyName");
        QTest::addColumn<QString>("value");

        QTest::newRow("translation") << "testString" << QStringLiteral("Awesome");
        QTest::newRow("singular translation") << "testStringSingular" << QStringLiteral("and 1 other window");
        QTest::newRow("plural translation") << "testStringPlural" << QStringLiteral("and 3 other windows");
        QTest::newRow("plural translation with domain") << "testStringPluralWithDomain" << QStringLiteral("in 3 seconds");
        QTest::newRow("null string arg") << "testNullStringArg" << QStringLiteral("Awesome ");
        QTest::newRow("zero") << "testZero" << QStringLiteral("I'm 0 years old");
    }

    void testLocalizedContext()
    {
        QFETCH(QString, propertyName);
        QFETCH(QString, value);

        KLocalizedContext ctx;
        QUrl input = QUrl::fromLocalFile(QFINDTESTDATA("test.qml"));

        QQmlEngine engine;
        engine.rootContext()->setContextObject(&ctx);
        QQmlComponent component(&engine, input, QQmlComponent::PreferSynchronous);
        QObject *object = component.create();

        if (!object) {
            qDebug() << "errors:" << component.errors();
        }

        QVERIFY(object);
        QVERIFY(!component.isLoading());
        QCOMPARE(object->property(propertyName.toUtf8().constData()).toString(), value);
    }
};

QTEST_MAIN(KI18nDeclarativeTest)

#include "ki18ndeclarativetest.moc"
