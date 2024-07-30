/*
    SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_FILEMONITOR_H
#define BALOO_FILEMONITOR_H

#include <QObject>
#include <QUrl>

#include "core_export.h"

#include <memory>

namespace Baloo {

/**
 * @class FileMonitor filemonitor.h <Baloo/FileMonitor>
 */
class BALOO_CORE_EXPORT FileMonitor : public QObject
{
    Q_OBJECT
public:
    explicit FileMonitor(QObject* parent = nullptr);
    ~FileMonitor() override;

    void addFile(const QString& fileUrl);
    void addFile(const QUrl& url);

    void setFiles(const QStringList& fileList);

    QStringList files() const;

    void clear();

Q_SIGNALS:
    void fileMetaDataChanged(const QString& fileUrl);

private Q_SLOTS:
    BALOO_CORE_NO_EXPORT void slotFileMetaDataChanged(const QStringList& fileUrl);

private:
    class Private;
    std::unique_ptr<Private> const d;
};

}
#endif // BALOO_FILEMONITOR_H
