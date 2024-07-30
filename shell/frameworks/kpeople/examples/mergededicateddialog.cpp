/*
    SPDX-FileCopyrightText: 2013 Franck Arrecot <franck.arrecot@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include <QApplication>

#include <personsmodel.h>
#include <personsmodelfeature.h>
#include <widgets/mergedialog.h>

using namespace KPeople;

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    MergeDialog dialog;
    PersonsModel *persons = new PersonsModel(&dialog);
    QList<KPeople::PersonsModelFeature> features;
    /* clang-format off */
    features << KPeople::PersonsModelFeature::emailModelFeature()
             << KPeople::PersonsModelFeature::avatarModelFeature()
             << KPeople::PersonsModelFeature::imModelFeature()
             << KPeople::PersonsModelFeature::fullNameModelFeature();
    /* clang-format on */
    persons->startQuery(features);

    dialog.setPersonsModel(persons);

    QObject::connect(persons, SIGNAL(modelInitialized()), &dialog, SLOT(show()));
    QObject::connect(&dialog, SIGNAL(finished(int)), &app, SLOT(quit()));

    return app.exec();
}
