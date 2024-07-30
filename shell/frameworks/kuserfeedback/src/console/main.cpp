/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include <kuserfeedback_version.h>
#include "mainwindow.h"

#include <QApplication>

int main(int argc, char** argv)
{
    QCoreApplication::setApplicationName(QStringLiteral("UserFeedbackConsole"));
    QCoreApplication::setApplicationVersion(QStringLiteral(KUSERFEEDBACK_VERSION_STRING));
    QCoreApplication::setOrganizationName(QStringLiteral("KDE"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("kde.org"));

    Q_INIT_RESOURCE(schematemplates);

    QApplication app(argc, argv);

    KUserFeedback::Console::MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}
