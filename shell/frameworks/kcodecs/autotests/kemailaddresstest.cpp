/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2004 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2009 Thomas McGuire <mcguire@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/
// krazy:excludeall=contractions

#include "kemailaddresstest.h"

#include "../src/kemailaddress.h"

#include <QDebug>
#include <QUrl>

#include <QTest>

QTEST_MAIN(KEmailAddressTest)

using namespace KEmailAddress;

Q_DECLARE_METATYPE(EmailParseResult)

void KEmailAddressTest::testGetNameAndEmail()
{
    QFETCH(QString, input);
    QFETCH(QString, expName);
    QFETCH(QString, expEmail);
    QFETCH(bool, expRetVal);

    QString name;
    QString email;
    bool retVal = extractEmailAddressAndName(input, email, name);
    QCOMPARE(retVal, expRetVal);
    QCOMPARE(name, expName);
    QCOMPARE(email, expEmail);
}

void KEmailAddressTest::testGetNameAndEmail_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("expName");
    QTest::addColumn<QString>("expEmail");
    QTest::addColumn<bool>("expRetVal");

    QTest::newRow("Empty input") << QString() << QString() << QString() << false;
    QTest::newRow("Email only") << "faure@kde.org" << QString() << "faure@kde.org" << false;
    QTest::newRow("Normal case") << "David Faure <faure@kde.org>"
                                 << "David Faure"
                                 << "faure@kde.org" << true;
    QTest::newRow("Double-quotes 1") << "\"Faure, David\" <faure@kde.org>"
                                     << "Faure, David"
                                     << "faure@kde.org" << true;
    QTest::newRow("Double-quotes 2") << "<faure@kde.org> \"David Faure\""
                                     << "David Faure"
                                     << "faure@kde.org" << true;
    QTest::newRow("Parenthesis 1") << "faure@kde.org (David Faure)"
                                   << "David Faure"
                                   << "faure@kde.org" << true;
    QTest::newRow("Parenthesis 2") << "(David Faure) faure@kde.org"
                                   << "David Faure"
                                   << "faure@kde.org" << true;
    QTest::newRow("Parenthesis 3") << "My Name (me) <me@home.net>"
                                   << "My Name (me)"
                                   << "me@home.net" << true; // #93513

    // As per https://intevation.de/roundup/kolab/issue858
    QTest::newRow("Nested parenthesis") << "faure@kde.org (David (The Man) Faure)"
                                        << "David (The Man) Faure"
                                        << "faure@kde.org" << true;
    QTest::newRow("Double-quotes inside parenthesis 1") << "faure@kde.org (David \"Crazy\" Faure)"
                                                        << "David \"Crazy\" Faure"
                                                        << "faure@kde.org" << true;
    QTest::newRow("Double-quotes inside parenthesis 2") << "(David \"Crazy\" Faure) faure@kde.org"
                                                        << "David \"Crazy\" Faure"
                                                        << "faure@kde.org" << true;
    QTest::newRow("Parenthesis inside double-quotes 1") << "\"Faure (David)\" <faure@kde.org>"
                                                        << "Faure (David)"
                                                        << "faure@kde.org" << true;
    QTest::newRow("Parenthesis inside double-quotes 2") << "<faure@kde.org> \"Faure (David)\""
                                                        << "Faure (David)"
                                                        << "faure@kde.org" << true;
    QTest::newRow("Space in email") << "David Faure < faure@kde.org >"
                                    << "David Faure"
                                    << "faure@kde.org" << true;
    QTest::newRow("'@' in name 1") << "faure@kde.org (a@b)"
                                   << "a@b"
                                   << "faure@kde.org" << true;
    QTest::newRow("'@' in name 2") << "\"a@b\" <faure@kde.org>"
                                   << "a@b"
                                   << "faure@kde.org" << true;

    // While typing, when there's no '@' yet
    QTest::newRow("while typing 1") << "foo"
                                    << "foo" << QString() << false;
    QTest::newRow("while typing 2") << "foo <"
                                    << "foo" << QString() << false;
    QTest::newRow("while typing 3") << "foo <b"
                                    << "foo"
                                    << "b" << true;

    // If multiple emails are there, only return the first one
    QTest::newRow("multiple emails") << "\"Faure, David\" <faure@kde.org>, KHZ <khz@khz.khz>"
                                     << "Faure, David"
                                     << "faure@kde.org" << true;

    QTest::newRow("domain literals") << "Matt Douhan <matt@[123.123.123.123]>"
                                     << "Matt Douhan"
                                     << "matt@[123.123.123.123]" << true;
    QTest::newRow("@ inside the comment") << "\"Matt@Douhan\" <matt@fruitsalad.org>"
                                          << "Matt@Douhan"
                                          << "matt@fruitsalad.org" << true;
    QTest::newRow("No '@'") << "foo <distlist>"
                            << "foo"
                            << "distlist" << true;
    QTest::newRow("Backslash in display name") << "\"Lastname\\, Firstname\""
                                                  " <firstname@lastname.com>"
                                               << "Lastname, Firstname"
                                               << "firstname@lastname.com" << true;
    QTest::newRow("# in domain") << "Matt Douhan <dm3tt@db0zdf.#rpl.deu.eu>"
                                 << "Matt Douhan"
                                 << "dm3tt@db0zdf.#rpl.deu.eu" << true;
}

