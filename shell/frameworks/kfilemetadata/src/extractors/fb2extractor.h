/*
    SPDX-FileCopyrightText: 2022 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef FB2EXTRACTOR_H
#define FB2EXTRACTOR_H

#include "extractorplugin.h"

namespace KFileMetaData
{
class Fb2Extractor : public ExtractorPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.kf5.kfilemetadata.ExtractorPlugin" FILE "fb2extractor.json")
    Q_INTERFACES(KFileMetaData::ExtractorPlugin)

public:
    explicit Fb2Extractor(QObject *parent = nullptr);

    void extract(ExtractionResult *result) override;
    QStringList mimetypes() const override;
};

}

#endif // FB2EXTRACTOR_H
