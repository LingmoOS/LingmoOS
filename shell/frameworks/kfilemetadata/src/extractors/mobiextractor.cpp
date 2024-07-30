/*
    SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>

    Code adapted from kdegraphics-mobipocket/strigi/
    SPDX-FileCopyrightText: 2008 Jakub Stachowski <qbast@go2.pl>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/


#include "mobiextractor.h"

#include <qmobipocket/mobipocket.h>

#include <QFile>
#include <QTextDocument>

using namespace KFileMetaData;

class QFileStream : public Mobipocket::Stream
{
public:
    QFileStream(const QString& name) : d(name) {
        d.open(QIODevice::ReadOnly);
    }
    int read(char* buf, int size) override {
        return d.read(buf, size);
    }
    bool seek(int pos) override {
        return d.seek(pos);
    }
private:
    QFile d;
};

MobiExtractor::MobiExtractor(QObject* parent)
    : ExtractorPlugin(parent)
{

}

static const QStringList supportedMimeTypes =
{
    QStringLiteral("application/x-mobipocket-ebook"),
};

QStringList MobiExtractor::mimetypes() const
{
    return supportedMimeTypes;
}

void MobiExtractor::extract(ExtractionResult* result)
{
    QFileStream stream(result->inputUrl());
    Mobipocket::Document doc(&stream);
    if (!doc.isValid())
        return;

    result->addType(Type::Document);

    if (result->inputFlags() & ExtractionResult::ExtractMetaData) {
        QMapIterator<Mobipocket::Document::MetaKey, QString> it(doc.metadata());
        while (it.hasNext()) {
            it.next();
            switch (it.key()) {
            case Mobipocket::Document::Title:
                result->add(Property::Title, it.value());
                break;
            case Mobipocket::Document::Author: {
                result->add(Property::Author, it.value());
                break;
            }
            case Mobipocket::Document::Description: {
                QTextDocument document;
                document.setHtml(it.value());

                QString plain = document.toPlainText();
                if (!plain.isEmpty())
                    result->add(Property::Description, it.value());
                break;
            }
            case Mobipocket::Document::Subject:
                result->add(Property::Subject, it.value());
                break;
            case Mobipocket::Document::Copyright:
                result->add(Property::Copyright, it.value());
                break;
            }
        }
    }

    if ((result->inputFlags() & ExtractionResult::Flag::ExtractPlainText) && !doc.hasDRM()) {
        QString html = doc.text();

        QTextDocument document;
        document.setHtml(html);

        result->append(document.toPlainText());
    }

}

#include "moc_mobiextractor.cpp"
