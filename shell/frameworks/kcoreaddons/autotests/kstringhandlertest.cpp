
#include "kstringhandlertest.h"

#include <QRegularExpression>
#include <QTest>

QTEST_MAIN(KStringHandlerTest)

#include "kstringhandler.h"

QString KStringHandlerTest::test = QStringLiteral("The quick brown fox jumped over the lazy bridge. ");

void KStringHandlerTest::capwords()
{
    QCOMPARE(KStringHandler::capwords(test), QStringLiteral("The Quick Brown Fox Jumped Over The Lazy Bridge. "));
}

void KStringHandlerTest::tagURLs()
{
    QString test = QStringLiteral("Click on https://foo@bar:www.kde.org/yoyo/dyne.html#a1 for info.");
    QCOMPARE(KStringHandler::tagUrls(test),
             QStringLiteral("Click on <a href=\"https://foo@bar:www.kde.org/yoyo/dyne.html#a1\">https://foo@bar:www.kde.org/yoyo/dyne.html#a1</a> for info."));

    test = QStringLiteral("http://www.foo.org/story$806");
    QCOMPARE(KStringHandler::tagUrls(test), QStringLiteral("<a href=\"http://www.foo.org/story$806\">http://www.foo.org/story$806</a>"));

    test = QStringLiteral("http://www.foo.org/bla-(bli)");
    QCOMPARE(KStringHandler::tagUrls(test), QStringLiteral("<a href=\"http://www.foo.org/bla-(bli)\">http://www.foo.org/bla-(bli)</a>"));

    test = QStringLiteral("http://www.foo.org/bla-bli");
    QCOMPARE(KStringHandler::tagUrls(test), QStringLiteral("<a href=\"http://www.foo.org/bla-bli\">http://www.foo.org/bla-bli</a>"));

    // Test with Unicode characters
    test = QStringLiteral("Click on https://foo@bar:www.kde.org/ÿöyo/dyne.html#a1 for info.");
    QCOMPARE(KStringHandler::tagUrls(test),
             QStringLiteral("Click on <a href=\"https://foo@bar:www.kde.org/ÿöyo/dyne.html#a1\">https://foo@bar:www.kde.org/ÿöyo/dyne.html#a1</a> for info."));
}

void KStringHandlerTest::perlSplitTextSep()
{
    QStringList expected;
    expected << QStringLiteral("some") << QStringLiteral("string") << QStringLiteral("for") << QStringLiteral("you__here");
    QCOMPARE(KStringHandler::perlSplit(QStringLiteral("__"), QStringLiteral("some__string__for__you__here"), 4), expected);

    expected.clear();
    expected << QStringLiteral("kparts") << QStringLiteral("reaches") << QStringLiteral("the parts other parts can't");
    QCOMPARE(KStringHandler::perlSplit(QLatin1Char(' '), QStringLiteral("kparts reaches the parts other parts can't"), 3), expected);
}

void KStringHandlerTest::perlSplitRegexSep()
{
    QCOMPARE(KStringHandler::perlSplit(QRegularExpression(QStringLiteral("[! ]")), QStringLiteral("Split me up ! I'm bored ! OK ?"), 3),
             (QStringList{QStringLiteral("Split"), QStringLiteral("me"), QStringLiteral("up ! I'm bored ! OK ?")}));

    QCOMPARE(KStringHandler::perlSplit(QRegularExpression(QStringLiteral("\\W")), QStringLiteral("aaa ggg cd ef"), 3),
             (QStringList{QStringLiteral("aaa"), QStringLiteral("ggg"), QStringLiteral("cd ef")}));

    // Test with Unicode characters
    QCOMPARE(KStringHandler::perlSplit(QRegularExpression(QStringLiteral("\\W")), QStringLiteral("aaa gǵg cd ef"), 3),
             (QStringList{QStringLiteral("aaa"), QStringLiteral("gǵg"), QStringLiteral("cd ef")}));
}

void KStringHandlerTest::obscure()
{
    // See bug 167900, obscure() produced chars that could not properly be converted to and from
    // UTF8. The result was that storing passwords with '!' in them did not work.
    QString test = QStringLiteral("!TEST!");
    QString obscured = KStringHandler::obscure(test);
    QByteArray obscuredBytes = obscured.toUtf8();
    QCOMPARE(KStringHandler::obscure(QString::fromUtf8(obscuredBytes.constData())), test);
}

// Zero-Width Space
static const QChar ZWSP(0x200b);
// Word Joiner
static const QChar WJ(0x2060);

