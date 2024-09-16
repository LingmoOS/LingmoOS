/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2001 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
/**
  @file
  This file is part of the API for handling calendar data and
  defines the Exception class.

  We don't use actual C++ exceptions right now. These classes are currently
  returned by an error function; but we can build upon them, if/when we start
  to use C++ exceptions.

  @brief
  Exception base class.

  @author Cornelius Schumacher \<schumacher@kde.org\>
*/

#include "exceptions.h"
#include "calformat.h"

using namespace KCalendarCore;

namespace KCalendarCore {
class ExceptionPrivate
{
public:
    /**
      The current exception code.
    */
    Exception::ErrorCode mCode;

    /** Arguments to pass to i18n(). */
    QStringList mArguments;
};

} // namespace KCalendarCore

Exception::Exception(const ErrorCode code, const QStringList &arguments)
    : d(new ExceptionPrivate)
{
    d->mCode = code;
    d->mArguments = arguments;
}

Exception::~Exception()
{
}

Exception::ErrorCode Exception::code() const
{
    return d->mCode;
}

QStringList Exception::arguments() const
{
    return d->mArguments;
}
