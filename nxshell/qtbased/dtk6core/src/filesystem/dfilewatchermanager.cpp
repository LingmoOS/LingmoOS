// SPDX-FileCopyrightText: 2017 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dfilewatchermanager.h"
#include "dfilewatcher.h"
#include "base/private/dobject_p.h"

#include <QMap>
#include <QUrl>

DCORE_BEGIN_NAMESPACE

class DFileWatcherManagerPrivate : public DObjectPrivate
{
public:
    DFileWatcherManagerPrivate(DFileWatcherManager *qq);

    QMap<QString, DFileWatcher *> watchersMap;

    D_DECLARE_PUBLIC(DFileWatcherManager)
};

DFileWatcherManagerPrivate::DFileWatcherManagerPrivate(DFileWatcherManager *qq)
    : DObjectPrivate(qq)
{
}

/*!
@~english
    \class Dtk::Core::DFileWatcherManager
    \inmodule dtkcore
    \brief The DFileWatcherManager class can help you manage file watchers and get signal when file got changed.
*/

DFileWatcherManager::DFileWatcherManager(QObject *parent)
    : QObject(parent)
    , DObject(*new DFileWatcherManagerPrivate(this))
{
}

DFileWatcherManager::~DFileWatcherManager() {}

/*!
@~english
  \brief Add file watcher for \a filePath to the file watcher manager.
  \return The file watcher which got created and added into the file watcher manager.
 */
DFileWatcher *DFileWatcherManager::add(const QString &filePath)
{
    Q_D(DFileWatcherManager);

    DFileWatcher *watcher = d->watchersMap.value(filePath);

    if (watcher) {
        return watcher;
    }

    watcher = new DFileWatcher(filePath, this);

    connect(watcher, &DFileWatcher::fileAttributeChanged, this, [this](const QUrl &url) {
        Q_EMIT fileAttributeChanged(url.toLocalFile());
    });
    connect(watcher, &DFileWatcher::fileClosed, this, [this](const QUrl &url) { Q_EMIT fileClosed(url.toLocalFile()); });
    connect(watcher, &DFileWatcher::fileDeleted, this, [this](const QUrl &url) { Q_EMIT fileDeleted(url.toLocalFile()); });
    connect(watcher, &DFileWatcher::fileModified, this, [this](const QUrl &url) { Q_EMIT fileModified(url.toLocalFile()); });
    connect(watcher, &DFileWatcher::fileMoved, this, [this](const QUrl &fromUrl, const QUrl &toUrl) {
        Q_EMIT fileMoved(fromUrl.toLocalFile(), toUrl.toLocalFile());
    });
    connect(watcher, &DFileWatcher::subfileCreated, this, [this](const QUrl &url) { Q_EMIT subfileCreated(url.toLocalFile()); });

    d->watchersMap[filePath] = watcher;
    watcher->startWatcher();

    return watcher;
}

/*!
@~english
  \brief Remove file watcher for \a filePath from the file watcher manager.
 */
void DFileWatcherManager::remove(const QString &filePath)
{
    Q_D(DFileWatcherManager);

    DFileWatcher *watcher = d->watchersMap.take(filePath);

    if (watcher) {
        watcher->deleteLater();
    }
}

/*!
@~english
  @brief Remove all file watcher
*/
void DFileWatcherManager::removeAll()
{
    Q_D(DFileWatcherManager);
    for (auto it : d->watchersMap) {
        it->deleteLater();
    }
    d->watchersMap.clear();
}

/*!
@~english
  @brief Show all file watcher
*/
QStringList DFileWatcherManager::watchedFiles() const
{
    Q_D(const DFileWatcherManager);
    return d->watchersMap.keys();
}

DCORE_END_NAMESPACE