void KEmailAddressTest::testIsValidEmailAddress()
{
    QFETCH(QString, input);
    QFETCH(EmailParseResult, expErrorCode);

    QCOMPARE(isValidAddress(input), expErrorCode);
}

void KEmailAddressTest::testIsValidEmailAddress_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<EmailParseResult>("expErrorCode");

    // Too many @'s
    QTest::newRow("1") << "matt@@fruitsalad.org" << TooManyAts;

    // Too few @'s
    QTest::newRow("2") << "mattfruitsalad.org" << TooFewAts;

    // An empty string
    QTest::newRow("3") << QString() << AddressEmpty;

    // email address starting with a @
    QTest::newRow("4") << "@mattfruitsalad.org" << MissingLocalPart;

    // make sure that starting @ and an additional @ in the same email address don't conflict
    // trap the starting @ first and break
    QTest::newRow("5") << "@matt@fruitsalad.org" << MissingLocalPart;

    // email address ending with a @
    QTest::newRow("6") << "mattfruitsalad.org@" << MissingDomainPart;

    // make sure that ending with@ and an additional @ in the email address don't conflict
    QTest::newRow("7") << "matt@fruitsalad.org@" << MissingDomainPart;

    // unbalanced Parens
    QTest::newRow("8") << "mattjongel)@fruitsalad.org" << UnbalancedParens;

    // unbalanced Parens the other way around
    QTest::newRow("9") << "mattjongel(@fruitsalad.org" << UnbalancedParens;

    // Correct parens just to make sure it works
    QTest::newRow("10") << "matt(jongel)@fruitsalad.org" << AddressOk;

    // Check that anglebrackets are closed
    QTest::newRow("11") << "matt douhan<matt@fruitsalad.org" << UnclosedAngleAddr;

    // Check that angle brackets are closed the other way around
    QTest::newRow("12") << "matt douhan>matt@fruitsalad.org" << UnopenedAngleAddr;

    // Check that angle brackets are closed the other way around, and anglebrackets in domainpart
    // instead of local part
    QTest::newRow("13") << "matt douhan matt@<fruitsalad.org" << UnclosedAngleAddr;

    // check that a properly formatted anglebrackets situation is OK
    QTest::newRow("14") << "matt douhan<matt@fruitsalad.org>" << AddressOk;

    // a full email address with comments angle brackets and the works should be valid too
    QTest::newRow("15") << "Matt (jongel) Douhan <matt@fruitsalad.org>" << AddressOk;

    // Double quotes
    QTest::newRow("16") << "\"Matt Douhan\" <matt@fruitsalad.org>" << AddressOk;

    // Double quotes inside parens
    QTest::newRow("17") << "Matt (\"jongel\") Douhan <matt@fruitsalad.org>" << AddressOk;

    // DOuble quotes not closed
    QTest::newRow("18") << "Matt \"jongel Douhan <matt@fruitsalad.org>" << UnbalancedQuote;

    // Parens inside double quotes
    QTest::newRow("19") << "Matt \"(jongel)\" Douhan <matt@fruitsalad.org>" << AddressOk;

    // Space in email
    QTest::newRow("20") << "Matt Douhan < matt@fruitsalad.org >" << AddressOk;

    // @ is allowed inisde doublequotes
    QTest::newRow("21") << "\"matt@jongel\" <matt@fruitsalad.org>" << AddressOk;

    // anglebrackets inside dbl quotes
    QTest::newRow("22") << "\"matt<blah blah>\" <matt@fruitsalad.org>" << AddressOk;

    // a , inside a double quoted string is OK, how do I know this? well Ingo says so
    // and it makes sense since it is also a separator of email addresses
    QTest::newRow("23") << "\"Douhan, Matt\" <matt@fruitsalad.org>" << AddressOk;

    // Domains literals also need to work
    QTest::newRow("24") << "Matt Douhan <matt@[123.123.123.123]>" << AddressOk;

    // Typo in domain literal address
    QTest::newRow("25") << "Matt Douhan <matt@[123.123.123,123]>" << UnexpectedComma;

    // Some more insane tests but still valid so they must work
    QTest::newRow("26") << "Matt Douhan <\"m@att\"@jongel.com>" << AddressOk;

    // BUG 99657
    QTest::newRow("27") << "matt@jongel.fibbel.com" << AddressOk;

    // BUG 98720
    QTest::newRow("28") << "mailto:@mydomain" << DisallowedChar;

    // correct error msg when a comma is inside <>
    QTest::newRow("29") << "Matt Douhan <matt@fruitsalad,org>" << UnexpectedComma;

    // several commentlevels
    QTest::newRow("30") << "Matt Douhan (hey(jongel)fibbel) <matt@fruitsalad.org>" << AddressOk;

    // several comment levels and one (the outer) being unbalanced
    QTest::newRow("31") << "Matt Douhan (hey(jongel)fibbel <matt@fruitsalad.org>" << UnbalancedParens;

    // several comment levels and one (the inner) being unbalanced
    QTest::newRow("32") << "Matt Douhan (hey(jongelfibbel) <matt@fruitsalad.org>" << UnbalancedParens;

    // an error inside a double quote is no error
    QTest::newRow("33") << "Matt Douhan \"(jongel\" <matt@fruitsalad.org>" << AddressOk;

    // inside a quoted string double quotes are only allowed in pairs as per rfc2822
    QTest::newRow("34") << "Matt Douhan \"jongel\"fibbel\" <matt@fruitsalad.org>" << UnbalancedQuote;

    // a questionmark is valid in an atom
    QTest::newRow("35") << "Matt? <matt@fruitsalad.org>" << AddressOk;

    // weird but OK
    QTest::newRow("36") << "\"testing, \\\"testing\" <matt@fruitsalad.org>" << AddressOk;

    // escape a quote to many to see if it makes it invalid
    QTest::newRow("37") << "\"testing, \\\"testing\\\" <matt@fruitsalad.org>" << UnbalancedQuote;

    // escape a parens and thus make a comma appear
    QTest::newRow("38") << "Matt (jongel, fibbel\\) <matt@fruitsalad.org>" << UnbalancedParens;

    // several errors inside doublequotes
    QTest::newRow("39") << "Matt \"(jongel,\\\" < fibbel\\)\" <matt@fruitsalad.org>" << AddressOk;

    // BUG 105705
    QTest::newRow("40") << "matt-@fruitsalad.org" << AddressOk;

    // underscore at the end of local part
    QTest::newRow("41") << "matt_@fruitsalad.org" << AddressOk;

    // how about ( comment ) in the domain part
    QTest::newRow("42") << "matt_@(this is a cool host)fruitsalad.org" << AddressOk;

    // To quote rfc2822 the test below is aesthetically displeasing, but perfectly legal.
    QTest::newRow("43") << "Pete(A wonderful \\) chap) <pete(his account)@silly.test(his host)>" << AddressOk;

    // quoted pair or not quoted pair
    QTest::newRow("44") << "\"jongel '\\\" fibbel\" <matt@fruitsalad.org>" << AddressOk;
    QTest::newRow("45") << "\"jongel '\" fibbel\" <matt@fruitsalad.org>" << UnbalancedQuote;

    // full atext support according to rfc2822
    QTest::newRow("46") << "!matt@fruitsalad.org" << AddressOk;
    QTest::newRow("47") << "#matt@fruitsalad.org" << AddressOk;
    QTest::newRow("48") << "$matt@fruitsalad.org" << AddressOk;
    QTest::newRow("49") << "%matt@fruitsalad.org" << AddressOk;
    QTest::newRow("50") << "&matt@fruitsalad.org" << AddressOk;
    QTest::newRow("51") << "'matt@fruitsalad.org" << AddressOk;
    QTest::newRow("52") << "*matt@fruitsalad.org" << AddressOk;
    QTest::newRow("53") << "+matt@fruitsalad.org" << AddressOk;
    QTest::newRow("54") << "/matt@fruitsalad.org" << AddressOk;
    QTest::newRow("55") << "=matt@fruitsalad.org" << AddressOk;
    QTest::newRow("56") << "?matt@fruitsalad.org" << AddressOk;
    QTest::newRow("57") << "^matt@fruitsalad.org" << AddressOk;
    QTest::newRow("58") << "_matt@fruitsalad.org" << AddressOk;
    QTest::newRow("59") << "-matt@fruitsalad.org" << AddressOk;
    QTest::newRow("60") << "`matt@fruitsalad.org" << AddressOk;
    QTest::newRow("61") << "{matt@fruitsalad.org" << AddressOk;
    QTest::newRow("62") << "|matt@fruitsalad.org" << AddressOk;
    QTest::newRow("63") << "}matt@fruitsalad.org" << AddressOk;
    QTest::newRow("64") << "~matt@fruitsalad.org" << AddressOk;
    QTest::newRow("65") << "matt%matt@fruitsalad.org" << AddressOk;

    // bug 105405
    QTest::newRow("66") << "[foobar] <matt@fruitsalad.org>" << InvalidDisplayName;
    QTest::newRow("67") << "matt \"[foobar]\" Douhan <matt@fruitsalad.org>" << AddressOk;

    QTest::newRow("68") << "Matt Douhan <matt\"@@\"fruitsalad.org>" << TooFewAts;

    QTest::newRow("# in domain") << "dm3tt@db0zdf.#rpl.deu.eu" << AddressOk;
    QTest::newRow("dot at the end") << "msadmin@guug.de." << AddressOk;
    QTest::newRow("dot at the end with brackets") << "Martin Schulte <martin.schulte@guug.de.>" << AddressOk;

    // TODO this should be a valid email address, but the checking for missing dots broke it.
    // QTest::newRow( "valid email address without dots" ) << "user@localhost" << AddressOk;
}

