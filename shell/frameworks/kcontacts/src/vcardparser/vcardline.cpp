/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2003 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "vcardline_p.h"

#include "parametermap_p.h"

using namespace KContacts;

VCardLine::VCardLine()
{
}

VCardLine::VCardLine(const QString &identifier)
    : VCardLine(identifier, {})
{
}

VCardLine::VCardLine(const QString &identifier, const QVariant &value)
    : mIdentifier(identifier)
    , mValue(value)
{
}

VCardLine::VCardLine(const VCardLine &line)
    : mParamMap(line.mParamMap)
    , mIdentifier(line.mIdentifier)
    , mGroup(line.mGroup)
    , mValue(line.mValue)
{
}

VCardLine::~VCardLine()
{
}

VCardLine &VCardLine::operator=(const VCardLine &line)
{
    if (&line == this) {
        return *this;
    }

    mParamMap = line.mParamMap;
    mValue = line.mValue;
    mIdentifier = line.mIdentifier;
    mGroup = line.mGroup;

    return *this;
}

bool VCardLine::operator==(const VCardLine &other) const
{
    // clang-format off
    return (mParamMap == other.parameterMap())
           && (mValue == other.value())
           && (mIdentifier == other.identifier())
           && (mGroup == other.group());
    // clang-format on
}

void VCardLine::setIdentifier(const QString &identifier)
{
    mIdentifier = identifier;
}

QString VCardLine::identifier() const
{
    return mIdentifier;
}

void VCardLine::setValue(const QVariant &value)
{
    mValue = value;
}

QVariant VCardLine::value() const
{
    return mValue;
}

void VCardLine::setGroup(const QString &group)
{
    mGroup = group;
}

QString VCardLine::group() const
{
    return mGroup;
}

bool VCardLine::hasGroup() const
{
    return !mGroup.isEmpty();
}

QStringList VCardLine::parameterList() const
{
    QStringList list;
    list.reserve(mParamMap.size());
    for (const auto &[param, values] : mParamMap) {
        list.append(param);
    }

    return list;
}

void VCardLine::addParameters(const ParameterMap &params)
{
    for (const auto &[param, list] : params) {
        addParameter(param, list.join(QLatin1Char(',')));
    }
}

void VCardLine::addParameter(const QString &param, const QString &value)
{
    auto it = mParamMap.findParam(param);
    if (it != mParamMap.end()) {
        if (!it->paramValues.contains(value)) { // not included yet
            it->paramValues.push_back(value);
        }
    } else {
        mParamMap.insertParam({param, QStringList{value}});
    }
}

QStringList VCardLine::parameters(const QString &param) const
{
    auto it = mParamMap.findParam(param);
    return it != mParamMap.cend() ? it->paramValues : QStringList{};
}

QString VCardLine::parameter(const QString &param) const
{
    auto it = mParamMap.findParam(param);
    return it != mParamMap.cend() && !it->paramValues.isEmpty() ? it->paramValues.at(0) : QString{};
}

ParameterMap VCardLine::parameterMap() const
{
    return mParamMap;
}
