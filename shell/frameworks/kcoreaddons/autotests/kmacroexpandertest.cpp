/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2003, 2008 Oswald Buddenhagen <ossi@kde.org>
    SPDX-FileCopyrightText: 2005 Thomas Braxton <brax108@cox.net>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include <QTest>
#include <kmacroexpander.h>

#include <QHash>
#include <QObject>

class KMacroExpanderTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void expandMacros();
    void expandMacrosShellQuote();
    void expandMacrosShellQuoteParens();
    void expandMacrosSubClass();
};

class MyCExpander : public KCharMacroExpander
{
    QString exp;

public:
    MyCExpander()
        : KCharMacroExpander()
        , exp(QStringLiteral("expanded"))
    {
    }

protected:
    bool expandMacro(QChar ch, QStringList &ret) override
    {
        if (ch == QLatin1Char('m')) {
            ret = QStringList(exp);
            return true;
        }
        return false;
    }
};

class MyWExpander : public KWordMacroExpander
{
    QString exp;

public:
    MyWExpander()
        : KWordMacroExpander()
        , exp(QStringLiteral("expanded"))
    {
    }

protected:
    bool expandMacro(const QString &str, QStringList &ret) override
    {
        if (str == QLatin1String("macro")) {
            ret = QStringList(exp);
            return true;
        }
        return false;
    }
};

void KMacroExpanderTest::expandMacros()
{
    QHash<QChar, QStringList> map;
    QStringList list;
    QString s;

    list << QStringLiteral("Restaurant \"Chew It\"");
    map.insert(QLatin1Char('n'), list);
    list.clear();
    list << QStringLiteral("element1") << QStringLiteral("'element2'");
    map.insert(QLatin1Char('l'), list);

    s = QStringLiteral("%% text %l text %n");
    QCOMPARE(KMacroExpander::expandMacros(s, map), QLatin1String("% text element1 'element2' text Restaurant \"Chew It\""));
    s = QStringLiteral("text \"%l %n\" text");
    QCOMPARE(KMacroExpander::expandMacros(s, map), QLatin1String("text \"element1 'element2' Restaurant \"Chew It\"\" text"));

    QHash<QChar, QString> map2;
    map2.insert(QLatin1Char('a'), QStringLiteral("%n"));
    map2.insert(QLatin1Char('f'), QStringLiteral("filename.txt"));
    map2.insert(QLatin1Char('u'), QStringLiteral("https://www.kde.org/index.html"));
    map2.insert(QLatin1Char('n'), QStringLiteral("Restaurant \"Chew It\""));
    s = QStringLiteral("Title: %a - %f - %u - %n - %%");
    QCOMPARE(KMacroExpander::expandMacros(s, map2), QLatin1String("Title: %n - filename.txt - https://www.kde.org/index.html - Restaurant \"Chew It\" - %"));

    QHash<QString, QString> smap;
    smap.insert(QStringLiteral("foo"), QStringLiteral("%n"));
    smap.insert(QStringLiteral("file"), QStringLiteral("filename.txt"));
    smap.insert(QStringLiteral("url"), QStringLiteral("https://www.kde.org/index.html"));
    smap.insert(QStringLiteral("name"), QStringLiteral("Restaurant \"Chew It\""));

    s = QStringLiteral("Title: %foo - %file - %url - %name - %");
    QCOMPARE(KMacroExpander::expandMacros(s, smap), QLatin1String("Title: %n - filename.txt - https://www.kde.org/index.html - Restaurant \"Chew It\" - %"));
    s = QStringLiteral("%foo - %file - %url - %name");
    QCOMPARE(KMacroExpander::expandMacros(s, smap), QLatin1String("%n - filename.txt - https://www.kde.org/index.html - Restaurant \"Chew It\""));

    s = QStringLiteral("Title: %{foo} - %{file} - %{url} - %{name} - %");
    QCOMPARE(KMacroExpander::expandMacros(s, smap), QLatin1String("Title: %n - filename.txt - https://www.kde.org/index.html - Restaurant \"Chew It\" - %"));
    s = QStringLiteral("%{foo} - %{file} - %{url} - %{name}");
    QCOMPARE(KMacroExpander::expandMacros(s, smap), QLatin1String("%n - filename.txt - https://www.kde.org/index.html - Restaurant \"Chew It\""));

    s = QStringLiteral("Title: %foo-%file-%url-%name-%");
    QCOMPARE(KMacroExpander::expandMacros(s, smap), QLatin1String("Title: %n-filename.txt-https://www.kde.org/index.html-Restaurant \"Chew It\"-%"));

    s = QStringLiteral("Title: %{file} %{url");
    QCOMPARE(KMacroExpander::expandMacros(s, smap), QLatin1String("Title: filename.txt %{url"));

    s = QStringLiteral(" * Copyright (C) 2008 %{AUTHOR}");
    smap.clear();
    QCOMPARE(KMacroExpander::expandMacros(s, smap), QLatin1String(" * Copyright (C) 2008 %{AUTHOR}"));
}

