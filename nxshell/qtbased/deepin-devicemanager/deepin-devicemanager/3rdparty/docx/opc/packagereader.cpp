// SPDX-FileCopyrightText: 2022 lpxxn <mi_duo@live.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "packagereader.h"
#include "physpkgreader.h"
#include "contenttypemap.h"
#include "serializedrelationships.h"
#include "packuri.h"

#include <QLoggingCategory>


namespace Docx {


PackageReader::PackageReader(SerializedRelationships *srels
                             , ContentTypeMap *contentTypes
                             , const QList<SerializedPart> &sparts)
    : m_srels(srels), m_contentTypes(contentTypes), m_sparts(sparts)
{

}

PackageReader *PackageReader::fromFile(const QString &pkgFile)
{
    PhysPkgReader *physReader = new PhysPkgReader(pkgFile);
    ContentTypeMap *contentypes = ContentTypeMap::fromXml(physReader->contentTypesData());
    SerializedRelationships *rels = PackageReader::srelsFrom(physReader, QStringLiteral("/"));
    QList<SerializedPart> sparts = PackageReader::loadSerializedParts(physReader, rels, contentypes);
    delete physReader;
    physReader = nullptr;
    return new PackageReader(rels, contentypes, sparts);
}

PackageReader *PackageReader::fromFile(QIODevice *device)
{
    PhysPkgReader *physReader = new PhysPkgReader(device);
    ContentTypeMap *contentypes = ContentTypeMap::fromXml(physReader->contentTypesData());
    SerializedRelationships *rels = PackageReader::srelsFrom(physReader, QStringLiteral("/"));
    QList<SerializedPart> sparts = PackageReader::loadSerializedParts(physReader, rels, contentypes);

    return new PackageReader(rels, contentypes, sparts);
}

/*!
 * \brief sourceUri 下的所有relationships
 * \param physReader
 * \param sourceUri
 * \return
 */
SerializedRelationships *PackageReader::srelsFrom(PhysPkgReader *physReader, const QString &sourceUri)
{
    PackURI packUri = PackURI(sourceUri);
    QByteArray rels_xml = physReader->relsFrom(packUri);
    return SerializedRelationships::loadFromData(packUri.baseURI(), rels_xml);
}

/*!
 * \brief read all xml file in all srels
 * \param physReader
 * \param srels
 * \param contentTypes
 * \return
 */
QList<SerializedPart> PackageReader::loadSerializedParts(PhysPkgReader *physReader, const SerializedRelationships *srels
                                                         , const ContentTypeMap *contentTypes)
{
    QList<SerializedPart> sparts;

    QVector<SerializedRelationship> rels = srels->rels();
    for (SerializedRelationship &rel : rels) {
        QString partname = rel.targetPartName();
        QString contenttype = contentTypes->contentType(partname);
        QString reltype = rel.relType();
        SerializedRelationships *partSrels = PackageReader::srelsFrom(physReader, partname);
        QByteArray blob = physReader->blobForm(partname);
        sparts.append(SerializedPart(partname, contenttype, reltype, blob, *partSrels));

        if (partSrels->rels().count() > 0) {
            sparts.append(PackageReader::loadSerializedParts(physReader, partSrels, contentTypes));
        }
    }

    return sparts;
}

QMap<QString, QVector<SerializedRelationship> > PackageReader::partRels() const
{
    QMap<QString, QVector<SerializedRelationship> > partMaps;

    partMaps[QStringLiteral("/")] = m_srels->rels();

    for (const SerializedPart p : m_sparts) {
        SerializedRelationships s = p.rels();
        if (s.count() > 0) {
            partMaps[p.partName()] = s.rels();
        }
    }
    return partMaps;
}

PackageReader::~PackageReader()
{
    if (m_contentTypes) {
        delete m_contentTypes;
        m_contentTypes = nullptr;
    }

    if (m_srels) {
        delete  m_srels;
        m_srels = nullptr;
    }
}

}

