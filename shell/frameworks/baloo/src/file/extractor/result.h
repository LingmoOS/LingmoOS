/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef EXTRACTIONRESULT_H
#define EXTRACTIONRESULT_H

#include <KFileMetaData/ExtractionResult>

#include "document.h"
#include "termgenerator.h"

/**
 * \class Result result.h
 *
 * \brief The result class is where all the data extracted by
 * the KFileMetaData extractors is saved to.
 * It implements the KFileMetaData::ExtractionResult interface.
 * The results can be retrieved as a Baloo::Document using
 * \c document() and stored in the database.
 *
 */
class Result : public KFileMetaData::ExtractionResult
{
public:
    Result();
    Result(const QString& url, const QString& mimetype, const Flags& flags = ExtractMetaData | ExtractPlainText);

    void add(KFileMetaData::Property::Property property, const QVariant& value) override;
    void append(const QString& text) override;
    void addType(KFileMetaData::Type::Type type) override;

    /**
     * Can be used to add extraction results to an existing
     * Baloo::Document. Has to be called before passing the
     * Result to KFileMetaData::Extractor::extract().
     * \param doc The Baloo::Document
     */
    void setDocument(const Baloo::Document& doc);

    /**
     * Returns the Baloo document to which the results from the extractors
     * are saved.
     */
    Baloo::Document& document() {
        return m_doc;
    }

    /**
     * Applies the finishing touches on the document, and makes
     * it ready to be pushed into the db.
     */
    void finish();

private:
    /**
     * The document that represents the file that is to be indexed.
     */
    Baloo::Document m_doc;
    /**
     * Contains the position state of the metadata and adds it to the document.
     */
    Baloo::TermGenerator m_termGen;
    /**
     * Contains the position state of plain text and adds it to the document.
     */
    Baloo::TermGenerator m_termGenForText;
    /**
     * Contains all indexed property data from the extractors.
     */
    KFileMetaData::PropertyMultiMap m_map;
};

#endif // EXTRACTIONRESULT_H