void KEmailAddressTest::testIsValidAddressList()
{
    QFETCH(QString, list);
    QFETCH(EmailParseResult, expErrorCode);

    QString badAddress;
    QCOMPARE(isValidAddressList(list, badAddress), expErrorCode);
}

void KEmailAddressTest::testIsValidAddressList_data()
{
    QTest::addColumn<QString>("list");
    QTest::addColumn<EmailParseResult>("expErrorCode");

    // bug  139477
    QTest::newRow("1") << "martin.schulte@guug.de, msadmin@guug.de,"
                          " msnewsletter@guug.de"
                       << AddressOk;
    QTest::newRow("2") << "martin.schulte@guug.de; msadmin@guug.de;"
                          " msnewsletter@guug.de"
                       << AddressOk;
    QTest::newRow("3") << "martin.schulte@guug.de, msadmin@guug.de.,"
                          " msnewsletter@guug.de"
                       << AddressOk;
    QTest::newRow("4") << "Martin Schulte <martin.schulte@guug.de>,"
                          " MS Admin <msadmin@guug.de>, MS News <msnewsletter@guug.de>"
                       << AddressOk;
    QTest::newRow("5") << "Martin Schulte <martin.schulte@guug.de>;"
                          " MS Admin <msadmin@guug.de>; MS News <msnewsletter@guug.de>"
                       << AddressOk;
    QTest::newRow("6") << "Martin Schulte <martin.schulte@guug.de.>,"
                          " MS Admin <msadmin@guug.de>, MS News <msnewsletter@guug.de>"
                       << AddressOk;
}

