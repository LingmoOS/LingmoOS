/*
    SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/


#include "office2007extractor.h"

#include "dublincoreextractor.h"
#include <memory>

#include <KZip>

#include <QDebug>
#include <QDomDocument>
#include <QXmlStreamReader>

using namespace KFileMetaData;

namespace {
inline QString cpNS()     { return QStringLiteral("http://schemas.openxmlformats.org/package/2006/metadata/core-properties"); }
} // namespace

Office2007Extractor::Office2007Extractor(QObject* parent)
    : ExtractorPlugin(parent)
{

}

const QStringList supportedMimeTypes = {
    QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document"),
    QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.template"),
    QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.presentation"),
    QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.slide"),
    QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.slideshow"),
    QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.template"),
    QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"),
    QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.template"),
};

QStringList Office2007Extractor::mimetypes() const
{
    return supportedMimeTypes;
}

void Office2007Extractor::extract(ExtractionResult* result)
{
    KZip zip(result->inputUrl());
    if (!zip.open(QIODevice::ReadOnly)) {
        qWarning() << "Document is not a valid ZIP archive";
        return;
    }

    const KArchiveDirectory* rootDir = zip.directory();
    if (!rootDir) {
        qWarning() << "Invalid document structure (main directory is missing)";
        return;
    }

    const QStringList rootEntries = rootDir->entries();
    if (!rootEntries.contains(QStringLiteral("docProps"))) {
        qWarning() << "Invalid document structure (docProps is missing)";
        return;
    }

    const KArchiveEntry* docPropEntry = rootDir->entry(QStringLiteral("docProps"));
    if (!docPropEntry->isDirectory()) {
        qWarning() << "Invalid document structure (docProps is not a directory)";
        return;
    }

    const KArchiveDirectory* docPropDirectory = dynamic_cast<const KArchiveDirectory*>(docPropEntry);

    const bool extractMetaData = result->inputFlags() & ExtractionResult::ExtractMetaData;

    const KArchiveFile* file = docPropDirectory->file(QStringLiteral("core.xml"));
    if (extractMetaData && file) {
        QDomDocument coreDoc(QStringLiteral("core"));
        coreDoc.setContent(file->data(), true);

        QDomElement cpElem = coreDoc.documentElement();

        if (!cpElem.isNull() && cpElem.namespaceURI() == cpNS()) {
            DublinCoreExtractor::extract(result, cpElem);
        }

        auto elem = cpElem.firstChildElement(QStringLiteral("keywords"));
        if (!elem.isNull() && elem.namespaceURI() == cpNS()) {
            QString str = elem.text();
            if (!str.isEmpty()) {
                result->add(Property::Keywords, str);
            }
        }
    }

    file = docPropDirectory->file(QStringLiteral("app.xml"));
    if (extractMetaData && file) {
        QDomDocument appDoc(QStringLiteral("app"));
        appDoc.setContent(file->data());

        QDomElement docElem = appDoc.documentElement();

        const QString mimeType = result->inputMimetype();
        if (mimeType == QLatin1String("application/vnd.openxmlformats-officedocument.wordprocessingml.document")) {
            QDomElement elem = docElem.firstChildElement(QStringLiteral("Pages"));
            if (!elem.isNull()) {
                bool ok = false;
                int pageCount = elem.text().toInt(&ok);
                if (ok) {
                    result->add(Property::PageCount, pageCount);
                }
            }

            elem = docElem.firstChildElement(QStringLiteral("Words"));
            if (!elem.isNull()) {
                bool ok = false;
                int wordCount = elem.text().toInt(&ok);
                if (ok) {
                    result->add(Property::WordCount, wordCount);
                }
            }

            elem = docElem.firstChildElement(QStringLiteral("Lines"));
            if (!elem.isNull()) {
                bool ok = false;
                int lineCount = elem.text().toInt(&ok);
                if (ok) {
                    result->add(Property::LineCount, lineCount);
                }
            }
        }

        QDomElement elem = docElem.firstChildElement(QStringLiteral("Application"));
        if (!elem.isNull()) {
            QString app = elem.text();
            if (!app.isEmpty()) {
                result->add(Property::Generator, app);
            }
        }
    }

    //
    // Plain Text
    //
    bool extractPlainText = (result->inputFlags() & ExtractionResult::ExtractPlainText);

    if (rootEntries.contains(QStringLiteral("word"))) {
        result->addType(Type::Document);

        if (!extractPlainText) {
            return;
        }

        const KArchiveEntry* wordEntry = rootDir->entry(QStringLiteral("word"));
        if (!wordEntry->isDirectory()) {
            qWarning() << "Invalid document structure (word is not a directory)";
            return;
        }

        const KArchiveDirectory* wordDirectory = dynamic_cast<const KArchiveDirectory*>(wordEntry);
        const QStringList wordEntries = wordDirectory->entries();

        if (wordEntries.contains(QStringLiteral("document.xml"))) {
            const KArchiveFile* file = wordDirectory->file(QStringLiteral("document.xml"));

            if (file) {
                std::unique_ptr<QIODevice> contentIODevice{file->createDevice()};
                extractTextWithTag(contentIODevice.get(), QStringLiteral("w:t"), result);
            }
        }
    }

    else if (rootEntries.contains(QStringLiteral("xl"))) {
        result->addType(Type::Document);
        result->addType(Type::Spreadsheet);

        if (!extractPlainText) {
            return;
        }

        const KArchiveEntry* xlEntry = rootDir->entry(QStringLiteral("xl"));
        if (!xlEntry->isDirectory()) {
            qWarning() << "Invalid document structure (xl is not a directory)";
            return;
        }

        const auto xlDirectory = dynamic_cast<const KArchiveDirectory*>(xlEntry);
        // TODO: Read the sheets from worksheets/*.xml, and dereference all cells
        // values in order
        const KArchiveFile* file = xlDirectory->file(QStringLiteral("sharedStrings.xml"));
        if (!file) {
            return;
        }
        std::unique_ptr<QIODevice> contentIODevice{file->createDevice()};
        extractTextWithTag(contentIODevice.get(), QStringLiteral("t"), result);
    }

    else if (rootEntries.contains(QStringLiteral("ppt"))) {
        result->addType(Type::Document);
        result->addType(Type::Presentation);

        if (!extractPlainText) {
            return;
        }

        const KArchiveEntry* pptEntry = rootDir->entry(QStringLiteral("ppt"));
        if (!pptEntry->isDirectory()) {
            qWarning() << "Invalid document structure (ppt is not a directory)";
            return;
        }

        const auto pptDirectory = dynamic_cast<const KArchiveDirectory*>(pptEntry);
        const auto slidesEntry = pptDirectory->entry(QStringLiteral("slides"));
        if (!slidesEntry || !slidesEntry->isDirectory()) {
            return;
        }

        const auto slidesDirectory = dynamic_cast<const KArchiveDirectory*>(slidesEntry);
        QStringList entries = slidesDirectory->entries();
        // TODO: Read the actual order from presentation.xml, and follow the
        // references in ppt/_rels/presentation.xml.rel
        std::sort(entries.begin(), entries.end());
        for (const QString & entryName : std::as_const(entries)) {
            const KArchiveFile* file = slidesDirectory->file(entryName);
            if (!file) {
                continue;
            }
            std::unique_ptr<QIODevice> contentIODevice{file->createDevice()};
            extractTextWithTag(contentIODevice.get(), QStringLiteral("a:t"), result);
        }
    }
}

void Office2007Extractor::extractTextWithTag(QIODevice* device, const QString& tag, ExtractionResult* result)
{
    QXmlStreamReader xml(device);

    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.qualifiedName().startsWith(tag) && xml.isStartElement()) {
            QString str = xml.readElementText(QXmlStreamReader::IncludeChildElements);

            if (!str.isEmpty()) {
                result->append(str);
            }
        }

        if (xml.isEndDocument() || xml.hasError()) {
            break;
        }
    }
}

#include "moc_office2007extractor.cpp"
