/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2010 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef ATTICA_BUILDSERVICE_H
#define ATTICA_BUILDSERVICE_H

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
 * @class Target buildservice.h <Attica/BuildService>
 *
 * The target in a build service.
 */
struct Target {
    QString id;
    QString name;
};

/**
 * @class BuildService buildservice.h <Attica/BuildService>
 *
 * Represents a build service.
 */
class ATTICA_EXPORT BuildService
{
public:
    typedef QList<BuildService> List;
    class Parser;

    BuildService();
    BuildService(const BuildService &other);
    BuildService &operator=(const BuildService &other);
    ~BuildService();

    void setId(const QString &);
    QString id() const;

    void setName(const QString &);
    QString name() const;

    void setUrl(const QString &);
    QString url() const;

    void addTarget(const Target &);
    QList<Target> targets() const;

    bool isValid() const;

private:
    class Private;
    QSharedDataPointer<Private> d;
};

}

#endif
