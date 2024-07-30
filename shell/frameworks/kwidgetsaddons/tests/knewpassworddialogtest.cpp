/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2007 Olivier Goffart <ogoffart at kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <iostream>
#include <knewpassworddialog.h>
#include <qapplication.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    KNewPasswordDialog dlg;
    dlg.setPasswordStrengthWarningLevel(30);
    dlg.setPrompt(QApplication::translate("main", "Enter a password for the test"));

    if (dlg.exec()) {
        std::cout << "Entered password: " << qPrintable(dlg.password()) << std::endl;
        return 0;
    } else {
        std::cout << "No password" << std::endl;
        return -1;
    }
}
