/*
    SPDX-FileCopyrightText: 2021 Ahmad Samir <a.samirh78@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kurlhandlertest.h"

#include "kurlhandler_p.h"

#include <QTest>

void KUrlHandlerTest::testOpeningDocsUrls_data()
{
    QTest::addColumn<QString>("appname");
    QTest::addColumn<QString>("helpUrl");
    QTest::addColumn<QString>("expected");

    const QString common = "https://docs.kde.org/index.php?branch=stable5&language=" + QLocale().name();

    QTest::newRow("okular-help-handbook") << "okular"
                                          << "help:/" << common + "&application=okular&path=index.html";

    QTest::newRow("okular-help-config-dialog") << "okular"
                                               << "help:/okular/configure.html" << common + "&application=okular&path=configure.html";

    QTest::newRow("systemsettings-fonts") << "systemsettings"
                                          << "help:/kcontrol/fonts/index.html" << common + "&application=kcontrol/fonts&path=index.html";

    QTest::newRow("systemsettings-clock") << "systemsettings"
                                          << "help:/kcontrol/clock/index.html" << common + "&application=kcontrol/clock&path=index.html";

    QTest::newRow("systemsettings-powerdevil") << "systemsettings"
                                               << "help:/kcontrol/powerdevil/index.html#advanced-settings"
                                               << common + "&application=kcontrol/powerdevil&path=index.html#advanced-settings";

    QTest::newRow("with-fragment") << "kinfocenter"
                                   << "help:/kinfocenter/index.html#kcm_memory" << common + "&application=kinfocenter&path=kcm_memory.html";

    QTest::newRow("with-fragment-2") << "okular"
                                     << "help:/okular/signatures.html#adding_digital_signatures"
                                     << common + "&application=okular&path=signatures.html#adding_digital_signatures";
}

void KUrlHandlerTest::testOpeningDocsUrls()
{
    QFETCH(QString, appname);
    QFETCH(QString, helpUrl);
    QFETCH(QString, expected);

    QCoreApplication::setOrganizationDomain(QStringLiteral("kde.org"));
    QCoreApplication::setApplicationName(appname);

    const QUrl actual = KUrlHandler().concatDocsUrl(QUrl(helpUrl));
    QCOMPARE(actual, QUrl(expected));
}

QTEST_MAIN(KUrlHandlerTest)

#include "moc_kurlhandlertest.cpp"
