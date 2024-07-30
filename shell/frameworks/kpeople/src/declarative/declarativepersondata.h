/*
    SPDX-FileCopyrightText: 2013 David Edmundson <D.Edmundson@lboro.ac.uk>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef DECLARATIVEPERSONDATA_H
#define DECLARATIVEPERSONDATA_H

#include "persondata.h"

#include <qqmlregistration.h>

class DeclarativePersonData : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(PersonData)

    Q_PROPERTY(QString personUri READ personUri WRITE setPersonUri NOTIFY personChanged)
    Q_PROPERTY(KPeople::PersonData *person READ person NOTIFY personChanged)
    Q_PROPERTY(QString photoImageProviderUri READ photoImageProviderUri NOTIFY photoImageProviderUriChanged)

public:
    explicit DeclarativePersonData(QObject *parent = nullptr);

    void setPersonUri(const QString &id);
    QString personUri() const;

    KPeople::PersonData *person() const;

    QString photoImageProviderUri() const;

Q_SIGNALS:
    void personChanged();
    void photoImageProviderUriChanged();

private:
    QString m_id;
    KPeople::PersonData *m_person = nullptr;
};

#endif // DECLARATIVEPERSONDATA_H
