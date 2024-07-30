/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2003, 2007-2008 Oswald Buddenhagen <ossi@kde.org>
    SPDX-FileCopyrightText: 2005 Thomas Braxton <brax108@cox.net>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include <kshell.h>
#include <kuser.h>

#include <QTest>

#include <QDir>
#include <QObject>
#include <QString>
#include <QStringList>

class KShellTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void tildeExpand();
    void tildeCollapse();
    void quoteArg();
    void joinArgs();
    void splitJoin();
    void quoteSplit();
    void quoteSplit_data();
    void abortOnMeta();
};

// The expansion of ~me isn't exactly QDir::homePath(), in case $HOME has a trailing slash, it's kept.
static QString myHomePath()
{
#ifdef Q_OS_WIN
    return QDir::homePath();
#else
    return QString::fromLocal8Bit(qgetenv("HOME"));
#endif
}

void KShellTest::tildeExpand()
{
    QString me(KUser().loginName());
    QCOMPARE(KShell::tildeExpand(QStringLiteral("~")), QDir::homePath());
    QCOMPARE(KShell::tildeExpand(QStringLiteral("~/dir")), QString(QDir::homePath() + QStringLiteral("/dir")));
    QCOMPARE(KShell::tildeExpand(QLatin1Char('~') + me), myHomePath());
    QCOMPARE(KShell::tildeExpand(QLatin1Char('~') + me + QStringLiteral("/dir")), QString(myHomePath() + QStringLiteral("/dir")));
#ifdef Q_OS_WIN
    QCOMPARE(KShell::tildeExpand(QStringLiteral("^~") + me), QString(QLatin1Char('~') + me));
#else
    QCOMPARE(KShell::tildeExpand(QStringLiteral("\\~") + me), QString(QStringLiteral("~") + me));
#endif
}

void KShellTest::tildeCollapse()
{
    QCOMPARE(KShell::tildeCollapse(QDir::homePath()), QStringLiteral("~"));
    QCOMPARE(KShell::tildeCollapse(QDir::homePath() + QStringLiteral("/Documents")), QStringLiteral("~/Documents"));
    QCOMPARE(KShell::tildeCollapse(QStringLiteral("/test/") + QDir::homePath()), QStringLiteral("/test/") + QDir::homePath());
}

void KShellTest::quoteArg()
{
#ifdef Q_OS_WIN
    QCOMPARE(KShell::quoteArg(QStringLiteral("a space")), QStringLiteral("\"a space\""));
    QCOMPARE(KShell::quoteArg(QStringLiteral("fds\\\"")), QStringLiteral("fds\\\\\\^\""));
    QCOMPARE(KShell::quoteArg(QStringLiteral("\\\\foo")), QStringLiteral("\\\\foo"));
    QCOMPARE(KShell::quoteArg(QStringLiteral("\"asdf\"")), QStringLiteral("\\^\"asdf\\^\""));
    QCOMPARE(KShell::quoteArg(QStringLiteral("with\\")), QStringLiteral("\"with\\\\\""));
    QCOMPARE(KShell::quoteArg(QStringLiteral("\\\\")), QStringLiteral("\"\\\\\\\\\""));
    QCOMPARE(KShell::quoteArg(QStringLiteral("\"a space\\\"")), QStringLiteral("\\^\"\"a space\"\\\\\\^\""));
    QCOMPARE(KShell::quoteArg(QStringLiteral("as df\\")), QStringLiteral("\"as df\\\\\""));
    QCOMPARE(KShell::quoteArg(QStringLiteral("foo bar\"\\\"bla")), QStringLiteral("\"foo bar\"\\^\"\\\\\\^\"\"bla\""));
    QCOMPARE(KShell::quoteArg(QStringLiteral("a % space")), QStringLiteral("\"a %PERCENT_SIGN% space\""));
#else
    QCOMPARE(KShell::quoteArg(QStringLiteral("a space")), QStringLiteral("'a space'"));
#endif
}

void KShellTest::joinArgs()
{
    QStringList list;
    list << QStringLiteral("this") << QStringLiteral("is") << QStringLiteral("a") << QStringLiteral("test");
    QCOMPARE(KShell::joinArgs(list), QStringLiteral("this is a test"));
}

