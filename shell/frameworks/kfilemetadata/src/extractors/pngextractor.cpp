/*
    SPDX-FileCopyrightText: 2022 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "pngextractor.h"
#include "propertyinfo.h"

#include <QImageReader>

using namespace KFileMetaData;

// Keywords specified in https://www.w3.org/TR/PNG/#11keywords
static const struct {
    QString key;
    Property::Property property;
} s_textMapping[] = {
    // Short (one line) title or caption for image
    {QStringLiteral("Title"), Property::Title},
    // Name of image's creator
    {QStringLiteral("Author"), Property::Author},
    // Description of image (possibly long)
    // Unfortunately, QImage puts all text keys with spaces, such as
    // "Raw profile type exif", into the "Description" key,
    // (cf. qt_getImageTextFromDescription), overriding the actual
    // PNG description, and making it useless.
    //{QStringLiteral("Description"), Property::Description},
    // Copyright notice
    {QStringLiteral("Copyright"), Property::Copyright},
    // Time of original image creation
    {QStringLiteral("Creation Time"), Property::CreationDate},
    // Software used to create the image
    {QStringLiteral("Software"), Property::Generator},
    // Disclaimer - Legal disclaimer
    // Warning - Warning of nature of content
    // Source - Device used to create the image
    // Miscellaneous comment
    {QStringLiteral("Comment"), Property::Comment},
};

PngExtractor::PngExtractor(QObject* parent)
    : ExtractorPlugin(parent)
{
}

QStringList PngExtractor::mimetypes() const
{
    return {
        QStringLiteral("image/png")
    };
}

void PngExtractor::extract(ExtractionResult* result)
{
    QImageReader reader(result->inputUrl(), "png");
    if (!reader.canRead()) {
        return;
    }

    result->addType(Type::Image);

    if (!result->inputFlags().testFlag(ExtractionResult::ExtractMetaData)) {
        return;
    }

    for (const auto &mapping : s_textMapping) {
        QString text = reader.text(mapping.key);
        if (text.isEmpty()) {
            // Spec says, keywords are case-sensitive but of course the real world looks different.
            text = reader.text(mapping.key.toLower());
        }
        if (text.isEmpty()) {
            continue;
        }

        const auto propertyInfo = PropertyInfo(mapping.property);

        if (propertyInfo.valueType() == QMetaType::QDateTime) {
            // "For the Creation Time keyword, the date format defined in section 5.2.14 of RFC 1123 is suggested"
            // which in turn references RFC822...
            const QDateTime dt = QDateTime::fromString(text, Qt::RFC2822Date);

            if (!dt.isValid()) {
                continue;
            }

            result->add(mapping.property, dt);
            continue;
        }

        result->add(mapping.property, text);
    }
}

#include "moc_pngextractor.cpp"
