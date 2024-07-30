/*
    Convenience dialog for displaying PersonDetailsView widgets
    SPDX-FileCopyrightText: 2013 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef PERSONDETAILSDIALOG_H
#define PERSONDETAILSDIALOG_H

#include <QDialog>

#include "../persondata.h"
#include <kpeople/widgets/kpeoplewidgets_export.h>

namespace KPeople
{
class PersonDetailsDialogPrivate;

class KPEOPLEWIDGETS_EXPORT PersonDetailsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PersonDetailsDialog(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    ~PersonDetailsDialog() override;

    void setPerson(PersonData *person);

private:
    Q_DECLARE_PRIVATE(PersonDetailsDialog)
    PersonDetailsDialogPrivate *d_ptr;
};

}
#endif // PERSONDETAILSDIALOG_H
