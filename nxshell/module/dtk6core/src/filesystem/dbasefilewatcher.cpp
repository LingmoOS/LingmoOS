// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dbasefilewatcher.h"
#include "private/dbasefilewatcher_p.h"

#include <QEvent>
#include <QDebug>

DCORE_BEGIN_NAMESPACE

QList<DBaseFileWatcher*> DBaseFileWatcherPrivate::watcherList;
DBaseFileWatcherPrivate::DBaseFileWatcherPrivate(DBaseFileWatcher *qq)
    : DObjectPrivate(qq)
{

}

/*!
@~english
    @class Dtk::Core::DBaseFileWatcher
    @ingroup dtkcore

    @brief The DBaseFileWatcher class provides an interface for monitoring files and directories for modifications.
*/

DBaseFileWatcher::~DBaseFileWatcher()
{
    stopWatcher();
    DBaseFileWatcherPrivate::watcherList.removeOne(this);
}

QUrl DBaseFileWatcher::fileUrl() const
{
    Q_D(const DBaseFileWatcher);

    return d->url;
}

/*!
@~english
  @brief Let file watcher start watching file changes.
  @sa stopWatcher(), restartWatcher()
 */
bool DBaseFileWatcher::startWatcher()
{
    Q_D(DBaseFileWatcher);

    if (d->started)
        return true;

    if (d->start()) {
        d->started = true;

        return true;
    }

    return false;
}

/*!
@~english
  @brief Stop watching file changes.
  @sa startWatcher(), restartWatcher()
 */
bool DBaseFileWatcher::stopWatcher()
{
    Q_D(DBaseFileWatcher);

    if (!d->started)
        return false;

    if (d->stop()) {
        d->started = false;

        return true;
    }

    return false;
}

/*!
@~english
  @brief Stop file watcher and then restart it to watching file changes.
  @sa startWatcher(), stopWatcher()
 */
bool DBaseFileWatcher::restartWatcher()
{
    bool ok = stopWatcher();
    return ok && startWatcher();
}

/*!
@~english
  @brief Set enable file watcher for \a subfileUrl or not
  @param[in] subfileUrl The given url
  @param[in] enabled Enable file change watching or not.
 */
void DBaseFileWatcher::setEnabledSubfileWatcher(const QUrl &subfileUrl, bool enabled)
{
    Q_UNUSED(subfileUrl)
    Q_UNUSED(enabled)
}

/*!
@~english
  @brief Emit a signal about \a targetUrl got a \a signal with \a arg1
  Example usage:

  @code
  DBaseFileWatcher::ghostSignal(QUrl("bookmark:///"), &DBaseFileWatcher::fileDeleted, QUrl("bookmark:///bookmarkFile1"));
  @endcode

  @return 成功发送返回 true,否则返回 false.
 */
bool DBaseFileWatcher::ghostSignal(const QUrl &targetUrl, DBaseFileWatcher::SignalType1 signal, const QUrl &arg1)
{
    if (!signal)
        return false;

    bool ok = false;

    for (DBaseFileWatcher *watcher : DBaseFileWatcherPrivate::watcherList) {
        if (watcher->fileUrl() == targetUrl) {
            ok = true;
            (watcher->*signal)(arg1);
        }
    }

    return ok;
}

/*!
@~english
  @brief Emit a signal about \a targetUrl got a \a signal with \a arg1 and \a arg2
  Example usage:
  @code
  DBaseFileWatcher::ghostSignal(QUrl("bookmark:///"), &DBaseFileWatcher::fileMoved, QUrl("bookmark:///bookmarkFile1"), QUrl("bookmark:///NewNameFile1"));
  @endcode
 */
bool DBaseFileWatcher::ghostSignal(const QUrl &targetUrl, DBaseFileWatcher::SignalType2 signal, const QUrl &arg1, const QUrl &arg2)
{
    if (!signal)
        return false;

    bool ok = false;

    for (DBaseFileWatcher *watcher : DBaseFileWatcherPrivate::watcherList) {
        if (watcher->fileUrl() == targetUrl) {
            ok = true;
            (watcher->*signal)(arg1, arg2);
        }
    }

    return ok;
}

DBaseFileWatcher::DBaseFileWatcher(DBaseFileWatcherPrivate &dd,
                                           const QUrl &url, QObject *parent)
    : QObject(parent)
    , DObject(dd)
{
    Q_ASSERT(url.isValid());

    d_func()->url = url;
    DBaseFileWatcherPrivate::watcherList << this;
}

DCORE_END_NAMESPACE

//#include "moc_dbasefilewatcher.cpp"
