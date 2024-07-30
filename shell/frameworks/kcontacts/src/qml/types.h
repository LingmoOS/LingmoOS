/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2024 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCONTACTS_QML_TYPES
#define KCONTACTS_QML_TYPES

#include <KContacts/Address>
#include <KContacts/AddressFormat>
#include <KContacts/Addressee>

#include <QQmlEngine>

struct AddressForeign {
    Q_GADGET
    QML_FOREIGN(KContacts::Address)
    QML_VALUE_TYPE(address)
    QML_STRUCTURED_VALUE
};

struct AddresseeForeign {
    Q_GADGET
    QML_FOREIGN(KContacts::Addressee)
    QML_VALUE_TYPE(addressee)
    QML_STRUCTURED_VALUE
};

struct EmailForeign {
    Q_GADGET
    QML_FOREIGN(KContacts::Email)
    QML_VALUE_TYPE(email)
    QML_CONSTRUCTIBLE_VALUE
};

struct PhoneNumberForeign {
    Q_GADGET
    QML_FOREIGN(KContacts::PhoneNumber)
    QML_VALUE_TYPE(phoneNumber)
};

struct ImppForeign {
    Q_GADGET
    QML_FOREIGN(KContacts::Impp)
    QML_VALUE_TYPE(impp)
};

struct PictureForeign {
    Q_GADGET
    QML_FOREIGN(KContacts::Picture)
    QML_VALUE_TYPE(picture)
};

struct GeoForeign {
    Q_GADGET
    QML_FOREIGN(KContacts::Geo)
    QML_VALUE_TYPE(geo)
};

namespace KContactForeign
{
Q_NAMESPACE
QML_NAMED_ELEMENT(KContacts)
QML_FOREIGN_NAMESPACE(KContacts)
};

#endif