void KEmailAddressTest::testIsValidSimpleEmailAddress()
{
    QFETCH(QString, input);
    QFETCH(bool, expResult);

    QCOMPARE(isValidSimpleAddress(input), expResult);
}

void KEmailAddressTest::testIsValidSimpleEmailAddress_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<bool>("expResult");

    // checks for "pure" email addresses in the form of xxx@yyy.tld
    QTest::newRow("") << "matt@fruitsalad.org" << true;
    QTest::newRow("") << QStringLiteral("test@täst.invalid") << true;
    // non-ASCII char as first char of IDN
    QTest::newRow("") << QStringLiteral("i_want@øl.invalid") << true;
    QTest::newRow("") << "matt@[123.123.123.123]" << true;
    QTest::newRow("") << "matt@[3.3.3.3]" << true;
    QTest::newRow("") << "matt@[4.4.4.4]" << true;
    QTest::newRow("") << "matt@[192.168.254.254]" << true;
    QTest::newRow("") << "\"matt\"@fruitsalad.org" << true;
    QTest::newRow("") << "-matt@fruitsalad.org" << true;
    QTest::newRow("") << "\"-matt\"@fruitsalad.org" << true;
    QTest::newRow("") << "matt@jongel.fibbel.com" << true;
    QTest::newRow("") << "Matt Douhan <matt@fruitsalad.org>" << false;
    // BUG 105705
    QTest::newRow("") << "matt-@fibbel.com" << true;
    QTest::newRow("") << "matt@fibbel-is-a-geek.com" << true;
    QTest::newRow("") << "matt_@fibbel.com" << true;
    // Check the defined chars for atext according to rfc2822
    QTest::newRow("") << "!matt@fruitsalad.org" << true;
    QTest::newRow("") << "#matt@fruitsalad.org" << true;
    QTest::newRow("") << "$matt@fruitsalad.org" << true;
    QTest::newRow("") << "%matt@fruitsalad.org" << true;
    QTest::newRow("") << "&matt@fruitsalad.org" << true;
    QTest::newRow("") << "'matt@fruitsalad.org" << true;
    QTest::newRow("") << "*matt@fruitsalad.org" << true;
    QTest::newRow("") << "+matt@fruitsalad.org" << true;
    QTest::newRow("") << "/matt@fruitsalad.org" << true;
    QTest::newRow("") << "=matt@fruitsalad.org" << true;
    QTest::newRow("") << "?matt@fruitsalad.org" << true;
    QTest::newRow("") << "^matt@fruitsalad.org" << true;
    QTest::newRow("") << "_matt@fruitsalad.org" << true;
    QTest::newRow("") << "-matt@fruitsalad.org" << true;
    QTest::newRow("") << "`matt@fruitsalad.org" << true;
    QTest::newRow("") << "{matt@fruitsalad.org" << true;
    QTest::newRow("") << "|matt@fruitsalad.org" << true;
    QTest::newRow("") << "}matt@fruitsalad.org" << true;
    QTest::newRow("") << "~matt@fruitsalad.org" << true;
    // BUG 108476
    QTest::newRow("") << "foo+matt@fruitsalad.org" << true;
    QTest::newRow("") << "bar=matt@fruitsalad.org" << true;
    QTest::newRow("") << "jongel-matt@fruitsalad.org" << true;
    QTest::newRow("") << "matt-@fruitsalad.org" << true;

    // check if the pure email address is wrong
    QTest::newRow("") << "mattfruitsalad.org" << false;
    QTest::newRow("") << "matt@[123.123.123.123" << false;
    QTest::newRow("") << "matt@123.123.123.123]" << false;
    QTest::newRow("") << "matt@[.123.123.123]" << false;
    QTest::newRow("") << "matt@[123.123.123]" << false;
    QTest::newRow("") << "\"matt@fruitsalad.org" << false;
    QTest::newRow("") << "matt\"@fruitsalad.org" << false;
    QTest::newRow("") << QString() << false;

    // BUG 203881
    QTest::newRow("") << "2advance@my-site.com" << true;

    // and here some insane but still valid cases
    QTest::newRow("") << "\"m@tt\"@fruitsalad.org" << true;

    QTest::newRow("") << "matt\"@@\"fruitsalad.org" << false;
    QTest::newRow("# in domain") << "dm3tt@db0zdf.#rpl.deu.eu" << true;

    // add tests for missing local/domain parts
    QTest::newRow("") << "@mattfruitsalad.org" << false;
    QTest::newRow("") << "matt@" << false;
    QTest::newRow("") << "@" << false;
}

