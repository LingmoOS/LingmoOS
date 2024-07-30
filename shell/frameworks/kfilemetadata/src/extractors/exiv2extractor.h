/*
    SPDX-FileCopyrightText: 2012 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/


#ifndef EXIV2EXTRACTOR_H
#define EXIV2EXTRACTOR_H

#include "extractorplugin.h"

namespace KFileMetaData
{

class Exiv2Extractor : public ExtractorPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.kf5.kfilemetadata.ExtractorPlugin"
                      FILE "exiv2extractor.json")
    Q_INTERFACES(KFileMetaData::ExtractorPlugin)

public:
    explicit Exiv2Extractor(QObject* parent = nullptr);

    void extract(ExtractionResult* result) override;
    QStringList mimetypes() const override;
};

} // namespace

#endif // EXIV2EXTRACTOR_H
