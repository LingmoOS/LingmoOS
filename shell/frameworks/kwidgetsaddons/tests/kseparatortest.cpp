/*
    SPDX-FileCopyrightText: 1997 Michael Roth <mroth@wirlweb.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QApplication>
#include <QBoxLayout>
#include <QWidget>

#include "kseparator.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QWidget toplevel;
    QBoxLayout *mainbox = new QBoxLayout(QBoxLayout::TopToBottom, &toplevel);
    mainbox->setContentsMargins(10, 10, 10, 10);

    KSeparator *sep1 = new KSeparator(Qt::Vertical, &toplevel);
    mainbox->addWidget(sep1);

    KSeparator *sep2 = new KSeparator(Qt::Horizontal, &toplevel);
    mainbox->addWidget(sep2);
    mainbox->activate();

    toplevel.show();
    return app.exec();
}
