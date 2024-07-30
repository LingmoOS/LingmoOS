/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2018 Ralf Habacker <ralf.habacker@freenet.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef ATTICA_CONFIG_H
#define ATTICA_CONFIG_H

#include <QSharedDataPointer>
#include <QString>

#include "attica_export.h"

namespace Attica
{
/**
 * @class Config config.h <Attica/Config>
 *
 * Represents a server config
 */
class ATTICA_EXPORT Config
{
public:
    typedef QList<Config> List;
    class Parser;

    /**
     * Creates an empty Config
     */
    Config();

    /**
     * Copy constructor.
     * @param other the Config to copy from
     */
    Config(const Config &other);

    /**
     * Assignment operator.
     * @param other the Config to assign from
     * @return pointer to this Config
     */
    Config &operator=(const Config &other);

    /**
     * Destructor.
     */
    ~Config();

    QString contact() const;
    QString host() const;
    QString version() const;
    bool ssl() const;
    QString website() const;

    void setContact(const QString &contact);
    void setHost(const QString &host);
    void setSsl(bool ssl);
    void setVersion(const QString &version);
    void setWebsite(const QString &website);

    /**
     * Checks whether this config is valid
     * @return @c true if config is valid, @c false otherwise
     */
    bool isValid() const;

private:
    class Private;
    QSharedDataPointer<Private> d;
};

}

#endif
