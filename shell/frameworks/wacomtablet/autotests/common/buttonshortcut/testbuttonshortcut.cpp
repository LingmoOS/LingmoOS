/*
 * This file is part of the KDE wacomtablet project. For copyright
 * information and license terms see the AUTHORS and COPYING files
 * in the top-level directory of this distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "common/buttonshortcut.h"

#include <QtTest>

using namespace Wacom;

/**
 * @file testbuttonshortcut.cpp
 *
 * @test UnitTest for the button shortcut testing
 */
class TestButtonShortcut : public QObject
{
    Q_OBJECT

private slots:
    void testAssignment();
    void testButton();
    void testInvalidKeyStrokes();

    void testKeyStrokes();
    void testKeyStrokes_data();

    void testModifier();
    void testModifier_data();

    void testQKeySequences();
    void testQKeySequences_data();

    void testEmpty();

private:
    void assertButton(const ButtonShortcut &shortcut, int buttonNumber) const;
    void assertEquals(const ButtonShortcut &shortcut1, const ButtonShortcut &shortcut2) const;
    void assertKeySequence(const ButtonShortcut &shortcut, const QString &sequence, bool sequenceIsQKeySeq, bool isModifier) const;
    void assertKeyStroke(const Wacom::ButtonShortcut &shortcut, const QString &sequence, bool isQKeySequence) const;
    void assertModifier(const ButtonShortcut &shortcut, const QString &sequence) const;
    void assertUnset(const ButtonShortcut &shortcut) const;
};

QTEST_MAIN(TestButtonShortcut)

void TestButtonShortcut::assertButton(const ButtonShortcut &shortcut, int buttonNumber) const
{
    QVERIFY(shortcut.isSet());
    QVERIFY(shortcut.isButton());
    QCOMPARE(ButtonShortcut::ShortcutType::BUTTON, shortcut.getType());

    QVERIFY(!shortcut.isKeystroke());
    QVERIFY(!shortcut.isModifier());

    QCOMPARE(buttonNumber, shortcut.getButton());
    QCOMPARE(QString::number(buttonNumber), shortcut.toString());
    QVERIFY(shortcut.toQKeySequenceString().isEmpty());
}

void TestButtonShortcut::assertEquals(const ButtonShortcut &shortcut1, const ButtonShortcut &shortcut2) const
{
    QVERIFY(shortcut1 == shortcut2);
    QVERIFY(!(shortcut1 != shortcut2));
    QVERIFY(shortcut1.getButton() == shortcut2.getButton());
    QVERIFY(shortcut1.getType() == shortcut2.getType());
    QVERIFY(shortcut1.isSet() == shortcut2.isSet());
    QVERIFY(shortcut1.isButton() == shortcut2.isButton());
    QVERIFY(shortcut1.isKeystroke() == shortcut2.isKeystroke());
    QVERIFY(shortcut1.isModifier() == shortcut2.isModifier());
    QCOMPARE(shortcut1.toString(), shortcut2.toString());
    QCOMPARE(shortcut2.toQKeySequenceString(), shortcut2.toQKeySequenceString());
}

void TestButtonShortcut::assertKeySequence(const ButtonShortcut &shortcut, const QString &sequence, bool sequenceIsQKeySeq, bool isModifier) const
{
    QVERIFY(shortcut.isSet());

    QVERIFY(!shortcut.isButton());

    QCOMPARE(0, shortcut.getButton());

    if (isModifier) {
        QVERIFY(shortcut.isModifier());
        QVERIFY(!shortcut.isKeystroke());
    } else {
        QVERIFY(shortcut.isKeystroke());
        QVERIFY(!shortcut.isModifier());
    }

    if (sequenceIsQKeySeq) {
        QCOMPARE(shortcut.toQKeySequenceString(), sequence);
    } else {
        QCOMPARE(shortcut.toString(), sequence);
    }
}

void TestButtonShortcut::assertKeyStroke(const ButtonShortcut &shortcut, const QString &sequence, bool isQKeySequence) const
{
    assertKeySequence(shortcut, sequence, isQKeySequence, false);
}

void TestButtonShortcut::assertModifier(const ButtonShortcut &shortcut, const QString &sequence) const
{
    assertKeySequence(shortcut, sequence, false, true);
}

