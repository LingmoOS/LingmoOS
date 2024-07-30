/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2013 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include <QApplication>
#include <QCommandLineParser>
#include <QLabel>
#include <kx11extras.h>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QCommandLineParser parser;
    parser.addPositionalArgument(QStringLiteral("WId"), QStringLiteral("window id for the window to take the icon from"), QStringLiteral("[WId]"));
    parser.addHelpOption();
    parser.process(app);
    QLabel label;
    label.setMinimumSize(250, 250);
    label.show();
    QString wId = parser.positionalArguments().first();
    label.setPixmap(KX11Extras::icon(wId.toULongLong(nullptr, 0), 250, 250, false, KX11Extras::WMHints));
    return app.exec();
}
