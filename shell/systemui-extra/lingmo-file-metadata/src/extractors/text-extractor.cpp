/*
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: baijunjie <baijunjie@kylinos.cn>
 *
 */
#include "text-extractor.h"
#include "thumbnail-utils.h"
#include <QFile>
#include <QDebug>
#include <QTextCodec>
#include <uchardet/uchardet.h>
#include <QPainter>
#include <QApplication>
#include <QTextDocument>
#include <QFontDatabase>

#define MAX_CONTENT_LENGTH 20480000
static const QString PLUGIN_NAME = "Text";
static const QString VERSION = "1.0";

using namespace LingmoUIFileMetadata;

const QStringList supportedMimeTypes = {
    QStringLiteral("text/plain")
};

TextExtractor::TextExtractor(QObject *parent) : ExtractorPlugin(parent)
{

}

void TextExtractor::extract(ExtractionResult *result)
{
    QFile file(result->inputUrl());

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    if (!(result->inputFlags() & ExtractionResult::ExtractPlainText || result->inputFlags() & ExtractionResult::ExtractThumbnail)) {
        file.close();
        return;
    }

    result->addType(Type::Text);
    QString lineContent;
    QString content;
    QByteArray encodedString = file.read(MAX_CONTENT_LENGTH);

    uchardet_t chardet = uchardet_new();
    if (uchardet_handle_data(chardet, encodedString.constData(), encodedString.size()) != 0)
        qWarning() << "Txt file encoding format detect fail!" << result->inputUrl();

    uchardet_data_end(chardet);
    const char *codec = uchardet_get_charset(chardet);

    if (QTextCodec::codecForName(codec) == nullptr)
        qWarning() << "Unsupported Text encoding format" << result->inputUrl() << QString::fromLocal8Bit(codec);

    QTextStream stream(encodedString, QIODevice::ReadOnly);
    stream.setCodec(codec);
    uchardet_delete(chardet);

    int lines = 0;
    lineContent = stream.readLine();
    while (!lineContent.isNull()) {
        result->append(lineContent);
        content.append(lineContent + "\n");
        lineContent = stream.readLine();
        lines++;
    }
    result->add(Property::LineCount, lines);
    file.close();
    encodedString.clear();
    chardet = nullptr;
    stream.flush();

    if (result->inputFlags() & ExtractionResult::ExtractThumbnail) {
        if (ThumbnailUtils::needGenerateThumbnail(result, PLUGIN_NAME, VERSION)) {
            if (qApp == nullptr || qobject_cast<QApplication*>(QApplication::instance()) == nullptr) {
                return;
            }

            QSize pixmapSize(512, 512);
            QImage thumbnail(pixmapSize, QImage::Format_A2BGR30_Premultiplied);
            thumbnail.fill(QColor(245, 245, 245));// light-grey background


            // one pixel for the rectangle, the rest. whitespace
            int xborder = 1 + pixmapSize.width() / 16;  // minimum x-border
            int yborder = 1 + pixmapSize.height() / 16; // minimum y-border

            // this font is supposed to look good at small sizes
            QFont font = QFontDatabase::systemFont(QFontDatabase::SmallestReadableFont);
            font.setPointSize(qMax(7, qMin(11, (pixmapSize.height() - 2 * yborder) / 16)));
            QFontMetrics fm(font);

            // calculate a better border so that the text is centered
            const QSizeF canvasSize(pixmapSize.width()  - 2 * xborder, pixmapSize.height() - 2 * yborder);

            QPainter p(&thumbnail);
            QTextDocument textDocument(content);

            // QTextDocument only supports one margin value for all borders,
            // so we do a page-in-page behind its back, and do our own borders
            textDocument.setDocumentMargin(0);
            textDocument.setPageSize(canvasSize);
            textDocument.setDefaultFont(font);

            QTextOption textOption(Qt::AlignTop | Qt::AlignLeft);
            textOption.setTabStopDistance(8 * p.fontMetrics().horizontalAdvance(QLatin1Char(' ')));
            textOption.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
            textDocument.setDefaultTextOption(textOption);

            // draw page-in-page, with clipping as needed
            p.translate(xborder, yborder);
            textDocument.drawContents(&p, QRectF(QPointF(0, 0), canvasSize));

            p.end();

            ThumbnailUtils::setThumbnail(result, thumbnail, PLUGIN_NAME, VERSION);
        }
    }
}

QStringList TextExtractor::mimetypes() const
{
    return supportedMimeTypes;
}
