/*
    SPDX-FileCopyrightText: 2024 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: MIT
*/

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QTest>

class Test14Qml : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testQml()
    {
        QQmlApplicationEngine engine;

        engine.loadFromModule("org.kde.kconfig.test14", "Test14Qml");

        QVERIFY(engine.rootObjects().size());
    }
};

QTEST_MAIN(Test14Qml)

#include "test14qml.moc"
