/*
    SPDX-FileCopyrightText: 2007 Pino Toscano <pino@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QApplication>

#include <kwalletwizard.h>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    KWalletWizard wizard;
    wizard.show();

    int ret = app.exec();

    return ret;
}
