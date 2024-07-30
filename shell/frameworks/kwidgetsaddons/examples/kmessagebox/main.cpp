/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
    SPDX-FileCopyrightText: 2018 Olivier Churlaud <olivier@churlaud.com>
    SPDX-FileCopyrightText: 2016 Juan Carlos Torres <carlosdgtorres@gmail.com>

    SPDX-License-Identifier: LGPL-3.0-or-later
*/

#include <KMessageBox>
#include <QApplication>

#include <iostream>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Define the question answer buttons
    KGuiItem primaryAction(QStringLiteral("Hello"), // the button label
                           QStringLiteral("view-filter"), // iconName, can be empty to fallback to default
                           QStringLiteral("This is a tooltip"),
                           QStringLiteral("This is a WhatsThis help text."));

    KGuiItem secondaryAction(QStringLiteral("Bye")); // the button label

    KMessageBox::ButtonCode res = KMessageBox::questionTwoActions(nullptr, // Parent, here none
                                                                           // The description can contain HTML
                                                                  QStringLiteral("Description to tell you to click<br />on <b>either</b> button"),
                                                                  QStringLiteral("My Title"),
                                                                  primaryAction,
                                                                  secondaryAction);

    if (res == KMessageBox::PrimaryAction) {
        std::cout << "You clicked Hello\n";
        return EXIT_SUCCESS;
    } else {
        std::cout << "You clicked Bye\n";
        return EXIT_FAILURE;
    }
}
