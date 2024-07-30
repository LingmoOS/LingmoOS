// krazy:excludeall=i18ncheckarg
/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2006 Chusslove Illich <caslav.ilic@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

// Tests explicitly use their own test catalogs.
#undef TRANSLATION_DOMAIN

#include "klocalizedstringtest.h"
#include "klocalizedtranslator.h"

#include <locale.h>

#include <QDebug>
#include <QProcess>
#include <QStandardPaths>
#include <QTest>
#include <QThread>

#include <libintl.h>

#include <klazylocalizedstring.h>
#include <klocalizedstring.h>

#include <QRegularExpression>
#include <QSet>
#include <QString>

void initEnvironment()
{
    // We need the default locale to be English otherwise the brokenTags test fails
    // since the "Opening and ending tag mismatch" text comes from Qt
    qputenv("LANG", "en_US.utf8");
    QStandardPaths::setTestModeEnabled(true);
}

Q_CONSTRUCTOR_FUNCTION(initEnvironment)

void KLocalizedStringTest::initTestCase()
{
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("ki18n-test"));

    m_hasFrench = true;
    m_hasCatalan = true;

    setlocale(LC_ALL, "ca_ES.utf8");
    if (setlocale(LC_ALL, nullptr) != QByteArray("ca_ES.utf8")) {
        qDebug() << "Failed to set locale to ca_ES.utf8.";
        m_hasCatalan = false;
    }

    if (m_hasFrench) {
        setlocale(LC_ALL, "fr_FR.utf8");
        if (setlocale(LC_ALL, nullptr) != QByteArray("fr_FR.utf8")) {
            qDebug() << "Failed to set locale to fr_FR.utf8.";
            m_hasFrench = false;
        } else {
            QLocale::setDefault(QLocale("fr_FR")); // the setlocale is "too late" for Qt that already has created the default QLocale, so set it manually
        }
    }
    if (m_hasFrench) {
        if (!m_tempDir.isValid()) {
            qDebug() << "Failed to create temporary directory for test data.";
            m_hasFrench = false;
        }
    }
    QDir dataDir(m_tempDir.path());
    if (m_hasFrench) {
        m_hasFrench = compileCatalogs({QFINDTESTDATA("po/fr/ki18n-test.po"), QFINDTESTDATA("po/fr/ki18n-test-qt.po")}, dataDir, "fr");
    }
    if (m_hasCatalan) {
        m_hasCatalan = compileCatalogs({QFINDTESTDATA("po/ca/ki18n-test.po")}, dataDir, "ca");
    }
    if (m_hasFrench) {
        const QByteArray dataDirs = qgetenv("XDG_DATA_DIRS") + ":" + QFile::encodeName(dataDir.path());
        qputenv("XDG_DATA_DIRS", dataDirs);
        // bind... dataDir.path()
        QStringList languages;
        languages.append("fr");
        KLocalizedString::setLanguages(languages);
    }

#if 0 // until locale system is ready
    if (m_hasFrench) {
        KLocale::global()->setLanguage(QStringList() << "fr" << "en_US");
    }
    KLocale::global()->setThousandsSeparator(QLatin1String(","));
    KLocale::global()->setDecimalSymbol(QLatin1String("."));
#endif
}

bool KLocalizedStringTest::compileCatalogs(const QStringList &testPoPaths, const QDir &dataDir, const QString &lang)
{
    const QString lcMessages = QString("locale/%1/LC_MESSAGES").arg(lang);
    if (!dataDir.mkpath(lcMessages)) {
        qDebug() << "Failed to create locale subdirectory "
                    "inside temporary directory.";
        return false;
    }
    QString msgfmt = QStandardPaths::findExecutable(QLatin1String("msgfmt"));
    if (msgfmt.isEmpty()) {
        qDebug() << "msgfmt(1) not found in path.";
        return false;
    }
    for (const QString &testPoPath : testPoPaths) {
        int pos_1 = testPoPath.lastIndexOf(QLatin1Char('/'));
        int pos_2 = testPoPath.lastIndexOf(QLatin1Char('.'));
        QString domain = testPoPath.mid(pos_1 + 1, pos_2 - pos_1 - 1);
        QString testMoPath;
        testMoPath = QString::fromLatin1("%1/%3/%2.mo").arg(dataDir.path(), domain, lcMessages);
        QProcess process;
        QStringList arguments;
        arguments << testPoPath << QLatin1String("-o") << testMoPath;
        process.start(msgfmt, arguments);
        process.waitForFinished(10000);
        if (process.exitCode() != 0) {
            qDebug() << QString::fromLatin1("msgfmt(1) could not compile %1.").arg(testPoPath);
            return false;
        }
    }
    return true;
}

