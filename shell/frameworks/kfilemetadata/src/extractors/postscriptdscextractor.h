/*
    SPDX-FileCopyrightText: 2018 Stefan Brüns <stefan.bruens@rwth-aachen.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/


#ifndef DSC_EXTRACTOR_H
#define DSC_EXTRACTOR_H

#include "extractorplugin.h"

namespace KFileMetaData
{

class DscExtractor : public ExtractorPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.kf5.kfilemetadata.ExtractorPlugin"
                      FILE "postscriptdscextractor.json")
    Q_INTERFACES(KFileMetaData::ExtractorPlugin)

public:
    explicit DscExtractor(QObject* parent = nullptr);

    QStringList mimetypes() const override;
    void extract(ExtractionResult* result) override;

private:
};

}

#endif // DSC_EXTRACTOR_H
