/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2012 Laszlo Papp <lpapp@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ATTICA_CLOUD_H
#define ATTICA_CLOUD_H

#include "attica_export.h"

#include <QList>
#include <QSharedDataPointer>
#include <QUrl>

namespace Attica
{

/**
 * @class Cloud cloud.h <Attica/Cloud>
 *
 * Represents a cloud service.
 */
class ATTICA_EXPORT Cloud
{
public:
    typedef QList<Cloud> List;
    class Parser;

    /**
     * Creates an empty Cloud
     */

    Cloud();

    /**
     * Copy constructor.
     * @param other the Cloud to copy from
     */

    Cloud(const Cloud &other);

    /**
     * Assignment operator.
     * @param other the Cloud to assign from
     * @return pointer to this Activity
     */

    Cloud &operator=(const Cloud &other);

    /**
     * Destructor.
     */

    ~Cloud();

    /**
     * Sets the name of the Cloud service
     *
     * @param name the new name
     */

    void setName(const QString &name);

    /**
     * Gets the name of the Cloud service.
     *
     * @return the name
     */

    QString name() const;

    /**
     * Sets the url of the Cloud service
     *
     * @param url the new url
     */

    void setUrl(const QString &url);

    /**
     * Gets the url of the Cloud service.
     *
     * @return the url
     */

    QString url() const;

    /**
     * Sets the icon of the Cloud service
     *
     * @param icon the new icon
     */

    void setIcon(const QUrl &icon);

    /**
     * Gets the icon of the Cloud service.
     *
     * @return the icon
     */

    QUrl icon() const;

    /**
     * Sets the quota of the Cloud service
     *
     * @param quota the new quota
     */

    void setQuota(qulonglong quota);

    /**
     * Gets the quota of the Cloud service.
     *
     * @return the quota
     */

    qulonglong quota() const;

    /**
     * Sets the free amount of the Cloud service
     *
     * @param free the new free amount
     */

    void setFree(qulonglong free);

    /**
     * Gets the free amount of the Cloud service.
     *
     * @return the free amount
     */

    qulonglong free() const;

    /**
     * Sets the used amount of the Cloud service
     *
     * @param used the new used amount
     */

    void setUsed(qulonglong used);

    /**
     * Gets the used amount of the Cloud service.
     *
     * @return the used amount
     */

    qulonglong used() const;

    /**
     * Sets the relative of the Cloud service
     *
     * @param relative the new relative
     */

    void setRelative(float relative);

    /**
     * Gets the relative of the Cloud service.
     *
     * @return the relative
     */

    float relative() const;

    /**
     * Sets the private key of the Cloud service
     *
     * @param privateKey the new privateKey
     */

    void setKey(const QString &privateKey);

    /**
     * Gets the private key of the Cloud service.
     *
     * @return the private key
     */

    QString key() const;

private:
    class Private;
    QSharedDataPointer<Private> d;
};

}

#endif
