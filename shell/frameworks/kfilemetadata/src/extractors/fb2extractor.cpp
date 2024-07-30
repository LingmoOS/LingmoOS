/*
    SPDX-FileCopyrightText: 2022 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "fb2extractor.h"
#include "kfilemetadata_debug.h"

#include <QDateTime>
#include <QFile>
#include <QXmlStreamReader>

#include <KZip>

#include <memory>

using namespace KFileMetaData;

Fb2Extractor::Fb2Extractor(QObject *parent)
    : ExtractorPlugin(parent)
{
}

namespace
{
static const QString regularMimeType()
{
    return QStringLiteral("application/x-fictionbook+xml");
}

static const QString compressedMimeType()
{
    return QStringLiteral("application/x-zip-compressed-fb2");
}

static const QStringList supportedMimeTypes = {regularMimeType(), compressedMimeType()};

}

QStringList Fb2Extractor::mimetypes() const
{
    return supportedMimeTypes;
}

void Fb2Extractor::extract(ExtractionResult *result)
{
    std::unique_ptr<QIODevice> device;
    std::unique_ptr<KZip> zip;

    if (result->inputMimetype() == regularMimeType()) {
        device.reset(new QFile(result->inputUrl()));
        if (!device->open(QIODevice::ReadOnly | QIODevice::Text)) {
            return;
        }

    } else if (result->inputMimetype() == compressedMimeType()) {
        zip.reset(new KZip(result->inputUrl()));
        if (!zip->open(QIODevice::ReadOnly)) {
            return;
        }

        const auto entries = zip->directory()->entries();
        if (entries.count() != 1) {
            return;
        }

        const QString entryPath = entries.first();
        if (!entryPath.endsWith(QLatin1String(".fb2"))) {
            return;
        }

        const auto *entry = zip->directory()->file(entryPath);
        if (!entry) {
            return;
        }

        device.reset(entry->createDevice());
    }

    result->addType(Type::Document);

    QXmlStreamReader xml(device.get());

    bool inFictionBook = false;
    bool inDescription = false;
    bool inTitleInfo = false;
    bool inAuthor = false;
    bool inDocumentInfo = false;
    bool inPublishInfo = false;
    bool inBody = false;

    QString authorFirstName;
    QString authorMiddleName;
    QString authorLastName;
    QString authorNickName;

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();

        if (xml.name() == QLatin1String("FictionBook")) {
            if (xml.isStartElement()) {
                inFictionBook = true;
            } else if (xml.isEndElement()) {
                break;
            }
        } else if (xml.name() == QLatin1String("description")) {
            if (xml.isStartElement()) {
                inDescription = true;
            } else if (xml.isEndElement()) {
                inDescription = false;
            }
        } else if (xml.name() == QLatin1String("title-info")) {
            if (xml.isStartElement()) {
                inTitleInfo = true;
            } else if (xml.isEndElement()) {
                inTitleInfo = false;
            }
        } else if (xml.name() == QLatin1String("document-info")) {
            if (xml.isStartElement()) {
                inDocumentInfo = true;
            } else if (xml.isEndElement()) {
                inDocumentInfo = false;
            }
        } else if (xml.name() == QLatin1String("publish-info")) {
            if (xml.isStartElement()) {
                inPublishInfo = true;
            } else if (xml.isEndElement()) {
                inPublishInfo = false;
            }
        } else if (xml.name() == QLatin1String("body")) {
            if (xml.isStartElement()) {
                inBody = true;
            } else if (xml.isEndElement()) {
                inBody = false;
            }
        }

        if (!inFictionBook) {
            continue;
        }

        if (inDescription && result->inputFlags() & ExtractionResult::ExtractMetaData) {
            if (inTitleInfo) {
                if (xml.isStartElement()) {
                    if (xml.name() == QLatin1String("author")) {
                        inAuthor = true;
                    } else if (inAuthor) {
                        if (xml.name() == QLatin1String("first-name")) {
                            authorFirstName = xml.readElementText();
                        } else if (xml.name() == QLatin1String("middle-name")) {
                            authorMiddleName = xml.readElementText();
                        } else if (xml.name() == QLatin1String("last-name")) {
                            authorLastName = xml.readElementText();
                        } else if (xml.name() == QLatin1String("nickname")) {
                            authorNickName = xml.readElementText();
                        }
                    } else if (xml.name() == QLatin1String("book-title")) {
                        result->add(Property::Title, xml.readElementText());
                    } else if (xml.name() == QLatin1String("annotation")) {
                        result->add(Property::Description, xml.readElementText(QXmlStreamReader::IncludeChildElements).trimmed());
                    } else if (xml.name() == QLatin1String("lang")) {
                        result->add(Property::Language, xml.readElementText());
                    } else if (xml.name() == QLatin1String("genre")) {
                        result->add(Property::Genre, xml.readElementText());
                    }
                } else if (xml.isEndElement()) {
                    inAuthor = false;

                    QStringList nameParts = {authorFirstName, authorMiddleName, authorLastName};
                    nameParts.removeAll(QString());

                    if (!nameParts.isEmpty()) {
                        result->add(Property::Author, nameParts.join(QLatin1Char(' ')));
                    } else if (!authorNickName.isEmpty()) {
                        result->add(Property::Author, authorNickName);
                    }

                    authorFirstName.clear();
                    authorMiddleName.clear();
                    authorLastName.clear();
                    authorNickName.clear();
                }
            } else if (inDocumentInfo) {
                if (xml.name() == QLatin1String("date")) {
                    // Date can be "not exact" but date "value", if present, is an xs:date
                    const auto dateValue = xml.attributes().value(QLatin1String("value"));
                    QDateTime dt = QDateTime::fromString(dateValue.toString());

                    if (!dt.isValid()) {
                        dt = ExtractorPlugin::dateTimeFromString(xml.readElementText());
                    }

                    if (dt.isValid()) {
                        result->add(Property::CreationDate, dt);
                    }
                } else if (xml.name() == QLatin1String("program-used")) {
                    result->add(Property::Generator, xml.readElementText());
                    // "Owner of the fb2 document copyrights"
                } else if (xml.name() == QLatin1String("publisher")) {
                    result->add(Property::Copyright, xml.readElementText());
                }
            } else if (inPublishInfo) {
                if (xml.name() == QLatin1String("publisher")) {
                    result->add(Property::Publisher, xml.readElementText());
                } else if (xml.name() == QLatin1String("year")) {
                    bool ok;
                    const int releaseYear = xml.readElementText().toInt(&ok);
                    if (ok) {
                        result->add(Property::ReleaseYear, releaseYear);
                    }
                }
            }
        } else if (inBody && result->inputFlags() & ExtractionResult::ExtractPlainText && xml.isCharacters() && !xml.isWhitespace()) {
            result->append(xml.text().toString());
        }
    }
}

#include "moc_fb2extractor.cpp"
