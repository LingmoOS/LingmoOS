/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2010 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef ATTICA_BUILDSERVICEJOB_H
#define ATTICA_BUILDSERVICEJOB_H

#include <QDate>
#include <QList>
#include <QMap>
#include <QSharedDataPointer>
#include <QStringList>
#include <QUrl>

#include "attica_export.h"

namespace Attica
{

/**
 * @class BuildServiceJob buildservicejob.h <Attica/BuildServiceJob>
 *
 * Represents a build service job.
 */
class ATTICA_EXPORT BuildServiceJob
{
public:
    typedef QList<BuildServiceJob> List;
    class Parser;

    BuildServiceJob();
    BuildServiceJob(const BuildServiceJob &other);
    BuildServiceJob &operator=(const BuildServiceJob &other);
    ~BuildServiceJob();

    void setId(const QString &);
    QString id() const;

    void setName(const QString &);
    QString name() const;

    void setUrl(const QString &);
    QString url() const;

    void setProjectId(const QString &);
    QString projectId() const;

    void setBuildServiceId(const QString &);
    QString buildServiceId() const;

    void setMessage(const QString &);
    QString message() const;

    void setTarget(const QString &);
    QString target() const;

    void setProgress(const qreal);
    qreal progress() const;

    void setStatus(const int);
    bool isRunning() const;
    bool isCompleted() const;
    bool isFailed() const;

    bool isValid() const;

private:
    class Private;
    QSharedDataPointer<Private> d;
};

}

#endif