static QString sj(const QString &str, KShell::Options flags, KShell::Errors *ret)
{
    return KShell::joinArgs(KShell::splitArgs(str, flags, ret));
}

void KShellTest::splitJoin()
{
    KShell::Errors err = KShell::NoError;

#ifdef Q_OS_WIN
    QCOMPARE(sj(QStringLiteral("\"(sulli)\" text"), KShell::NoOptions, &err), QStringLiteral("\"(sulli)\" text"));
    QVERIFY(err == KShell::NoError);

    QCOMPARE(sj(QStringLiteral(" ha\\ lo "), KShell::NoOptions, &err), QStringLiteral("\"ha\\\\\" lo"));
    QVERIFY(err == KShell::NoError);

    QCOMPARE(sj(QStringLiteral("say \" error"), KShell::NoOptions, &err), QString());
    QVERIFY(err == KShell::BadQuoting);

    QCOMPARE(sj(QStringLiteral("no \" error\""), KShell::NoOptions, &err), QStringLiteral("no \" error\""));
    QVERIFY(err == KShell::NoError);

    QCOMPARE(sj(QStringLiteral("say \" still error"), KShell::NoOptions, &err), QString());
    QVERIFY(err == KShell::BadQuoting);

    QCOMPARE(sj(QStringLiteral("BLA;asdf sdfess d"), KShell::NoOptions, &err), QStringLiteral("\"BLA;asdf\" sdfess d"));
    QVERIFY(err == KShell::NoError);

    QCOMPARE(sj(QStringLiteral("B\"L\"A&sdf FOO|bar sdf wer "), KShell::NoOptions, &err), QStringLiteral("\"BLA&sdf\" \"FOO|bar\" sdf wer"));
    QVERIFY(err == KShell::NoError);

    QCOMPARE(sj(QStringLiteral("\"\"\"just \"\" fine\"\"\""), KShell::NoOptions, &err), QStringLiteral("\\^\"\"just \"\\^\"\" fine\"\\^\""));
    QVERIFY(err == KShell::NoError);
#else
    QCOMPARE(sj(QString::fromUtf8("\"~qU4rK\" 'text' 'jo'\"jo\" $'crap' $'\\\\\\'\\e\\x21' ha\\ lo \\a"), KShell::NoOptions, &err),
             QString::fromUtf8("'~qU4rK' text jojo crap '\\'\\''\x1b!' 'ha lo' a"));
    QVERIFY(err == KShell::NoError);

    QCOMPARE(sj(QStringLiteral("\"~qU4rK\" 'text'"), KShell::TildeExpand, &err), QStringLiteral("'~qU4rK' text"));
    QVERIFY(err == KShell::NoError);

    QCOMPARE(sj(QStringLiteral("~\"qU4rK\" 'text'"), KShell::TildeExpand, &err), QStringLiteral("'~qU4rK' text"));
    QVERIFY(err == KShell::NoError);

    QCOMPARE(sj(QStringLiteral("~/\"dir\" 'text'"), KShell::TildeExpand, &err), QString(QDir::homePath() + QStringLiteral("/dir text")));
    QVERIFY(err == KShell::NoError);

    QCOMPARE(sj(QStringLiteral("~ 'text' ~"), KShell::TildeExpand, &err), QString(QDir::homePath() + QStringLiteral(" text ") + QDir::homePath()));
    QVERIFY(err == KShell::NoError);

    QCOMPARE(sj(QStringLiteral("\\~ blah"), KShell::TildeExpand, &err), QStringLiteral("'~' blah"));
    QVERIFY(err == KShell::NoError);

    QCOMPARE(sj(QStringLiteral("~qU4rK ~") + KUser().loginName(), KShell::TildeExpand, &err), QString(QStringLiteral("'~qU4rK' ") + myHomePath()));
    QVERIFY(err == KShell::NoError);

    const QString unicodeSpaceFileName = QStringLiteral("test　テスト.txt"); // #345140
    QCOMPARE(sj(unicodeSpaceFileName, KShell::AbortOnMeta | KShell::TildeExpand, &err), unicodeSpaceFileName);
    QVERIFY(err == KShell::NoError);
#endif
}

