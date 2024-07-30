/*
    This file is part of a KMetaData File Extractor
    SPDX-FileCopyrightText: 2013 Denis Steckelmacher <steckdenis@yahoo.fr>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/


#ifndef OFFICE_EXTRACTOR_H
#define OFFICE_EXTRACTOR_H

#include "extractorplugin.h"

namespace KFileMetaData
{

class OfficeExtractor : public ExtractorPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.kf5.kfilemetadata.ExtractorPlugin"
                      FILE "officeextractor.json")
    Q_INTERFACES(KFileMetaData::ExtractorPlugin)

public:
    explicit OfficeExtractor(QObject* parent = nullptr);

    QStringList mimetypes() const override;
    void extract(ExtractionResult* result) override;

private:
    void findExe(const QString& mimeType, const QString& name, QString& fullPath);
    QString textFromFile(const QString& fileUrl, const QString& command, QStringList& arguments);

private:
    QStringList m_available_mime_types;

    QString m_catdoc;
    QString m_catppt;
    QString m_xls2csv;
};
}

#endif