void KLocalizedStringTest::correctSubs()
{
    if (!m_hasFrench) {
        QSKIP("French test files not usable.");
    }
    // Warm up.
    QCOMPARE(i18n("Daisies, daisies"), QString("Daisies, daisies"));

    // Placeholder in the middle.
    QCOMPARE(i18n("Fault in %1 unit", QString("AE35")), QString("Fault in AE35 unit"));
    // Placeholder at the start.
    QCOMPARE(i18n("%1, Tycho Magnetic Anomaly 1", QString("TMA-1")), QString("TMA-1, Tycho Magnetic Anomaly 1"));
    // Placeholder at the end.
    QCOMPARE(i18n("...odd things happening at %1", QString("Clavius")), QString("...odd things happening at Clavius"));
    QCOMPARE(i18n("Group %1", 1), QString("Group 1"));

    // Two placeholders.
    QCOMPARE(i18n("%1 and %2", QString("Bowman"), QString("Poole")), QString("Bowman and Poole"));
    // Two placeholders in inverted order.
    QCOMPARE(i18n("%2 and %1", QString("Poole"), QString("Bowman")), QString("Bowman and Poole"));

    // % which is not of placeholder.
    QCOMPARE(i18n("It's going to go %1% failure in 72 hours.", 100), QString("It's going to go 100% failure in 72 hours."));

    // Usual plural.
    QCOMPARE(i18np("%1 pod", "%1 pods", 1), QString("1 pod"));
    QCOMPARE(i18np("%1 pod", "%1 pods", 10), QString("10 pods"));

    // No plural-number in singular.
    QCOMPARE(i18np("A pod", "%1 pods", 1), QString("A pod"));
    QCOMPARE(i18np("A pod", "%1 pods", 10), QString("10 pods"));

    // No plural-number in singular or plural.
    QCOMPARE(i18np("A pod", "Few pods", 1), QString("A pod"));
    QCOMPARE(i18np("A pod", "Few pods", 10), QString("Few pods"));

    // First of two arguments as plural-number.
    QCOMPARE(i18np("A pod left on %2", "%1 pods left on %2", 1, QString("Discovery")), QString("A pod left on Discovery"));
    QCOMPARE(i18np("A pod left on %2", "%1 pods left on %2", 2, QString("Discovery")), QString("2 pods left on Discovery"));

    // Second of two arguments as plural-number.
    QCOMPARE(i18np("%1 has a pod left", "%1 has %2 pods left", QString("Discovery"), 1), QString("Discovery has a pod left"));
    QCOMPARE(i18np("%1 has a pod left", "%1 has %2 pods left", QString("Discovery"), 2), QString("Discovery has 2 pods left"));

    // No plural-number in singular or plural, but another argument present.
    QCOMPARE(i18np("A pod left on %2", "Some pods left on %2", 1, QString("Discovery")), QString("A pod left on Discovery"));
    QCOMPARE(i18np("A pod left on %2", "Some pods left on %2", 2, QString("Discovery")), QString("Some pods left on Discovery"));

    // Visual formatting.
    // FIXME: Needs much more tests.
    QCOMPARE(xi18n("E = mc^2"), QString("E = mc^2"));
    QCOMPARE(xi18n("E &lt; mc^2"), QString("E < mc^2"));
    QCOMPARE(xi18n("E ? <emphasis>mc^2</emphasis>"), QString("E ? *mc^2*"));
    QCOMPARE(xi18n("E &lt; <emphasis>mc^2</emphasis>"), QString("E < *mc^2*"));
    QCOMPARE(xi18nc("@label", "E &lt; <emphasis>mc^2</emphasis>"), QString("E < *mc^2*"));
    QCOMPARE(xi18nc("@info", "E &lt; <emphasis>mc^2</emphasis>"), QString("<html>E &lt; <i>mc^2</i></html>"));
    QCOMPARE(xi18nc("@info:status", "E &lt; <emphasis>mc^2</emphasis>"), QString("E < *mc^2*"));
    QCOMPARE(xi18nc("@info:progress", "E &lt; <emphasis>mc^2</emphasis>"), QString("E < *mc^2*"));
    QCOMPARE(xi18nc("@info:tooltip", "E &lt; <emphasis>mc^2</emphasis>"), QString("<html>E &lt; <i>mc^2</i></html>"));
    QCOMPARE(xi18nc("@info:shell", "E &lt; <emphasis>mc^2</emphasis>"), QString("E < *mc^2*"));
    QCOMPARE(xi18n("E = mc^&#x0032;"), QString("E = mc^2"));
    QCOMPARE(xi18n("E = mc^&#0050;"), QString("E = mc^2"));

    // with additional whitespace
    QCOMPARE(xi18nc(" @info:progress ", "E &lt; <emphasis>mc^2</emphasis>"), QString("E < *mc^2*"));
    QCOMPARE(xi18nc(" @info:tooltip ", "E &lt; <emphasis>mc^2</emphasis>"), QString("<html>E &lt; <i>mc^2</i></html>"));
    QCOMPARE(xi18nc(" @info: progress ", "E &lt; <emphasis>mc^2</emphasis>"), // not parsed as a cue
             QString("<html>E &lt; <i>mc^2</i></html>"));
    QCOMPARE(xi18nc(" @info: tooltip ", "E &lt; <emphasis>mc^2</emphasis>"), // not parsed as a cue
             QString("<html>E &lt; <i>mc^2</i></html>"));

    QTest::ignoreMessage(QtWarningMsg, "\"Unknown subcue ':doesnotexist' in UI marker in context {@info:doesnotexist}.\"");
    QCOMPARE(xi18nc("@info:doesnotexist", "E &lt; <emphasis>mc^2</emphasis>"), QString("<html>E &lt; <i>mc^2</i></html>"));

    // Number formatting.
    QCOMPARE(ki18n("%1").subs(42).toString(), QString("42"));
    QCOMPARE(ki18n("%1").subs(42, 5).toString(), QString("   42"));
    QCOMPARE(ki18n("%1").subs(42, -5, 10, QChar('_')).toString(), QString("42___"));
    QCOMPARE(ki18n("%1").subs(4.2, 5, 'f', 2).toString(), QString(" 4,20"));
}

