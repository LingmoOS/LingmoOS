/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2001-2003 Cornelius Schumacher <schumacher@kde.org>

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

#ifndef KCALCORE_EXCEPTIONS_H
#define KCALCORE_EXCEPTIONS_H

#include <QString>
#include <QStringList>

#include <memory>

namespace KCalendarCore {
class ExceptionPrivate;

/**
  Exception base class, currently used as a fancy kind of error code
  and not as an C++ exception.
*/
class Q_CORE_EXPORT Exception
{
public:
    /**
      The different types of error codes
    */
    enum ErrorCode {
        LoadError, /**< Load error */
        SaveError, /**< Save error */
        ParseErrorIcal, /**< Parse error in libical */
        ParseErrorKcal, /**< Parse error in libkcal */
        NoCalendar, /**< No calendar component found */
        CalVersion1, /**< vCalendar v1.0 detected */
        CalVersion2, /**< iCalendar v2.0 detected */
        CalVersionUnknown, /**< Unknown calendar format detected */
        Restriction, /**< Restriction violation */
        UserCancel, /**< User canceled the operation */
        NoWritableFound, /**< No writable resource is available */
        SaveErrorOpenFile,
        SaveErrorSaveFile,
        LibICalError,
        VersionPropertyMissing,
        ExpectedCalVersion2,
        ExpectedCalVersion2Unknown,
        ParseErrorNotIncidence,
        ParseErrorEmptyMessage,
        ParseErrorUnableToParse,
        ParseErrorMethodProperty,
    };

    /**
      Construct an exception.
      @param code is the error code.
      @param arguments is a list of arguments that can be passed
             to an i18n engine to help build a descriptive message for the user, a common
             argument is for example the filename where the error occurred.
    */
    explicit Exception(const ErrorCode code, const QStringList &arguments = QStringList());

    /**
      Destructor.
    */
    virtual ~Exception();

    /**
      Returns the error code.
      @return The ErrorCode for this exception.
    */
    Q_REQUIRED_RESULT virtual ErrorCode code() const;

    /**
      Returns the arguments.
      @return A QStringList with the argument list for this exception.
    */
    Q_REQUIRED_RESULT virtual QStringList arguments() const;

private:
    std::unique_ptr<ExceptionPrivate> d;
};

} // namespace KCalendarCore

#endif