void TestButtonShortcut::assertUnset(const ButtonShortcut &shortcut) const
{
    QVERIFY(!shortcut.isSet());
    QVERIFY(!shortcut.isButton());
    QVERIFY(!shortcut.isKeystroke());
    QVERIFY(!shortcut.isModifier());

    QCOMPARE(0, shortcut.getButton());
    QCOMPARE(QLatin1String("0"), shortcut.toString());
    QVERIFY(shortcut.toQKeySequenceString().isEmpty());
}

void TestButtonShortcut::testAssignment()
{
    ButtonShortcut buttonShortcut;
    ButtonShortcut modifierShortcut;
    ButtonShortcut strokeShortcut;

    buttonShortcut.setButton(2);
    modifierShortcut.set(QLatin1String("key ctrl shift"));
    strokeShortcut.set(QLatin1String("key ctrl a"));

    // Test Constructors.
    ButtonShortcut copyShortcut(buttonShortcut);
    assertEquals(copyShortcut, buttonShortcut);
    assertButton(copyShortcut, buttonShortcut.getButton());

    ButtonShortcut copyShortcut2(modifierShortcut.toString());
    assertEquals(copyShortcut2, modifierShortcut);
    assertModifier(copyShortcut2, modifierShortcut.toString());

    ButtonShortcut copyShortcut3(buttonShortcut.getButton());
    assertEquals(copyShortcut3, buttonShortcut);
    assertButton(copyShortcut3, buttonShortcut.getButton());

    // Test Assignment Operators
    copyShortcut = modifierShortcut;
    assertEquals(copyShortcut, modifierShortcut);
    assertModifier(copyShortcut, modifierShortcut.toString());

    copyShortcut = modifierShortcut.toString();
    assertEquals(copyShortcut, modifierShortcut);
    assertModifier(copyShortcut, modifierShortcut.toString());

    copyShortcut = strokeShortcut;
    assertEquals(copyShortcut, strokeShortcut);
    assertKeyStroke(copyShortcut, strokeShortcut.toString(), false);

    copyShortcut = strokeShortcut.toString();
    assertEquals(copyShortcut, strokeShortcut);
    assertKeyStroke(copyShortcut, strokeShortcut.toString(), false);

    copyShortcut = buttonShortcut;
    assertEquals(copyShortcut, buttonShortcut);
    assertButton(copyShortcut, buttonShortcut.getButton());

    copyShortcut = buttonShortcut.toString();
    assertEquals(copyShortcut, buttonShortcut);
    assertButton(copyShortcut, buttonShortcut.getButton());
}

void TestButtonShortcut::testButton()
{
    int minButtonNr = 1;
    int medButtonNr = 5;
    int maxButtonNr = 32;

    ButtonShortcut shortcut(medButtonNr);
    assertButton(shortcut, medButtonNr);

    shortcut.setButton(minButtonNr);
    assertButton(shortcut, minButtonNr);

    shortcut.setButton(medButtonNr);
    assertButton(shortcut, medButtonNr);

    shortcut.set(QString::number(minButtonNr));
    assertButton(shortcut, minButtonNr);

    shortcut.clear();
    assertUnset(shortcut);

    shortcut.set(QString::number(maxButtonNr));
    assertButton(shortcut, maxButtonNr);

    shortcut.setButton(maxButtonNr);
    assertButton(shortcut, maxButtonNr);

    shortcut.clear();
    assertUnset(shortcut);

    shortcut.set(QLatin1String("button 2"));
    assertButton(shortcut, 2);

    shortcut.set(QLatin1String("Button 3"));
    assertButton(shortcut, 3);

    shortcut.set(QLatin1String(" button +4 "));
    assertButton(shortcut, 4);
}

void TestButtonShortcut::testEmpty()
{
    ButtonShortcut shortcut1;
    ButtonShortcut shortcut2;

    // make sure the shortcuts are equal in every aspect
    assertEquals(shortcut1, shortcut2);

    // make sure the shortcut is not set to anything
    assertUnset(shortcut1);
}

void TestButtonShortcut::testInvalidKeyStrokes()
{
    QList<QString> inputList;

    inputList.append(QLatin1String("A+B"));
    inputList.append(QLatin1String("Ctrl+b+a"));

    ButtonShortcut shortcut;

    for (QList<QString>::iterator inputIter = inputList.begin(); inputIter != inputList.end(); ++inputIter) {
        shortcut.set(*inputIter);
        assertUnset(shortcut);
    }
}

