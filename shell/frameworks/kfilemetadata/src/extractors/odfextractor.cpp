/*
    SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>
    SPDX-FileCopyrightText: 2012 Jörg Ehrichs <joerg.ehrichs@gmx.de>
    SPDX-FileCopyrightText: 2016 Christoph Cullmann <cullmann@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/


#include "odfextractor.h"
#include <memory>

#include <KZip>

#include <QDebug>
#include <QDomDocument>
#include <QFile>
#include <QXmlStreamReader>

namespace {

inline QString dcNS()     { return QStringLiteral("http://purl.org/dc/elements/1.1/"); }
inline QString metaNS()   { return QStringLiteral("urn:oasis:names:tc:opendocument:xmlns:meta:1.0"); }
inline QString officeNS() { return QStringLiteral("urn:oasis:names:tc:opendocument:xmlns:office:1.0"); }
inline QString bodyTag()  { return QStringLiteral("body"); }

QDomElement firstChildElementNS(const QDomNode &node, const QString &nsURI, const QString &localName)
{
    for (auto e = node.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
        if (e.localName() == localName && e.namespaceURI() == nsURI) {
            return e;
        }
    }

    return QDomElement();
}

const QStringList supportedMimeTypes = {
    QStringLiteral("application/vnd.oasis.opendocument.text"),
    QStringLiteral("application/vnd.oasis.opendocument.text-template"),
    QStringLiteral("application/vnd.oasis.opendocument.text-master"),
    QStringLiteral("application/vnd.oasis.opendocument.text-master-template"),
    QStringLiteral("application/vnd.oasis.opendocument.text-flat-xml"),
    QStringLiteral("application/vnd.oasis.opendocument.presentation"),
    QStringLiteral("application/vnd.oasis.opendocument.presentation-template"),
    QStringLiteral("application/vnd.oasis.opendocument.presentation-flat-xml"),
    QStringLiteral("application/vnd.oasis.opendocument.spreadsheet"),
    QStringLiteral("application/vnd.oasis.opendocument.spreadsheet-template"),
    QStringLiteral("application/vnd.oasis.opendocument.spreadsheet-flat-xml"),
    QStringLiteral("application/vnd.oasis.opendocument.graphics"),
    QStringLiteral("application/vnd.oasis.opendocument.graphics-template"),
    QStringLiteral("application/vnd.oasis.opendocument.graphics-flat-xml"),
};

}

using namespace KFileMetaData;

OdfExtractor::OdfExtractor(QObject* parent)
    : ExtractorPlugin(parent)
{

}

QStringList OdfExtractor::mimetypes() const
{
    return supportedMimeTypes;
}

void OdfExtractor::extract(ExtractionResult* result)
{
    if (result->inputMimetype().endsWith(QLatin1String("-flat-xml"))) {
        QFile file(result->inputUrl());
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return;
        }

        result->addType(Type::Document);
        if (result->inputMimetype() == QLatin1String("application/vnd.oasis.opendocument.presentation-flat-xml")) {
            result->addType(Type::Presentation);
        } else if (result->inputMimetype() == QLatin1String("application/vnd.oasis.opendocument.spreadsheet-flat-xml")) {
            result->addType(Type::Spreadsheet);
        } else if (result->inputMimetype() == QLatin1String("application/vnd.oasis.opendocument.graphics-flat-xml")) {
            result->addType(Type::Image);
        }

        if (result->inputFlags() & ExtractionResult::ExtractMetaData) {
            parseMetaData(QStringLiteral("document"), file.readAll(), result);
        }

        if (result->inputFlags() & ExtractionResult::ExtractPlainText) {
            file.seek(0);
            extractPlainText(&file, result);
        }

        return;
    }

    KZip zip(result->inputUrl());
    if (!zip.open(QIODevice::ReadOnly)) {
        qWarning() << "Document is not a valid ZIP archive";
        return;
    }

    const KArchiveDirectory* directory = zip.directory();
    if (!directory) {
        qWarning() << "Invalid document structure (main directory is missing)";
        return;
    }

    // we need a meta xml file in the archive!
    const auto metaXml = directory->file(QStringLiteral("meta.xml"));
    if (!metaXml) {
        qWarning() << "Invalid document structure (meta.xml is missing)";
        return;
    }

    if (result->inputFlags() & ExtractionResult::ExtractMetaData) {
        parseMetaData(QStringLiteral("document-meta"), metaXml->data(), result);
    }

    result->addType(Type::Document);
    if ((result->inputMimetype() == QLatin1String("application/vnd.oasis.opendocument.presentation")) ||
        (result->inputMimetype() == QLatin1String("application/vnd.oasis.opendocument.presentation-template"))) {
        result->addType(Type::Presentation);
    }
    else if ((result->inputMimetype() == QLatin1String("application/vnd.oasis.opendocument.spreadsheet")) ||
             (result->inputMimetype() == QLatin1String("application/vnd.oasis.opendocument.spreadsheet-template"))) {
        result->addType(Type::Spreadsheet);
    }
    else if (result->inputMimetype() == QLatin1String("application/vnd.oasis.opendocument.graphics") ||
             result->inputMimetype() == QLatin1String("application/vnd.oasis.opendocument.graphics-template")) {
        result->addType(Type::Image);
    }

    if (!(result->inputFlags() & ExtractionResult::ExtractPlainText)) {
        return;
    }

    // for content indexing, we need content xml file
    const auto contentXml = directory->file(QStringLiteral("content.xml"));
    if (!contentXml) {
        qWarning() << "Invalid document structure (content.xml is missing)";
        return;
    }

    std::unique_ptr<QIODevice> contentIODevice{contentXml->createDevice()};
    extractPlainText(contentIODevice.get(), result);
}

void OdfExtractor::parseMetaData(const QString &documentElementId, const QByteArray &data, ExtractionResult *result)
{
    QDomDocument metaData(QStringLiteral("metaData"));
    metaData.setContent(data, true);

    // parse metadata ...
    QDomElement meta = firstChildElementNS(firstChildElementNS(metaData,
                                                               officeNS(), documentElementId),
                                           officeNS(), QStringLiteral("meta"));

    QDomNode n = meta.firstChild();
    while (!n.isNull()) {
        QDomElement e = n.toElement();
        if (!e.isNull()) {
            const QString namespaceURI = e.namespaceURI();
            const QString localName = e.localName();

            // Dublin Core
            if (namespaceURI == dcNS()) {
                if (localName == QLatin1String("description")) {
                    result->add(Property::Description, e.text());
                } else if (localName == QLatin1String("subject")) {
                    result->add(Property::Subject, e.text());
                } else if (localName == QLatin1String("title")) {
                    result->add(Property::Title, e.text());
                } else if (localName == QLatin1String("creator")) {
                    result->add(Property::Author, e.text());
                } else if (localName == QLatin1String("language")) {
                    result->add(Property::Language, e.text());
                }
            }
            // Meta Properties
            else if (namespaceURI == metaNS()) {
                if (localName == QLatin1String("document-statistic")) {
                    bool ok = false;
                    int pageCount = e.attributeNS(metaNS(), QStringLiteral("page-count")).toInt(&ok);
                    if (ok) {
                        result->add(Property::PageCount, pageCount);
                    }

                    int wordCount = e.attributeNS(metaNS(), QStringLiteral("word-count")).toInt(&ok);
                    if (ok) {
                        result->add(Property::WordCount, wordCount);
                    }
                } else if (localName == QLatin1String("keyword")) {
                    QString keywords = e.text();
                    result->add(Property::Keywords, keywords);
                } else if (localName == QLatin1String("generator")) {
                    result->add(Property::Generator, e.text());
                } else if (localName == QLatin1String("creation-date")) {
                    QDateTime dt = ExtractorPlugin::dateTimeFromString(e.text());
                    if (!dt.isNull()) {
                        result->add(Property::CreationDate, dt);
                    }
                }
            }
        }
        n = n.nextSibling();
    }
}

void OdfExtractor::extractPlainText(QIODevice *device, ExtractionResult *result)
{
    bool inOfficeBody = false;

    QXmlStreamReader xml(device);
    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isStartElement() && !inOfficeBody && xml.namespaceUri() == officeNS() && xml.name() == bodyTag()) {
            inOfficeBody = true;
        } else if (xml.isEndElement() && inOfficeBody && xml.namespaceUri() == officeNS() && xml.name() == bodyTag()) {
            break;
        }

        if (inOfficeBody && xml.isCharacters() && !xml.isWhitespace()) {
            const QString str = xml.text().toString();
            result->append(str);
        }

        if (xml.hasError() || xml.isEndDocument()) {
            break;
        }
    }
}

#include "moc_odfextractor.cpp"
