/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2015-2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "email.h"
#include "parametermap_p.h"

#include <QDataStream>
#include <QStringList>

using namespace KContacts;

class Q_DECL_HIDDEN Email::Private : public QSharedData
{
public:
    Private()
    {
    }

    Private(const Private &other)
        : QSharedData(other)
    {
        mParamMap = other.mParamMap;
        mail = other.mail;
    }

    ParameterMap mParamMap;

    QString mail;
};

Email::Email()
    : d(new Private)
{
}

Email::Email(const QString &mail)
    : d(new Private)
{
    d->mail = mail;
}

Email::Email(const Email &other)
    : d(other.d)
{
}

Email::~Email()
{
}

bool Email::operator==(const Email &other) const
{
    return (d->mParamMap == other.d->mParamMap) && (d->mail == other.mail());
}

bool Email::operator!=(const Email &other) const
{
    return !(other == *this);
}

Email &Email::operator=(const Email &other)
{
    if (this != &other) {
        d = other.d;
    }

    return *this;
}

QString Email::toString() const
{
    QString str = QLatin1String("Email {\n");
    str += QStringLiteral("    mail: %1\n").arg(d->mail);
    str += d->mParamMap.toString();
    str += QLatin1String("}\n");
    return str;
}

void Email::setParams(const ParameterMap &params)
{
    d->mParamMap = params;
}

ParameterMap Email::params() const
{
    return d->mParamMap;
}

void Email::setEmail(const QString &mail)
{
    d->mail = mail;
}

QString Email::mail() const
{
    return d->mail;
}

bool Email::isValid() const
{
    return !d->mail.isEmpty();
}

struct email_type_name {
    const char *name;
    Email::Type type;
};

static const email_type_name email_type_names[] = {
    {"HOME", Email::Home},
    {"WORK", Email::Work},
    {"OTHER", Email::Other},
};

Email::Type KContacts::Email::type() const
{
    const auto it = d->mParamMap.findParam(QLatin1String("type"));
    if (it == d->mParamMap.end()) {
        return Unknown;
    }

    Type type = Unknown;
    for (const auto &s : it->paramValues) {
        const auto it = std::find_if(std::begin(email_type_names), std::end(email_type_names), [&s](const email_type_name &t) {
            return QLatin1String(t.name) == s;
        });
        if (it != std::end(email_type_names)) {
            type |= (*it).type;
        }
    }
    return type;
}

void Email::setType(Type type)
{
    const auto oldType = this->type();

    const QString paramName(QStringLiteral("type"));

    ParameterMap::iterator theIt;

    auto it = d->mParamMap.findParam(paramName);
    if (it != d->mParamMap.end()) {
        theIt = it;
    } else {
        theIt = d->mParamMap.insertParam({paramName, {}});
    }

    for (const auto &t : email_type_names) {
        if (((type ^ oldType) & t.type) == 0) {
            continue; // no change
        }

        if (type & t.type) {
            theIt->paramValues.push_back(QLatin1String(t.name));
        } else {
            theIt->paramValues.removeAll(QLatin1String(t.name));
        }
    }
}

bool Email::isPreferred() const
{
    auto it = d->mParamMap.findParam(QLatin1String("pref"));
    if (it != d->mParamMap.end()) {
        return !it->paramValues.isEmpty() && it->paramValues.at(0) == QLatin1Char('1');
    }

    it = d->mParamMap.findParam(QLatin1String("type"));
    if (it != d->mParamMap.end()) {
        return it->paramValues.contains(QLatin1String("PREF"), Qt::CaseInsensitive);
    }

    return false;
}

void Email::setPreferred(bool preferred)
{
    if (preferred == isPreferred()) {
        return;
    }

    const QString paramName(QStringLiteral("type"));

    auto typeIt = d->mParamMap.findParam(paramName);
    QStringList types = typeIt != d->mParamMap.end() ? typeIt->paramValues : QStringList{};

    if (!preferred) {
        auto prefIt = d->mParamMap.findParam(QLatin1String("pref"));
        if (prefIt != d->mParamMap.end()) {
            d->mParamMap.erase(prefIt);
        }

        types.removeAll(QLatin1String("PREF"));
    } else {
        types.push_back(QStringLiteral("PREF"));
    }

    typeIt = d->mParamMap.findParam(paramName);
    if (typeIt != d->mParamMap.end()) {
        typeIt->paramValues = types;
    } else {
        d->mParamMap.insertParam({paramName, types});
    }
}

QDataStream &KContacts::operator<<(QDataStream &s, const Email &email)
{
    return s << email.d->mParamMap << email.d->mail;
}

QDataStream &KContacts::operator>>(QDataStream &s, Email &email)
{
    s >> email.d->mParamMap >> email.d->mail;
    return s;
}

#include "moc_email.cpp"
