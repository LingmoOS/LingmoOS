/*
    This file is part of the KFileMetaData project
    SPDX-FileCopyrightText: 2016 Varun Joshi <varunj.1011@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef EXTERNALEXTRACTOR_H
#define EXTERNALEXTRACTOR_H

#include "extractorplugin.h"

namespace KFileMetaData {
class ExternalExtractorPrivate;
class ExternalExtractor : public ExtractorPlugin
{

    Q_OBJECT

public:
    explicit ExternalExtractor(QObject* parent = nullptr);
    explicit ExternalExtractor(const QString& pluginPath);
    ~ExternalExtractor() override;

    QStringList mimetypes() const override;
    void extract(ExtractionResult* result) override;

private:
    std::unique_ptr<ExternalExtractorPrivate> d_ptr;
    Q_DECLARE_PRIVATE(ExternalExtractor)
};
}

#endif // EXTERNALEXTRACTOR_H
