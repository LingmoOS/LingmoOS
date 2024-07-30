/*
    SPDX-FileCopyrightText: 2012 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/


#ifndef PLAINTEXTEXTRACTOR_H
#define PLAINTEXTEXTRACTOR_H

#include "extractorplugin.h"

namespace KFileMetaData
{

class PlainTextExtractor : public ExtractorPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.kf5.kfilemetadata.ExtractorPlugin"
                      FILE "plaintextextractor.json")
    Q_INTERFACES(KFileMetaData::ExtractorPlugin)

public:
    explicit PlainTextExtractor(QObject* parent = nullptr);

    QStringList mimetypes() const override;
    void extract(ExtractionResult* result) override;
};

}

#endif // PLAINTEXTEXTRACTOR_H
