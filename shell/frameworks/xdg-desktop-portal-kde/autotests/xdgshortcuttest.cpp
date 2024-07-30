/*
 * SPDX-FileCopyrightText: 2022 Aleix Pol Gonzalez <aleixpol@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include <QStandardPaths>
#include <QTest>

#include "../src/xdgshortcut.h"

class XdgShortcutTest : public QObject
{
    Q_OBJECT
public:
    XdgShortcutTest(QObject *parent = nullptr)
        : QObject(parent)
    {
        QStandardPaths::setTestModeEnabled(true);
    }

private Q_SLOTS:
    void initTestCase()
    {
    }

    void testCheckShortcut_data()
    {
        QTest::addColumn<QString>("expression");
        QTest::addColumn<QKeySequence>("result");

        QTest::newRow("a") << "a" << QKeySequence(Qt::Key_A);
        QTest::newRow("ctrla") << "CTRL+a" << QKeySequence(Qt::Key_A | Qt::ControlModifier);
        QTest::newRow("ctrlshifta") << "CTRL+SHIFT+a" << QKeySequence(Qt::Key_A | Qt::ControlModifier | Qt::ShiftModifier);
        QTest::newRow("ctrlaltreturn") << "CTRL+ALT+Return" << QKeySequence(Qt::Key_Return | Qt::ControlModifier | Qt::AltModifier);
        QTest::newRow("withweirdtoken") << "CTRL+a;Banana" << QKeySequence(Qt::Key_A | Qt::ControlModifier);
        QTest::newRow("justcontrol") << "Control_L" << QKeySequence(Qt::Key_Control);
    }

    void testCheckShortcut()
    {
        QFETCH(QString, expression);
        QFETCH(QKeySequence, result);

        const auto shortcut = XdgShortcut::parse(expression);
        QVERIFY(shortcut.has_value());
        QCOMPARE(*shortcut, result);
    }
};

QTEST_GUILESS_MAIN(XdgShortcutTest)

#include "xdgshortcuttest.moc"