void KLocalizedStringTest::wrongSubs()
{
#ifndef NDEBUG
    // Too many arguments.
    QVERIFY(i18n("Europa", 1) != QString("Europa"));

    // Too few arguments.
    QVERIFY(i18n("%1, %2 and %3", QString("Hunter"), QString("Kimball")) != QString("Hunter, Kimball and %3"));

    // Gaps in placheholder numbering.
    QVERIFY(ki18n("Beyond the %2").subs("infinity").toString() != QString("Beyond the infinity"));

    // Plural argument not supplied.
    QVERIFY(ki18np("1 pod", "%1 pods").toString() != QString("1 pod"));
    QVERIFY(ki18np("1 pod", "%1 pods").toString() != QString("%1 pods"));
#endif
}

void KLocalizedStringTest::semanticTags()
{
    KLocalizedString::setLanguages({"en"});

    // <application/>
    QCOMPARE(xi18nc("@action:inmenu", "Open with <application>%1</application>", "Okteta"), QString("Open with Okteta"));
    QCOMPARE(xi18nc("@info", "Open with <application>%1</application>", "Okteta"), QString("<html>Open with Okteta</html>"));
    // <bcode/>
    QCOMPARE(xi18nc("@info:whatsthis",
                    "You can try the following snippet:<bcode>"
                    "\\begin{equation}\n"
                    "  C_{x_i} = \\frac{C_z^2}{e \\pi \\lambda}\n"
                    "\\end{equation}"
                    "</bcode>"),
             QString("<html>You can try the following snippet:\n\n<pre>"
                     "\\begin{equation}\n"
                     "  C_{x_i} = \\frac{C_z^2}{e \\pi \\lambda}\n"
                     "\\end{equation}"
                     "</pre></html>"));
    // <command/>
    QCOMPARE(xi18nc("@info", "This will call <command>%1</command> internally.", "true"), QString("<html>This will call <tt>true</tt> internally.</html>"));
    QCOMPARE(xi18nc("@info", "Consult man entry for <command section='%2'>%1</command>", "true", 1),
             QString("<html>Consult man entry for <tt>true(1)</tt></html>"));
    // <email/>
    QCOMPARE(xi18nc("@info", "Send bug reports to <email>%1</email>.", "konqi@kde.org"),
             QString("<html>Send bug reports to &lt;<a href=\"mailto:konqi@kde.org\">konqi@kde.org</a>&gt;.</html>"));
    QCOMPARE(xi18nc("@info", "Send praises to <email address='%1'>%2</email>.", "konqi@kde.org", "Konqi"),
             QString("<html>Send praises to <a href=\"mailto:konqi@kde.org\">Konqi</a>.</html>"));
    // <emphasis/>
    QCOMPARE(xi18nc("@info:progress", "Checking <emphasis>feedback</emphasis> circuits..."), QString("Checking *feedback* circuits..."));
    QCOMPARE(xi18nc("@info:progress", "Checking <emphasis strong='true'>feedback</emphasis> circuits..."), QString("Checking **feedback** circuits..."));
    // <envar/>
    QCOMPARE(xi18nc("@info", "Assure that your <envar>PATH</envar> is properly set."),
             QString("<html>Assure that your <tt>$PATH</tt> is properly set.</html>"));
    // <filename/>
    QCOMPARE(xi18nc("@info", "Cannot read <filename>%1</filename>.", "data.dat"), QString("<html>Cannot read \u2018<tt>data.dat</tt>\u2019.</html>"));
    // TODO: is nested <tt><tt></tt></tt> really wanted?
#ifndef Q_OS_WIN
    QString homeFooRc("<html>\u2018<tt><tt>$HOME</tt>/.foorc</tt>\u2019 does not exist.</html>");
#else
    // TODO $HOME -> %HOME% ?
    QString homeFooRc("<html>\u2018<tt><tt>$HOME</tt>\\.foorc</tt>\u2019 does not exist.</html>");
#endif
    QCOMPARE(xi18nc("@info", "<filename><envar>HOME</envar>/.foorc</filename> does not exist."), homeFooRc);

    // <icode/>
    QCOMPARE(xi18nc("@info:tooltip", "Execute <icode>svn merge</icode> on selected revisions."),
             QString("<html>Execute <tt>svn merge</tt> on selected revisions.</html>"));
    // <interface/>
    QCOMPARE(xi18nc("@info:whatsthis", "If you make a mistake, click <interface>Reset</interface> to start again."),
             QString("<html>If you make a mistake, click <i>Reset</i> to start again.</html>"));
    QCOMPARE(xi18nc("@info:whatsthis", "The line colors can be changed under <interface>Settings->Visuals</interface>."),
             QString("<html>The line colors can be changed under <i>Settings-&gt;Visuals</i>.</html>"));
    // <link/>
    QCOMPARE(xi18nc("@info:tooltip", "Go to <link>%1</link> website.", "http://kde.org/"),
             QString("<html>Go to <a href=\"http://kde.org/\">http://kde.org/</a> website.</html>"));
    QCOMPARE(xi18nc("@info:tooltip", "Go to <link url='%1'>%2</link>.", "http://kde.org/", "the KDE website"),
             QString("<html>Go to <a href=\"http://kde.org/\">the KDE website</a>.</html>"));
    // <message/>
    QCOMPARE(xi18nc("@info", "The fortune cookie says: <message>%1</message>", "Nothing"), QString("<html>The fortune cookie says: <i>Nothing</i></html>"));
    // <nl/>
#ifndef Q_OS_WIN
    QString deleteEtcPasswd("<html>Do you really want to delete:<br/>\u2018<tt>/etc/passwd</tt>\u2019</html>");
#else
    QString deleteEtcPasswd("<html>Do you really want to delete:<br/>\u2018<tt>\\etc\\passwd</tt>\u2019</html>");
#endif
    QCOMPARE(xi18nc("@info", "Do you really want to delete:<nl/><filename>%1</filename>", "/etc/passwd"), deleteEtcPasswd);

    // check <nl/> within filename doesn't break (Windows path separators)
#ifndef Q_OS_WIN
    QString filenameWithNewline("<html>\u2018<tt>/filename/with<br/>/newline</tt>\u2019</html>");
#else
    QString filenameWithNewline("<html>\u2018<tt>\\filename\\with<br/>\\newline</tt>\u2019</html>");
#endif
    QCOMPARE(xi18nc("@info", "<filename>/filename/with<nl/>/newline</filename>"), filenameWithNewline);

    // <numid/>
    QEXPECT_FAIL("", "what happened to <numid/>? TODO.", Continue);
    QCOMPARE(xi18nc("@info:progress", "Connecting to <numid>%1</numid>...", 22), QString("<html>Connecting to <tt>22</tt></html>"));
    QCOMPARE(xi18nc("@info", "Replace <placeholder>name</placeholder> with your name."), QString("<html>Replace &lt;<i>name</i>&gt; with your name.</html>"));
    QCOMPARE(xi18nc("@item:inlistbox", "<placeholder>All images</placeholder>"), QString("<All images>"));
    // <resource/>
    QCOMPARE(xi18nc("@info", "Apply color scheme <resource>%1</resource>?", "XXX"), QString("<html>Apply color scheme “XXX”?</html>"));
    QCOMPARE(xi18nc("@info:whatsthis", "Cycle through layouts using <shortcut>Alt+Space</shortcut>."),
             QString("<html>Cycle through layouts using <b>Alt+Space</b>.</html>"));
    // <note/>
    QCOMPARE(xi18nc("@info",
                    "Probably the best known of all duck species is the Mallard. "
                    "It breeds throughout the temperate areas around the world. "
                    "<note>Most domestic ducks are derived from Mallard.</note>"),
             QString("<html>Probably the best known of all duck species is the Mallard. "
                     "It breeds throughout the temperate areas around the world. "
                     "<i>Note</i>: Most domestic ducks are derived from Mallard.</html>"));
    QCOMPARE(xi18nc("@info", "<note label='Trivia'>Most domestic ducks are derived from Mallard.</note>"),
             QString("<html><i>Trivia</i>: Most domestic ducks are derived from Mallard.</html>"));
    // <warning/>
    QCOMPARE(xi18nc("@info",
                    "Really delete this key?"
                    "<warning>This cannot be undone.</warning>"),
             QString("<html>Really delete this key?"
                     "<b>Warning</b>: This cannot be undone.</html>"));
    QCOMPARE(xi18nc("@info", "<warning label='Danger'>This cannot be undone.</warning>"), QString("<html><b>Danger</b>: This cannot be undone.</html>"));
}

