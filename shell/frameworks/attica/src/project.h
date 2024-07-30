/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2010 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ATTICA_PROJECT_H
#define ATTICA_PROJECT_H

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
 * @class Project project.h <Attica/Project>
 *
 * Represents a project.
 */
class ATTICA_EXPORT Project
{
public:
    typedef QList<Project> List;
    class Parser;

    Project();
    Project(const Project &other);
    Project &operator=(const Project &other);
    ~Project();

    void setId(const QString &);
    QString id() const;

    void setName(const QString &);
    QString name() const;

    void setVersion(const QString &);
    QString version() const;

    void setUrl(const QString &);
    QString url() const;

    void setLicense(const QString &);
    QString license() const;

    void setSummary(const QString &);
    QString summary() const;

    void setDescription(const QString &);
    QString description() const;

    void setDevelopers(const QStringList &);
    QStringList developers() const;

    void setRequirements(const QString &);
    QString requirements() const;

    void setSpecFile(const QString &);
    QString specFile() const;

    void addExtendedAttribute(const QString &key, const QString &value);
    QString extendedAttribute(const QString &key) const;

    QMap<QString, QString> extendedAttributes() const;

    bool isValid() const;

private:
    class Private;
    QSharedDataPointer<Private> d;
};

}

#endif