void TestButtonShortcut::testKeyStrokes()
{
    QFETCH(QString, input);
    QFETCH(QString, expected_output);
    ButtonShortcut shortcut;

    shortcut.set(input);
    assertKeyStroke(shortcut, expected_output, false);

    shortcut.clear();
    assertUnset(shortcut);

    shortcut.set(input.toLower());
    assertKeyStroke(shortcut, expected_output.toLower(), false);
}

void TestButtonShortcut::testKeyStrokes_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("expected_output");

    QTest::addRow("A") << "A"
                       << "key a";
    QTest::addRow("key A") << "key A"
                           << "key a";
    QTest::addRow("key +A") << "key +A"
                            << "key a";

    QTest::addRow("Ctrl+X") << "Ctrl+X"
                            << "key ctrl x";
    QTest::addRow("Ctrl X") << "Ctrl X"
                            << "key ctrl x";
    QTest::addRow("key Ctrl+X") << "key Ctrl+X"
                                << "key ctrl x";
    QTest::addRow("key +Ctrl +X") << "key +Ctrl +X"
                                  << "key ctrl x";
    QTest::addRow("key Ctrl X") << "key Ctrl X"
                                << "key ctrl x";
    QTest::addRow("key +Ctrl +X -X") << "key +Ctrl +X -X"
                                     << "key ctrl x";
    QTest::addRow("key +Ctrl +X -x -a") << "key +Ctrl +X -x -a"
                                        << "key ctrl x";

    QTest::addRow("Meta+X") << "Meta+X"
                            << "key super x";
    QTest::addRow("Meta X") << "Meta X"
                            << "key super x";
    QTest::addRow("key Meta+X") << "key Meta+X"
                                << "key super x";
    QTest::addRow("key +Meta +X") << "key +Meta +X"
                                  << "key super x";
    QTest::addRow("key Meta X") << "key Meta X"
                                << "key super x";

    QTest::addRow("+") << "+"
                       << "key plus";
    QTest::addRow("Ctrl++") << "Ctrl++"
                            << "key ctrl plus";
    QTest::addRow("key +") << "key +"
                           << "key plus";
    QTest::addRow("key ++") << "key ++"
                            << "key plus";
    QTest::addRow("key Ctrl +") << "key Ctrl +"
                                << "key ctrl plus";
    QTest::addRow("key +Ctrl ++") << "key +Ctrl ++"
                                  << "key ctrl plus";

    QTest::addRow("-") << "-"
                       << "key minus";
    QTest::addRow("Ctrl+-") << "Ctrl+-"
                            << "key ctrl minus";
    QTest::addRow("key -") << "key -"
                           << "key minus";
    QTest::addRow("key +-") << "key +-"
                            << "key minus";
    QTest::addRow("key Ctrl -") << "key Ctrl -"
                                << "key ctrl minus";
    QTest::addRow("key +Ctrl +-") << "key +Ctrl +-"
                                  << "key ctrl minus";
}

void TestButtonShortcut::testModifier()
{
    QFETCH(QString, input);
    QFETCH(QString, expected_output);
    ButtonShortcut shortcut;

    shortcut.set(input);
    assertModifier(shortcut, expected_output);

    shortcut.clear();
    assertUnset(shortcut);

    shortcut.set(input.toLower());
    assertModifier(shortcut, expected_output.toLower());
}

