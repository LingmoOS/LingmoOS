/*
    This file is part of the KFileMetaData project
    SPDX-FileCopyrightText: 2016 Varun Joshi <varunj.1011@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef EXTERNALWRITER_H
#define EXTERNALWRITER_H

#include "writerplugin.h"

namespace KFileMetaData {
class ExternalWriterPrivate;
class ExternalWriter : public WriterPlugin
{
    Q_OBJECT

public:
    explicit ExternalWriter(QObject* parent = nullptr);
    explicit ExternalWriter(const QString& pluginPath);
    ~ExternalWriter() override;

    void write(const WriteData& data) override;
    QStringList writeMimetypes() const override;

private:
    bool runtimeInstalled() const;
    bool dependenciesSatisfied() const;

    std::unique_ptr<ExternalWriterPrivate> d_ptr;
    Q_DECLARE_PRIVATE(ExternalWriter)
};
}

#endif // EXTERNALWRITER_H