void KEmailAddressTest::testGetEmailAddress()
{
    QFETCH(QString, input);
    QFETCH(QString, expResult);

    QCOMPARE(extractEmailAddress(input), expResult);
}

void KEmailAddressTest::testGetEmailAddress_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("expResult");

    QTest::newRow("1") << "matt@fruitsalad.org"
                       << "matt@fruitsalad.org";
    QTest::newRow("2") << "Matt Douhan <matt@fruitsalad.org>"
                       << "matt@fruitsalad.org";
    QTest::newRow("3") << "\"Matt Douhan <blah blah>\" <matt@fruitsalad.org>"
                       << "matt@fruitsalad.org";
    QTest::newRow("4") << "\"Matt <blah blah>\" <matt@fruitsalad.org>"
                       << "matt@fruitsalad.org";
    QTest::newRow("5") << "Matt Douhan (jongel) <matt@fruitsalad.org" << QString();
    QTest::newRow("6") << "Matt Douhan (m@tt) <matt@fruitsalad.org>"
                       << "matt@fruitsalad.org";
    QTest::newRow("7") << "\"Douhan, Matt\" <matt@fruitsalad.org>"
                       << "matt@fruitsalad.org";
    QTest::newRow("8") << "\"Matt Douhan (m@tt)\" <matt@fruitsalad.org>"
                       << "matt@fruitsalad.org";
    QTest::newRow("9") << "\"Matt Douhan\" (matt <matt@fruitsalad.org>" << QString();
    QTest::newRow("10") << "Matt Douhan <matt@[123.123.123.123]>"
                        << "matt@[123.123.123.123]";
    QTest::newRow("11") << "dm3tt@db0zdf.#rpl.deu.eu"
                        << "dm3tt@db0zdf.#rpl.deu.eu";
}

