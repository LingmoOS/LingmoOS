/*
    SPDX-FileCopyrightText: 2012 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/


#ifndef FFMPEGEXTRACTOR_H
#define FFMPEGEXTRACTOR_H

#include "extractorplugin.h"

namespace KFileMetaData
{

class FFmpegExtractor : public ExtractorPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.kf5.kfilemetadata.ExtractorPlugin"
                      FILE "ffmpegextractor.json")
    Q_INTERFACES(KFileMetaData::ExtractorPlugin)

public:
    explicit FFmpegExtractor(QObject* parent = nullptr);

    void extract(ExtractionResult* result) override;
    QStringList mimetypes() const override;

    friend class ffmpegExtractorTest;
};
}

#endif // FFMPEGEXTRACTOR_H
