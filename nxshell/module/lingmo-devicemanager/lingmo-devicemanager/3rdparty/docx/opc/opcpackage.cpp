// SPDX-FileCopyrightText: 2022 lpxxn <mi_duo@live.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "opcpackage.h"
#include "packagereader.h"
#include "part.h"
#include "../package.h"
#include "rel.h"
#include "constants.h"
#include "../parts/documentpart.h"

#include <QList>

using namespace Docx;

OpcPackage::OpcPackage()
    : m_rels(nullptr)
{

}

DocumentPart *OpcPackage::mainDocument()
{
    Part *part = partByRelated(Constants::OFFICE_DOCUMENT);
    DocumentPart *mainPart = dynamic_cast<DocumentPart *>(part);
    return mainPart;
}

Part *OpcPackage::partByRelated(const QString &reltype)
{
    return m_rels->partWithReltype(reltype);
}

QList<Part *> OpcPackage::parts() const
{
    QList<Part *> theParts;
    partsbyRels(m_rels, &theParts);
    return theParts;
}


OpcPackage::~OpcPackage()
{
    delete m_rels;
}

void OpcPackage::partsbyRels(const Relationships *rels, QList<Part *> *parts) const
{
    QList<Relationship *> relsCol = rels->rels().values();
    for (const Relationship *rel : relsCol) {
        Part *p = rel->target();
        parts->append(p);
        Relationships *pRels = p->rels();
        if (pRels->count() > 0) {
            partsbyRels(pRels, parts);
        }
    }
}

Unmarshaller::Unmarshaller()
{

}
/*!
 * \brief 分解PackageReader里的内容
 * \param pkgReader
 * \param package
 */
void Unmarshaller::unmarshal(PackageReader *pkgReader, Package *package)
{
    QMap<QString, Part *> parts;
    parts = Unmarshaller::unmarshalParts(pkgReader, package);
    Unmarshaller::unmarshalRelationships(pkgReader, package, parts);

    for (Part *p : parts.values()) {
        p->afterUnmarshal();
    }
    package->afterUnmarshal();
}

/*!
 * \brief 分解各部分xml文件找出
 * \param pkgReader
 * \param package
 * \return
 */
QMap<QString, Part *> Unmarshaller::unmarshalParts(PackageReader *pkgReader, Package *package)
{
    QMap<QString, Part *> parts;
    QList<SerializedPart> sparts = pkgReader->sparts();
    for (const SerializedPart &p : sparts) {
        parts[p.partName()] = PartFactory::newPart(p.partName(), p.contentType(), p.relType(), p.blob(), package);
    }
    return parts;
}

/*!
 * \brief 分解relationships
 * \param pkgReader
 * \param package
 * \param parts
 */
void Unmarshaller::unmarshalRelationships(PackageReader *pkgReader, Package *package, const QMap<QString, Part *> &parts)
{
    QMap<QString, QVector<SerializedRelationship> > partRel = pkgReader->partRels();

    for (const QString &key : partRel.keys()) {
        QVector<SerializedRelationship> rels = partRel[key];
        for (const SerializedRelationship &r : rels) {
            Part *target = parts[r.targetPartName()];
            if (key.isEmpty() || key == QStringLiteral("/")) {
                package->loadRel(r.relType(), r.target(), target, r.rId(), r.isExternal());
            } else {
                Part *part = parts[key];
                part->loadRel(r.relType(), r.target(), target, r.rId(), r.isExternal());
            }
        }
    }
}

Unmarshaller::~Unmarshaller()
{

}
