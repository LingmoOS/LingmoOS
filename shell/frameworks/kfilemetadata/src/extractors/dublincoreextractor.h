/*
    Helper class to extract XML encoded Dublin Core metadata

    SPDX-FileCopyrightText: 2018 Stefan Brüns <stefan.bruens@rwth-aachen.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/


#ifndef DUBLINCORE_EXTRACTOR_H
#define DUBLINCORE_EXTRACTOR_H

#include <QDomNode>

namespace KFileMetaData
{

class ExtractionResult;

class DublinCoreExtractor
{
public:
    /**
      * Extract DC metadata from an XML fragment
      *
      * Prerequisites:
      * - DC element nodes are immediate children to \p fragment
      * - The \c QDomDocument has been parsed with enabled namespaceProcessing
      *
      * \sa QDomDocument::setContent
      */
    static void extract(ExtractionResult* result, const QDomNode& fragment);

};

} // namespace KFileMetaData

#endif // DUBLINCORE_EXTRACTOR_H

