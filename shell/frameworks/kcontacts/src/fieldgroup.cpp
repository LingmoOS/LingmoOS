/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2016-2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "fieldgroup.h"
#include "parametermap_p.h"

#include <QDataStream>
#include <QStringList>

using namespace KContacts;

class Q_DECL_HIDDEN FieldGroup::Private : public QSharedData
{
public:
    Private()
    {
    }

    Private(const Private &other)
        : QSharedData(other)
    {
        mParamMap = other.mParamMap;
        fieldGroupName = other.fieldGroupName;
        value = other.value;
    }

    ParameterMap mParamMap;
    QString fieldGroupName;
    QString value;
};

FieldGroup::FieldGroup()
    : d(new Private)
{
}

FieldGroup::FieldGroup(const FieldGroup &other)
    : d(other.d)
{
}

FieldGroup::FieldGroup(const QString &FieldGroupName)
    : d(new Private)
{
    d->fieldGroupName = FieldGroupName;
}

FieldGroup::~FieldGroup()
{
}

void FieldGroup::setFieldGroupName(const QString &fieldGroup)
{
    d->fieldGroupName = fieldGroup;
}

QString FieldGroup::fieldGroupName() const
{
    return d->fieldGroupName;
}

bool FieldGroup::isValid() const
{
    return !d->fieldGroupName.isEmpty();
}

void FieldGroup::setValue(const QString &value)
{
    d->value = value;
}

QString FieldGroup::value() const
{
    return d->value;
}

void FieldGroup::setParams(const ParameterMap &params)
{
    d->mParamMap = params;
}

ParameterMap FieldGroup::params() const
{
    return d->mParamMap;
}

bool FieldGroup::operator==(const FieldGroup &other) const
{
    return (d->mParamMap == other.d->mParamMap) && (d->fieldGroupName == other.fieldGroupName()) && (d->value == other.value());
}

bool FieldGroup::operator!=(const FieldGroup &other) const
{
    return !(other == *this);
}

FieldGroup &FieldGroup::operator=(const FieldGroup &other)
{
    if (this != &other) {
        d = other.d;
    }

    return *this;
}

QString FieldGroup::toString() const
{
    QString str = QLatin1String("FieldGroup {\n");
    str += QStringLiteral("    FieldGroupName: %1 Value %2\n").arg(d->fieldGroupName).arg(d->value);
    str += d->mParamMap.toString();
    str += QLatin1String("}\n");
    return str;
}

QDataStream &KContacts::operator<<(QDataStream &s, const FieldGroup &fieldGroup)
{
    return s << fieldGroup.d->mParamMap << fieldGroup.d->fieldGroupName << fieldGroup.d->value;
}

QDataStream &KContacts::operator>>(QDataStream &s, FieldGroup &fieldGroup)
{
    s >> fieldGroup.d->mParamMap >> fieldGroup.d->fieldGroupName >> fieldGroup.d->value;
    return s;
}