void KMacroExpanderTest::expandMacrosShellQuote()
{
    QHash<QChar, QStringList> map;
    QStringList list;
    QString s;

    list << QStringLiteral("Restaurant \"Chew It\"");
    map.insert(QLatin1Char('n'), list);
    list.clear();
    list << QStringLiteral("element1") << QStringLiteral("'element2'") << QStringLiteral("\"element3\"");
    map.insert(QLatin1Char('l'), list);

#ifdef Q_OS_WIN
    s = QStringLiteral("text %l %n text");
    QCOMPARE(KMacroExpander::expandMacrosShellQuote(s, map),
             QLatin1String("text element1 'element2' \\^\"element3\\^\" \"Restaurant \"\\^\"\"Chew It\"\\^\" text"));

    s = QStringLiteral("text \"%l %n\" text");
    QCOMPARE(KMacroExpander::expandMacrosShellQuote(s, map),
             QLatin1String("text \"element1 'element2' \"\\^\"\"element3\"\\^\"\" Restaurant \"\\^\"\"Chew It\"\\^\"\"\" text"));
#else
    s = QStringLiteral("text %l %n text");
    QCOMPARE(KMacroExpander::expandMacrosShellQuote(s, map), QLatin1String("text element1 ''\\''element2'\\''' '\"element3\"' 'Restaurant \"Chew It\"' text"));

    s = QStringLiteral("text \"%l %n\" text");
    QCOMPARE(KMacroExpander::expandMacrosShellQuote(s, map), QLatin1String("text \"element1 'element2' \\\"element3\\\" Restaurant \\\"Chew It\\\"\" text"));
#endif

    QHash<QChar, QString> map2;
    map2.insert(QLatin1Char('a'), QStringLiteral("%n"));
    map2.insert(QLatin1Char('f'), QStringLiteral("filename.txt"));
    map2.insert(QLatin1Char('u'), QStringLiteral("https://www.kde.org/index.html"));
    map2.insert(QLatin1Char('n'), QStringLiteral("Restaurant \"Chew It\""));

#ifdef Q_OS_WIN
    s = QStringLiteral("Title: %a - %f - %u - %n - %% - %VARIABLE% foo");
    QCOMPARE(
        KMacroExpander::expandMacrosShellQuote(s, map2),
        QLatin1String(
            "Title: %PERCENT_SIGN%n - filename.txt - https://www.kde.org/index.html - \"Restaurant \"\\^\"\"Chew It\"\\^\" - %PERCENT_SIGN% - %VARIABLE% foo"));

    s = QStringLiteral("kedit --caption %n %f");
    map2.insert(QLatin1Char('n'), QStringLiteral("Restaurant 'Chew It'"));
    QCOMPARE(KMacroExpander::expandMacrosShellQuote(s, map2), QLatin1String("kedit --caption \"Restaurant 'Chew It'\" filename.txt"));

    s = QStringLiteral("kedit --caption \"%n\" %f");
    QCOMPARE(KMacroExpander::expandMacrosShellQuote(s, map2), QLatin1String("kedit --caption \"Restaurant 'Chew It'\" filename.txt"));

    map2.insert(QLatin1Char('n'), QStringLiteral("Restaurant \"Chew It\""));
    QCOMPARE(KMacroExpander::expandMacrosShellQuote(s, map2), QLatin1String("kedit --caption \"Restaurant \"\\^\"\"Chew It\"\\^\"\"\" filename.txt"));

    map2.insert(QLatin1Char('n'), QStringLiteral("Restaurant %HOME%"));
    QCOMPARE(KMacroExpander::expandMacrosShellQuote(s, map2), QLatin1String("kedit --caption \"Restaurant %PERCENT_SIGN%HOME%PERCENT_SIGN%\" filename.txt"));

    s = QStringLiteral("kedit c:\\%f");
    QCOMPARE(KMacroExpander::expandMacrosShellQuote(s, map2), QLatin1String("kedit c:\\filename.txt"));

    s = QStringLiteral("kedit \"c:\\%f\"");
    QCOMPARE(KMacroExpander::expandMacrosShellQuote(s, map2), QLatin1String("kedit \"c:\\filename.txt\""));

    map2.insert(QLatin1Char('f'), QStringLiteral("\"filename.txt\""));
    QCOMPARE(KMacroExpander::expandMacrosShellQuote(s, map2), QLatin1String("kedit \"c:\\\\\"\\^\"\"filename.txt\"\\^\"\"\""));

    map2.insert(QLatin1Char('f'), QStringLiteral("path\\"));
    QCOMPARE(KMacroExpander::expandMacrosShellQuote(s, map2), QLatin1String("kedit \"c:\\path\\\\\"\"\""));
#else
    s = QStringLiteral("Title: %a - %f - %u - %n - %%");
    QCOMPARE(KMacroExpander::expandMacrosShellQuote(s, map2),
             QLatin1String("Title: %n - filename.txt - https://www.kde.org/index.html - 'Restaurant \"Chew It\"' - %"));

    s = QStringLiteral("kedit --caption %n %f");
    map2.insert(QLatin1Char('n'), QStringLiteral("Restaurant 'Chew It'"));
    QCOMPARE(KMacroExpander::expandMacrosShellQuote(s, map2), QLatin1String("kedit --caption 'Restaurant '\\''Chew It'\\''' filename.txt"));

    s = QStringLiteral("kedit --caption \"%n\" %f");
    QCOMPARE(KMacroExpander::expandMacrosShellQuote(s, map2), QLatin1String("kedit --caption \"Restaurant 'Chew It'\" filename.txt"));

    map2.insert(QLatin1Char('n'), QStringLiteral("Restaurant \"Chew It\""));
    QCOMPARE(KMacroExpander::expandMacrosShellQuote(s, map2), QLatin1String("kedit --caption \"Restaurant \\\"Chew It\\\"\" filename.txt"));

    map2.insert(QLatin1Char('n'), QStringLiteral("Restaurant $HOME"));
    QCOMPARE(KMacroExpander::expandMacrosShellQuote(s, map2), QLatin1String("kedit --caption \"Restaurant \\$HOME\" filename.txt"));

    map2.insert(QLatin1Char('n'), QStringLiteral("Restaurant `echo hello`"));
    QCOMPARE(KMacroExpander::expandMacrosShellQuote(s, map2), QLatin1String("kedit --caption \"Restaurant \\`echo hello\\`\" filename.txt"));

    s = QStringLiteral("kedit --caption \"`echo %n`\" %f");
    QCOMPARE(KMacroExpander::expandMacrosShellQuote(s, map2), QLatin1String("kedit --caption \"$( echo 'Restaurant `echo hello`')\" filename.txt"));
#endif
}

