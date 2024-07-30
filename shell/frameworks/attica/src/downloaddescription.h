/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2009 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef DOWNLOADDESCRIPTION_H
#define DOWNLOADDESCRIPTION_H

#include <QSharedData>
#include <QString>

#include "attica_export.h"

namespace Attica
{

/**
 * @class DownloadDescription downloaddescription.h <Attica/DownloadDescription>
 *
 * Represents a download description.
 */
class ATTICA_EXPORT DownloadDescription
{
public:
    enum Type {
        FileDownload = 0,
        LinkDownload,
        PackageDownload,
    };

    DownloadDescription();
    DownloadDescription(const DownloadDescription &other);

    DownloadDescription &operator=(const DownloadDescription &other);
    ~DownloadDescription();

    /**
      The id of the description - as one Content can have multiple download descriptions associated.
      This will simply be 1, 2, ...
      */
    int id() const;

    Attica::DownloadDescription::Type type() const;
    bool hasPrice() const;
    QString category() const;
    QString name() const;
    QString link() const;
    QString distributionType() const;
    QString priceReason() const;
    QString priceAmount() const;
    uint size() const;
    QString gpgFingerprint() const;
    QString gpgSignature() const;
    QString packageName() const;
    QString repository() const;
    /**
     * Get the list of tags for this download description
     * @since 5.50
     */
    QStringList tags() const;

    void setId(int id);
    void setType(Attica::DownloadDescription::Type type);
    void setHasPrice(bool hasPrice);
    void setCategory(const QString &category);
    void setName(const QString &name);
    void setLink(const QString &link);
    void setDistributionType(const QString &distributionType);
    void setPriceReason(const QString &priceReason);
    void setPriceAmount(const QString &priceAmount);
    void setSize(uint size);
    void setGpgFingerprint(const QString &fingerprint);
    void setGpgSignature(const QString &signature);
    void setPackageName(const QString &packageName);
    void setRepository(const QString &repository);
    /**
     * Set the list of tags for this download description
     * @since 5.50
     */
    void setTags(const QStringList &tags);

    /**
     * The download version as set on the remote. May be QString() when not set.
     * @since 6.5
     */
    [[nodiscard]] QString version() const;

    /**
     * @since 6.5
     */
    void setVersion(const QString &version);

private:
    class Private;
    QSharedDataPointer<Private> d;
};

}

#endif // DOWNLOADDESCRIPTION_H
