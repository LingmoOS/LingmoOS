/*
    Convenience dialog for displaying PersonDetailsView widgets
    SPDX-FileCopyrightText: 2013 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "persondetailsdialog.h"

#include <QDialogButtonBox>
#include <QVBoxLayout>

#include "persondetailsview.h"

using namespace KPeople;

namespace KPeople
{
class PersonDetailsDialogPrivate
{
public:
    PersonDetailsView *view;
};
}

PersonDetailsDialog::PersonDetailsDialog(QWidget *parent, Qt::WindowFlags f)
    : QDialog(parent, f)
    , d_ptr(new PersonDetailsDialogPrivate)
{
    Q_D(PersonDetailsDialog);
    QVBoxLayout *layout = new QVBoxLayout(this);

    d->view = new PersonDetailsView(this);
    layout->addWidget(d->view);

    QDialogButtonBox *buttons = new QDialogButtonBox(this);
    buttons->setStandardButtons(QDialogButtonBox::Ok);
    layout->addWidget(buttons);

    connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
}

PersonDetailsDialog::~PersonDetailsDialog()
{
    delete d_ptr;
}

void PersonDetailsDialog::setPerson(PersonData *person)
{
    Q_D(PersonDetailsDialog);
    d->view->setPerson(person);

    setWindowTitle(person->name());
}

#include "moc_persondetailsdialog.cpp"
