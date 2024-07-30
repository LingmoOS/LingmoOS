/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#include "integers.h"

#include "util.h"

QVariant AddFilter::doFilter(const QVariant &input, const QVariant &argument, bool autoescape) const
{
    Q_UNUSED(autoescape)

    if (isSafeString(input)) {
        if (isSafeString(argument))
            return getSafeString(input) + getSafeString(argument);
        return input;
    }

    if (input.userType() == qMetaTypeId<QVariantList>()) {
        if (argument.userType() == qMetaTypeId<QVariantList>())
            return input.value<QVariantList>() + argument.value<QVariantList>();
        return input;
    }

    if (input.userType() == qMetaTypeId<QStringList>()) {
        if (argument.userType() == QMetaType::QStringList)
            return input.value<QStringList>() + argument.value<QStringList>();
        return input;
    }

    if (input.userType() == qMetaTypeId<int>()) {
        if (argument.userType() == qMetaTypeId<int>())
            return input.value<int>() + argument.value<int>();
        return input;
    }

    if (input.userType() == qMetaTypeId<uint>()) {
        if (argument.userType() == qMetaTypeId<uint>())
            return input.value<uint>() + argument.value<uint>();
        return input;
    }

    if (input.canConvert<double>()) {
        if (argument.canConvert<double>())
            return input.value<double>() + argument.value<double>();
        return input;
    }

    if (input.userType() == qMetaTypeId<long long>()) {
        if (argument.userType() == qMetaTypeId<long long>())
            return input.value<long long>() + argument.value<long long>();
        return input;
    }

    if (input.userType() == qMetaTypeId<unsigned long long>()) {
        if (input.userType() == qMetaTypeId<unsigned long long>())
            return input.value<unsigned long long>() + argument.value<unsigned long long>();
        return input;
    }
    return input;
}

QVariant GetDigitFilter::doFilter(const QVariant &input, const QVariant &argument, bool autoescape) const
{
    Q_UNUSED(autoescape)
    auto value = getSafeString(input);

    bool ok;
    (void)value.get().toInt(&ok);
    if (!ok)
        return QString();

    if (value.get().size() < 1)
        return value;

    auto arg = getSafeString(argument).get().toInt();

    if (value.get().size() < arg)
        return value;

    return SafeString(value.get().at(value.get().size() - arg));
}
