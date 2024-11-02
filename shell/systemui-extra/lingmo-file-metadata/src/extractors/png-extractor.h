/*
    SPDX-FileCopyrightText: 2022 Kai Uwe Broulik <kde@broulik.de>
    SPDX-FileCopyrightText: 2023 Junjie Bai <baijunjie@kylinos.cn>

    SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef LINGMO_FILE_METADATA_PNGEXTRACTOR_H
#define LINGMO_FILE_METADATA_PNGEXTRACTOR_H

#include "extractor-plugin.h"

namespace LingmoUIFileMetadata {

class PngExtractor : public ExtractorPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.lingmo.lingmofilemetadata.ExtractorPlugin"
                      FILE "png-extractor.json")
    Q_INTERFACES(LingmoUIFileMetadata::ExtractorPlugin)

public:
    explicit PngExtractor(QObject *parent = nullptr);
    void extract(ExtractionResult *result) override;
    QStringList mimetypes() const override;

};
} // LingmoUIFileMetadata

#endif //LINGMO_FILE_METADATA_PNGEXTRACTOR_H
