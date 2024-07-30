/*
    Persons Model Example
    SPDX-FileCopyrightText: 2012 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include <QApplication>
#include <QDebug>

#include <persondata.h>
#include <widgets/persondetailsview.h>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    if (app.arguments().count() < 2) {
        qWarning() << "Missing argument: \"" << qPrintable(app.arguments().at(0)) << " <contact id>\"";
        return 1;
    }

    KPeople::PersonData *person = new KPeople::PersonData(app.arguments().last());

    KPeople::PersonDetailsView w;
    w.setPerson(person);

    w.show();

    return app.exec();
}
