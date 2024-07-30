/*
    SPDX-FileCopyrightText: 2012 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef _KFILEMETADATA_EXTRACTORCOLLECTION_H
#define _KFILEMETADATA_EXTRACTORCOLLECTION_H

#include "extractor.h"
#include "kfilemetadata_export.h"

#include <memory>

namespace KFileMetaData
{
class ExtractorCollectionPrivate;
/**
 * \class ExtractorCollection extractorcollection.h <KFileMetaData/ExtractorCollection>
 *
 * \brief The ExtractorCollection is a helper class which internally
 * loads all the extractor plugins.
 *
 * It can be used to fetch a certain
 * subset of these plugins based on a given mimetype.
 *
 * Once the appropriate plugins have been fetched, an ExtractionResult
 * should be created and passed to the plugin's extract function.
 *
 * \author Vishesh Handa <me@vhanda.in>
 */
class KFILEMETADATA_EXPORT ExtractorCollection
{
public:
    explicit ExtractorCollection();
    virtual ~ExtractorCollection();

    /**
     * Fetch the extractors which can be used to extract
     * data for the respective file with the given MIME type.
     *
     * If no match is found then the best matching plugins
     * are returned, determined by MIME type inheritance.
     *
     * \sa QMimeType::allAncestors
     */
    QList<Extractor*> fetchExtractors(const QString& mimetype) const;

private:
    friend class ExtractorCollectionTest;
    // exported for ExtractorCollectionTest
    QList<Extractor*> allExtractors();

    const std::unique_ptr<ExtractorCollectionPrivate> d;
};
}

#endif