void KLocalizedStringTest::setFormatForMarker()
{
    KLocalizedString::setLanguages({"en"});

    QCOMPARE(xi18nc("@info:tooltip", "Hello world"), QString("<html>Hello world</html>"));
    KuitSetup &setup = Kuit::setupForDomain(KLocalizedString::applicationDomain());
    setup.setFormatForMarker("@info:tooltip", Kuit::PlainText);
    QCOMPARE(xi18nc("@info:tooltip", "Hello world"), QString("Hello world"));
}

void KLocalizedStringTest::removeAcceleratorMarker()
{
    // No accelerator marker.
    QCOMPARE(KLocalizedString::removeAcceleratorMarker(QString()), QString());
    QCOMPARE(KLocalizedString::removeAcceleratorMarker("Foo bar"), QString("Foo bar"));

    // Run of the mill.
    QCOMPARE(KLocalizedString::removeAcceleratorMarker("&Foo bar"), QString("Foo bar"));
    QCOMPARE(KLocalizedString::removeAcceleratorMarker("Foo &bar"), QString("Foo bar"));
    QCOMPARE(KLocalizedString::removeAcceleratorMarker("Foo b&ar"), QString("Foo bar"));
    // - presence of escaped ampersands
    QCOMPARE(KLocalizedString::removeAcceleratorMarker("Foo && Bar"), QString("Foo & Bar"));
    QCOMPARE(KLocalizedString::removeAcceleratorMarker("Foo && &Bar"), QString("Foo & Bar"));
    QCOMPARE(KLocalizedString::removeAcceleratorMarker("&Foo && Bar"), QString("Foo & Bar"));

    // CJK-style markers.
    QCOMPARE(KLocalizedString::removeAcceleratorMarker("Foo bar (&F)"), QString("Foo bar"));
    QCOMPARE(KLocalizedString::removeAcceleratorMarker("(&F) Foo bar"), QString("Foo bar"));
    // - interpunction after/before parenthesis still qualifies CJK marker
    QCOMPARE(KLocalizedString::removeAcceleratorMarker("Foo bar (&F):"), QString("Foo bar:"));
    QCOMPARE(KLocalizedString::removeAcceleratorMarker("Foo bar (&F)..."), QString("Foo bar..."));
    QCOMPARE(KLocalizedString::removeAcceleratorMarker("...(&F) foo bar"), QString("...foo bar"));
    // - alphanumerics around parenthesis disqualify CJK marker
    QCOMPARE(KLocalizedString::removeAcceleratorMarker("Foo (&F) bar"), QString("Foo (F) bar"));
    // - something removed raw ampersands, leaving dangling reduced CJK markers.
    // Remove reduced markers only if CJK characters are found in the string.
    QCOMPARE(KLocalizedString::removeAcceleratorMarker(QString::fromUtf8("Foo bar (F)")), QString::fromUtf8("Foo bar (F)"));
    QCOMPARE(KLocalizedString::removeAcceleratorMarker(QString::fromUtf8("印刷(P)...")), QString::fromUtf8("印刷..."));

    // Shady cases, where ampersand is obviously not a marker
    // and should have been escaped, but it was not.
    QCOMPARE(KLocalizedString::removeAcceleratorMarker("&"), QString("&"));
    QCOMPARE(KLocalizedString::removeAcceleratorMarker("Foo bar &"), QString("Foo bar &"));
    QCOMPARE(KLocalizedString::removeAcceleratorMarker("Foo & Bar"), QString("Foo & Bar"));
}