void KStringHandlerTest::preProcessWrap_data()
{
    QTest::addColumn<QString>("string");
    QTest::addColumn<QString>("expected");

    // Should result in no additional breaks
    QTest::newRow("spaces") << "foo bar baz"
                            << "foo bar baz";

    // Should insert a ZWSP after each '_'
    QTest::newRow("underscores") << "foo_bar_baz" << QString(QStringLiteral("foo_") + ZWSP + QStringLiteral("bar_") + ZWSP + QStringLiteral("baz"));

    // Should insert a ZWSP after each '-'
    QTest::newRow("hyphens") << "foo-bar-baz" << QString(QStringLiteral("foo-") + ZWSP + QStringLiteral("bar-") + ZWSP + QStringLiteral("baz"));

    // Should insert a ZWSP after each '.'
    QTest::newRow("periods") << "foo.bar.baz" << QString(QStringLiteral("foo.") + ZWSP + QStringLiteral("bar.") + ZWSP + QStringLiteral("baz"));

    // Should insert a ZWSP after each ','
    QTest::newRow("commas") << "foo,bar,baz" << QString(QStringLiteral("foo,") + ZWSP + QStringLiteral("bar,") + ZWSP + QStringLiteral("baz"));

    // Should result in no additional breaks since the '_'s are followed by spaces
    QTest::newRow("mixed underscores and spaces") << "foo_ bar_ baz"
                                                  << "foo_ bar_ baz";

    // Should result in no additional breaks since the '_' is the last char
    QTest::newRow("ends with underscore") << "foo_"
                                          << "foo_";

    // Should insert a ZWSP before '(' and after ')'
    QTest::newRow("parens") << "foo(bar)baz" << QString(QStringLiteral("foo") + ZWSP + QStringLiteral("(bar)") + ZWSP + QStringLiteral("baz"));

    // Should insert a ZWSP before '[' and after ']'
    QTest::newRow("brackets") << "foo[bar]baz" << QString(QStringLiteral("foo") + ZWSP + QStringLiteral("[bar]") + ZWSP + QStringLiteral("baz"));

    // Should insert a ZWSP before '{' and after '}'
    QTest::newRow("curly braces") << "foo{bar}baz" << QString(QStringLiteral("foo") + ZWSP + QStringLiteral("{bar}") + ZWSP + QStringLiteral("baz"));

    // Should insert a ZWSP before '(' but not after ')' since it's the last char
    QTest::newRow("ends with ')'") << "foo(bar)" << QString(QStringLiteral("foo") + ZWSP + QStringLiteral("(bar)"));

    // Should insert a single ZWSP between the '_' and the '('
    QTest::newRow("'_' followed by '('") << "foo_(bar)" << QString(QStringLiteral("foo_") + ZWSP + QStringLiteral("(bar)"));

    // Should insert ZWSP's between the '_' and the '[', between the double
    // '['s and the double ']'s, but not before and after 'bar'
    QTest::newRow("'_' before double brackets") << "foo_[[bar]]"
                                                << QString(QStringLiteral("foo_") + ZWSP + QStringLiteral("[") + ZWSP + QStringLiteral("[bar]") + ZWSP
                                                           + QStringLiteral("]"));

    // Should only insert ZWSP's between the double '['s and the double ']'s
    QTest::newRow("space before double brackets") << "foo [[bar]]"
                                                  << QString(QStringLiteral("foo [") + ZWSP + QStringLiteral("[bar]") + ZWSP + QStringLiteral("]"));

    // Shouldn't result in any additional breaks since the '(' is preceded
    // by a space, and the ')' is followed by a space.
    QTest::newRow("parens with spaces") << "foo (bar) baz"
                                        << "foo (bar) baz";

    // Should insert a WJ (Word Joiner) before a single quote
    QTest::newRow("single quote") << "foo'bar" << QString(QStringLiteral("foo") + WJ + QStringLiteral("'bar"));

    // Should insert a ZWSP between sub-words, but not before nor after the word
    QTest::newRow("camelCase") << "camelCase" << QString(QStringLiteral("camel") + ZWSP + QStringLiteral("Case"));

    // Why limiting yourself to ASCII? More and more programming languages these days allow for Unicode identifiers.
    QTest::newRow("camelCase international") << "приветМир" << QString(QStringLiteral("привет") + ZWSP + QStringLiteral("Мир"));

    // Should insert a ZWSP between sub-words, but not before first (upper case) letter
    QTest::newRow("PascalCase") << "PascalCase" << QString(QStringLiteral("Pascal") + ZWSP + QStringLiteral("Case"));

    // But if a string already contains whitespaces, no need to insert ZWSPs
    QTest::newRow("camelCase with whitespaces") << "camelCase PascalCase"
                                                << "camelCase PascalCase";

    // However, single quote and other tricks should still work.
    QTest::newRow("single quote with whitespaces") << "foo(bar) PascalCase'camelCase"
                                                   << QString(QStringLiteral("foo") + ZWSP + QStringLiteral("(bar) PascalCase") + WJ
                                                              + QStringLiteral("'camelCase"));
}

