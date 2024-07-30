/*
    SPDX-FileCopyrightText: 2013 David Edmundson <D.Edmundson@lboro.ac.uk>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "declarativepersondata.h"

#include "../persondata.h"
#include "avatarimageprovider.h"
#include "kpeople_debug.h"
#include <imageprovideruri_p.h>

#include <QStringBuilder>

DeclarativePersonData::DeclarativePersonData(QObject *parent)
    : QObject(parent)
    , m_person(nullptr)
{
    connect(this, &DeclarativePersonData::personChanged, this, &DeclarativePersonData::photoImageProviderUriChanged);
}

void DeclarativePersonData::setPersonUri(const QString &id)
{
    if (id == m_id) {
        return;
    }

    m_id = id;
    delete m_person;
    if (m_id.isEmpty()) {
        m_person = nullptr;
    } else {
        m_person = new KPeople::PersonData(id, this);
        connect(m_person, &KPeople::PersonData::dataChanged, this, &DeclarativePersonData::photoImageProviderUriChanged);
    }

    Q_EMIT personChanged();
}

KPeople::PersonData *DeclarativePersonData::person() const
{
    return m_person;
}

QString DeclarativePersonData::photoImageProviderUri() const
{
    return ::photoImageProviderUri(m_id);
}

QString DeclarativePersonData::personUri() const
{
    return m_id;
}

#include "moc_declarativepersondata.cpp"