void KLocalizedStringTest::miscMethods()
{
    KLocalizedString k;
    QVERIFY(k.isEmpty());

    if (m_hasFrench) {
        QSet<QString> availableLanguages;
        availableLanguages.insert("fr");
        availableLanguages.insert("en_US");
        if (m_hasCatalan) {
            availableLanguages.insert("ca");
        }
        QCOMPARE(KLocalizedString::availableApplicationTranslations(), availableLanguages);
    }
}

// Same as translateToFrench, but using libintl directly (bindtextdomain+dgettext).
// Useful for debugging. This changes global state, though, so it's skipped by default.
void KLocalizedStringTest::translateToFrenchLowlevel()
{
    if (!m_hasFrench) {
        QSKIP("French test files not usable.");
    }
    QSKIP("Skipped by default to avoid changing global state.");
    // fr_FR locale was set by initTestCase already.
    if (QFile::exists("/usr/share/locale/fr/LC_MESSAGES/ki18n-test.mo")) {
        bindtextdomain("ki18n-test", "/usr/share/locale");
        QCOMPARE(QString::fromUtf8(dgettext("ki18n-test", "Loadable modules")), QString::fromUtf8("Modules chargeables"));
    }
}

void KLocalizedStringTest::translateToFrench()
{
    if (!m_hasFrench) {
        QSKIP("French test files not usable.");
    }
    QCOMPARE(i18n("Loadable modules"), QString::fromUtf8("Modules chargeables"));
    QCOMPARE(i18n("Job"), QString::fromUtf8("Tâche"));
}

