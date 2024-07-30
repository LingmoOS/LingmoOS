/*
    SPDX-FileCopyrightText: 2011 Martin Klapetek <mklapetek@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef PERSON_DETAILS_VIEW_H
#define PERSON_DETAILS_VIEW_H

#include <QWidget>

#include <kpeople/widgets/kpeoplewidgets_export.h>

#include "../persondata.h"

class QLabel;
class QVBoxLayout;

namespace KPeople
{
class PersonDetailsViewPrivate;

/**
 * Use PersonDetailsView to integrate a person's information
 * in the GUI.
 *
 * @since 5.8
 */
class KPEOPLEWIDGETS_EXPORT PersonDetailsView : public QWidget
{
    Q_OBJECT
public:
    explicit PersonDetailsView(QWidget *parent = nullptr);
    ~PersonDetailsView() override;

public Q_SLOTS:
    /** Specifies the @p person for which the details will be displayed. */
    void setPerson(PersonData *person);
    //     void setPersonsModel(PersonsModel *model);

private Q_SLOTS:
    KPEOPLEWIDGETS_NO_EXPORT void reload();

private:
    Q_DECLARE_PRIVATE(PersonDetailsView)
    PersonDetailsViewPrivate *d_ptr;
};
}

#endif // PERSON_DETAILS_VIEW_H