class DummyMacroExpander : public KMacroExpanderBase
{
public:
    DummyMacroExpander()
        : KMacroExpanderBase(QChar(0x4567))
    {
    }

protected:
    int expandPlainMacro(const QString &, int, QStringList &) override
    {
        return 0;
    }
    int expandEscapedMacro(const QString &, int, QStringList &) override
    {
        return 0;
    }
};

void KMacroExpanderTest::expandMacrosShellQuoteParens()
{
    QString s;

    s = QStringLiteral("( echo \"just testing (parens)\" ) ) after");
    int pos = 0;
    DummyMacroExpander kmx;
    QVERIFY(kmx.expandMacrosShellQuote(s, pos));
    QCOMPARE(s.mid(pos), QLatin1String(") after"));
    QVERIFY(!kmx.expandMacrosShellQuote(s));
}

void KMacroExpanderTest::expandMacrosSubClass()
{
    QString s;

    MyCExpander mx1;
    s = QStringLiteral("subst %m but not %n equ %%");
    mx1.expandMacros(s);
    QCOMPARE(s, QLatin1String("subst expanded but not %n equ %"));

    MyWExpander mx2;
    s = QStringLiteral("subst %macro but not %not equ %%");
    mx2.expandMacros(s);
    QCOMPARE(s, QLatin1String("subst expanded but not %not equ %"));
}

QTEST_MAIN(KMacroExpanderTest)

#include "kmacroexpandertest.moc"
