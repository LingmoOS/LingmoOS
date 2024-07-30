// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2023 Harald Sitter <sitter@kde.org>

#include <QCoreApplication>

#include <sentrypostman.h>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    SentryPostman postman;
    postman.run();

    return app.exec();
}
