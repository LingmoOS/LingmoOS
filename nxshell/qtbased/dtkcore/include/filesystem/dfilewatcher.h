// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DFILEWATCHER_H
#define DFILEWATCHER_H

#include "dbasefilewatcher.h"

DCORE_BEGIN_NAMESPACE

class DFileWatcherPrivate;
class LIBDTKCORESHARED_EXPORT DFileWatcher : public DBaseFileWatcher
{
    Q_OBJECT

public:
    explicit DFileWatcher(const QString &filePath, QObject *parent = 0);

private Q_SLOTS:
    void onFileDeleted(const QString &path, const QString &name);
    void onFileAttributeChanged(const QString &path, const QString &name);
    void onFileMoved(const QString &fromPath, const QString &fromName,
                     const QString &toPath, const QString &toName);
    void onFileCreated(const QString &path, const QString &name);
    void onFileModified(const QString &path, const QString &name);
    void onFileClosed(const QString &path, const QString &name);

private:
    D_DECLARE_PRIVATE(DFileWatcher)
};

DCORE_END_NAMESPACE

#endif // DFILEWATCHER_H
