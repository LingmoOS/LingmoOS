/*
    SPDX-FileCopyrightText: 2013 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef EMAIL_DETAILS_WIDGET_H
#define EMAIL_DETAILS_WIDGET_H

#include "abstractfieldwidgetfactory.h"

using namespace KPeople;

class EmailFieldsPlugin : public AbstractFieldWidgetFactory
{
public:
    QString label() const override;
    int sortWeight() const override;
    QWidget *createDetailsWidget(const PersonData &person, QWidget *parent) const override;
};

#endif // EMAIL_DETAILS_WIDGET_H
