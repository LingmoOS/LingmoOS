/*
    SPDX-FileCopyrightText: 2016 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef DOWNLOADLINKINFO_H
#define DOWNLOADLINKINFO_H

#include "KNSCore/Entry"

#include <QSharedData>

class DownloadLinkInfoPrivate;
/**
 * @short One downloadable item as contained within one content item
 *
 * A simple data container which wraps a KNSCore::Entry::DownloadLinkInformation
 * instance and provides property accessors for each of the pieces of information stored
 * in it.
 */
class DownloadLinkInfo
{
    Q_GADGET
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString priceAmount READ priceAmount CONSTANT)
    Q_PROPERTY(QString distributionType READ distributionType CONSTANT)
    Q_PROPERTY(QString descriptionLink READ descriptionLink CONSTANT)
    Q_PROPERTY(int id READ id CONSTANT)
    Q_PROPERTY(bool isDownloadtypeLink READ isDownloadtypeLink CONSTANT)
    Q_PROPERTY(quint64 size READ size CONSTANT)
    Q_PROPERTY(QString formattedSize READ formattedSize CONSTANT)
    Q_PROPERTY(QString icon READ icon CONSTANT)

public:
    explicit DownloadLinkInfo(const KNSCore::Entry::DownloadLinkInformation &data);
    DownloadLinkInfo(const DownloadLinkInfo &);
    DownloadLinkInfo &operator=(const DownloadLinkInfo &);
    ~DownloadLinkInfo();

    QString name() const;
    QString priceAmount() const;
    QString distributionType() const;
    QString descriptionLink() const;
    int id() const;
    bool isDownloadtypeLink() const;
    quint64 size() const;
    QString formattedSize() const;
    QString icon() const;

private:
    QSharedDataPointer<DownloadLinkInfoPrivate> d;
};

#endif // DOWNLOADLINKINFO_H
