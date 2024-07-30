/* -*- C++ -*-
    Base class for exceptions in ThreadWeaver.

    SPDX-FileCopyrightText: 2005-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "exception.h"

namespace ThreadWeaver
{
Exception::Exception(const QString &message)
    : std::runtime_error(message.toStdString())
    , m_message(message)
{
}

Exception::~Exception() throw()
{
}

QString Exception::message() const
{
    return m_message;
}

JobAborted::JobAborted(const QString &message)
    : Exception(message)
{
}

JobFailed::JobFailed(const QString &message)
    : Exception(message)
{
}

AbortThread::AbortThread(const QString &message)
    : Exception(message)
{
}

}
