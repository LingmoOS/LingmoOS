/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef JOBDOUBLE_H
#define JOBDOUBLE_H

#include <QFile>
#include <QTextStream>

#include <apijob.h>

namespace Bugzilla
{
class JobDouble : public APIJob
{
public:
    using APIJob::APIJob;

    JobDouble(const QString &fixture)
        : m_fixture(fixture)
    {
    }

    QByteArray data() const override
    {
        Q_ASSERT(!m_fixture.isEmpty());
        QFile file(m_fixture);
        if (!file.open(QFile::ReadOnly | QFile::Text)) {
            return {};
        }
        QTextStream in(&file);
        return in.readAll().toUtf8();
    }

    QString m_fixture;
};

} // namespace Bugzilla

#endif // JOBDOUBLE_H
