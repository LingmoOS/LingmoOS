/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "file.h"
#include "global.h"
#include "database.h"
#include "transaction.h"
#include "idutils.h"
#include "propertydata.h"

#include <QJsonDocument>
#include <QFileInfo>
#include <QJsonObject>

using namespace Baloo;

class BALOO_CORE_NO_EXPORT File::Private {
public:
    QString url;
    KFileMetaData::PropertyMultiMap propertyMap;
};

File::File()
    : d(new Private)
{
}

File::File(const File& f)
    : d(new Private(*f.d))
{
}

File::File(const QString& url)
    : d(new Private)
{
    d->url = QFileInfo(url).canonicalFilePath();
}

File::~File() = default;

const File& File::operator=(const File& f)
{
    if (&f != this) {
        *d = *f.d;
    }
    return *this;
}

QString File::path() const
{
    return d->url;
}

KFileMetaData::PropertyMultiMap File::properties() const
{
    return d->propertyMap;
}

QVariant File::property(KFileMetaData::Property::Property property) const
{
    return d->propertyMap.value(property);
}

bool File::load(const QString& url)
{
    d->url = QFileInfo(url).canonicalFilePath();
    d->propertyMap.clear();
    return load();
}

bool File::load()
{
    const QString& url = d->url;
    if (url.isEmpty() || !QFile::exists(url)) {
        return false;
    }

    Database *db = globalDatabaseInstance();
    if (!db->open(Database::ReadOnlyDatabase)) {
        return false;
    }

    quint64 id = filePathToId(QFile::encodeName(d->url));
    if (!id) {
        return false;
    }

    QByteArray arr;
    {
        Transaction tr(db, Transaction::ReadOnly);
        arr = tr.documentData(id);
    }
    // Ignore empty JSON documents, i.e. "" or "{}"
    if (arr.isEmpty() || arr.size() <= 2) {
        return false;
    }

    const QJsonDocument jdoc = QJsonDocument::fromJson(arr);
    d->propertyMap = Baloo::jsonToPropertyMap(jdoc.object());

    return true;
}