void KEmailAddressTest::testCheckSplitEmailAddrList()
{
    QFETCH(QString, input);
    QFETCH(QStringList, expResult);

    QCOMPARE(splitAddressList(input), expResult);
}

void KEmailAddressTest::testCheckSplitEmailAddrList_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QStringList>("expResult");

    QTest::newRow("") << "kloecker@kde.org (Kloecker, Ingo)" << (QStringList() << QStringLiteral("kloecker@kde.org (Kloecker, Ingo)"));
    QTest::newRow("") << "Matt Douhan <matt@fruitsalad.org>, Foo Bar <foo@bar.com>"
                      << (QStringList() << QStringLiteral("Matt Douhan <matt@fruitsalad.org>") << QStringLiteral("Foo Bar <foo@bar.com>"));
    QTest::newRow("") << "\"Matt, Douhan\" <matt@fruitsalad.org>, Foo Bar <foo@bar.com>"
                      << (QStringList() << QStringLiteral("\"Matt, Douhan\" <matt@fruitsalad.org>") << QStringLiteral("Foo Bar <foo@bar.com>"));
    QTest::newRow("") << "\"Lastname\\, Firstname\" <firstname.lastname@example.com>"
                      << (QStringList() << QStringLiteral("\"Lastname\\, Firstname\" <firstname.lastname@example.com>"));
    QTest::newRow("") << "First Collective Last <collectivename.lastname@example.com>" // Uses non-ASCII spacing
                      << (QStringList() << QStringLiteral("First Collective Last <collectivename.lastname@example.com>"));
    QTest::newRow("") << "Foo Bar <foo@bar.com>, First Collective Last <collectivename.lastname@example.com>" // Uses non-ASCII spacing
                      << (QStringList() << QStringLiteral("Foo Bar <foo@bar.com>")
                                        << QStringLiteral("First Collective Last <collectivename.lastname@example.com>"));
    QTest::newRow("")
        << "Foo  Bar <foo@bar.com>,  \"   First  Collective Last  \"  <collectivename.lastname@example.com>" // Uses multiple and quoted surrounding spaces
        << (QStringList() << QStringLiteral("Foo  Bar <foo@bar.com>")
                          << QStringLiteral("\"   First  Collective Last  \"  <collectivename.lastname@example.com>"));
}

