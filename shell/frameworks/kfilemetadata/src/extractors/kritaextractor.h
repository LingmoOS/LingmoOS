/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/


#ifndef KRITAEXTRACTOR_H
#define KRITAEXTRACTOR_H

#include "extractorplugin.h"

namespace KFileMetaData
{

class KritaExtractor : public ExtractorPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.kf5.kfilemetadata.ExtractorPlugin"
                      FILE "kritaextractor.json")
    Q_INTERFACES(KFileMetaData::ExtractorPlugin)

public:
    explicit KritaExtractor(QObject* parent = nullptr);

    void extract(ExtractionResult* result) override;
    QStringList mimetypes() const override;
};
}

#endif // KRITAEXTRACTOR_H
