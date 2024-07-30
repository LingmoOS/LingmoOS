/*
    SPDX-FileCopyrightText: 2018 Stefan Brüns <stefan.bruens@rwth-aachen.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/


#include "xmlextractor.h"
#include "kfilemetadata_debug.h"
#include "dublincoreextractor.h"

#include <QDomDocument>
#include <QFile>
#include <QXmlStreamReader>

#ifdef SVG_XML_COMPRESSED_SUPPORT
#include <KCompressionDevice>
#endif

namespace {

//inline QString dcElementNS()     { return QStringLiteral("http://purl.org/dc/elements/1.1/"); }
inline QString svgNS()    { return QStringLiteral("http://www.w3.org/2000/svg"); }
inline QString rdfNS()    { return QStringLiteral("http://www.w3.org/1999/02/22-rdf-syntax-ns#"); }
inline QString ccNS()     { return QStringLiteral("http://creativecommons.org/ns#"); }

void extractSvgText(KFileMetaData::ExtractionResult* result, const QDomElement &node)
{
    if (node.namespaceURI() != svgNS()) {
        return;
    }

    if ((node.localName() == QLatin1String("g")) ||
        (node.localName() == QLatin1String("a"))) {
        QDomElement e = node.firstChildElement();
        for (; !e.isNull(); e = e.nextSiblingElement()) {
            extractSvgText(result, e);
        }
    } else if (node.localName() == QLatin1String("text")) {
        qCDebug(KFILEMETADATA_LOG) << node.text();
        result->append(node.text());
    }
}

static const QStringList supportedMimeTypes = {
    QStringLiteral("application/xml"),
    QStringLiteral("image/svg+xml"),
    QStringLiteral("image/svg+xml-compressed"),
    QStringLiteral("image/svg"),
};

}

namespace KFileMetaData
{

XmlExtractor::XmlExtractor(QObject* parent)
    : ExtractorPlugin(parent)
{

}

QStringList XmlExtractor::mimetypes() const
{
    return supportedMimeTypes;
}

void XmlExtractor::extract(ExtractionResult* result)
{
    auto flags = result->inputFlags();

    QFile file(result->inputUrl());
    if (!file.open(QIODevice::ReadOnly)) {
        qCWarning(KFILEMETADATA_LOG) << "Document is not a valid file";
        return;
    }


    if ((result->inputMimetype() == QLatin1String("image/svg")) ||
        (result->inputMimetype() == QLatin1String("image/svg+xml-compressed")) ||
        (result->inputMimetype() == QLatin1String("image/svg+xml"))) {

        result->addType(Type::Image);

        QIODevice *ioDevice = &file;
#ifdef SVG_XML_COMPRESSED_SUPPORT
        std::unique_ptr<KCompressionDevice> gzReader;
        if (result->inputMimetype() == QLatin1String("image/svg+xml-compressed")) {
            gzReader.reset(new KCompressionDevice(&file, false, KCompressionDevice::CompressionType::GZip));
            if (!gzReader->open(QIODevice::ReadOnly)) {
                return;
            }
            ioDevice = gzReader.get();
        }
#else
        if (result->inputMimetype() == QLatin1String("image/svg+xml-compressed")) {
            return;
        }
#endif

        QDomDocument doc;
        const bool processNamespaces = true;
        doc.setContent(ioDevice, processNamespaces);
        QDomElement svg = doc.firstChildElement();

        if (!svg.isNull()
            && svg.localName() == QLatin1String("svg")
            && svg.namespaceURI() == svgNS()) {

            QDomElement e = svg.firstChildElement();
            for (; !e.isNull(); e = e.nextSiblingElement()) {
                if (e.namespaceURI() != svgNS()) {
                    continue;
                }

                if (e.localName() == QLatin1String("metadata")) {
                    if (!(flags & ExtractionResult::ExtractMetaData)) {
                        continue;
                    }

                    auto rdf = e.firstChildElement(QLatin1String("RDF"));
                    if (rdf.isNull() || rdf.namespaceURI() != rdfNS()) {
                        continue;
                    }

                    auto cc = rdf.firstChildElement(QLatin1String("Work"));
                    if (cc.isNull() || cc.namespaceURI() != ccNS()) {
                        continue;
                    }

                    DublinCoreExtractor::extract(result, cc);

                } else if (e.localName() == QLatin1String("defs")) {
                    // skip
                    continue;
                } else if (flags & ExtractionResult::ExtractPlainText) {
                    // extract
                    extractSvgText(result, e);
                }
            }
        }
    } else {
        result->addType(Type::Text);

        if (flags & ExtractionResult::ExtractPlainText) {
            QXmlStreamReader stream(&file);
            while (!stream.atEnd()) {
                QXmlStreamReader::TokenType token = stream.readNext();

                if (token == QXmlStreamReader::Characters) {
                    QString text = stream.text().trimmed().toString();
                    if (!text.isEmpty()) {
                        result->append(text);
                    }
                }
            }
        }
    }
}

} // namespace KFileMetaData

#include "moc_xmlextractor.cpp"
