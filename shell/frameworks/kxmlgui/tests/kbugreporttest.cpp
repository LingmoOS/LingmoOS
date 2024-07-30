/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2005 Joseph Wenninger <jowenn@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KAboutData>
#include <KLocalizedString>
#include <QApplication>
#include <kbugreport.h>

int main(int argc, char **argv)
{
    QApplication a(argc, argv);
    a.setQuitOnLastWindowClosed(false);

    // First a bug report to bugs.kde.org
    KAboutData about(QStringLiteral("kbugreporttest"), i18n("kbugreporttest"), QStringLiteral("version"));
    KBugReport rep(about);
    rep.exec();

    // Then a bug report by email.
    // Change the email address to check if it worked :)
    KAboutData about1(QStringLiteral("kbugreporttest"),
                      i18n("kbugreporttest"),
                      QStringLiteral("version"),
                      i18n("description"),
                      KAboutLicense::Unknown,
                      i18n("copyright"),
                      i18n("bug report tool"),
                      QString(),
                      QStringLiteral("null@bugs.kde.org"));
    KBugReport rep1(about1);
    rep1.exec();

    // Then a web bug report.
    KAboutData about2(QStringLiteral("kbugreporttest"),
                      i18n("kbugreporttest"),
                      QStringLiteral("version"),
                      i18n("description"),
                      KAboutLicense::Unknown,
                      i18n("copyright"),
                      i18n("bug report tool"),
                      QString(),
                      QStringLiteral("https://bugs.kde.org"));
    KBugReport rep2(about2);
    rep2.exec();

    return 0;
}
