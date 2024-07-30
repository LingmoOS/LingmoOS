/*
    Duplicates Example
    SPDX-FileCopyrightText: 2014 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "mergedialog.h"
#include <personsmodel.h>

#include <QApplication>
#include <QTimer>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    KPeople::MergeDialog *dialog = new KPeople::MergeDialog;
    KPeople::PersonsModel *model = new KPeople::PersonsModel(&app);

    QTimer *t = new QTimer(&app);
    t->setInterval(500);
    t->setSingleShot(true);
    QObject::connect(model, SIGNAL(modelInitialized(bool)), t, SLOT(start()));
    QObject::connect(model, SIGNAL(rowsInserted(QModelIndex, int, int)), t, SLOT(start()));
    QObject::connect(t, &QTimer::timeout, [dialog, model]() {
        dialog->setPersonsModel(model);
        dialog->show();
    });

    return app.exec();
}
