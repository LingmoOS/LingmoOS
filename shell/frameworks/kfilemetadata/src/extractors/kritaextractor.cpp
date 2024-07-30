/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/


#include "kritaextractor.h"
#include "kfilemetadata_debug.h"

#include <KZip>
#include <QXmlStreamReader>

using namespace KFileMetaData;

KritaExtractor::KritaExtractor(QObject* parent)
    : ExtractorPlugin(parent)
{
}

QStringList KritaExtractor::mimetypes() const
{
    return {QStringLiteral("application/x-krita")};
}

void KritaExtractor::extract(ExtractionResult* result)
{
    // Krita files are secretly zip files
    KZip zip(result->inputUrl());
    if (!zip.open(QIODevice::ReadOnly)) {
        return;
    }

    result->addType(Type::Image);

    if (!result->inputFlags().testFlag(ExtractionResult::ExtractMetaData)) {
        return;
    }

    // Read main image information, e.g width and height
    {
        const KArchiveFile *entry = zip.directory()->file(QLatin1String("maindoc.xml"));
        if (!entry) {
            return;
        }

        std::unique_ptr<QIODevice> fileDevice{entry->createDevice()};

        // There is only one element of the maindoc we care about: the IMAGE element.
        // The document width and height are stored as attributes.
        QXmlStreamReader xml{fileDevice.get()};
        while (xml.readNextStartElement()) {
            if (xml.name() == QLatin1String("IMAGE")) {
                bool ok = false;
                const int width = xml.attributes().value(QLatin1String("width")).toInt(&ok);
                if (ok && width != 0) {
                    result->add(Property::Width, width);
                }

                const int height = xml.attributes().value(QLatin1String("height")).toInt(&ok);
                if (ok && height != 0) {
                    result->add(Property::Height, height);
                }

                break;
            }
        }
    }

    // Read extra metadata entered by the author, e.g. title and description
    {
        const KArchiveFile *entry = zip.directory()->file(QLatin1String("documentinfo.xml"));
        if (!entry) {
            return;
        }

        std::unique_ptr<QIODevice> fileDevice{entry->createDevice()};

        // The documentinfo xml schema is very simple, every field in the GUI is exposed
        // as an element of the same name. The one exception is "description" which seems
        // to be stored under <abstract>.
        static const QHash<QStringView, Property::Property> propertyMapping {{
            {QStringLiteral("title"), Property::Title},
            {QStringLiteral("license"), Property::License},
            {QStringLiteral("keyword"), Property::Keywords},
            {QStringLiteral("abstract"), Property::Description},
        }};

        QXmlStreamReader xml{fileDevice.get()};
        if (!xml.readNextStartElement()  || xml.name() != QStringLiteral("document-info")) {
            return;
        }

        while (xml.readNextStartElement()) {
            const QStringView elementName = xml.name();

            if (elementName == QStringLiteral("about")) {
                while (xml.readNextStartElement()) {
                    const QStringView childElementName = xml.name();

                    const Property::Property property = propertyMapping.value(childElementName);
                    if (property != Property::Empty) {
                        const QString value = xml.readElementText();
                        result->add(property, value);
                    } else if (childElementName == QStringLiteral("creation-date")) {
                        const QString value = xml.readElementText();

                            const QDateTime creationDate = QDateTime::fromString(value, Qt::ISODate);
                            if (creationDate.isValid()) {
                                result->add(Property::CreationDate, creationDate);
                            }
                        } else {
                            xml.skipCurrentElement();
                        }
                    }
                } else if (elementName == QStringLiteral("author")) {
                    while (xml.readNextStartElement()) {
                        const QStringView childElementName = xml.name();

                    if (childElementName == QStringLiteral("full-name")) {
                        const QString value = xml.readElementText();
                        if (!value.isEmpty()) {
                            result->add(Property::Author, value);
                        }
                    } else {
                        xml.skipCurrentElement();
                    }
                }
            } else {
                xml.skipCurrentElement();
            }
        }
    }
}

#include "moc_kritaextractor.cpp"
