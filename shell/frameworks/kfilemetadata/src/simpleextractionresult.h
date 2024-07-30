/*
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KFILEMETADATA_SimpleExtractionResult_H
#define KFILEMETADATA_SimpleExtractionResult_H

#include "extractionresult.h"
#include <QList>
#include <QString>

#include <memory>

namespace KFileMetaData {

class SimpleExtractionResultPrivate;

/**
 * \class SimpleExtractionResult simpleextractionresult.h <KFileMetaData/SimpleExtractionResult>
 *
 * A simple ExtractionResult implementation which stores
 * all the data in memory.
 *
 * This should ideally not be used in production applications as
 * it holds all of the plain text in memory, and that can get quite large
 * when extracting certain documents.
 */
class KFILEMETADATA_EXPORT SimpleExtractionResult : public ExtractionResult
{
public:
    SimpleExtractionResult(const QString& url, const QString& mimetype = QString(), const Flags& flags = Flags{ExtractPlainText | ExtractMetaData});
    SimpleExtractionResult(const SimpleExtractionResult& rhs);
    ~SimpleExtractionResult() override;

    SimpleExtractionResult& operator=(const SimpleExtractionResult& rhs);
    bool operator==(const SimpleExtractionResult& rhs) const;

    void add(Property::Property property, const QVariant& value) override;
    void addType(Type::Type type) override;
    void append(const QString& text) override;

    /**
     * Returns the properties of the extraction result.
     */
    PropertyMultiMap properties() const;

    QString text() const;
    QList<Type::Type> types() const;

private:
    const std::unique_ptr<SimpleExtractionResultPrivate> d;
};

}

#endif // KFILEMETADATA_SimpleExtractionResult_H
