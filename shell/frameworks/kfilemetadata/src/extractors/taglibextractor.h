/*
    SPDX-FileCopyrightText: 2012 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/


#ifndef TAGLIBEXTRACTOR_H
#define TAGLIBEXTRACTOR_H

#include "extractorplugin.h"

namespace KFileMetaData
{

class TagLibExtractor : public ExtractorPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.kf5.kfilemetadata.ExtractorPlugin"
                      FILE "taglibextractor.json")
    Q_INTERFACES(KFileMetaData::ExtractorPlugin)

public:
    explicit TagLibExtractor(QObject* parent = nullptr);

    void extract(ExtractionResult* result) override;
    QStringList mimetypes() const override;

};

}

#endif // TAGLIBEXTRACTOR_H
