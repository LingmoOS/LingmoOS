/* -*- C++ -*-
    Helper class for unit tests.

    SPDX-FileCopyrightText: 2005-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "AppendCharacterAndVerifyJob.h"

AppendCharacterAndVerifyJob::AppendCharacterAndVerifyJob()
    : AppendCharacterJob()
{
}

void AppendCharacterAndVerifyJob::setValues(QChar c, QString *stringref, const QString &expected)
{
    AppendCharacterJob::setValues(c, stringref);
    m_expected = expected;
}

void AppendCharacterAndVerifyJob::run(ThreadWeaver::JobPointer, ThreadWeaver::Thread *)
{
    using namespace ThreadWeaver;
    QMutexLocker locker(&s_GlobalMutex);
    stringRef()->append(character());
    if (m_expected.mid(0, stringRef()->length()) != *stringRef()) {
        TWDEBUG(3, "It broke!");
    }
    TWDEBUG(3, "AppendCharacterJob::run: %c appended, result is %s.\n", character().toLatin1(), qPrintable(*stringRef()));
}
