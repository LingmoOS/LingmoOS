/*
    This file is part of the KDE Libraries
    SPDX-FileCopyrightText: 2013 Kevin Ottens <ervin+bluesystems@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include <QApplication>
#include <QVBoxLayout>

#include <kratingwidget.h>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QWidget window;
    auto *layout = new QVBoxLayout(&window);

    KRatingWidget *enabled = new KRatingWidget(&window);
    layout->addWidget(enabled);

    KRatingWidget *disabled = new KRatingWidget(&window);
    disabled->setEnabled(false);
    layout->addWidget(disabled);

    window.show();

    return app.exec();
}
