/*
    SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>
    SPDX-FileCopyrightText: 2016 Christoph Cullmann <cullmann@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/


#include "epubextractor.h"
#include "kfilemetadata_debug.h"

#include <epub.h>

#include <QDateTime>
#include <QRegularExpression>

using namespace KFileMetaData;

EPubExtractor::EPubExtractor(QObject* parent)
    : ExtractorPlugin(parent)
{

}

namespace
{
static const QStringList supportedMimeTypes = {
    QStringLiteral("application/epub+zip"),
};

const QStringList fetchMetadata(struct epub* e, const epub_metadata& type)
{
    int size = 0;
    unsigned char** data = epub_get_metadata(e, type, &size);
    if (data) {
        QStringList strList;
        strList.reserve(size);
        for (int i = 0; i < size; i++) {
            // skip nullptr entries, can happen for broken xml files
            // also skip empty entries
            if (!data[i] || !data[i][0]) {
                continue;
            }

            strList << QString::fromUtf8((char*)data[i]);
            free(data[i]);
        }
        free(data);

        return strList;
    }
    return QStringList();
}
}

QStringList EPubExtractor::mimetypes() const
{
    return supportedMimeTypes;
}

void EPubExtractor::extract(ExtractionResult* result)
{
    // open epub, return on exit, file will be closed again at end of function
    auto ePubDoc = epub_open(result->inputUrl().toUtf8().constData(), 1);
    if (!ePubDoc) {
        qCWarning(KFILEMETADATA_LOG) << "Invalid document";
        return;
    }

    result->addType(Type::Document);

    if (result->inputFlags() & ExtractionResult::ExtractMetaData) {

        for (const QString& value : fetchMetadata(ePubDoc, EPUB_TITLE)) {
            result->add(Property::Title, value);
        }

        for (const QString& value : fetchMetadata(ePubDoc, EPUB_SUBJECT)) {
            result->add(Property::Subject, value);
        }

        for (QString value : fetchMetadata(ePubDoc, EPUB_CREATOR)) {
            // Prefix added by libepub when no opf:role is specified
            if (value.startsWith(QLatin1String("Author: "), Qt::CaseSensitive)) {
                value = value.mid(8).simplified();
            } else {
                // Find 'opf:role' prefix added by libepub
                int index = value.indexOf(QLatin1String(": "), Qt::CaseSensitive);
                if (index > 0) {
                    value = value.mid(index + 2).simplified();
                }
            }

            // Name is provided as "<name>(<file-as>)" when opf:file-as property
            // is specified, "<name>(<name>)" otherwise. Strip the last part
            int index = value.indexOf(QLatin1Char('('));
            if (index > 0) {
                value = value.mid(0, index);
            }

            result->add(Property::Author, value);
        }

        // The Contributor just seems to be mostly Calibre aka the Generator
        /*
    value = fetchMetadata(ePubDoc, EPUB_CONTRIB);
    if( !value.isEmpty() ) {
        SimpleResource con;
        con.addType( NCO::Contact() );
        con.addProperty( NCO::fullname(), value );

        fileRes.addProperty( NCO::contributor(), con );
        graph << con;
    }*/

        for (const QString& value : fetchMetadata(ePubDoc, EPUB_PUBLISHER)) {
            result->add(Property::Publisher, value);
        }

        for (const QString& value : fetchMetadata(ePubDoc, EPUB_DESCRIPTION)) {
            result->add(Property::Description, value);
        }

        for (QString value : fetchMetadata(ePubDoc, EPUB_DATE)) {
            if (value.startsWith(QLatin1String("Unspecified:"), Qt::CaseInsensitive)) {
                value = value.mid(12).simplified();
            } else if (value.startsWith(QLatin1String("publication:"), Qt::CaseInsensitive)) {
                value = value.mid(12).simplified();
            } else {
                continue;
            }
            QDateTime dt = ExtractorPlugin::dateTimeFromString(value);
            if (!dt.isNull()) {
                result->add(Property::CreationDate, dt);
                result->add(Property::ReleaseYear, dt.date().year());
            }
        }
    }

    //
    // Plain Text
    //
    if (result->inputFlags() & ExtractionResult::ExtractPlainText) {
        if (auto iter = epub_get_iterator(ePubDoc, EITERATOR_SPINE, 0)) {
            do {
                char* curr = epub_it_get_curr(iter);
                if (!curr) {
                    continue;
                }

                QString html = QString::fromUtf8(curr);
                html.remove(QRegularExpression(QStringLiteral("<[^>]*>")));
                result->append(html);
            } while (epub_it_get_next(iter));

            epub_free_iterator(iter);
        }

        auto tit = epub_get_titerator(ePubDoc, TITERATOR_NAVMAP, 0);
        if (!tit) {
            tit = epub_get_titerator(ePubDoc, TITERATOR_GUIDE, 0);
        }
        if (tit) {
            if (epub_tit_curr_valid(tit)) {
                do {
                    // get link, iterator handles freeing of it
                    char* clink = epub_tit_get_curr_link(tit);

                    // epub_get_data returns -1 on failure
                    char* data = nullptr;
                    const int size = epub_get_data(ePubDoc, clink, &data);
                    if (size >= 0 && data) {
                        QString html = QString::fromUtf8(data, size);
                        // strip html tags
                        html.remove(QRegularExpression(QStringLiteral("<[^>]*>")));

                        result->append(html);
                        free(data);
                    }
                } while (epub_tit_next(tit));
            }
            epub_free_titerator(tit);
        }
    }

    // close epub file again
    epub_close(ePubDoc);
}

#include "moc_epubextractor.cpp"
