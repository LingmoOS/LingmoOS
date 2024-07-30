/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2010 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "buildservicejoboutput.h"

using namespace Attica;

class Q_DECL_HIDDEN BuildServiceJobOutput::Private : public QSharedData
{
public:
    QString output;

    Private()
    {
    }
};

BuildServiceJobOutput::BuildServiceJobOutput()
    : d(new Private)
{
}

BuildServiceJobOutput::BuildServiceJobOutput(const BuildServiceJobOutput &other)
    : d(other.d)
{
}

BuildServiceJobOutput &BuildServiceJobOutput::operator=(const Attica::BuildServiceJobOutput &other)
{
    d = other.d;
    return *this;
}

BuildServiceJobOutput::~BuildServiceJobOutput()
{
}

void BuildServiceJobOutput::setOutput(const QString &output)
{
    d->output = output;
}

QString BuildServiceJobOutput::output() const
{
    return d->output;
}

bool BuildServiceJobOutput::isValid() const
{
    return !(d->output.isNull());
}
