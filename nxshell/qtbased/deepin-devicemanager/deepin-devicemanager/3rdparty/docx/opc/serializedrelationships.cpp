// SPDX-FileCopyrightText: 2022 lpxxn <mi_duo@live.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "serializedrelationships.h"
#include "constants.h"

#include <QDomDocument>

namespace Docx {

SerializedRelationship::SerializedRelationship()
{

}

SerializedRelationship::SerializedRelationship(const QString &baseURI, const QString &id, const QString &relType, const QString &target, const QString &targetModel)
    : m_baseURI(baseURI), m_rId(id), m_relType(relType), m_target(target), m_targetMode(targetModel)
{

}

QString SerializedRelationship::baseURI() const
{
    return m_baseURI;
}

QString SerializedRelationship::rId() const
{
    return m_rId;
}

QString SerializedRelationship::relType() const
{
    return m_relType;
}

QString SerializedRelationship::target() const
{
    return m_target;
}

QString SerializedRelationship::targetPartName() const
{
    QString path = m_baseURI + m_target;
    const QString reStr = "../";
    if (path.contains(reStr)) {

        int dotIndex = path.lastIndexOf(reStr);

        int rLen = path.length() - dotIndex - reStr.length();
        path = path.right(rLen);
    }
    return path;
}

bool SerializedRelationship::isExternal() const
{
    return m_targetMode == Constants::EXTERNAL;
}

SerializedRelationship::~SerializedRelationship()
{

}

// End SerializedRelationship

SerializedRelationships::SerializedRelationships()
{

}

SerializedRelationships *SerializedRelationships::loadFromData(const QString baseURI, const QByteArray &relsXml)
{
    SerializedRelationships *rels = new SerializedRelationships();
    if (relsXml.isEmpty())
        return rels;

    QDomDocument dom;
    dom.setContent(relsXml);
    QDomNodeList nodes = dom.elementsByTagName(QStringLiteral("Relationship"));
    for (int i = 0; i < nodes.count(); i++) {
        QDomNode n = nodes.at(i);
        QDomElement ele = n.toElement();

        rels->m_rels.append(SerializedRelationship(baseURI,
                                                   ele.attribute(QStringLiteral("Id")),
                                                   ele.attribute(QStringLiteral("Type")),
                                                   ele.attribute(QStringLiteral("Target"),
                                                   ele.attribute(QStringLiteral("TargetMode"), Constants::INTERNAL))));
    }
    return rels;

}

void SerializedRelationships::addRelationship(const SerializedRelationship &rel)
{
    m_rels.append(rel);
}

void SerializedRelationships::addRelationships(const QVector<SerializedRelationship> &rels)
{
    m_rels += rels;
}

QVector<SerializedRelationship> SerializedRelationships::rels() const
{
    return m_rels;
}

int SerializedRelationships::count() const
{
    return m_rels.count();
}

SerializedRelationships::~SerializedRelationships()
{

}

// End SerializedRelationships


SerializedPart::SerializedPart(const PackURI &partName, const QString &contentType, const QString &reltype, const QByteArray &blob, const SerializedRelationships &srels)
    : m_partName(partName), m_contentType(contentType), m_reltype(reltype), m_blob(blob), m_srels(srels)
{

}

PackURI SerializedPart::partName() const
{
    return m_partName;
}

QString SerializedPart::contentType() const
{
    return m_contentType;
}

QString SerializedPart::relType() const
{
    return m_reltype;
}

QByteArray SerializedPart::blob() const
{
    return m_blob;
}




}
