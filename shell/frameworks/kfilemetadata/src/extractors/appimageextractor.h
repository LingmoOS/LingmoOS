/*
    SPDX-FileCopyrightText: 2019 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef APPIMAGEEXTRACTOR_H
#define APPIMAGEEXTRACTOR_H

#include "extractorplugin.h"

namespace KFileMetaData
{

class AppImageExtractor : public ExtractorPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.kf5.kfilemetadata.ExtractorPlugin"
                      FILE "appimageextractor.json")
    Q_INTERFACES(KFileMetaData::ExtractorPlugin)

public:
    explicit AppImageExtractor(QObject* parent = nullptr);

public:
    void extract(ExtractionResult* result) override;
    QStringList mimetypes() const override;
};

}

#endif