void KShellTest::quoteSplit_data()
{
    QTest::addColumn<QString>("string");

    QTest::newRow("no space") << QStringLiteral("hiho");
    QTest::newRow("regular space") << QStringLiteral("hi there");
    QTest::newRow("special space") << QString::fromUtf8("如何定期清潔典型的電風扇　講義.pdf");
}

void KShellTest::quoteSplit()
{
    QFETCH(QString, string);

    // Splitting a quote arg should always just return one argument
    const QStringList args = KShell::splitArgs(KShell::quoteArg(string));
    QCOMPARE(args.count(), 1);
}

void KShellTest::abortOnMeta()
{
    KShell::Errors err1 = KShell::NoError;
    KShell::Errors err2 = KShell::NoError;

    QCOMPARE(sj(QStringLiteral("text"), KShell::AbortOnMeta, &err1), QStringLiteral("text"));
    QVERIFY(err1 == KShell::NoError);

#ifdef Q_OS_WIN
    QVERIFY(KShell::splitArgs(QStringLiteral("BLA & asdf sdfess d"), KShell::AbortOnMeta, &err1).isEmpty());
    QVERIFY(err1 == KShell::FoundMeta);

    QVERIFY(KShell::splitArgs(QStringLiteral("foo %PATH% bar"), KShell::AbortOnMeta, &err1).isEmpty());
    QVERIFY(err1 == KShell::FoundMeta);
    QCOMPARE(sj(QStringLiteral("foo %PERCENT_SIGN% bar"), KShell::AbortOnMeta, &err1), QStringLiteral("foo %PERCENT_SIGN% bar"));
    QVERIFY(err1 == KShell::NoError);

    QCOMPARE(sj(QStringLiteral("@foo ^& bar"), KShell::AbortOnMeta, &err1), QStringLiteral("foo \"&\" bar"));
    QVERIFY(err1 == KShell::NoError);

    QCOMPARE(sj(QStringLiteral("\"BLA|asdf\" sdfess d"), KShell::AbortOnMeta, &err1), QStringLiteral("\"BLA|asdf\" sdfess d"));
    QVERIFY(err1 == KShell::NoError);

    QCOMPARE(sj(QStringLiteral("B\"L\"A\"|\"sdf \"FOO | bar\" sdf wer"), KShell::AbortOnMeta, &err1), QStringLiteral("\"BLA|sdf\" \"FOO | bar\" sdf wer"));
    QVERIFY(err1 == KShell::NoError);

    QCOMPARE(sj(QStringLiteral("b-q me \\\\^|\\\\\\^\""), KShell::AbortOnMeta, &err1), QStringLiteral("b-q me \"\\\\|\"\\\\\\^\""));
    QVERIFY(err1 == KShell::NoError);
#else
    QCOMPARE(sj(QStringLiteral("say \" error"), KShell::NoOptions, &err1), QString());
    QVERIFY(err1 != KShell::NoError);

    QCOMPARE(sj(QStringLiteral("say \" still error"), KShell::AbortOnMeta, &err1), QString());
    QVERIFY(err1 != KShell::NoError);

    QVERIFY(sj(QStringLiteral("say `echo no error`"), KShell::NoOptions, &err1) != sj(QStringLiteral("say `echo no error`"), KShell::AbortOnMeta, &err2));
    QVERIFY(err1 != err2);

    QVERIFY(sj(QStringLiteral("BLA=say echo meta"), KShell::NoOptions, &err1) != sj(QStringLiteral("BLA=say echo meta"), KShell::AbortOnMeta, &err2));
    QVERIFY(err1 != err2);

    QVERIFY(sj(QStringLiteral("B\"L\"A=say FOO=bar echo meta"), KShell::NoOptions, &err1)
            == sj(QStringLiteral("B\"L\"A=say FOO=bar echo meta"), KShell::AbortOnMeta, &err2));
#endif
}

QTEST_MAIN(KShellTest)

#include "kshelltest.moc"
