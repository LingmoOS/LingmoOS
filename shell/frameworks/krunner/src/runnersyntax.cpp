/*
    SPDX-FileCopyrightText: 2009 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2023 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "runnersyntax.h"

#include <KLocalizedString>

namespace KRunner
{
class RunnerSyntaxPrivate
{
public:
    RunnerSyntaxPrivate(const QStringList &_exampleQueries, const QString &_description)
        : exampleQueries(prepareExampleQueries(_exampleQueries))
        , description(_description)
    {
    }

    static QStringList prepareExampleQueries(const QStringList &queries)
    {
        Q_ASSERT_X(!queries.isEmpty(), "KRunner::RunnerSyntax", "List of example queries must not be empty");
        QStringList exampleQueries;
        for (const QString &query : queries) {
            Q_ASSERT_X(!query.isEmpty(), "KRunner::RunnerSyntax", "Example query must not be empty!");
            const static QString termDescription = i18n("search term");
            const QString termDesc(QLatin1Char('<') + termDescription + QLatin1Char('>'));
            exampleQueries.append(QString(query).replace(QLatin1String(":q:"), termDesc));
        }
        return exampleQueries;
    }

    const QStringList exampleQueries;
    const QString description;
};

RunnerSyntax::RunnerSyntax(const QStringList &exampleQueries, const QString &description)
    : d(new RunnerSyntaxPrivate(exampleQueries, description))
{
    Q_ASSERT_X(!exampleQueries.isEmpty(), "KRunner::RunnerSyntax", "Example queries must not be empty");
}

RunnerSyntax::RunnerSyntax(const RunnerSyntax &other)
    : d(new RunnerSyntaxPrivate(*other.d))
{
}

RunnerSyntax::~RunnerSyntax() = default;

RunnerSyntax &RunnerSyntax::operator=(const RunnerSyntax &rhs)
{
    d.reset(new RunnerSyntaxPrivate(*rhs.d));
    return *this;
}

QStringList RunnerSyntax::exampleQueries() const
{
    return d->exampleQueries;
}

QString RunnerSyntax::description() const
{
    return d->description;
}

} // KRunner namespace
