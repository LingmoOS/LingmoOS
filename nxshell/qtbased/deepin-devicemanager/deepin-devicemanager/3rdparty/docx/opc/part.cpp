// SPDX-FileCopyrightText: 2022 lpxxn <mi_duo@live.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "part.h"
#include "constants.h"
#include "../parts/imagepart.h"
#include "../parts/documentpart.h"
#include "opcpackage.h"
#include "rel.h"


namespace Docx {

Part::Part(const PackURI &partName, const QString &contentType, const QByteArray &blob, Package *package)
{
    m_partName = partName;
    m_contentType = contentType;
    m_blob = blob;
    m_package = package;

    m_rels = new Relationships(partName.baseURI());
}

Part* Part::load(const PackURI &partName, const QString &contentType, const QByteArray &blob, Package *package)
{
    return new Part(partName, contentType, blob, package);
}

void Part::loadRel(const QString &reltype, const QString &targetRef, Part *target, const QString rId, bool isternal)
{
    m_rels->addRelationship(reltype, targetRef, target, rId, isternal);
}

PackURI Part::partName() const
{
    return m_partName;
}

QString Part::contentType() const
{
    return m_contentType;
}

QByteArray Part::blob() const
{
    return m_blob;
}

Relationships *Part::rels() const
{
    return m_rels;
}

void Part::afterUnmarshal()
{

}

/*!
 * \brief 把target 添加到Relationships里
 * \param target
 * \param reltype
 * \param isExternal
 * \return
 */
QString Part::relateTo(Part *target, const QString &reltype, const QString &baseUri, bool isExternal, const QString &targetRef)
{
    Relationship *ship;
    if (isExternal)
        ship = m_rels->getOrAddExtPart(reltype, targetRef);
    else
        ship = m_rels->getOrAddPart(reltype, target, baseUri);
    return ship->rId();
}

Part::~Part()
{
    delete m_rels;
}

PartFactory::PartFactory()
{

}

Part *PartFactory::newPart(const PackURI &partname, const QString &contentType, const QString &reltype, const QByteArray &blob, Package *package)
{
    if (reltype == Constants::IMAGE){
        return ImagePart::load(partname, contentType, blob);
    }
    if (contentType == Constants::WML_DOCUMENT_MAIN) {
        return DocumentPart::load(partname, contentType, blob, package);
    }
    //    else if (contentType == Constants::OPC_CORE_PROPERTIES) {
    //        return nullptr;
    //    } else if (contentType == Constants::WML_NUMBERING) {
    //        return nullptr;
    //    } else if (contentType == Constants::WML_STYLES) {
    //        return nullptr;
    //    }

    return Part::load(partname, contentType, blob, package);
}

// end PartFactory


// XmlPart start

XmlPart::XmlPart(const QString &partName, const QString &contentType, const QByteArray &blob, Package *package)
    : Part(partName, contentType, blob, package)
{

}

XmlPart::~XmlPart()
{

}

}
