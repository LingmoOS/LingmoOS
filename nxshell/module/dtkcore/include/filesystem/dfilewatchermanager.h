// SPDX-FileCopyrightText: 2017 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DFILEWATCHERMANAGER_H
#define DFILEWATCHERMANAGER_H

#include "dtkcore_global.h"
#include "dobject.h"

#include <QObject>
#include <QString>

DCORE_BEGIN_NAMESPACE

class DFileWatcher;

class DFileWatcherManagerPrivate;
class LIBDTKCORESHARED_EXPORT DFileWatcherManager : public QObject, public DObject
{
    Q_OBJECT

public:
    explicit DFileWatcherManager(QObject *parent = 0);
    ~DFileWatcherManager();

    DFileWatcher *add(const QString &filePath);
    void remove(const QString &filePath);
    void removeAll();
    QStringList watchedFiles() const;
Q_SIGNALS:
    void fileDeleted(const QString &filePath);
    void fileAttributeChanged(const QString &filePath);
    void fileMoved(const QString &fromFilePath, const QString &toFilePath);
    void subfileCreated(const QString &filePath);
    void fileModified(const QString &filePath);
    void fileClosed(const QString &filePath);

private:
    QScopedPointer<DFileWatcherManagerPrivate> d_ptr;

    D_DECLARE_PRIVATE(DFileWatcherManager)
    Q_DISABLE_COPY(DFileWatcherManager)
};

DCORE_END_NAMESPACE

#endif // DFILEWATCHERMANAGER_H
