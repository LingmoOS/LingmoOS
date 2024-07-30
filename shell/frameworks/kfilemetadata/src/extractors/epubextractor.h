/*
    SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/


#ifndef EPUBEXTRACTOR_H
#define EPUBEXTRACTOR_H

#include "extractorplugin.h"

namespace KFileMetaData
{

class EPubExtractor : public ExtractorPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.kf5.kfilemetadata.ExtractorPlugin"
                      FILE "epubextractor.json")
    Q_INTERFACES(KFileMetaData::ExtractorPlugin)

public:
    explicit EPubExtractor(QObject* parent = nullptr);

    void extract(ExtractionResult* result) override;
    QStringList mimetypes() const override;
};
}

#endif // EPUBEXTRACTOR_H
