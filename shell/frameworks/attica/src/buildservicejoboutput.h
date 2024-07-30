/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2010 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef ATTICA_BUILDSERVICEJOBOUTPUT_H
#define ATTICA_BUILDSERVICEJOBOUTPUT_H

#include <QList>
#include <QSharedDataPointer>
#include <QString>

#include "attica_export.h"

namespace Attica
{

/**
 * @class BuildServiceJobOutput buildservicejoboutput.h <Attica/BuildServiceJobOutput>
 *
 * Represents the ouput of a build service job.
 */
class ATTICA_EXPORT BuildServiceJobOutput
{
public:
    typedef QList<BuildServiceJobOutput> List;
    class Parser;

    BuildServiceJobOutput();
    BuildServiceJobOutput(const BuildServiceJobOutput &other);
    BuildServiceJobOutput &operator=(const BuildServiceJobOutput &other);
    ~BuildServiceJobOutput();

    void setOutput(const QString &output);
    QString output() const;

    bool isRunning() const;
    bool isCompleted() const;
    bool isFailed() const;

    bool isValid() const;

private:
    class Private;
    QSharedDataPointer<Private> d;
};

} // namespace Attica

#endif // ATTICA_BUILDSERVICEJOBOUTPUT_H
