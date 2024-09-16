// SPDX-FileCopyrightText: 2022 lpxxn <mi_duo@live.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "contenttypemap.h"
#include "constants.h"

#include <QDomDocument>
#include <QMultiMap>
#include <QBuffer>
#include <QXmlStreamWriter>

namespace Docx {

ContentTypeMap::ContentTypeMap()
{

}

ContentTypeMap *ContentTypeMap::fromXml(const QByteArray &contentTypesXml)
{
    QDomDocument doc;
    doc.setContent(contentTypesXml);

    QDomNodeList defaults =  doc.elementsByTagName("Default");
    QDomNodeList overrides =  doc.elementsByTagName("Override");

    ContentTypeMap *types = new ContentTypeMap();
    for (int i = 0, sz = defaults.count(); i < sz; i++) {
        QDomNode n = defaults.at(i);
        QString ext = n.toElement().attribute("Extension");
        QString cty = n.toElement().attribute("ContentType");
        types->addDefault(ext, cty);
    }

    for (int i = 0; i < overrides.count(); i++) {
        QDomNode n = overrides.at(i);
        QString ext = n.toElement().attribute("PartName");
        QString cty = n.toElement().attribute("ContentType");
        types->addOverride(ext, cty);
    }

    return types;
}

void ContentTypeMap::addDefault(const QString &extension, const QString &contentType)
{
    m_defaults.insert(extension, contentType);
}

void ContentTypeMap::addOverride(const QString &partName, const QString &contentType)
{
    m_overrides.insert(partName, contentType);
}

void ContentTypeMap::addContentType(const PackURI &name, const QString &contentType)
{    
    QMultiMap<QString, QString> default_content_types;
    default_content_types.insert("bin",     Constants::PML_PRINTER_SETTINGS);
    default_content_types.insert("bin",     Constants::SML_PRINTER_SETTINGS);
    default_content_types.insert("bin",     Constants::WML_PRINTER_SETTINGS);
    default_content_types.insert("bmp",     Constants::BMP);
    default_content_types.insert("emf",     Constants::X_EMF);
    default_content_types.insert("fntdata", Constants::X_FONTDATA);
    default_content_types.insert("gif",     Constants::GIF);
    default_content_types.insert("jpe",     Constants::JPEG);
    default_content_types.insert("jpeg",    Constants::JPEG);
    default_content_types.insert("jpg",     Constants::JPEG);
    default_content_types.insert("png",     Constants::PNG);
    default_content_types.insert("rels",    Constants::OPC_RELATIONSHIPS);
    default_content_types.insert("tif",     Constants::TIFF);
    default_content_types.insert("tiff",    Constants::TIFF);
    default_content_types.insert("wdp",     Constants::MS_PHOTO);
    default_content_types.insert("wmf",     Constants::X_WMF);
    default_content_types.insert("xlsx",    Constants::SML_SHEET);
    default_content_types.insert("xml",     Constants::XML);

    m_defaults.insert(QStringLiteral("rels"), QStringLiteral("application/vnd.openxmlformats-package.relationships+xml"));
    m_defaults.insert(QStringLiteral("xml"), QStringLiteral("application/xml"));

    if (default_content_types.contains(name.ext(), contentType))
        m_defaults.insert(name.ext(), contentType);
    else
        m_overrides.insert(name, contentType);

}

QString ContentTypeMap::contentType(const PackURI &partname) const
{
    if (m_overrides.contains(partname.fullURI()))
        return m_overrides.value(partname.fullURI());
    if (m_defaults.contains(partname.ext()))
        return m_defaults.value(partname.ext());
    return QStringLiteral("");
}

QByteArray ContentTypeMap::blob()
{
    QByteArray data;
    QBuffer buffer(&data);
    buffer.open(QIODevice::WriteOnly);

    QXmlStreamWriter writer(&buffer);

    writer.writeStartDocument(QStringLiteral("1.0"), true);
    writer.writeStartElement(QStringLiteral("Types"));
    writer.writeAttribute(QStringLiteral("xmlns"), QStringLiteral("http://schemas.openxmlformats.org/package/2006/content-types"));

    // add Default Element
    auto addDefault = [&writer](const QString &exten, const QString &type){
        writer.writeStartElement(QStringLiteral("Default"));
        writer.writeAttribute(QStringLiteral("Extension"), exten);
        writer.writeAttribute(QStringLiteral("ContentType"), type);
        writer.writeEndElement(); // end Defalut
    };

//    addDefault(QStringLiteral("rels"), QStringLiteral("application/vnd.openxmlformats-package.relationships+xml"));
//    addDefault(QStringLiteral("xml"), QStringLiteral("application/xml"));

    QMapIterator<QString, QString> defIter(m_defaults);
    while (defIter.hasNext()) {
        defIter.next();
        addDefault(defIter.key(), defIter.value());
    }

    // add Override Element
    QMapIterator<QString, QString> overIter(m_overrides);
    while (overIter.hasNext()) {
        overIter.next();
        writer.writeStartElement(QStringLiteral("Override"));
        writer.writeAttribute(QStringLiteral("PartName"), QStringLiteral("/") + overIter.key());
        writer.writeAttribute(QStringLiteral("ContentType"), overIter.value());
        writer.writeEndElement(); // end Defalut
    }

    writer.writeEndElement(); // end Types
    writer.writeEndDocument();
    return data;
}

ContentTypeMap::~ContentTypeMap()
{

}

}