void KEmailAddressTest::testNormalizeAddressesAndEncodeIDNs()
{
    QFETCH(QString, input);
    QFETCH(QString, expResult);

    QCOMPARE(normalizeAddressesAndEncodeIdn(input), expResult);
}

void KEmailAddressTest::testNormalizeAddressesAndEncodeIDNs_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("expResult");

    QTest::newRow("") << "matt@fruitsalad.org"
                      << "matt@fruitsalad.org";
    QTest::newRow("") << "Matt Douhan <matt@fruitsalad.org>"
                      << "Matt Douhan <matt@fruitsalad.org>";
    QTest::newRow("") << "Matt Douhan (jongel) <matt@fruitsalad.org>"
                      << "Matt Douhan (jongel) <matt@fruitsalad.org>";
    QTest::newRow("") << "Matt Douhan (jongel,fibbel) <matt@fruitsalad.org>"
                      << "Matt Douhan (jongel,fibbel) <matt@fruitsalad.org>";
    QTest::newRow("") << "matt@fruitsalad.org (jongel,fibbel)"
                      << "\"jongel,fibbel\" <matt@fruitsalad.org>";
    QTest::newRow("") << "matt@fruitsalad.org (\"jongel,fibbel\")"
                      << "\"jongel,fibbel\" <matt@fruitsalad.org>";

#if QT_VERSION >= 0x060300
    // Test using IDNA-2008 domain name, before Qt 6.3 this incorrectly encodes
    // the email address to matt@strasse.de (IDNA-2003) instead
    QTest::newRow("") << "Matt Douhan <matt@straße.de>"
                      << "Matt Douhan <matt@xn--strae-oqa.de>";
#endif

    QTest::newRow("") << "First Collective Last <collectivename.lastname@example.com>" // Uses non-ASCII spacing
                      << "First Collective Last <collectivename.lastname@example.com>";
    QTest::newRow("") << "Foo Bar <foo@bar.com> , First Collective Last <collectivename.lastname@example.com>" // Uses non-ASCII spacing
                      << "Foo Bar <foo@bar.com>, First Collective Last <collectivename.lastname@example.com>";
    QTest::newRow("")
        << "Foo  Bar  <foo@bar.com>,  \"   First  Collective Last  \"  <collectivename.lastname@example.com>" // Uses multiple and quoted surrounding spaces
        << "Foo  Bar <foo@bar.com>, \"   First  Collective Last  \" <collectivename.lastname@example.com>";
}

void KEmailAddressTest::testNormalizeAddressesAndDecodeIDNs()
{
    QFETCH(QString, input);
    QFETCH(QString, expResult);

    QCOMPARE(normalizeAddressesAndDecodeIdn(input), expResult);
}

