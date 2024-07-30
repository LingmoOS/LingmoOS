/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2007 Olivier Goffart <ogoffart at kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QApplication>
#include <QMap>

#include <kpassworddialog.h>

#include <iostream>

// We can't depend on i18n in this test, but still -- show good practice (other than the line below) :-)
#define i18n QString::fromLatin1

int main(int argc, char *argv[])
{
    QApplication::setApplicationName(QStringLiteral("KNewPasswordDialogTest"));
    QApplication a(argc, argv);

    // step 1  simple password
    {
        KPasswordDialog dlg(nullptr, KPasswordDialog::ShowKeepPassword);
        dlg.setPrompt(
            i18n("This is a long prompt line. It is important it to be long so we can test the dialog does not get broken because of multiline labels. Please "
                 "enter a password:"));
        dlg.addCommentLine(i18n("This is a rather large left comment line"),
                           i18n("Right part of the comment line has to be long too so be test the layouting works really ok. Please visit http://www.kde.org"));

        if (dlg.exec()) {
            std::cout << "Entered password: " << qPrintable(dlg.password()) << std::endl;
        } else {
            std::cout << "No password" << std::endl;
            return -1;
        }
    }

    // step 2 readonly username
    {
        KPasswordDialog dlg(nullptr, KPasswordDialog::ShowUsernameLine | KPasswordDialog::UsernameReadOnly);
        dlg.setPrompt(i18n("Enter a password for the test"));
        dlg.setUsername(QStringLiteral("konqui"));
        dlg.addCommentLine(i18n("Site"), i18n("http://www.kde.org"));

        if (dlg.exec()) {
            std::cout << "Entered password: " << qPrintable(dlg.password()) << std::endl;
        } else {
            std::cout << "No password" << std::endl;
            return -1;
        }
    }

    // step 3 with some username preset
    {
        KPasswordDialog dlg(nullptr, KPasswordDialog::ShowUsernameLine);
        dlg.setPrompt(i18n("Enter a password for the test"));
        QMap<QString, QString> logins;
        logins.insert(QStringLiteral("konqui"), QStringLiteral("foo"));
        logins.insert(QStringLiteral("watson"), QStringLiteral("bar"));
        logins.insert(QStringLiteral("ogoffart"), QString());

        dlg.setKnownLogins(logins);

        if (dlg.exec()) {
            std::cout << "Entered password: " << qPrintable(dlg.password()) << " for username " << qPrintable(dlg.username()) << std::endl;
        } else {
            std::cout << "No password" << std::endl;
            return -1;
        }
    }

    return 0;
}
