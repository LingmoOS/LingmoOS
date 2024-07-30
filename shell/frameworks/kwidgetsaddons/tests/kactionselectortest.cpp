/*
    SPDX-FileCopyrightText: 2006 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <kactionselector.h>

#include <QApplication>
#include <QListWidget>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    KActionSelector actionSelector(nullptr);
    actionSelector.availableListWidget()->addItems(QStringList() << QStringLiteral("A") << QStringLiteral("B") << QStringLiteral("C") << QStringLiteral("D")
                                                                 << QStringLiteral("E"));
    actionSelector.selectedListWidget()->addItems(QStringList() << QStringLiteral("1") << QStringLiteral("2"));
    actionSelector.show();

    return app.exec();
}
