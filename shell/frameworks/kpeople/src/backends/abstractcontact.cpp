/*
    SPDX-FileCopyrightText: 2014 Aleix Pol i Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "abstractcontact.h"

using namespace KPeople;

const QString AbstractContact::NameProperty = QStringLiteral("name");
const QString AbstractContact::EmailProperty = QStringLiteral("email");
const QString AbstractContact::PhoneNumberProperty = QStringLiteral("phoneNumber");
const QString AbstractContact::AllPhoneNumbersProperty = QStringLiteral("all-phoneNumber");
const QString AbstractContact::PresenceProperty = QStringLiteral("presence");
const QString AbstractContact::AllEmailsProperty = QStringLiteral("all-email");
const QString AbstractContact::PictureProperty = QStringLiteral("picture");
const QString AbstractContact::VCardProperty = QStringLiteral("vcard");
const QString AbstractContact::GroupsProperty = QStringLiteral("all-groups");

AbstractContact::AbstractContact()
{
}

AbstractContact::~AbstractContact()
{
}