void KLocalizedStringTest::testLocalizedTranslator()
{
    if (!m_hasFrench) {
        QSKIP("French test files not usable.");
    }
    QScopedPointer<KLocalizedTranslator> translator(new KLocalizedTranslator());
    QCoreApplication *app = QCoreApplication::instance();
    app->installTranslator(translator.data());

    // no translation domain and no context
    QCOMPARE(app->translate("foo", "Job"), QStringLiteral("Job"));

    // adding the translation domain still lacks the context
    translator->setTranslationDomain(QStringLiteral("ki18n-test"));
    QCOMPARE(app->translate("foo", "Job"), QStringLiteral("Job"));

    translator->addContextToMonitor(QStringLiteral("foo"));
    // now it should translate
    QCOMPARE(app->translate("foo", "Job"), QStringLiteral("Tâche"));
    // other context shouldn't translate
    QCOMPARE(app->translate("bar", "Job"), QStringLiteral("Job"));
    // with a mismatching disambiguation it shouldn't translate
    QCOMPARE(app->translate("foo", "Job", "bar"), QStringLiteral("Job"));
}

void KLocalizedStringTest::addCustomDomainPath()
{
    if (!m_hasFrench) {
        QSKIP("French test files not usable.");
    }
    QTemporaryDir dir;
    compileCatalogs({QFINDTESTDATA("po/fr/ki18n-test2.po")}, dir.path(), "fr");
    KLocalizedString::addDomainLocaleDir("ki18n-test2", dir.path() + "/locale");

    QSet<QString> expectedAvailableTranslations({"en_US", "fr"});
    QCOMPARE(KLocalizedString::availableDomainTranslations("ki18n-test2"), expectedAvailableTranslations);
    QCOMPARE(i18nd("ki18n-test2", "Cheese"), QString::fromUtf8("Fromage"));
}

