/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#include "logic.h"

#include "util.h"

QVariant DefaultFilter::doFilter(const QVariant &input, const QVariant &argument, bool autoescape) const
{
    Q_UNUSED(autoescape)
    if (!input.isValid() || getSafeString(input).get().isEmpty())
        return argument;
    return getSafeString(input);
}

QVariant DefaultIfNoneFilter::doFilter(const QVariant &input, const QVariant &argument, bool autoescape) const
{
    Q_UNUSED(autoescape)
    if (!input.isValid())
        return argument;
    return getSafeString(input);
}

QVariant DivisibleByFilter::doFilter(const QVariant &input, const QVariant &argument, bool autoescape) const
{
    Q_UNUSED(autoescape)
    return (getSafeString(input).get().toInt() % QVariant(argument).value<int>() == 0) ? QStringLiteral("true") : QString();
}

QVariant YesNoFilter::doFilter(const QVariant &input, const QVariant &argument, bool autoescape) const
{
    Q_UNUSED(autoescape)
    auto arg = getSafeString(argument);
    QString yes;
    QString no;
    QString maybe;
    if (arg.get().isEmpty()) {
        yes = QStringLiteral("yes");
        no = QStringLiteral("no");
        maybe = QStringLiteral("maybe");
    } else {
        auto argList = arg.get().split(QLatin1Char(','));
        auto numArgs = argList.size();
        if ((numArgs < 2) || (numArgs > 3)) {
            return input.value<QString>();
        }
        if (numArgs == 2) {
            yes = argList.first();
            no = argList.at(1);
            maybe = argList.at(1);
        } else if (numArgs == 3) {
            yes = argList.first();
            no = argList.at(1);
            maybe = argList.at(2);
        }
    }
    if (!input.isValid())
        return maybe;
    if (!getSafeString(input).get().isEmpty())
        return yes;
    return no;
}