void TestButtonShortcut::testModifier_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("expected_output");

    QTest::addRow("Ctrl+Alt+Shift+Super") << "Ctrl+Alt+Shift+Super"
                                          << "key ctrl alt shift super";
    QTest::addRow("Ctrl+Alt+Shift+Meta") << "Ctrl+Alt+Shift+Meta"
                                         << "key ctrl alt shift super";
    QTest::addRow("Shift+Meta+Ctrl") << "Shift+Meta+Ctrl"
                                     << "key shift super ctrl";
    QTest::addRow("Alt+Ctrl") << "Alt+Ctrl"
                              << "key alt ctrl";
    QTest::addRow("Shift") << "Shift"
                           << "key shift";
    QTest::addRow("Meta") << "Meta"
                          << "key super";
    QTest::addRow("Super") << "Super"
                           << "key super";

    QTest::addRow("key Ctrl+Alt+Shift+Super") << "key Ctrl+Alt+Shift+Super"
                                              << "key ctrl alt shift super";
    QTest::addRow("key Ctrl+Alt+Shift+Meta") << "key Ctrl+Alt+Shift+Meta"
                                             << "key ctrl alt shift super";
    QTest::addRow("key Shift+Meta+Ctrl") << "key Shift+Meta+Ctrl"
                                         << "key shift super ctrl";
    QTest::addRow("key Alt+Ctrl") << "key Alt+Ctrl"
                                  << "key alt ctrl";
    QTest::addRow("key Shift") << "key Shift"
                               << "key shift";
    QTest::addRow("key Meta") << "key Meta"
                              << "key super";
    QTest::addRow("key Super") << "key Super"
                               << "key super";

    QTest::addRow("key +Ctrl +Alt +Shift +Super") << "key +Ctrl +Alt +Shift +Super"
                                                  << "key ctrl alt shift super";
    QTest::addRow("key +Ctrl +Alt +Shift +Meta") << "key +Ctrl +Alt +Shift +Meta"
                                                 << "key ctrl alt shift super";
    QTest::addRow("key +Shift +Meta +Ctrl") << "key +Shift +Meta +Ctrl"
                                            << "key shift super ctrl";
    QTest::addRow("key +Alt +Ctrl") << "key +Alt +Ctrl"
                                    << "key alt ctrl";
    QTest::addRow("key +Shift") << "key +Shift"
                                << "key shift";
    QTest::addRow("key +Meta") << "key +Meta"
                               << "key super";
    QTest::addRow("key +Super") << "key +Super"
                                << "key super";

    QTest::addRow("Ctrl Alt Shift Super") << "Ctrl Alt Shift Super"
                                          << "key ctrl alt shift super";
    QTest::addRow("Ctrl Alt Shift Meta") << "Ctrl Alt Shift Meta"
                                         << "key ctrl alt shift super";
    QTest::addRow("Shift Meta Ctrl") << "Shift Meta Ctrl"
                                     << "key shift super ctrl";
    QTest::addRow("Alt Ctrl") << "Alt Ctrl"
                              << "key alt ctrl";
    QTest::addRow("Shift") << "Shift"
                           << "key shift";
    QTest::addRow("Meta") << "Meta"
                          << "key super";
    QTest::addRow("Super") << "Super"
                           << "key super";
}

void TestButtonShortcut::testQKeySequences()
{
    QFETCH(QString, input);
    QFETCH(QString, expected_output);
    ButtonShortcut shortcut;

    shortcut.set(input);
    assertKeyStroke(shortcut, expected_output, true);

    shortcut.clear();
    assertUnset(shortcut);

    shortcut.set(input.toLower());
    assertKeyStroke(shortcut, expected_output, true);
}

void TestButtonShortcut::testQKeySequences_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("expected_output");

    QTest::addRow("A") << "A"
                       << "A";
    QTest::addRow("key A") << "key A"
                           << "A";
    QTest::addRow("key +A") << "key +A"
                            << "A";

    QTest::addRow("Ctrl+X") << "Ctrl+X"
                            << "Ctrl+X";
    QTest::addRow("Ctrl X") << "Ctrl X"
                            << "Ctrl+X";
    QTest::addRow("key Ctrl+X") << "key Ctrl+X"
                                << "Ctrl+X";
    QTest::addRow("key +Ctrl +X") << "key +Ctrl +X"
                                  << "Ctrl+X";
    QTest::addRow("key Ctrl X") << "key Ctrl X"
                                << "Ctrl+X";

    QTest::addRow("Meta+X") << "Meta+X"
                            << "Meta+X";
    QTest::addRow("Meta X") << "Meta X"
                            << "Meta+X";
    QTest::addRow("key Meta+X") << "key Meta+X"
                                << "Meta+X";
    QTest::addRow("key +Meta +X") << "key +Meta +X"
                                  << "Meta+X";
    QTest::addRow("key Meta X") << "key Meta X"
                                << "Meta+X";
}

#include "testbuttonshortcut.moc"
