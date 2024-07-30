/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#include "contact.h"

Address::Address(QObject *parent)
    : QObject(parent)
{
}

int Address::houseNumber() const
{
    return m_houseNumber;
}

QString Address::city()
{
    return m_city;
}

QString Address::streetName()
{
    return m_streetName;
}

void Address::setCity(const QString &city)
{
    m_city = city;
}

void Address::setHouseNumber(int houseNumber)
{
    m_houseNumber = houseNumber;
}

void Address::setStreetName(const QString &streetName)
{
    m_streetName = streetName;
}

Contact::Contact(QObject *parent)
    : QObject(parent)
    , m_address(0)
{
}

QString Contact::name() const
{
    return m_name;
}

void Contact::setName(const QString &name)
{
    m_name = name;
}

QString Contact::email() const
{
    return m_email;
}

void Contact::setEmail(const QString &email)
{
    m_email = email;
}

QString Contact::phone() const
{
    return m_phone;
}

void Contact::setPhone(const QString &phone)
{
    m_phone = phone;
}

QObject *Contact::address() const
{
    return m_address;
}

void Contact::setAddress(Address *address)
{
    m_address = address;
}

QString Contact::nickname() const
{
    return m_nickname;
}

void Contact::setNickname(const QString &nickname)
{
    m_nickname = nickname;
}

QString Contact::salaryCurrency() const
{
    return m_salaryCurrency;
}

void Contact::setSalaryCurrency(const QString &salaryCurrency)
{
    m_salaryCurrency = salaryCurrency;
}

double Contact::salary() const
{
    return m_salary;
}

void Contact::setSalary(double salary)
{
    m_salary = salary;
}

double Contact::rating() const
{
    return m_rating;
}

void Contact::setRating(double rating)
{
    m_rating = rating;
}

QDate Contact::birthday() const
{
    return m_birthday;
}

void Contact::setBirthday(const QDate &birthday)
{
    m_birthday = birthday;
}

QVariant Contact::data(int role) const
{
    if (role != Qt::DisplayRole)
        return QListWidgetItem::data(role);
    return m_name;
}

QVariant Contact::friends() const
{
    return QVariant::fromValue(m_friends);
}

void Contact::setFriends(const QList<QObject *> &friends)
{
    m_friends = friends;
}

#include "moc_contact.cpp"
