/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KSYCOCARESOURCELIST_H
#define KSYCOCARESOURCELIST_H

#include <QString>
#include <vector>

struct KSycocaResource {
    // resource is just used in the databaseChanged signal
    // subdir is always under QStandardPaths::GenericDataLocation. E.g. mime, etc.
    KSycocaResource(const QByteArray &resource, const QString &subdir, const QString &filter)
        : resource(resource)
        , subdir(subdir)
        , extension(filter.mid(1))
    {
    }

    const QByteArray resource;
    const QString subdir;
    const QString extension;
};

using KSycocaResourceList = std::vector<KSycocaResource>;

#endif