void KEmailAddressTest::testNormalizeAddressesAndDecodeIDNs_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("expResult");

    QTest::newRow("") << "=?us-ascii?Q?Surname=2C=20Name?= <nobody@example.org>"
                      << "\"Surname, Name\" <nobody@example.org>";

    QTest::newRow("") << "=?iso-8859-1?B?5Hf8b2xmLPZBbmRyZWFz?= <nobody@example.org>" << QStringLiteral("\"äwüolf,öAndreas\" <nobody@example.org>");

    QTest::newRow("") << QStringLiteral("\"Andreas Straß\" <nobody@example.org>") << QStringLiteral("\"Andreas Straß\" <nobody@example.org>");

    QTest::newRow("") << QStringLiteral("\"András\" \"Manţia\" <amantia@kde.org>") << QStringLiteral("\"András\" \"Manţia\" <amantia@kde.org>");
}

void KEmailAddressTest::testQuoteIfNecessary()
{
    QFETCH(QString, input);
    QFETCH(QString, expResult);
    QBENCHMARK {
        QCOMPARE(quoteNameIfNecessary(input), expResult);
    }
}

void KEmailAddressTest::testQuoteIfNecessary_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("expResult");

    QTest::newRow("") << "Matt Douhan"
                      << "Matt Douhan";
    QTest::newRow("") << "Douhan, Matt"
                      << "\"Douhan, Matt\"";
    QTest::newRow("") << "Matt \"jongel\" Douhan"
                      << "\"Matt \\\"jongel\\\" Douhan\"";
    QTest::newRow("") << "Matt \\\"jongel\\\" Douhan"
                      << "\"Matt \\\"jongel\\\" Douhan\"";
    QTest::newRow("") << "trailing '\\\\' should never occur \\"
                      << "\"trailing '\\\\' should never occur \\\"";
    QTest::newRow("") << "\"don't quote again\""
                      << "\"don't quote again\"";
    QTest::newRow("") << "\"leading double quote"
                      << "\"\\\"leading double quote\"";
    QTest::newRow("") << "trailing double quote\""
                      << "\"trailing double quote\\\"\"";
#if 0
    BEFORE: static QRegularExpression
      RESULT : KEmailAddressTest::testQuoteIfNecessary():
      0.015 msecs per iteration (total: 64, iterations: 4096)
      0.019 msecs per iteration (total: 80, iterations: 4096)
      0.019 msecs per iteration (total: 81, iterations: 4096)
      0.0244 msecs per iteration (total: 100, iterations: 4096)
      0.0079 msecs per iteration (total: 65, iterations: 8192)
      0.020 msecs per iteration (total: 82, iterations: 4096)
      0.020 msecs per iteration (total: 82, iterations: 4096)

      AFTER:
      RESULT : KEmailAddressTest::testQuoteIfNecessary():
      0.0097 msecs per iteration (total: 80, iterations: 8192)
      0.010 msecs per iteration (total: 82, iterations: 8192)
      0.015 msecs per iteration (total: 63, iterations: 4096)
      0.0076 msecs per iteration (total: 63, iterations: 8192)
      0.0098 msecs per iteration (total: 81, iterations: 8192)
      0.010 msecs per iteration (total: 87, iterations: 8192)
#endif
}

void KEmailAddressTest::testMailtoUrls()
{
    QFETCH(QString, input);
    const QUrl url = encodeMailtoUrl(input);
    qDebug() << url;
    QCOMPARE(url.scheme().toLatin1().data(), "mailto");
    QCOMPARE(decodeMailtoUrl(url), input);
    qDebug() << decodeMailtoUrl(url);
}

void KEmailAddressTest::testMailtoUrls_data()
{
    QTest::addColumn<QString>("input");

    QTest::newRow("") << "tokoe@domain.com";
    QTest::newRow("") << QStringLiteral("\"Tobias König\" <tokoe@domain.com>");
    QTest::newRow("") << QStringLiteral("\"Alberto Simões\" <alberto@example.com");
    QTest::newRow("") << QStringLiteral("Alberto Simões <alberto@example.com");
}

#include "moc_kemailaddresstest.cpp"
