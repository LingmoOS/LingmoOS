/*
    SPDX-FileCopyrightText: 2012 Vishesh Handa <me@vhanda.in>
    SPDX-FileCopyrightText: 2023 kirito <baijunjie@kylinos.cn>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef LINGMO_FILE_METADATA_TAGLIBEXTRACTOR_H
#define LINGMO_FILE_METADATA_TAGLIBEXTRACTOR_H

#include "extractor-plugin.h"

namespace LingmoUIFileMetadata{
class TaglibExtractor : public ExtractorPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.lingmo.lingmofilemetadata.ExtractorPlugin"
                      FILE "taglib-extractor.json")
    Q_INTERFACES(LingmoUIFileMetadata::ExtractorPlugin)
public:
    explicit TaglibExtractor(QObject* parent = nullptr);

    void extract(ExtractionResult* result) override;
    QStringList mimetypes() const override;

    friend class TaglibExtractorTest;
};
}




#endif //LINGMO_FILE_METADATA_TAGLIBEXTRACTOR_H
