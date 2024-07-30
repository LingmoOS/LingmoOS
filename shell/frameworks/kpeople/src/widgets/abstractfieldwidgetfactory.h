/*
    SPDX-FileCopyrightText: 2013 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef ABSTRACTFIELDWIDGETFACTORY_H
#define ABSTRACTFIELDWIDGETFACTORY_H

#include <QWidget>

#include <kpeople/widgets/kpeoplewidgets_export.h>

namespace KPeople
{
class AbstractFieldWidgetFactoryPrivate;
class PersonData;

class KPEOPLEWIDGETS_EXPORT AbstractFieldWidgetFactory : public QObject
{
    Q_OBJECT
public:
    explicit AbstractFieldWidgetFactory(QObject *parent = nullptr);
    ~AbstractFieldWidgetFactory() override;

    virtual QString label() const = 0;
    virtual int sortWeight() const
    {
        return 100;
    }

    virtual QWidget *createDetailsWidget(const KPeople::PersonData &person, QWidget *parent) const = 0;
};
}

#endif // ABSTRACTFIELDWIDGETFACTORY_H
