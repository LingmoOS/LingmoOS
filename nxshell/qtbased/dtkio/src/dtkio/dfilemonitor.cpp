// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dfilemonitor_p.h"
#include "dfilemonitor.h"

#include "dfilefuture.h"

DIO_BEGIN_NAMESPACE
using DTK_CORE_NAMESPACE::DError;
using DTK_CORE_NAMESPACE::DExpected;
using DTK_CORE_NAMESPACE::DUnexpected;

DFileMonitorPrivate::DFileMonitorPrivate(DFileMonitor *monitor)
    : q(monitor)
{
}

DFileMonitorPrivate::~DFileMonitorPrivate()
{
}

GFileMonitor *DFileMonitorPrivate::createMonitor(GFile *gfile, WatchType type)
{
    if (!gfile) {
        setError(IOErrorCode::FileNotFound);
        return nullptr;
    }

    g_autoptr(GCancellable) cancellable = g_cancellable_new();
    g_autoptr(GError) gerror = nullptr;
    GFileMonitorFlags monitorFlags = GFileMonitorFlags(G_FILE_MONITOR_WATCH_MOUNTS | G_FILE_MONITOR_WATCH_MOVES | G_FILE_MONITOR_WATCH_HARD_LINKS);
    switch (type) {
    case WatchType::Auto:
        gmonitor = g_file_monitor(gfile, monitorFlags, cancellable, &gerror);
        break;
    case WatchType::File:
        gmonitor = g_file_monitor_file(gfile, monitorFlags, cancellable, &gerror);
        break;
    case WatchType::Dir:
        gmonitor = g_file_monitor_directory(gfile, monitorFlags, cancellable, &gerror);
        break;
    }

    if (!gmonitor)
        setError(IOErrorCode(gerror->code));

    return gmonitor;
}

void DFileMonitorPrivate::setError(IOErrorCode code)
{
    error.setErrorCode(code);
    error.setErrorMessage(IOErrorMessage(code));
}

void DFileMonitorPrivate::watchCallback(GFileMonitor *monitor, GFile *child, GFile *other, GFileMonitorEvent eventType, gpointer userData)
{
    Q_UNUSED(monitor);

    DFileMonitor *watcher = static_cast<DFileMonitor *>(userData);
    if (!watcher) {
        return;
    }

    QUrl childUrl;
    QUrl otherUrl;

    g_autofree gchar *childStr = g_file_get_path(child);
    if (childStr != nullptr && *childStr != '/') {
        childUrl = QUrl::fromLocalFile(childStr);
    } else {
        g_autofree gchar *uri = g_file_get_uri(child);
        childUrl = QUrl::fromUserInput(uri);
    }

    if (other) {
        g_autofree gchar *otherStr = g_file_get_path(other);
        if (otherStr != nullptr && *otherStr != '/') {
            otherUrl = QUrl::fromLocalFile(otherStr);
        } else {
            g_autofree gchar *uri = g_file_get_uri(other);
            otherUrl = QUrl::fromUserInput(uri);
        }
    }

    if (childUrl.path().startsWith("//"))
        childUrl.setPath(childUrl.path().mid(1));
    if (otherUrl.path().startsWith("//"))
        otherUrl.setPath(otherUrl.path().mid(1));

    switch (eventType) {
    case G_FILE_MONITOR_EVENT_CHANGED:
        watcher->fileChanged(childUrl);
        break;
    case G_FILE_MONITOR_EVENT_CHANGES_DONE_HINT:
        break;
    case G_FILE_MONITOR_EVENT_DELETED:
        watcher->fileDeleted(childUrl);
        break;
    case G_FILE_MONITOR_EVENT_CREATED:
        watcher->fileAdded(childUrl);
        break;
    case G_FILE_MONITOR_EVENT_ATTRIBUTE_CHANGED:
        watcher->fileChanged(childUrl);
        break;
    case G_FILE_MONITOR_EVENT_PRE_UNMOUNT:
        break;
    case G_FILE_MONITOR_EVENT_UNMOUNTED:
        watcher->fileDeleted(childUrl);
        break;
    case G_FILE_MONITOR_EVENT_MOVED_IN:
        watcher->fileAdded(childUrl);
        break;
    case G_FILE_MONITOR_EVENT_MOVED_OUT:
        watcher->fileDeleted(childUrl);
        break;
    case G_FILE_MONITOR_EVENT_RENAMED:
        watcher->fileRenamed(childUrl, otherUrl);
        break;

    //case G_FILE_MONITOR_EVENT_MOVED:
    default:
        g_assert_not_reached();
        break;
    }
}

DFileMonitor::DFileMonitor(const QUrl &url, QObject *parent)
    : QObject(parent), d(new DFileMonitorPrivate(this))
{
    d->url = url;
}

DFileMonitor::~DFileMonitor()
{
}

DExpected<QUrl> DFileMonitor::url() const
{
    return d->url;
}

DExpected<void> DFileMonitor::setTimeRate(quint32 msec)
{
    d->timeRate = msec;
}

DExpected<quint32> DFileMonitor::timeRate() const
{
    return d->timeRate;
}

DExpected<void> DFileMonitor::setWatchType(WatchType type)
{
    d->watchType = type;
}

DExpected<WatchType> DFileMonitor::watchType() const
{
    return d->watchType;
}

DExpected<bool> DFileMonitor::running() const
{
    return d->gmonitor != nullptr;
}

DExpected<bool> DFileMonitor::start()
{
    return start(d->timeRate);
}

DExpected<bool> DFileMonitor::start(quint32 timeRate)
{
    // stop first
    stop();

    d->timeRate = timeRate;

    QString urlStr = d->url.toString();
    if (d->url.scheme() == "file" && d->url.path() == "/")
        urlStr.append("/");   ///< gio can not monitor '/' directly, should change path to '//'
    d->gfile = g_file_new_for_uri(urlStr.toStdString().c_str());
    d->gmonitor = d->createMonitor(d->gfile, d->watchType);

    if (!d->gmonitor) {
        g_object_unref(d->gfile);
        d->gfile = nullptr;

        return false;
    }

    g_file_monitor_set_rate_limit(d->gmonitor, d->timeRate);

    g_signal_connect(d->gmonitor, "changed", G_CALLBACK(&DFileMonitorPrivate::watchCallback), this);

    return true;
}

bool DFileMonitor::stop()
{
    bool ret = false;
    if (d->gmonitor) {
        ret = g_file_monitor_cancel(d->gmonitor);
        g_object_unref(d->gmonitor);
        d->gmonitor = nullptr;
    }
    if (d->gfile) {
        g_object_unref(d->gfile);
        d->gfile = nullptr;
    }
    return ret;
}

DError DFileMonitor::lastError() const
{
    return d->error;
}

DIO_END_NAMESPACE
