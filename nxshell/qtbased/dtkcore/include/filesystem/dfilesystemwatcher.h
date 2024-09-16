// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DFILESYSTEMWATCHER_H
#define DFILESYSTEMWATCHER_H

#include "dtkcore_global.h"
#include "dobject.h"

#include <QObject>

DCORE_BEGIN_NAMESPACE

class DFileSystemWatcherPrivate;
class LIBDTKCORESHARED_EXPORT DFileSystemWatcher : public QObject, public DObject
{
    Q_OBJECT
    D_DECLARE_PRIVATE(DFileSystemWatcher)

public:
    DFileSystemWatcher(QObject *parent = Q_NULLPTR);
    DFileSystemWatcher(const QStringList &paths, QObject *parent = Q_NULLPTR);
    ~DFileSystemWatcher();

    bool addPath(const QString &file);
    QStringList addPaths(const QStringList &files);
    bool removePath(const QString &file);
    QStringList removePaths(const QStringList &files);

    QStringList files() const;
    QStringList directories() const;

Q_SIGNALS:
    void fileDeleted(const QString &path, const QString &name, QPrivateSignal);
    void fileAttributeChanged(const QString &path, const QString &name, QPrivateSignal);
    void fileClosed(const QString &path, const QString &name, QPrivateSignal);
    void fileMoved(const QString &fromPath, const QString &fromName,
                   const QString &toPath, const QString &toName, QPrivateSignal);
    void fileCreated(const QString &path, const QString &name, QPrivateSignal);
    void fileModified(const QString &path, const QString &name, QPrivateSignal);

private:
    Q_PRIVATE_SLOT(d_func(), void _q_readFromInotify())
};

DCORE_END_NAMESPACE

#endif // DFILESYSTEMWATCHER_H
