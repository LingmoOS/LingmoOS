/*
    SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/


#ifndef OFFICE_2007_EXTRACTOR_H
#define OFFICE_2007_EXTRACTOR_H

#include "extractorplugin.h"

#include <QIODevice>

class KArchiveDirectory;

namespace KFileMetaData
{

class Office2007Extractor : public ExtractorPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.kf5.kfilemetadata.ExtractorPlugin"
                      FILE "office2007extractor.json")
    Q_INTERFACES(KFileMetaData::ExtractorPlugin)

public:
    explicit Office2007Extractor(QObject* parent = nullptr);

    QStringList mimetypes() const override;
    void extract(ExtractionResult* result) override;

private:
    void extractTextWithTag(QIODevice* device, const QString& tag, ExtractionResult* result);
    void extractAllText(QIODevice* device, ExtractionResult* result);
    void extractTextFromFiles(const KArchiveDirectory* archiveDir, ExtractionResult* result);
};
}

#endif // OFFICE_2007_EXTRACTOR_H
