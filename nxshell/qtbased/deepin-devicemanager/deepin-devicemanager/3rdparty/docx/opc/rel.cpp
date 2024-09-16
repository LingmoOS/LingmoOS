// SPDX-FileCopyrightText: 2022 lpxxn <mi_duo@live.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "rel.h"
//#include "packuri.h"

#include <QXmlStreamWriter>
#include <QBuffer>

namespace Docx {


Relationship::Relationship(const QString &rId, const QString &reltype, const QString &targetRef, Part *target, const QString &baseURI, bool external)
    : m_rId(rId), m_reltype(reltype), m_targetRef(targetRef), m_target(target), m_baseURI(baseURI), m_isexternal(external)
{

}

QString Relationship::targetRef() const
{
    return m_targetRef;
//    if (m_isexternal)
//        return m_targetRef;
//    else
//        return target()->partName().relativeRef(m_baseURI);
}

Relationship::~Relationship()
{
    delete m_target;
}


Relationships::Relationships(const QString &baseURI)
    : m_baseURI(baseURI)
{

}

Relationship *Relationships::addRelationship(const QString &reltype, const QString &targetRef, Part *target, const QString &rId, bool external)
{
    Relationship *rel = new Relationship(rId, reltype, targetRef, target, m_baseURI, external);
    m_rels[rId] = rel;
    if (!external)
        m_targetPartsByrId[rId] = target;
    return rel;
}

Part *Relationships::partWithReltype(const QString &reltype)
{
    for (Relationship *s : m_rels.values()) {
        if (s->relType() == reltype)
            return s->target();
    }
    return nullptr;
}

int Relationships::count() const
{
    return m_rels.count();
}

QMap<QString, Relationship *> Relationships::rels() const
{
    return m_rels;
}

QByteArray Relationships::blob() const
{
    QByteArray data;
    QBuffer buffer(&data);
    buffer.open(QIODevice::WriteOnly);
    QXmlStreamWriter writer(&buffer);

    writer.writeStartDocument(QStringLiteral("1.0"), true);
    writer.writeStartElement(QStringLiteral("Relationships"));
    writer.writeAttribute(QStringLiteral("xmlns"), QStringLiteral("http://schemas.openxmlformats.org/package/2006/relationships"));

    QMapIterator<QString, Relationship *> iter(m_rels);
    while (iter.hasNext()) {
        iter.next();
        Relationship *rel = iter.value();
        writer.writeStartElement(QStringLiteral("Relationship"));
        writer.writeAttribute(QStringLiteral("Id"), rel->rId());
        writer.writeAttribute(QStringLiteral("Type"), rel->relType());
        PackURI p = rel->target()->partName();

        writer.writeAttribute(QStringLiteral("Target"), rel->targetRef());

        writer.writeEndElement();
    }

    writer.writeEndElement();
    writer.writeEndDocument();
    return data;
}

/*!
 * \brief 添加Part Relactionship
 * \param reltype
 * \param target
 * \return
 */
Relationship *Relationships::getOrAddPart(const QString &reltype, Part *target, const QString &baseUri)
{
    Relationship *rel = getMatching(reltype, target);
    if (rel)
        return rel;
    QString rId = nextrId();
    return addRelationship(reltype, target->partName().relativeRef(baseUri), target, rId);
}

/*!
 * \brief 添加扩展Relactionship
 * \param reltype
 * \param target
 * \return
 */
Relationship *Relationships::getOrAddExtPart(const QString &reltype, const QString &targetref)
{
    Relationship *rel = getMatchingExt(reltype, targetref);
    if (rel)
        return rel;

    QString rId = nextrId();
    return addRelationship(reltype, targetref, nullptr, rId, true);
}

Relationships::~Relationships()
{
    m_targetPartsByrId.clear();
    m_rels.clear();
}

Relationship *Relationships::getMatching(const QString &reltype, Part *target)
{
    for (Relationship *rel : m_rels.values()) {
        if (reltype != rel->relType())
            continue;
        if (rel->target() != target)
            continue;

        return rel;
    }
    return nullptr;
}

Relationship *Relationships::getMatchingExt(const QString &reltype, const QString &targetref)
{
    for (Relationship *rel : m_rels.values()) {
        if (reltype != rel->relType())
            return nullptr;
        if (!rel->isExternal())
            return nullptr;
        if (rel->targetRef() != targetref)
            return nullptr;

        return rel;
    }
    return nullptr;
}

QString Relationships::nextrId()
{
    int size = m_rels.count() + 2;
    QList<QString> keys = m_rels.keys();
    QString rId(QStringLiteral("rId1"));
    for (int i = 1; i < size; i++) {
        rId = QString("rId%1").arg(i);
        if (!keys.contains(rId))
            return rId;
    }
    return "";
}


}

