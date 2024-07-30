/*
    SPDX-FileCopyrightText: 2005 Ingo Kloecker <kloecker@kde.org>
    SPDX-FileCopyrightText: 2007 Allen Winter <winter@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "ktexttohtmltest.h"
#include "kcoreaddons_debug.h"

#include "../src/lib/text/ktexttohtml.h"
#include "../src/lib/text/ktexttohtml_p.h"

#include <QDebug>
#include <QTest>
#include <QUrl>

QTEST_MAIN(KTextToHTMLTest)

Q_DECLARE_METATYPE(KTextToHTML::Options)

#ifndef Q_OS_WIN
void initLocale()
{
    setenv("LC_ALL", "en_US.utf-8", 1);
}
Q_CONSTRUCTOR_FUNCTION(initLocale)
#endif

void KTextToHTMLTest::testGetEmailAddress()
{
    // empty input
    const QString emptyQString;
    KTextToHTMLHelper ll1(emptyQString, 0);
    QVERIFY(ll1.getEmailAddress().isEmpty());

    // no '@' at scan position
    KTextToHTMLHelper ll2(QStringLiteral("foo@bar.baz"), 0);
    QVERIFY(ll2.getEmailAddress().isEmpty());

    // '@' in local part
    KTextToHTMLHelper ll3(QStringLiteral("foo@bar@bar.baz"), 7);
    QVERIFY(ll3.getEmailAddress().isEmpty());

    // empty local part
    KTextToHTMLHelper ll4(QStringLiteral("@bar.baz"), 0);
    QVERIFY(ll4.getEmailAddress().isEmpty());
    KTextToHTMLHelper ll5(QStringLiteral(".@bar.baz"), 1);
    QVERIFY(ll5.getEmailAddress().isEmpty());
    KTextToHTMLHelper ll6(QStringLiteral(" @bar.baz"), 1);
    QVERIFY(ll6.getEmailAddress().isEmpty());
    KTextToHTMLHelper ll7(QStringLiteral(".!#$%&'*+-/=?^_`{|}~@bar.baz"), qstrlen(".!#$%&'*+-/=?^_`{|}~"));
    QVERIFY(ll7.getEmailAddress().isEmpty());

    // allowed special chars in local part of address
    KTextToHTMLHelper ll8(QStringLiteral("a.!#$%&'*+-/=?^_`{|}~@bar.baz"), qstrlen("a.!#$%&'*+-/=?^_`{|}~"));
    QCOMPARE(ll8.getEmailAddress(), QStringLiteral("a.!#$%&'*+-/=?^_`{|}~@bar.baz"));

    // '@' in domain part
    KTextToHTMLHelper ll9(QStringLiteral("foo@bar@bar.baz"), 3);
    QVERIFY(ll9.getEmailAddress().isEmpty());

    // domain part without dot
    KTextToHTMLHelper lla(QStringLiteral("foo@bar"), 3);
    QVERIFY(lla.getEmailAddress().isEmpty());
    KTextToHTMLHelper llb(QStringLiteral("foo@bar."), 3);
    QVERIFY(llb.getEmailAddress().isEmpty());
    KTextToHTMLHelper llc(QStringLiteral(".foo@bar"), 4);
    QVERIFY(llc.getEmailAddress().isEmpty());
    KTextToHTMLHelper lld(QStringLiteral("foo@bar "), 3);
    QVERIFY(lld.getEmailAddress().isEmpty());
    KTextToHTMLHelper lle(QStringLiteral(" foo@bar"), 4);
    QVERIFY(lle.getEmailAddress().isEmpty());
    KTextToHTMLHelper llf(QStringLiteral("foo@bar-bar"), 3);
    QVERIFY(llf.getEmailAddress().isEmpty());

    // empty domain part
    KTextToHTMLHelper llg(QStringLiteral("foo@"), 3);
    QVERIFY(llg.getEmailAddress().isEmpty());
    KTextToHTMLHelper llh(QStringLiteral("foo@."), 3);
    QVERIFY(llh.getEmailAddress().isEmpty());
    KTextToHTMLHelper lli(QStringLiteral("foo@-"), 3);
    QVERIFY(lli.getEmailAddress().isEmpty());

    // simple address
    KTextToHTMLHelper llj(QStringLiteral("foo@bar.baz"), 3);
    QCOMPARE(llj.getEmailAddress(), QStringLiteral("foo@bar.baz"));
    KTextToHTMLHelper llk(QStringLiteral("foo@bar.baz."), 3);
    QCOMPARE(llk.getEmailAddress(), QStringLiteral("foo@bar.baz"));
    KTextToHTMLHelper lll(QStringLiteral(".foo@bar.baz"), 4);
    QCOMPARE(lll.getEmailAddress(), QStringLiteral("foo@bar.baz"));
    KTextToHTMLHelper llm(QStringLiteral("foo@bar.baz-"), 3);
    QCOMPARE(llm.getEmailAddress(), QStringLiteral("foo@bar.baz"));
    KTextToHTMLHelper lln(QStringLiteral("-foo@bar.baz"), 4);
    QCOMPARE(lln.getEmailAddress(), QStringLiteral("foo@bar.baz"));
    KTextToHTMLHelper llo(QStringLiteral("foo@bar.baz "), 3);
    QCOMPARE(llo.getEmailAddress(), QStringLiteral("foo@bar.baz"));
    KTextToHTMLHelper llp(QStringLiteral(" foo@bar.baz"), 4);
    QCOMPARE(llp.getEmailAddress(), QStringLiteral("foo@bar.baz"));
    KTextToHTMLHelper llq(QStringLiteral("foo@bar-bar.baz"), 3);
    QCOMPARE(llq.getEmailAddress(), QStringLiteral("foo@bar-bar.baz"));
}

void KTextToHTMLTest::testGetUrl()
{
    QStringList brackets;
    brackets << QString() << QString(); // no brackets
    brackets << QStringLiteral("<") << QStringLiteral(">");
    brackets << QStringLiteral("[") << QStringLiteral("]");
    brackets << QStringLiteral("\"") << QStringLiteral("\"");
    brackets << QStringLiteral("<link>") << QStringLiteral("</link>");

    for (int i = 0; i < brackets.count(); i += 2) {
        testGetUrl2(brackets[i], brackets[i + 1]);
    }
}

void KTextToHTMLTest::testGetUrl2(const QString &left, const QString &right)
{
    QStringList schemas;
    schemas << QStringLiteral("http://");
    schemas << QStringLiteral("https://");
    schemas << QStringLiteral("vnc://");
    schemas << QStringLiteral("fish://");
    schemas << QStringLiteral("ftp://");
    schemas << QStringLiteral("ftps://");
    schemas << QStringLiteral("sftp://");
    schemas << QStringLiteral("smb://");
    schemas << QStringLiteral("file://");
    schemas << QStringLiteral("irc://");
    schemas << QStringLiteral("ircs://");

    QStringList urls;
    urls << QStringLiteral("www.kde.org");
    urls << QStringLiteral("user@www.kde.org");
    urls << QStringLiteral("user:pass@www.kde.org");
    urls << QStringLiteral("user:pass@www.kde.org:1234");
    urls << QStringLiteral("user:pass@www.kde.org:1234/sub/path");
    urls << QStringLiteral("user:pass@www.kde.org:1234/sub/path?a=1");
    urls << QStringLiteral("user:pass@www.kde.org:1234/sub/path?a=1#anchor");
    urls << QStringLiteral("user:pass@www.kde.org:1234/sub/\npath  \n /long/  path \t  ?a=1#anchor");
    urls << QStringLiteral("user:pass@www.kde.org:1234/sub/path/special(123)?a=1#anchor");
    urls << QStringLiteral("user:pass@www.kde.org:1234/sub/path:with:colon/special(123)?a=1#anchor");
    urls << QStringLiteral("user:pass@www.kde.org:1234/sub/path:with:colon/special(123)?a=1#anchor[bla");
    urls << QStringLiteral("user:pass@www.kde.org:1234/sub/path:with:colon/special(123)?a=1#anchor[bla]");
    urls << QStringLiteral("user:pass@www.kde.org:1234/\nsub/path:with:colon/\nspecial(123)?\na=1#anchor[bla]");
    urls << QStringLiteral("user:pass@www.kde.org:1234/  \n  sub/path:with:colon/  \n\t   \t   special(123)?") + QStringLiteral("\n\t  \n\t   a=1#anchor[bla]");
    urls << QStringLiteral("en.wikipedia.org/wiki/%C3%98_(disambiguation)");

    for (const QString &schema : std::as_const(schemas)) {
        for (QString url : std::as_const(urls)) {
            // by definition: if the URL is enclosed in brackets, the URL itself is not allowed
            // to contain the closing bracket, as this would be detected as the end of the URL
            if ((left.length() == 1) && (url.contains(right[0]))) {
                continue;
            }

            // if the url contains a whitespace, it must be enclosed with brackets
            if ((url.contains(QLatin1Char('\n')) || url.contains(QLatin1Char('\t')) || url.contains(QLatin1Char(' '))) && left.isEmpty()) {
                continue;
            }

            QString test(left + schema + url + right);
            KTextToHTMLHelper ll(test, left.length());
            QString gotUrl = ll.getUrl();

            // we want to have the url without whitespace
            url.remove(QLatin1Char(' '));
            url.remove(QLatin1Char('\n'));
            url.remove(QLatin1Char('\t'));

            bool ok = (gotUrl == (schema + url));
            if (!ok) {
                qCDebug(KCOREADDONS_DEBUG) << "got:" << gotUrl;
            }
            QVERIFY2(ok, qPrintable(test));
        }
    }

    QStringList urlsWithoutSchema;
    urlsWithoutSchema << QStringLiteral(".kde.org");
    urlsWithoutSchema << QStringLiteral(".kde.org:1234/sub/path");
    urlsWithoutSchema << QStringLiteral(".kde.org:1234/sub/path?a=1");
    urlsWithoutSchema << QStringLiteral(".kde.org:1234/sub/path?a=1#anchor");
    urlsWithoutSchema << QStringLiteral(".kde.org:1234/sub/path/special(123)?a=1#anchor");
    urlsWithoutSchema << QStringLiteral(".kde.org:1234/sub/path:with:colon/special(123)?a=1#anchor");
    urlsWithoutSchema << QStringLiteral(".kde.org:1234/sub/path:with:colon/special(123)?a=1#anchor[bla");
    urlsWithoutSchema << QStringLiteral(".kde.org:1234/sub/path:with:colon/special(123)?a=1#anchor[bla]");
    urlsWithoutSchema << QStringLiteral(".kde.org:1234/\nsub/path:with:colon/\nspecial(123)?\na=1#anchor[bla]");
    urlsWithoutSchema << QStringLiteral(".kde.org:1234/  \n  sub/path:with:colon/  \n\t   \t   special(123)?") + QStringLiteral("\n\t  \n\t   a=1#anchor[bla]");

    QStringList starts;
    starts << QStringLiteral("www") << QStringLiteral("ftp") << QStringLiteral("news:www");

    for (const QString &start : std::as_const(starts)) {
        for (QString url : std::as_const(urlsWithoutSchema)) {
            // by definition: if the URL is enclosed in brackets, the URL itself is not allowed
            // to contain the closing bracket, as this would be detected as the end of the URL
            if ((left.length() == 1) && (url.contains(right[0]))) {
                continue;
            }

            // if the url contains a whitespace, it must be enclosed with brackets
            if ((url.contains(QLatin1Char('\n')) || url.contains(QLatin1Char('\t')) || url.contains(QLatin1Char(' '))) && left.isEmpty()) {
                continue;
            }

            QString test(left + start + url + right);
            KTextToHTMLHelper ll(test, left.length());
            QString gotUrl = ll.getUrl();

            // we want to have the url without whitespace
            url.remove(QLatin1Char(' '));
            url.remove(QLatin1Char('\n'));
            url.remove(QLatin1Char('\t'));

            bool ok = (gotUrl == (start + url));
            if (!ok) {
                qCDebug(KCOREADDONS_DEBUG) << "got:" << gotUrl;
            }
            QVERIFY2(ok, qPrintable(gotUrl));
        }
    }

    // test max url length
    QString url = QStringLiteral("https://www.kde.org/this/is/a_very_loooooong_url/test/test/test");
    {
        KTextToHTMLHelper ll(url, 0, 10);
        QVERIFY(ll.getUrl().isEmpty()); // url too long
    }
    {
        KTextToHTMLHelper ll(url, 0, url.length() - 1);
        QVERIFY(ll.getUrl().isEmpty()); // url too long
    }
    {
        KTextToHTMLHelper ll(url, 0, url.length());
        QCOMPARE(ll.getUrl(), url);
    }
    {
        KTextToHTMLHelper ll(url, 0, url.length() + 1);
        QCOMPARE(ll.getUrl(), url);
    }

    // mailto
    {
        QString addr = QStringLiteral("mailto:test@kde.org");
        QString test(left + addr + right);
        KTextToHTMLHelper ll(test, left.length());

        QString gotUrl = ll.getUrl();

        bool ok = (gotUrl == addr);
        if (!ok) {
            qCDebug(KCOREADDONS_DEBUG) << "got:" << gotUrl;
        }
        QVERIFY2(ok, qPrintable(gotUrl));
    }
}

void KTextToHTMLTest::testHtmlConvert_data()
{
    QTest::addColumn<QString>("plainText");
    QTest::addColumn<KTextToHTML::Options>("flags");
    QTest::addColumn<QString>("htmlText");

    // Linker error when using PreserveSpaces, therefore the hardcoded 0x01 or 0x09

    // Test preserving whitespace correctly
    QTest::newRow("") << " foo" << KTextToHTML::Options(KTextToHTML::PreserveSpaces) << "&nbsp;foo";
    QTest::newRow("") << "  foo" << KTextToHTML::Options(KTextToHTML::PreserveSpaces) << "&nbsp;&nbsp;foo";
    QTest::newRow("") << "  foo  " << KTextToHTML::Options(KTextToHTML::PreserveSpaces) << "&nbsp;&nbsp;foo&nbsp;&nbsp;";
    QTest::newRow("") << "  foo " << KTextToHTML::Options(KTextToHTML::PreserveSpaces) << "&nbsp;&nbsp;foo&nbsp;";
    QTest::newRow("") << "bla bla bla bla bla" << KTextToHTML::Options(KTextToHTML::PreserveSpaces) << "bla bla bla bla bla";
    QTest::newRow("") << "bla bla bla \n  bla bla bla " << KTextToHTML::Options(KTextToHTML::PreserveSpaces)
                      << "bla bla bla&nbsp;<br />\n&nbsp;&nbsp;bla bla bla&nbsp;";
    QTest::newRow("") << "bla bla  bla" << KTextToHTML::Options(KTextToHTML::PreserveSpaces) << "bla bla&nbsp;&nbsp;bla";
    QTest::newRow("") << " bla bla \n bla bla a\n  bla bla " << KTextToHTML::Options(KTextToHTML::PreserveSpaces)
                      << "&nbsp;bla bla&nbsp;<br />\n&nbsp;bla bla a<br />\n"
                         "&nbsp;&nbsp;bla bla&nbsp;";

    // Test highlighting with *, / and _
    QTest::newRow("") << "Ce paragraphe _contient_ des mots ou des _groupes de mots_ à mettre en"
                         " forme…"
                      << KTextToHTML::Options(KTextToHTML::PreserveSpaces | KTextToHTML::HighlightText)
                      << "Ce paragraphe <u>_contient_</u> des mots ou des"
                         " <u>_groupes de mots_</u> à mettre en forme…";
    QTest::newRow("punctation-bug") << "Ce texte *a l'air* de _fonctionner_, à condition"
                                       " d’utiliser le guillemet ASCII."
                                    << KTextToHTML::Options(KTextToHTML::PreserveSpaces | KTextToHTML::HighlightText)
                                    << "Ce texte <b>*a l'air*</b> de <u>_fonctionner_</u>, à"
                                       " condition d’utiliser le guillemet ASCII.";
    QTest::newRow("punctation-bug") << "Un répertoire /est/ un *dossier* où on peut mettre des"
                                       " *fichiers*."
                                    << KTextToHTML::Options(KTextToHTML::PreserveSpaces | KTextToHTML::HighlightText)
                                    << "Un répertoire <i>/est/</i> un"
                                       " <b>*dossier*</b> où on peut mettre des <b>*fichiers*</b>.";
    QTest::newRow("punctation-bug") << "*BLA BLA BLA BLA*." << KTextToHTML::Options(KTextToHTML::PreserveSpaces | KTextToHTML::HighlightText)
                                    << "<b>BLA BLA BLA BLA</b>.";
    QTest::newRow("") << "Je vais tenter de repérer des faux positif*" << KTextToHTML::Options(KTextToHTML::PreserveSpaces | KTextToHTML::HighlightText)
                      << "Je vais tenter de repérer des faux positif*";
    QTest::newRow("") << "*Ouais !* *Yes!*" << KTextToHTML::Options(KTextToHTML::PreserveSpaces | KTextToHTML::HighlightText)
                      << "<b>*Ouais !*</b> <b>*Yes!*</b>";

    QTest::newRow("multispace") << "*Ouais     foo*" << KTextToHTML::Options(KTextToHTML::PreserveSpaces | KTextToHTML::HighlightText)
                                << "<b>*Ouais     foo*</b>";

    QTest::newRow("multispace3") << "*Ouais:     foo*" << KTextToHTML::Options(KTextToHTML::PreserveSpaces | KTextToHTML::HighlightText)
                                 << "<b>*Ouais:     foo*</b>";

    QTest::newRow("multi-") << "** Ouais:  foo **" << KTextToHTML::Options(KTextToHTML::PreserveSpaces | KTextToHTML::HighlightText)
                            << "** Ouais:&nbsp;&nbsp;foo **";

    QTest::newRow("multi-") << "*** Ouais:  foo ***" << KTextToHTML::Options(KTextToHTML::PreserveSpaces | KTextToHTML::HighlightText)
                            << "*** Ouais:&nbsp;&nbsp;foo ***";

    QTest::newRow("nohtmlversion") << "* Ouais:     foo *" << KTextToHTML::Options(KTextToHTML::PreserveSpaces | KTextToHTML::HighlightText)
                                   << "* Ouais:&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;foo *";

    QTest::newRow("nohtmlversion2") << "*Ouais:     foo *" << KTextToHTML::Options(KTextToHTML::PreserveSpaces | KTextToHTML::HighlightText)
                                    << "*Ouais:&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;foo *";

    QTest::newRow("nohtmlversion3") << "* Ouais:     foo*" << KTextToHTML::Options(KTextToHTML::PreserveSpaces | KTextToHTML::HighlightText)
                                    << "* Ouais:&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;foo*";

    QTest::newRow("nohtmlversion3") << "* Ouais: *ff sfsdf* foo *" << KTextToHTML::Options(KTextToHTML::PreserveSpaces | KTextToHTML::HighlightText)
                                    << "* Ouais: <b>*ff sfsdf*</b> foo *";

    QTest::newRow("") << "the /etc/{rsyslog.d,syslog-ng.d}/package.rpmnew file"
                      << KTextToHTML::Options(KTextToHTML::PreserveSpaces | KTextToHTML::HighlightText)
                      << "the /etc/{rsyslog.d,syslog-ng.d}/package.rpmnew file";

    // This test has problems with the encoding, apparently.
    // QTest::newRow( "" ) << "*Ça fait plaisir de pouvoir utiliser des lettres accentuées dans du"
    //                       " texte mis en forme*." << 0x09 << "<b>Ça fait plaisir de pouvoir"
    //                       " utiliser des lettres accentuées dans du texte mis en forme</b>.";

    // Bug reported by dfaure, the <hostname> would get lost
    QTest::newRow("") << "QUrl url(\"http://strange<hostname>/\");" << KTextToHTML::Options(KTextToHTML::ReplaceSmileys | KTextToHTML::HighlightText)
                      << "QUrl url(&quot;<a href=\"http://strange<hostname>/\">"
                         "http://strange&lt;hostname&gt;/</a>&quot;);";

    // Bug: 211128 - plain text emails should not replace ampersand & with &amp;
    QTest::newRow("bug211128") << "https://green-site/?Ticket=85&Page=next" << KTextToHTML::Options(KTextToHTML::PreserveSpaces)
                               << "<a href=\"https://green-site/?Ticket=85&Page=next\">"
                                  "https://green-site/?Ticket=85&amp;Page=next</a>";

    QTest::newRow("dotBeforeEnd") << "Look at this file: www.example.com/example.h" << KTextToHTML::Options(KTextToHTML::PreserveSpaces)
                                  << "Look at this file: <a href=\"http://www.example.com/example.h\">"
                                     "www.example.com/example.h</a>";
    QTest::newRow("dotInMiddle") << "Look at this file: www.example.com/.bashrc" << KTextToHTML::Options(KTextToHTML::PreserveSpaces)
                                 << "Look at this file: <a href=\"http://www.example.com/.bashrc\">"
                                    "www.example.com/.bashrc</a>";

    // A dot at the end of an URL is explicitly ignored
    QTest::newRow("dotAtEnd") << "Look at this file: www.example.com/test.cpp." << KTextToHTML::Options(KTextToHTML::PreserveSpaces)
                              << "Look at this file: <a href=\"http://www.example.com/test.cpp\">"
                                 "www.example.com/test.cpp</a>.";

    // Bug 313719 - URL in parenthesis
    QTest::newRow("url-in-parenthesis-1") << "KDE (website https://www.kde.org)" << KTextToHTML::Options(KTextToHTML::PreserveSpaces)
                                          << "KDE (website <a href=\"https://www.kde.org\">https://www.kde.org</a>)";
    QTest::newRow("url-in-parenthesis-2") << "KDE website (https://www.kde.org)" << KTextToHTML::Options(KTextToHTML::PreserveSpaces)
                                          << "KDE website (<a href=\"https://www.kde.org\">https://www.kde.org</a>)";
    QTest::newRow("url-in-parenthesis-3") << "bla (https://www.kde.org - section 5.2)" << KTextToHTML::Options(KTextToHTML::PreserveSpaces)
                                          << "bla (<a href=\"https://www.kde.org\">https://www.kde.org</a> - section 5.2)";

    // Fix url as foo <<url> <url>> when we concatened them.
    QTest::newRow("url-with-url")
        << "foo <https://www.kde.org/ <https://www.kde.org/>>" << KTextToHTML::Options(KTextToHTML::PreserveSpaces)
        << "foo &lt;<a href=\"https://www.kde.org/ \">https://www.kde.org/ </a>&lt;<a href=\"https://www.kde.org/\">https://www.kde.org/</a>&gt;&gt;";

    // Fix url exploit
    QTest::newRow("url-exec-html") << "https://\"><!--" << KTextToHTML::Options(KTextToHTML::PreserveSpaces) << "https://&quot;&gt;&lt;!--";

    QTest::newRow("url-exec-html-2") << "https://192.168.1.1:\"><!--" << KTextToHTML::Options(KTextToHTML::PreserveSpaces)
                                     << "https://192.168.1.1:&quot;&gt;&lt;!--";

    QTest::newRow("url-exec-html-3") << "https://<IP>:\"><!--" << KTextToHTML::Options(KTextToHTML::PreserveSpaces) << "https://&lt;IP&gt;:&quot;&gt;&lt;!--";

    QTest::newRow("url-exec-html-4") << "https://<IP>:/\"><!--" << KTextToHTML::Options(KTextToHTML::PreserveSpaces) << "https://&lt;IP&gt;:/&quot;&gt;&lt;!--";

    QTest::newRow("url-exec-html-5") << "https://<IP>:/\"><script>alert(1);</script><!--" << KTextToHTML::Options(KTextToHTML::PreserveSpaces)
                                     << "https://&lt;IP&gt;:/&quot;&gt;&lt;script&gt;alert(1);&lt;/script&gt;&lt;!--";

    QTest::newRow("url-exec-html-6") << "https://<IP>:/\"><script>alert(1);</script><!--\nTest2" << KTextToHTML::Options(KTextToHTML::PreserveSpaces)
                                     << "https://&lt;IP&gt;:/&quot;&gt;&lt;script&gt;alert(1);&lt;/script&gt;&lt;!--\nTest2";

    QTest::newRow("url-with-ref-in-[") << "https://www.kde.org[1]" << KTextToHTML::Options(KTextToHTML::PreserveSpaces)
                                       << "<a href=\"https://www.kde.org\">https://www.kde.org</a>[1]";

    QTest::newRow("url-with-ref-in-[2") << "[http://www.example.org/][whatever]" << KTextToHTML::Options(KTextToHTML::PreserveSpaces)
                                        << "[<a href=\"http://www.example.org/\">http://www.example.org/</a>][whatever]";
    // Bug 346132
    QTest::newRow("url-with-ref-in-<") << "http://www.foo.bar<http://foo.bar/>" << KTextToHTML::Options(KTextToHTML::PreserveSpaces)
                                       << "<a href=\"http://www.foo.bar\">http://www.foo.bar</a>&lt;<a href=\"http://foo.bar/\">http://foo.bar/</a>&gt;";

    QTest::newRow("url-with-ref-in-]") << "[Please visit our booth 24-25 http://example.com/]" << KTextToHTML::Options(KTextToHTML::PreserveSpaces)
                                       << "[Please visit our booth 24-25 <a href=\"http://example.com/\">http://example.com/</a>]";

    QTest::newRow("two url with space") << "http://www.kde.org/standards/kcfg/1.0 http://www.kde.org/" << KTextToHTML::Options(KTextToHTML::PreserveSpaces)
                                        << "<a href=\"http://www.kde.org/standards/kcfg/1.0\">http://www.kde.org/standards/kcfg/1.0</a> <a "
                                           "href=\"http://www.kde.org/\">http://www.kde.org/</a>";

    // Bug kmail
    QTest::newRow("two url with space-2")
        << "@@ -55,6 +55,10 @@ xsi:schemaLocation=\"http://www.kde.org/standards/kcfg/1.0 http://www.kde.org/"
        << KTextToHTML::Options(KTextToHTML::PreserveSpaces)
        << "@@ -55,6 +55,10 @@ xsi:schemaLocation=&quot;<a href=\"http://www.kde.org/standards/kcfg/1.0\">http://www.kde.org/standards/kcfg/1.0</a> <a "
           "href=\"http://www.kde.org/\">http://www.kde.org/</a>";

    const auto opt = KTextToHTML::PreserveSpaces | KTextToHTML::ConvertPhoneNumbers;
    // tel: urls
    QTest::newRow("tel url compact") << "bla bla <tel:+491234567890> bla bla" << opt
                                     << "bla bla &lt;<a href=\"tel:+491234567890\">tel:+491234567890</a>&gt; bla bla";
    QTest::newRow("tel url fancy") << "bla bla tel:+49-321-123456 bla bla" << opt << "bla bla <a href=\"tel:+49-321-123456\">tel:+49-321-123456</a> bla bla";

    // negative tel: url tests
    QTest::newRow("empty tel url") << "bla tel: blub" << opt << "bla tel: blub";

    // phone numbers
    QTest::newRow("tel compact international") << "call +49123456789, then hang up" << opt
                                               << "call <a href=\"tel:+49123456789\">+49123456789</a>, then hang up";
    QTest::newRow("tel parenthesis/spaces international")
        << "phone:+33 (01) 12 34 56 78 blub" << opt << "phone:<a href=\"tel:+330112345678\">+33 (01) 12 34 56 78</a> blub";
    QTest::newRow("tel dashes international") << "bla +44-321-1-234-567" << opt << "bla <a href=\"tel:+443211234567\">+44-321-1-234-567</a>";
    QTest::newRow("tel dashes/spaces international") << "+1 123-456-7000 blub" << opt << "<a href=\"tel:+11234567000\">+1 123-456-7000</a> blub";
    QTest::newRow("tel spaces international") << "bla +32 1 234 5678 blub" << opt << "bla <a href=\"tel:+3212345678\">+32 1 234 5678</a> blub";
    QTest::newRow("tel slash domestic") << "bla 030/12345678 blub" << opt << "bla <a href=\"tel:03012345678\">030/12345678</a> blub";
    QTest::newRow("tel slash/space domestic") << "Tel.: 089 / 12 34 56 78" << opt << "Tel.: <a href=\"tel:08912345678\">089 / 12 34 56 78</a>";
    QTest::newRow("tel follow by parenthesis") << "Telefon: 0 18 05 / 12 23 46 (14 Cent/Min.*)" << opt
                                               << "Telefon: <a href=\"tel:01805122346\">0 18 05 / 12 23 46</a> (14 Cent/Min.*)";
    QTest::newRow("tel space single digit at end") << "0123/123 456 7" << opt << "<a href=\"tel:01231234567\">0123/123 456 7</a>";
    QTest::newRow("tel space around dash") << "bla +49 (0) 12 23 - 45 6000 blub" << opt
                                           << "bla <a href=\"tel:+4901223456000\">+49 (0) 12 23 - 45 6000</a> blub";
    QTest::newRow("tel two numbers speparated by dash")
        << "bla +49 (0) 12 23 46 78 - +49 0123/123 456 78 blub" << opt
        << "bla <a href=\"tel:+49012234678\">+49 (0) 12 23 46 78</a> - <a href=\"tel:+49012312345678\">+49 0123/123 456 78</a> blub";

    // negative tests for phone numbers
    QTest::newRow("non-tel number") << "please send 1200 cakes" << opt << "please send 1200 cakes";
    QTest::newRow("non-tel alpha-numeric") << "bla 1-123-456-ABCD blub" << opt << "bla 1-123-456-ABCD blub";
    QTest::newRow("non-tel alpha prefix") << "ABCD0123-456-789" << opt << "ABCD0123-456-789";
    QTest::newRow("non-tel date") << "bla 02/03/2019 blub" << opt << "bla 02/03/2019 blub";
    QTest::newRow("non-tel too long") << "bla +012-4567890123456 blub" << opt << "bla +012-4567890123456 blub";
    QTest::newRow("non-tel unbalanced") << "bla +012-456789(01 blub" << opt << "bla +012-456789(01 blub";
    QTest::newRow("non-tel nested") << "bla +012-4(56(78)90)1 blub" << opt << "bla +012-4(56(78)90)1 blub";
    QTest::newRow("tel extraction disabled") << "call +49123456789 now" << KTextToHTML::Options(KTextToHTML::PreserveSpaces) << "call +49123456789 now";

    QTest::newRow("bug-414360")
        << "https://www.openstreetmap.org/directions?engine=graphhopper_foot&route=44.85765%2C-0.55931%3B44.85713%2C-0.56117#map=18/44.85756/-0.56094"
        << KTextToHTML::Options(KTextToHTML::PreserveSpaces)
        << "<a "
           "href=\"https://www.openstreetmap.org/directions?engine=graphhopper_foot&route=44.85765%2C-0.55931%3B44.85713%2C-0.56117#map=18/44.85756/"
           "-0.56094\">https://www.openstreetmap.org/directions?engine=graphhopper_foot&amp;route=44.85765%2C-0.55931%3B44.85713%2C-0.56117#map=18/44.85756/"
           "-0.56094</a>";

    // xmpp bug 422291
    QTest::newRow("xmpp1") << "xmpp:username@server.tld" << KTextToHTML::Options(KTextToHTML::PreserveSpaces)
                           << "<a href=\"xmpp:username@server.tld\">xmpp:username@server.tld</a>";
    QTest::newRow("xmpp2") << "xmpp:conversations@conference.siacs.eu" << KTextToHTML::Options(KTextToHTML::PreserveSpaces)
                           << "<a href=\"xmpp:conversations@conference.siacs.eu\">xmpp:conversations@conference.siacs.eu</a>";
    QTest::newRow("xmpp3") << "xmpp:conversations@conference.siacs.eu?join" << KTextToHTML::Options(KTextToHTML::PreserveSpaces)
                           << "<a href=\"xmpp:conversations@conference.siacs.eu?join\">xmpp:conversations@conference.siacs.eu?join</a>";

    // Test news: only
    QTest::newRow("news") << "news: " << KTextToHTML::Options(KTextToHTML::PreserveSpaces) << "news:&nbsp;";

    QTest::newRow("ftp") << "ftp: " << KTextToHTML::Options(KTextToHTML::PreserveSpaces) << "ftp:&nbsp;";
    QTest::newRow("mailto") << "mailto: " << KTextToHTML::Options(KTextToHTML::PreserveSpaces) << "mailto:&nbsp;";
    QTest::newRow("empty") << "" << KTextToHTML::Options(KTextToHTML::PreserveSpaces) << "";
}

void KTextToHTMLTest::testHtmlConvert()
{
    QFETCH(QString, plainText);
    QFETCH(KTextToHTML::Options, flags);
    QFETCH(QString, htmlText);

    QEXPECT_FAIL("punctation-bug", "Linklocator does not properly detect punctation as boundaries", Continue);

    const QString actualHtml = KTextToHTML::convertToHtml(plainText, flags);
    QCOMPARE(actualHtml, htmlText);
}

#define s(x) QStringLiteral(x)

void KTextToHTMLTest::testEmoticons_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("output");
    QTest::newRow("empty") << QString() << QString();
    QTest::newRow("trailing") << s("Hello :-)") << s("Hello 🙂");
    QTest::newRow("embedded") << s("Hello :-) How are you?") << s("Hello 🙂 How are you?");
    QTest::newRow("leading") << s(":-( Bye") << s("🙁 Bye");
    QTest::newRow("embedded-html") << s("<b>:(</b>") << s("&lt;b&gt;:(&lt;/b&gt;");
    QTest::newRow("html-attribute") << s("<img src=\"...\" title=\":-)\" />") << s("&lt;img src=&quot;...&quot; title=&quot;:-)&quot; /&gt;");
    QTest::newRow("broken-1") << s(":))") << s("😆");
    QTest::newRow("broken-4") << s(":D and :-D are not the same as :d and :-d") << s("😀 and 😀 are not the same as :d and :-d");
    QTest::newRow("broken-5") << s("4d:D>:)F:/&gt;:-(:Pu:d9") << s("4d:D&gt;:)F:/&amp;gt;:-(:Pu:d9");
    QTest::newRow("broken-6") << s("&lt;::pvar:: test=1&gt;") << s("&amp;lt;::pvar:: test=1&amp;gt;");
    QTest::newRow("working-5") << s("(&amp;)") << s("(&amp;amp;)");
    QTest::newRow("working-6") << s("Bla (&nbsp;)") << s("Bla (&amp;nbsp;)");
    QTest::newRow("working-7") << s("a non-breaking space (&nbsp;) character") << s("a non-breaking space (&amp;nbsp;) character");

    QTest::newRow("angle-bracket-1") << s(">:)") << s("😈");
    QTest::newRow("angle-bracket-2") << s("<b>:)") << s("&lt;b&gt;:)");
}

void KTextToHTMLTest::testEmoticons()
{
    QFETCH(QString, input);
    QFETCH(QString, output);
    QCOMPARE(KTextToHTML::convertToHtml(input, KTextToHTML::ReplaceSmileys | KTextToHTML::IgnoreUrls), output);
}

void KTextToHTMLTest::testEmoticonsNoReplace_data()
{
    QTest::addColumn<QString>("input");
    QTest::newRow("empty") << QString();
    QTest::newRow("no-space-spearator") << s("Very happy! :-):-)");
    QTest::newRow("broken-2") << s("In a sentence:practical example");
    QTest::newRow("broken-8") << s("-+-[-:-(-:-)-:-]-+-");
    QTest::newRow("broken-9") << s("::shrugs::");
    QTest::newRow("broken-10") << s(":Ptesting:P");
    QTest::newRow("working-1") << s(":):)");
    QTest::newRow("working-4") << s("http://www.kde.org");
    QTest::newRow("working-3") << s("End of sentence:p");
    QTest::newRow("xmpp-1") << s("an xmpp emoticon (%)");
}

void KTextToHTMLTest::testEmoticonsNoReplace()
{
    QFETCH(QString, input);
    QCOMPARE(KTextToHTML::convertToHtml(input, KTextToHTML::ReplaceSmileys | KTextToHTML::IgnoreUrls), input);
}

#include "moc_ktexttohtmltest.cpp"
