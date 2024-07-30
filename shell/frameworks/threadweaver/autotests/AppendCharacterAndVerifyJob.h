/* -*- C++ -*-
    Helper class for unit tests.

    SPDX-FileCopyrightText: 2005-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef APPENDCHARACTERANDVERIFYJOB_H
#define APPENDCHARACTERANDVERIFYJOB_H

#include <AppendCharacterJob.h>

class AppendCharacterAndVerifyJob : public AppendCharacterJob
{
public:
    AppendCharacterAndVerifyJob();
    void setValues(QChar character, QString *stringref, const QString &expected);
    void run(ThreadWeaver::JobPointer, ThreadWeaver::Thread *) override;

private:
    QString m_expected;
};

#endif // APPENDCHARACTERANDVERIFYJOB_H
