/*
    This file is part of the KFileMetaData project
    SPDX-FileCopyrightText: 2016 Varun Joshi <varunj.1011@gmail.com>
    SPDX-FileCopyrightText: 2016 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef _KFILEMETADTA_WRITERPLUGIN_H
#define _KFILEMETADTA_WRITERPLUGIN_H

#include "kfilemetadata_export.h"
#include <QStringList>

#include "writedata.h"

namespace KFileMetaData
{

/**
 * \class WriterPlugin writerplugin.h <KFileMetaData/WriterPlugin>
 *
 * \brief The WriterPlugin is the base class for all file metadata
 * writers.
 *
 * It is responsible for writing the metadata to a file.
 */
class KFILEMETADATA_EXPORT WriterPlugin : public QObject
{
    Q_OBJECT
public:
    explicit WriterPlugin(QObject* parent);
    ~WriterPlugin() override;

    virtual QStringList writeMimetypes() const = 0;

    virtual void write(const WriteData& data) = 0;
};
}

Q_DECLARE_INTERFACE(KFileMetaData::WriterPlugin, "org.kde.kf5.kfilemetadata.WriterPlugin")
#endif // _KFILEMETADTA_WRITERPLUGIN_H
