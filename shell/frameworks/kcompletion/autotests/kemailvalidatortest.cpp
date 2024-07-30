/*
   SPDX-FileCopyrightText: 2017-2023 Laurent Montel <montel@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KEmailValidator>
#include <QObject>
#include <QTest>

Q_DECLARE_METATYPE(QValidator::State)

class KEmailValidatorTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void shouldValidateEmail_data()
    {
        QTest::addColumn<QString>("email");
        QTest::addColumn<QValidator::State>("state");
        QTest::newRow("empty") << QString() << QValidator::Intermediate;
        QTest::newRow("email") << QStringLiteral("foo@kde.org") << QValidator::Acceptable;
        QTest::newRow("notanemail") << QStringLiteral("foo") << QValidator::Intermediate;
        QTest::newRow("space") << QStringLiteral("foo ") << QValidator::Invalid;
        QTest::newRow("space1") << QStringLiteral(" foo") << QValidator::Invalid;
        QTest::newRow("email2") << QStringLiteral("<foo@kde.org>") << QValidator::Intermediate;
        QTest::newRow("email3") << QStringLiteral("\"bla\" <foo@kde.org>") << QValidator::Invalid;
    }

    void shouldValidateEmail()
    {
        QFETCH(QString, email);
        QFETCH(QValidator::State, state);
        KEmailValidator validator;
        int pos;
        QCOMPARE(validator.validate(email, pos), state);
    }
};

QTEST_GUILESS_MAIN(KEmailValidatorTest)

#include "kemailvalidatortest.moc"