void KLocalizedStringTest::multipleLanguages()
{
    if (!m_hasFrench || !m_hasCatalan) {
        QSKIP("French or Catalan test files not usable.");
    }
    KLocalizedString::setLanguages({"ca"});
    QCOMPARE(i18n("Job"), QString::fromUtf8("Job")); // This is not the actual catalan translation but who cares
    KLocalizedString::setLanguages({"fr"});
    QCOMPARE(i18n("Job"), QString::fromUtf8("Tâche"));
    KLocalizedString::setLanguages({"ca", "fr"});
    QCOMPARE(i18n("Job"), QString::fromUtf8("Job")); // This is not the actual catalan translation but who cares

    KLocalizedString::setLanguages({"ca"});
    QCOMPARE(i18n("Loadable modules"), QString::fromUtf8("Loadable modules")); // The po doesn't have a translation so we get the English text
    KLocalizedString::setLanguages({"fr"});
    QCOMPARE(i18n("Loadable modules"), QString::fromUtf8("Modules chargeables"));
    KLocalizedString::setLanguages({"ca", "fr"});
    QCOMPARE(i18n("Loadable modules"), QString::fromUtf8("Modules chargeables")); // The Catalan po doesn't have a translation so we get the English text
}

void KLocalizedStringTest::untranslatedText()
{
    if (!m_hasFrench) {
        QSKIP("French test files not usable.");
    }
    KLocalizedString s = ki18n("Job");
    KLocalizedString::setLanguages({"fr"});
    QCOMPARE(s.untranslatedText(), "Job");
    QCOMPARE(s.toString(), QString::fromUtf8("Tâche"));
    QCOMPARE(s.untranslatedText(), "Job");
}

void KLocalizedStringTest::brokenStructTagUsages()
{
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("Structuring tag \\('title'\\) cannot be subtag of phrase tag \\('emphasis'\\) in message {.*}."));
    QCOMPARE(xi18nc("@info", "<emphasis><title>History</title></emphasis>"), QString("<html><i>History</i></html>"));
}

void KLocalizedStringTest::brokenTags()
{
    QTest::ignoreMessage(
        QtWarningMsg,
        QRegularExpression("Markup error in message {.*}: Opening and ending tag mismatch.. Last tag parsed: email. Complete message follows"));
    QCOMPARE(xi18nc("@info", "Send bug reports to <email>%1<email>.", "konqi@kde.org"), // notice the missing '/' before "email"
             QString("<html>Send bug reports to <email>konqi@kde.org<email>.</html>"));
}

#include <QFutureSynchronizer>
#include <QThreadPool>
#include <QtConcurrentRun>

void KLocalizedStringTest::testThreads()
{
    QThreadPool::globalInstance()->setMaxThreadCount(10);
    QFutureSynchronizer<void> sync;
    sync.addFuture(QtConcurrent::run(&KLocalizedStringTest::correctSubs, this));
    sync.addFuture(QtConcurrent::run(&KLocalizedStringTest::correctSubs, this));
    sync.addFuture(QtConcurrent::run(&KLocalizedStringTest::correctSubs, this));
    sync.addFuture(QtConcurrent::run(&KLocalizedStringTest::translateToFrench, this));
    sync.waitForFinished();
    QThreadPool::globalInstance()->setMaxThreadCount(1); // delete those threads
}

void KLocalizedStringTest::testLazy()
{
    if (!m_hasFrench) {
        QSKIP("French test files not usable.");
    }
    KLocalizedString s = kli18n("Job");
    KLocalizedString::setLanguages({"fr"});
    QCOMPARE(s.toString(), QString::fromUtf8("Tâche"));
}

QTEST_MAIN(KLocalizedStringTest)

#include "moc_klocalizedstringtest.cpp"
