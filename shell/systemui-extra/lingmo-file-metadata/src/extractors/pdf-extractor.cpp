/*
    SPDX-FileCopyrightText: 2012 Vishesh Handa <me@vhanda.in>
    SPDX-FileCopyrightText: 2012 Jörg Ehrichs <joerg.ehrichs@gmx.de>
    SPDX-FileCopyrightText: 2022 kirito <baijunjie@kylinos.cn>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/


#include "pdf-extractor.h"
#include "thumbnail-utils.h"
#include <QImage>
#include <QDebug>

using namespace LingmoUIFileMetadata;

static const QString VERSION = "1.0";
static const QString PLUGIN_NAME = "Pdf";

const QStringList supportedMimeTypes = {
    QStringLiteral("application/pdf")
};

PdfExtractor::PdfExtractor(QObject *parent) : ExtractorPlugin(parent)
{

}

void PdfExtractor::extract(ExtractionResult *result)
{
    if (!supportedMimeTypes.contains(result->inputMimetype()) ) {
        return;
    }
    const QString fileUrl = result->inputUrl();
    std::unique_ptr<Poppler::Document> pdfDoc(Poppler::Document::load(fileUrl));

    if (!pdfDoc || pdfDoc->isLocked()) {
        if (result->inputFlags() & ExtractionResult::ExtractThumbnail) {
            if (ThumbnailUtils::needGenerateThumbnail(result, PLUGIN_NAME, VERSION)) {
                ThumbnailUtils::handleFailureOfThumbnail(result->inputUrl(), PLUGIN_NAME, VERSION);
            }
        }
        return;
    }

    result->addType(Type::Document);

    if (result->inputFlags() & ExtractionResult::ExtractMetaData) {
        const QString title = pdfDoc->title();
        if (!title.isEmpty()) {
            result->add(Property::Title, title);
        }

        const QString subject = pdfDoc->subject();
        if (!subject.isEmpty()) {
            result->add(Property::Subject, subject);
        }

        const QString author = pdfDoc->author();
        if (!author.isEmpty()) {
            result->add(Property::Author, author);
        }

        const QString generator = pdfDoc->producer();
        if (!generator.isEmpty()) {
            result->add(Property::Generator, generator);
        }

        const QDateTime creationDate = pdfDoc->creationDate();
        if (!creationDate.isNull()) {
            result->add(Property::CreationDate, creationDate);
        }

        const int numPages = pdfDoc->numPages();
        if (numPages > 0) {
            result->add(Property::PageCount, numPages);
        }
    }

    if (result->inputFlags() & ExtractionResult::ExtractThumbnail) {
        if (ThumbnailUtils::needGenerateThumbnail(result, PLUGIN_NAME, VERSION)) {
            std::unique_ptr<Poppler::Page> coverPage(pdfDoc->page(0));
            double imageSize = 128.0;
            if (result->thumbnailRequest().targetSize().isValid()) {
                int max = qMax(result->thumbnailRequest().targetSize().width(), result->thumbnailRequest().targetSize().height());
                if (max >=1024) {
                    imageSize = 1024.0;
                } else if (max >=512) {
                    imageSize = 512.0;
                } else if (max >=256) {
                    imageSize = 256.0;
                }
            }
            double rate = imageSize / qMax(coverPage->pageSizeF().width(), coverPage->pageSizeF().height());
            QImage thumbnail = coverPage->renderToImage(72.0 * rate, 72.0 * rate);
            ThumbnailUtils::setThumbnail(result, thumbnail, PLUGIN_NAME, VERSION);
        }
    }

    if (!(result->inputFlags() & ExtractionResult::ExtractPlainText)) {
        return;
    }

    for (int i = 0; i < pdfDoc->numPages(); i++) {
        std::unique_ptr<Poppler::Page> page(pdfDoc->page(i));
        if (!page) { // broken pdf files do not return a valid page
            qWarning() << "Could not read page content from" << fileUrl;
            break;
        }
        result->append(page->text(QRectF()));
    }
}

QStringList PdfExtractor::mimetypes() const
{
    return supportedMimeTypes;
}
