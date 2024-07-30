/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999-2007 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kbuildmimetypefactory_p.h"
#include "ksycoca.h"
#include "ksycocadict_p.h"
#include "ksycocaresourcelist_p.h"

#include <QDebug>
#include <QHash>
#include <QIODevice>
#include <QStandardPaths>
#include <assert.h>

KBuildMimeTypeFactory::KBuildMimeTypeFactory(KSycoca *db)
    : KMimeTypeFactory(db)
{
    // We want all xml files under xdgdata/mime - but not mime/packages/*.xml
    m_resourceList.emplace_back("xdgdata-mime", QStringLiteral("mime"), QStringLiteral("*.xml"));
}

KBuildMimeTypeFactory::~KBuildMimeTypeFactory()
{
}

KSycocaEntry::List KBuildMimeTypeFactory::allEntries() const
{
    assert(sycoca()->isBuilding());
    return m_entryDict->values();
}

KSycocaEntry *KBuildMimeTypeFactory::createEntry(const QString &file) const
{
    // file=text/plain.xml  ->  name=plain.xml dirName=text
    Q_ASSERT(!file.startsWith(QLatin1String("mime/")));

    const int pos = file.lastIndexOf(QLatin1Char('/'));
    if (pos == -1) { // huh?
        return nullptr;
    }
    const auto dirName = QStringView(file).left(pos);
    if (dirName == QLatin1String("packages")) { // special subdir
        return nullptr;
    }

    const int dot = file.lastIndexOf(QLatin1Char('.'));
    if (dot == -1) { // huh?
        return nullptr;
    }
    const QString name = file.left(dot);

    // qDebug() << "Creating MIME type" << name << "from file" << file;

    MimeTypeEntry *e = new MimeTypeEntry(file, name);
    return e;
}

void KBuildMimeTypeFactory::saveHeader(QDataStream &str)
{
    KSycocaFactory::saveHeader(str);
}

void KBuildMimeTypeFactory::save(QDataStream &str)
{
    KSycocaFactory::save(str);

    str << qint32(0);

    const qint64 endOfFactoryData = str.device()->pos();

    // Update header (pass #3)
    saveHeader(str);

    // Seek to end.
    str.device()->seek(endOfFactoryData);
}

KMimeTypeFactory::MimeTypeEntry::Ptr KBuildMimeTypeFactory::createFakeMimeType(const QString &name)
{
    const QString file = name; // hack
    KSycocaEntry::Ptr entry = m_entryDict->value(file);
    if (!entry) {
        MimeTypeEntry *e = new MimeTypeEntry(file, name);
        entry = e;
    }

    Q_ASSERT(entry && entry->isValid());
    addEntry(entry);
    return KMimeTypeFactory::MimeTypeEntry::Ptr(static_cast<MimeTypeEntry *>(entry.data()));
}
