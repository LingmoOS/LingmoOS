/*
    SPDX-FileCopyrightText: 2012 Vishesh Handa <me@vhanda.in>
    SPDX-FileCopyrightText: 2022 iaom <zhangpengfei@kylinos.cn>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/


#ifndef FFMPEGEXTRACTOR_H
#define FFMPEGEXTRACTOR_H

#include "extractor-plugin.h"

namespace LingmoUIFileMetadata
{

class FFmpegExtractor : public ExtractorPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.lingmo.lingmofilemetadata.ExtractorPlugin"
                      FILE "ffmpeg-extractor.json")
    Q_INTERFACES(LingmoUIFileMetadata::ExtractorPlugin)

public:
    explicit FFmpegExtractor(QObject* parent = nullptr);

    void extract(ExtractionResult* result) override;
    QStringList mimetypes() const override;

    friend class ffmpegExtractorTest;
};
}

#endif // FFMPEGEXTRACTOR_H
