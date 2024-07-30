/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2013 David Faure <faure+bluesystem@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "klanguagebutton.h"

#include <QApplication>

int main(int argc, char **argv)
{
    QApplication::setApplicationName(QStringLiteral("KLanguageButtonTest"));
    QApplication app(argc, argv);

    KLanguageButton button;
    button.loadAllLanguages();
    button.show();
    return app.exec();
}
