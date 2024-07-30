/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2013 Aurélien Gâteau <agateau@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include <iostream>

#include <kmessagebox.h>

#include <QApplication>

int main(int argc, char **argv)
{
    QApplication::setApplicationName(QStringLiteral("kmessageboxwidtest"));
    QApplication app(argc, argv);
    if (argc != 2) {
        std::cerr << "Usage: " << qPrintable(QApplication::applicationName()) << " <window id>\n";
        std::cerr << '\n';
        std::cerr << "You can get a window id using a tool like `xwininfo`.\n";
        return 1;
    }

    bool ok;
    int winId = QByteArray(argv[1]).toInt(&ok, 0);
    if (!ok) {
        std::cerr << '"' << argv[1] << "\" is not a number\n";
        return 1;
    }

    int ret = KMessageBox::warningContinueCancelWId(winId, QStringLiteral("Are you sure you want to continue?"), QStringLiteral("Dangerous stuff"));

    std::cout << "Returned " << ret << '\n';
    return ret;
}