// Little helper function to make tests diagnostics more readable by humans
static QString replaceZwsp(const QString &string)
{
    const QString replacement = QStringLiteral("<ZWSP>");
    QString result;
    result.reserve(string.length() + string.count(ZWSP) * replacement.length());
    for (const auto i : string) {
        if (i == ZWSP) {
            result += replacement;
        } else {
            result += i;
        }
    }

    return result;
}

void KStringHandlerTest::preProcessWrap()
{
    QFETCH(QString, string);
    QFETCH(QString, expected);

    QCOMPARE(replaceZwsp(KStringHandler::preProcessWrap(string)), replaceZwsp(expected));
}

void KStringHandlerTest::logicalLength_data()
{
    QTest::addColumn<QString>("string");
    QTest::addColumn<int>("expected");

    QTest::newRow("Latin") << "foo bar baz" << 11;
    QTest::newRow("Chinese") << QString::fromUtf8("\xe4\xbd\xa0\xe5\xa5\xbd") << 4;
    QTest::newRow("Japanese") << QString::fromUtf8("\xe9\x9d\x92\xe3\x81\x84\xe7\xa9\xba") << 6;
    QTest::newRow("Korean") << QString::fromUtf8("\xed\x95\x9c\xea\xb5\xad\xec\x96\xb4") << 6;
    QTest::newRow("Mixed") << QString::fromUtf8("KDE\xe6\xa1\x8c\xe9\x9d\xa2") << 7;
}

void KStringHandlerTest::logicalLength()
{
    QFETCH(QString, string);
    QFETCH(int, expected);
    QCOMPARE(KStringHandler::logicalLength(string), expected);
}

void KStringHandlerTest::lsqueeze_data()
{
    QTest::addColumn<QString>("string");
    QTest::addColumn<int>("length");
    QTest::addColumn<QString>("expected");

    QTest::newRow("kde_is_awesome") << "KDE is awesome" << 11 << "... awesome";
    QTest::newRow("kde_is_really_awesome") << "KDE is really awesome" << 20 << "...is really awesome";
    QTest::newRow("kde_is_really_awesome_full") << "KDE is really awesome" << 30 << "KDE is really awesome";
}

void KStringHandlerTest::lsqueeze()
{
    QFETCH(QString, string);
    QFETCH(int, length);
    QFETCH(QString, expected);

    QCOMPARE(KStringHandler::lsqueeze(string, length), expected);
}

void KStringHandlerTest::csqueeze_data()
{
    QTest::addColumn<QString>("string");
    QTest::addColumn<int>("length");
    QTest::addColumn<QString>("expected");

    QTest::newRow("kde_is_awesome") << "KDE is awesome" << 11 << "KDE ...some";
    QTest::newRow("kde_is_really_awesome") << "KDE is really awesome" << 20 << "KDE is r... awesome";
    QTest::newRow("kde_is_really_awesome_full") << "KDE is really awesome" << 30 << "KDE is really awesome";
}

void KStringHandlerTest::csqueeze()
{
    QFETCH(QString, string);
    QFETCH(int, length);
    QFETCH(QString, expected);

    QCOMPARE(KStringHandler::csqueeze(string, length), expected);
}

void KStringHandlerTest::rsqueeze_data()
{
    QTest::addColumn<QString>("string");
    QTest::addColumn<int>("length");
    QTest::addColumn<QString>("expected");

    QTest::newRow("kde_is_awesome") << "KDE is awesome" << 11 << "KDE is a...";
    QTest::newRow("kde_is_really_awesome") << "KDE is really awesome" << 20 << "KDE is really awe...";
    QTest::newRow("kde_is_really_awesome_full") << "KDE is really awesome" << 30 << "KDE is really awesome";
}

void KStringHandlerTest::rsqueeze()
{
    QFETCH(QString, string);
    QFETCH(int, length);
    QFETCH(QString, expected);

    QCOMPARE(KStringHandler::rsqueeze(string, length), expected);
}

#include "moc_kstringhandlertest.cpp"
