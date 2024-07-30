/*
    SPDX-FileCopyrightText: 2005 Ingo Kloecker <kloecker@kde.org>
    SPDX-FileCopyrightText: 2007 Allen Winter <winter@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "../src/lib/text/ktexttohtml.h"
#include "../src/lib/text/ktexttohtml_p.h"

#include <QDebug>
#include <QTest>
#include <QUrl>

Q_DECLARE_METATYPE(KTextToHTML::Options)

class KTextToHTMLTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void benchHtmlConvert_data()
    {
        QTest::addColumn<QString>("text");
        QTest::addColumn<KTextToHTML::Options>("options");

        auto text = QStringLiteral("foo bar asdf :)").repeated(1000);
        QTest::newRow("plain") << text << KTextToHTML::Options();
        QTest::newRow("preserve-spaces") << text << KTextToHTML::Options(KTextToHTML::PreserveSpaces);
        QTest::newRow("highlight-text") << text << KTextToHTML::Options(KTextToHTML::HighlightText);
        QTest::newRow("replace-smileys") << text << KTextToHTML::Options(KTextToHTML::ReplaceSmileys);
        QTest::newRow("preserve-spaces+highlight-text") << text << KTextToHTML::Options(KTextToHTML::PreserveSpaces | KTextToHTML::HighlightText);
        QTest::newRow("preserve-spaces+highlight-text+replace-smileys")
            << text << KTextToHTML::Options(KTextToHTML::PreserveSpaces | KTextToHTML::HighlightText | KTextToHTML::ReplaceSmileys);
    }

    void benchHtmlConvert()
    {
        QFETCH(QString, text);
        QFETCH(KTextToHTML::Options, options);

        QBENCHMARK {
            const QString html = KTextToHTML::convertToHtml(text, options);
            Q_UNUSED(html);
        }
    }
};

QTEST_MAIN(KTextToHTMLTest)

#include "ktexttohtmlbenchmarktest.moc"
