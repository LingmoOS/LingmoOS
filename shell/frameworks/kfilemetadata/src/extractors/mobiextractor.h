/*
    SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/


#ifndef MOBIEXTRACTOR_H
#define MOBIEXTRACTOR_H

#include "extractorplugin.h"

namespace KFileMetaData
{

class MobiExtractor : public ExtractorPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.kf5.kfilemetadata.ExtractorPlugin"
                      FILE "mobiextractor.json")
    Q_INTERFACES(KFileMetaData::ExtractorPlugin)

public:
    explicit MobiExtractor(QObject* parent = nullptr);

    void extract(ExtractionResult* result) override;
    QStringList mimetypes() const override;
};
}

#endif // MOBIEXTRACTOR_H
