/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2002 Carsten Pfeiffer <pfeiffer@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <ktextedit.h>

#include <QAction>
#include <QApplication>
#include <QFile>
#include <QTest>

int main(int argc, char **argv)
{
    QApplication::setApplicationName(QStringLiteral("ktextedittest"));
    QApplication app(argc, argv);
    KTextEdit *edit = new KTextEdit();

    // QAction* action = new QAction("Select All", edit);
    // action->setShortcut( Qt::CTRL | Qt::Key_Underscore );
    // edit->addAction(action);
    // QObject::connect(action, SIGNAL(triggered()), edit, SLOT(selectAll()));

    QFile file(QFINDTESTDATA(QLatin1String("ktextedittest.cpp")));
    if (file.open(QIODevice::ReadOnly)) {
        edit->setPlainText(QLatin1String(file.readAll()));
        file.close();
    }

    edit->resize(600, 600);
    edit->show();
    return app.exec();
}
