/*
    SPDX-FileCopyrightText: 2022 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef PNGEXTRACTOR_H
#define PNGEXTRACTOR_H

#include "extractorplugin.h"

namespace KFileMetaData
{

class PngExtractor : public ExtractorPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.kf5.kfilemetadata.ExtractorPlugin"
                      FILE "pngextractor.json")
    Q_INTERFACES(KFileMetaData::ExtractorPlugin)

public:
    explicit PngExtractor(QObject *parent = nullptr);

    void extract(ExtractionResult *result) override;
    QStringList mimetypes() const override;
};

} // namespace KFileMetaData

#endif // PNGEXTRACTOR_H
